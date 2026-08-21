# Form Validation Rules

## Introduction

This document is the single source of truth for **input-validation rules** on
Katalog's data-entry forms, for **how validation feedback is presented**, and for
**input normalization** — the silent cleanup applied to a value the user picks or
types. It exists so each rule is greppable and doubles as a manual-test charter:
every row in the tables below is a case that must hold after any change to the
relevant screen.

Scope: K3 (Qt Quick) primarily; K2 (Qt Widgets) noted where it diverges.

## Message presentation (how)

Choose the feedback channel by whether the user must act. This mirrors the
Kirigami HIG and the project rule in `CLAUDE.md`.

| Situation | Channel | Component |
|-----------|---------|-----------|
| Blocking validation error (a field is missing/invalid and stops the action) | Inline, non-modal banner at the top of the form, dismissible, auto-hidden when the offending field is edited | `Kirigami.InlineMessage`, `type: MessageType.Warning`, `showCloseButton: true` |
| Transient success / info (no action needed) | Auto-dismissing toast | `showPassiveNotification(...)` |
| A decision is required (Yes/No, destructive confirmation) | Modal | `Controls.Dialog` / `Kirigami.PromptDialog` |

**Never** use a modal dialog or a passive notification for routine field
validation, and **never** use an inline banner for a transient success message.

## Copy style

- No "Please"; no bare imperative orders. State the requirement in sentence case.
- Reuse the K2 source string when one exists; otherwise follow the style above
  and get per-string approval (see `.claude/agents/translations.md`).

## Validation rules (what)

### Tags page (`PageTagsForm.qml`)

| # | Rule | Trigger | Message |
|---|------|---------|---------|
| T1 | A tag cannot be created without a folder. | "Tag the folder" with an empty Folder field | `Select or enter a folder to tag.` |
| T2 | A tag cannot be created without a tag name. | "Tag the folder" with an empty Tag field (folder present) | `Select or enter a tag name.` |

Folder is validated before tag name (top-to-bottom field order). On success the
Folder field is cleared and the Tag combo is reset.

### Create page (`PageCreateForm.qml`)

| # | Rule | Trigger | Message |
|---|------|---------|---------|
| C1 | A catalog needs a name. | "Create" with an empty Catalog name | `Provide a name for this new catalog.` |
| C2 | A catalog needs a source path. | "Create" with an empty Source path | `Provide a path for this new catalog.` |
| C3 | A catalog needs a Storage. | "Create" with no Storage selected | `Select a Storage for this new catalog.\n(Selection panel on the left and dropdown list)` |
| C4 | An empty source folder requires confirmation (decision, not a blocking error). | "Create" when the source folder contains no file | Modal Yes/No: `The source folder does not contain any file. …` |

C1–C3 are blocking validation (InlineMessage). C4 is a decision and stays a
modal dialog.

## Input normalization (what is silently cleaned up)

Normalization is **not** validation: it never blocks an action and never shows a
message. It rewrites the value the user picked or typed so that equivalent input
produces an identical stored value. Rules use the `N` prefix.

Because these rows describe a transformation rather than a message, this table
carries **Applies to** and **Status** columns instead of Trigger/Message.

| # | Rule | Applies to | Status |
|---|------|------------|--------|
| N1 | A path value has a single trailing separator (`/`) removed **at the moment it is picked from the folder dialog or typed into the field**, not only when the form is saved. | K3 path fields listed in N3 | [Implemented] |
| N2 | A filesystem root is preserved exactly as-is and is never stripped: the Linux root `/` and a Windows drive root `X:/`. A test based only on string length is not sufficient and must not be used. | Same fields as N1 | [Planned] |
| N3 | The fields subject to N1/N2 are: the device/catalog **source path** (Devices create/edit form and Create page) and the **exclude-folder paths** — both the per-catalog exclude folder and the global exclude directory. | K3 (`qt_quick/`) | [Implemented] |
| N4 | "Generate name from path" builds the catalog name from the **already-normalized** path, so a generated name never ends with `_`. | Create page, K2 and K3 | [Planned] |
| N5 | The normalization rule has exactly **one** implementation, in `core/` (`Catalog::normalizeSourcePath`), which `Catalog::setSourcePath()` also calls. The UI layer MUST NOT re-implement or duplicate the rule — K3 reaches it through an `AppManager` invokable. | `core/`, `qt_quick/` | [Planned] |
| N6 | Normalization MUST NOT alter path separators, character case, or any non-trailing character. It removes a trailing separator and nothing else. | All fields in N3 | [Implemented] |
| N7 | **Known accepted drift:** K2 (`qt_widgets/`) does not apply N1–N3; its device form stores the path verbatim. This is deliberate for the current iteration — K2 is in maintenance mode — and is to be closed later by routing K2 through the same `core/` function. | K2 (`qt_widgets/`) | [Backlog] |

## Manual test charter

For each validation rule above (T*, C*): trigger the stated condition, confirm
the exact message and the correct channel appear, confirm the action did **not**
proceed (for blocking rules), and confirm the banner disappears when the
offending field is edited.

For the normalization rules (N*), in K3:

- N1 — pick `/mnt/drive/Photos/` from the folder dialog on the Devices edit form:
  the field shows `/mnt/drive/Photos` immediately, before saving. Type the same
  value by hand: same result.
- N2 — pick `/` (Linux): the field still shows `/`. On Windows, pick `C:/`: the
  field still shows `C:/`, not `C:`.
- N3 — repeat N1 on the Create page source path, on the per-catalog exclude
  folder field, and on the global exclude directory field.
- N4 — with the source path `/mnt/drive/Photos/`, press "Generate name from
  path": the name is `_mnt_drive_Photos`, with no trailing `_`.
- N5 — grep `qt_quick/` for a second trailing-slash strip: there must be none;
  the only rule lives in `core/catalog.cpp`.
- N6 — a path containing `//` in the middle, mixed case, or a Windows path is
  returned unchanged apart from the trailing separator.
- N7 — the same K2 checks are expected to **fail** until the drift is closed;
  that is the documented state, not a regression.
