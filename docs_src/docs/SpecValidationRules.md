# Form Validation Rules

## Introduction

This document is the single source of truth for **input-validation rules** on
Katalog's data-entry forms and for **how validation feedback is presented**. It
exists so each rule is greppable and doubles as a manual-test charter: every row
in the tables below is a case that must hold after any change to the relevant
screen.

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

## Manual test charter

For each rule above: trigger the stated condition, confirm the exact message and
the correct channel appear, confirm the action did **not** proceed (for blocking
rules), and confirm the banner disappears when the offending field is edited.
