---
id: SpecValidationRules
title: Form Validation Rules — Feedback Channel, Dialog Button Order and Input Normalization
description: The app-wide K3 rules for validating data-entry forms, for choosing the channel that reports feedback to the user, for where the confirming and dismissing buttons sit in a modal dialog, and for the silent normalization applied to a value the user picks or types
version: "2.13"
---

# Form Validation Rules

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Version](https://img.shields.io/badge/Version-2.13-blue) ![Implementation](https://img.shields.io/badge/Implementation-partial-yellow)

## Introduction

This document is the single source of truth for **input-validation rules** on
Katalog's data-entry forms, for **how validation feedback is presented**, for
**dialog button order** — where the confirming and the dismissing button sit in
any modal dialog — and for **input normalization**, the silent cleanup applied to
a value the user picks or types. It exists so each rule is greppable and doubles
as a manual-test charter: every row in the tables below is a case that must hold
after any change to the relevant screen.

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

## Dialog button order (how)

The table above says *use a modal*; it never said where that modal's buttons go,
so nothing governed it and K3 drifted into three different constructions at once.
The user reported on 2026-09-18 that the **Delete File** confirmations in Search
results and Explore render `[Cancel] [Delete]` — the dismissing button on the
left, the destructive one on the right — which is inverted relative to the
platform convention and relative to K2, whose `QMessageBox` confirmations take
the platform layout for free.

Rules use the `D` prefix.

| # | Rule | Applies to | Status |
|---|------|------------|--------|
| D1 | A modal dialog's buttons are ordered by the **active Qt Quick Controls style for the platform**, never by their position in the QML source. Every button is declared with a *role* and the style places it. This matters beyond KDE: K3 beta1 ships on Windows and macOS, whose conventional order differs, so any order frozen into the source is wrong on at least one platform. | K3 (`qt_quick/`) | [Implemented] |
| D2 | A dialog's accept and reject buttons are supplied **either** through `standardButtons` **or** through a `Controls.DialogButtonBox` given as the dialog's `footer`, whose buttons each carry an explicit `Controls.DialogButtonBox.buttonRole`. `customFooterActions` MUST NOT supply the accept or the reject button. Reason, measured on Plasma with Qt 6.11.2: Kirigami instantiates each `customFooterActions` entry as a **role-less** `QQC2.Button` *inside* the `DialogButtonBox` (`/usr/lib64/qt6/qml/org/kde/kirigami/dialogs/Dialog.qml:463-472`), and a role-less button sorts **after every role-carrying standard button**, so a custom action always lands rightmost and keeps only its source order relative to other custom actions. Mixing `standardButtons: Cancel` with a custom affirming action therefore *guarantees* `[Cancel] [Affirm]` on every platform. | K3 (`qt_quick/`) | [Implemented] |
| D3 | The affirming button of a confirmation takes **`AcceptRole`** and the dismissing button **`RejectRole`**, wired through the button box's `onAccepted` / `onRejected`. `DestructiveRole` MUST NOT be used even for a destructive confirmation: `QQuickDialogButtonBox` emits neither `accepted()` nor `rejected()` for it, so the button would be laid out but dead. The reference construction is `qt_quick/PageDevicesView.qml:160-170`. | K3 (`qt_quick/`) | [Implemented] |
| D4 | `customFooterActions` and `footerLeadingComponent` remain allowed for a **genuine extra** — an action that is neither the accept nor the reject of the dialog. An extra that is not a decision at all belongs in `footerLeadingComponent`, which places it away from the decision pair: `MetadataDialog.qml`'s **Copy JSON** is such an extra and keeps `standardButtons: Kirigami.Dialog.Close` beside it. An extra that *does* belong on the decision row takes a third form — a button with **`ActionRole`** in the same `DialogButtonBox`, handled by its own `onClicked` rather than by `onAccepted` — so that it too is placed by role and not by source position: `PageSearchForm.qml`'s **Keep Last 10** and **Clear** beside the `RejectRole` `Cancel`. *(The `ActionRole` form was added to this row on 2026-09-19 to match what was built, and ratified by the user the same day.)* | K3 (`qt_quick/`) | [Implemented] |
| D5 | In a destructive confirmation the affirming button keeps its explicit verb label (`Delete`, `Split`, `Continue`) rather than a bare `OK`, and the dismissing button is `Cancel`. `D1`–`D4` govern **order, role and construction only**: applying them rewords nothing. | K3 (`qt_quick/`) | [Implemented] |
| D6 | K2 (`qt_widgets/`) is out of scope. Its `QMessageBox` confirmations already take the platform layout and are not modified — K2 is in maintenance mode. A difference between a K2 and a K3 dialog's internals is not drift; only a difference in the *rendered* order would be. | K2 (`qt_widgets/`) | [Implemented] |

> **What `[Implemented]` means on these rows.** The code is in and builds clean;
> it does **not** mean the manual test charter below has passed. Two of its
> checks are still open and are deliberately left unticked: the rendered order
> has not yet been seen in the running application against a real collection, and
> the Windows and macOS order has not been checked at all — that needs the beta
> testers. Should either check fail, these rows are wrong and become drift to
> report, not a new requirement to write.

Verified on 2026-09-19 before and after the change, on Plasma with Qt 6.11.2:
headless probes confirmed each construction — a `Kirigami.Dialog` with a role
footer gives `[Delete] [Cancel]` with both `onAccepted` and `onRejected` firing,
`Kirigami.PromptDialog` behaves identically, `footerLeadingComponent` gives
`[Copy JSON] … [Close]`, and two `ActionRole` buttons plus a `RejectRole` give
`[Keep Last 10] [Clear] [Cancel]` with `onRejected` firing. `ninja` in
`build/Debug-QtQuick` builds clean with all five files compiled by
`qmlcachegen`, and `qmllint6` reports no error on any of them. `DestructiveRole`
appears nowhere, and a grep for `customFooterActions` in `qt_quick/` returns
comment lines only.

### The seven dialogs this covers

Scope was set by the user on 2026-09-19: **all seven**, not only the two
reported. Five were defective under `D2` because a role-less custom action was
mixed with a role-carrying standard button; two were hard-coded because *every*
button was a role-less custom action, so the order was frozen in source even
where it happened to look right.

The table records the state **as found on 2026-09-18**. All seven were rebuilt
on 2026-09-19 to the `D2`/`D3`/`D4` construction — a `Controls.DialogButtonBox`
footer with `AcceptRole` / `RejectRole` wired through `onAccepted` / `onRejected`
— and the faults below no longer hold.

| Dialog | File | Fault as found |
|--------|------|----------------|
| Delete File (Search results) | `qt_quick/PageSearchResultsForm.qml` | `D2` — `standardButtons: Cancel` + custom `Delete`; renders `[Cancel] [Delete]`. The reported case. |
| Delete File (Explore) | `qt_quick/PageExploreFiles.qml` | `D2` — identical construction, identical result. The reported case. |
| Checksum Mismatch (Search results) | `qt_quick/PageSearchResultsForm.qml` | `D2` — `standardButtons: Cancel` + custom `Update Checksum`. |
| Checksum Mismatch (Explore) | `qt_quick/PageExploreFiles.qml` | `D2` — same. |
| Extended Metadata | `qt_quick/MetadataDialog.qml` | `D2`/`D4` — `standardButtons: Close` + custom `Copy JSON` put the extra to the right of the closing button. `Copy JSON` moves to `footerLeadingComponent`; `Close` stays a standard button. |
| Search History | `qt_quick/PageSearchForm.qml` | Hard-coded — no `standardButtons`; `Keep Last 10`, `Clear` and `Cancel` are all role-less actions. `Cancel` becomes `RejectRole`; the other two stay extras under `D4`. |
| Run listed links | `qt_quick/PageBackupForm.qml` | Hard-coded — `standardButtons: NoButton` + custom `Continue` and `Cancel`. |

> **Run listed links and `BKP-F19`.** That dialog is owned by `SpecBackup.md`
> (`BKP-F19`, `[Implemented]`) and is included here deliberately, with the user's
> explicit go-ahead on 2026-09-19; `D2` carries **no exemption** for it. Nothing
> in `BKP-F19` changes: the same dialog, the same four conditional lines, the same
> two button labels, and on KDE the same rendered order — `Continue` stays to the
> left of `Cancel`. Only the construction becomes role-based, so the order now
> follows the platform on Windows and macOS instead of being frozen. This spec
> does not amend `SpecBackup.md`.

### Constructions that were measured and do not work

Recorded so they are not retried. Both were probed on this machine (Plasma,
Qt 6.11.2) before the working fix was found:

- **Retitling a standard button** — `Dialog.standardButton(Kirigami.Dialog.Ok).text = "Delete"`. The retitle does not stick.
- **Injecting a role onto a footer action's button** — setting `DialogButtonBox.buttonRole` on the button returned by `customFooterButton()`. The role is ignored and the button still sorts last.

The only construction that produces `[Delete] [Cancel]` is replacing the dialog's
`footer` with a `Controls.DialogButtonBox` whose buttons carry explicit roles,
which is what `D2` and `D3` require.

### Strings

This change spends **no new source text**. It adds exactly one existing text,
`Cancel`, to two files — `qt_quick/PageSearchResultsForm.qml` and
`qt_quick/PageExploreFiles.qml` — approved by the user on 2026-09-19, because a
dialog that supplies its own `DialogButtonBox` must also supply that button's
label, where `standardButtons` previously supplied it. The text `Cancel` is
already in use in K3 and is already a spec'd string (`SpecBackup.md` string
table). Under K3's per-QML-file `qsTr()` contexts the two additions appear as
**two new `.ts` entries**, one per file; that is expected and MUST NOT be
reported as drift, on the same footing as `DAS-C14`. The two entries MUST stay
byte-identical to each other and to every other `Cancel` in K3. **No other
string is added, reworded, moved or deleted** by this change — in particular
`Delete`, `Update Checksum`, `Copy JSON`, `Keep Last 10`, `Clear` and `Continue`
are untouched.

## Field label alignment (how)

An app-wide K3 layout convention for forms, recorded here because this page
already holds the app-wide K3 form rules. Raised by the user on 2026-09-26 on
the Search page: in the File name section, the "text" and "exclude" labels beside
the term lists sat top-aligned rather than level with the first line of the
list. Approved by the user the same day. Rules use the `L` prefix.

| # | Rule | Applies to | Status |
|---|------|------------|--------|
| L1 | A field label is vertically centred on the first line of the input it names. When the input grows to several lines (a list of terms, a wrapping row of options), the label stays level with that first line, not with the middle of the whole block. Likewise, a text shown in a row beside a button (e.g. the version number beside "Release Notes" in Settings) is vertically centred on that button. *Coverage on 2026-09-26: Search — File name "text" and "exclude" labels (the reported case), and the "On" / "Scope" labels of the duplicates and differences sections; Settings — Application row labels (Version, Behavior, Theme, Language) and the version number / release date beside the Release Notes button. Other forms are not yet audited against this rule.* | K3 (`qt_quick/`) | [Implemented] |

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

For the dialog button rules (D*), in K3:

- D1 / D2 (the reported case) — open the **Delete File** confirmation from a
  Search results row and from an Explore row. The dismissing button sits where
  every other Plasma dialog puts it, and the pair matches the Split Catalog
  dialog on the Devices page seen side by side. `Escape` still dismisses and
  `Return` still triggers the affirming button.
- D1 (platform) — run the same two dialogs on Windows and on macOS. Each follows
  its own platform order, and the two platforms do **not** agree with each other.
  A build where all three platforms show the same order has frozen the order and
  fails this row.
- D2 (construction) — grep `qt_quick/` for `customFooterActions`: every remaining
  occurrence supplies an extra only. No occurrence sits beside a
  `standardButtons` value that names `Cancel`, `Ok`, `Yes`, `No` or `Close` as
  the dialog's decision pair.
- D3 — grep `qt_quick/` for `DestructiveRole`: no occurrence. Then trigger each
  of the seven dialogs' affirming button and confirm the action actually runs —
  a button that renders but does nothing is the `DestructiveRole` trap.
- D4 — in the **Extended Metadata** dialog, `Copy JSON` sits apart from `Close`
  rather than to its right, and still copies and still shows its notification.
  In **Search History**, `Keep Last 10` and `Clear` still open their own
  confirmations from their `onClicked`, and dismissing the dialog with `Cancel`
  or `Escape` runs neither of them.
- D5 / strings — read the label on every button of the seven dialogs: each is
  byte-identical to before the change. With the interface in French, the two new
  `Cancel` buttons are translated, proving the existing text was reused. Run
  `ninja translations_lupdate`: no new **source text** appears, and the only new
  entries are the two per-context `Cancel` rows.
- Run listed links / `BKP-F19` — re-run the `BKP-F19` charter rows in
  `SpecBackup.md` unchanged: the four conditional lines, the counts, and
  `Cancel` starting nothing. On KDE, `Continue` is still left of `Cancel`.

For the field label alignment rule (L*), in K3:

- L1 — on the Search page, add three or more terms to the File name "text" and
  "exclude" lists: each label stays level with the **first** term, not with the
  middle of the list. Check the same for the "On" / "Scope" labels in the
  duplicates and differences sections, narrowing the window so their options
  wrap. In Settings, Application section, the Version, Behavior, Theme and
  Language labels sit level with the first line of their row, and the version
  number and release date are vertically centred on the Release Notes button.

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
