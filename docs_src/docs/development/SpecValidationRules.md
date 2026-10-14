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
a value the user picks or types, and for **where a folder or file picker opens**. It exists so each rule is greppable and doubles
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

## Dialog content layout (how)

The maintainer reported on 2026-10-14, from a Windows beta4 run, the warning
`QML Dialog: Binding loop detected for property "implicitHeight"` (from
`Fusion/Dialog.qml`) on the first-run welcome and first-run ready dialogs in
`qt_quick/Main.qml`. Fusion is forced on Windows and macOS by
`qt_quick/main.cpp` and is also the style of the AppImage; the KDE
`org.kde.desktop` style does not show the warning, so it is invisible on the
Linux/KDE development machine.

Measured on 2026-10-14 with a standalone `qml6` offscreen repro: a wrapping
`Controls.Label` used directly as a dialog's `contentItem` gives two
binding-loop warnings under Fusion, Basic and Material, and none under
`org.kde.desktop`. Removing the explicit `width`, the RichText format, the
footer or `anchors.centerIn` does not remove the warnings; removing the wrap
does. Wrapping the Label in a `ColumnLayout` with `Layout.fillWidth: true` gives
no warning in any of the four styles and the same dialog height (236 px under
Fusion, 295 px under `org.kde.desktop`, before and after).

| # | Rule | Applies to | Status |
|---|------|------------|--------|
| D7 | A `Controls.Dialog` (or any `Popup`) whose content is word-wrapped text MUST NOT use a wrapping `Controls.Label` directly as its `contentItem`. The `contentItem` is a layout container (`ColumnLayout`) holding the Label, and the Label carries `Layout.fillWidth: true` with no explicit `width:` binding (e.g. to `availableWidth`). Reason: under the stock Qt Quick Controls styles (Fusion, Basic, Material) the direct construction causes a binding loop on the dialog's `implicitHeight`. Applying this rule changes no user-visible string and no rendered dialog size. *(Approved by the maintainer on 2026-10-14, relayed by the main agent.)* | K3 (`qt_quick/`) | [Planned] |

The Kirigami warning `Failed to find a Kirigami platform plugin for style
Fusion` is a separate matter, not covered by `D7`; it is a known limitation
under `DPL-C4` (`SpecK3Deployment.md`).

## Field label alignment (how)

An app-wide K3 layout convention for forms, recorded here because this page
already holds the app-wide K3 form rules. Raised by the user on 2026-09-26 on
the Search page: in the File name section, the "text" and "exclude" labels beside
the term lists sat top-aligned rather than level with the first line of the
list. Approved by the user the same day. Rules use the `L` prefix.

| # | Rule | Applies to | Status |
|---|------|------------|--------|
| L1 | A field label is vertically centred on the first line of the input it names. When the input grows to several lines (a list of terms, a wrapping row of options), the label stays level with that first line, not with the middle of the whole block. Likewise, a text shown in a row beside a button (e.g. the version number beside "Release Notes" in Settings) is vertically centred on that button. *Coverage on 2026-09-26: Search — File name "text" and "exclude" labels (the reported case), and the "On" / "Scope" labels of the duplicates and differences sections; Settings — Application row labels (Version, Behavior, Theme, Language) and the version number / release date beside the Release Notes button. Other forms are not yet audited against this rule.* | K3 (`qt_quick/`) | [Implemented] |

## Picker start folder (where)

An app-wide K3 rule for where a folder or file picker opens. The user reported on
the Create page that the folder dialog ignored the Source path field: Qt Quick
`FolderDialog` / `FileDialog` apply `currentFolder` only on their first show, so a
reused dialog reopened on the last folder browsed. K3 now creates a fresh dialog
per open. Approved by the user on 2026-10-13. Rule `P3` keeps the Tags workflow,
where the Folder field is cleared after each tag (`T1`–`T3`) and the user tags
neighbouring folders. Rules use the `P` prefix.

| # | Rule | Applies to | Status |
|---|------|------------|--------|
| P1 | `P2`–`P5` apply to every K3 folder or file dialog opened by a "Select the path"-style button or a drawer action: Create (source path, per-catalog exclude folder, global exclude directory); Device edit (path, exclude folder); Tags (folder); Settings (image folder, export to folder, import file, import folder); drawer Open → SQLite Database and Open → Collection Folder. Each listed dialog is a separate dialog for `P3`. | K3 (`qt_quick/`) | [Implemented] |
| P2 | If the dialog's related field holds a path, the dialog opens on that path. For a file picker whose field holds a file, it opens on that file's folder. | Dialogs in `P1` | [Implemented] |
| P3 | If the related field is empty, or the dialog has no related field, the dialog opens on the last folder picked in **that same dialog** during the current session. For a file picker, the picked folder is the folder containing the picked file. Cancelling a dialog does not change its remembered folder. | Dialogs in `P1` | [Implemented] |
| P4 | If nothing has been picked in that dialog yet this session, it opens on the collection browse folder: the `.db` file's folder in File mode, the collection folder in Memory mode, the user's home folder in Hosted mode (`AppManager::getCollectionBrowsePath`). | Dialogs in `P1` | [Implemented] |
| P5 | The remembered folder of `P3` is never persisted: no settings key is written, and it is forgotten when K3 closes. | K3 (`qt_quick/`) | [Implemented] |
| P6 | Settings "Export to SQLite file" is outside `P1`. Its save dialog keeps proposing `<browse folder>/export.db` (`SpecCollectionExport.md` CEX-F3). | K3 (`qt_quick/`) | [Implemented] |
| P7 | K2 (`qt_widgets/`) is out of scope and unchanged: `QFileDialog::getExistingDirectory` uses the field's path or the system default. This rule adds no user-visible string and no `core/` change. | K2, `core/` | [Implemented] |

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
| T3 | A folder cannot get the same tag twice. *(K3 only; approved by the maintainer 2026-10-10, with its new string. K2 unchanged.)* | "Tag the folder" when this folder path (trimmed, exact match) already has this tag name | `Folder already tagged with %1.` (%1 = the tag name) |

Folder is validated before tag name (top-to-bottom field order), then the duplicate check (T3). On success the
Folder field is cleared and the Tag combo is reset.

### Edit Device page (`PageDeviceEditForm.qml`)

Every refused save is a blocking validation error: a `Kirigami.InlineMessage` (Warning) at the top of the form, with a close button, hidden when the name or path is edited. *(Fixed 2026-10-10: it was the modal `editValidationDialog`.)* The delete error stays a dialog: it reports an action, not a field.

| # | Rule | Trigger | Message |
|---|------|---------|---------|
| E1 | A device needs a name. | Save with an empty Name | `Provide a name for this device.` |
| E2 | Two catalogs cannot share a name. | Save with a catalog name already used | `There is already a catalog with this name: %1\nChoose a different name.` |
| E3 | A catalog in the Physical group sits under a Storage device. | Save with another parent | `A Catalog in the Physical group can only be set under a Storage device.` |

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
- D7 — grep `qt_quick/` for `contentItem: Controls.Label`: no occurrence whose
  Label sets `wrapMode`. Run K3 with `QT_QUICK_CONTROLS_STYLE=Fusion` (and on
  Windows or macOS, where Fusion is forced) and open the first-run welcome and
  first-run ready dialogs plus any other converted dialog: no
  `Binding loop detected for property "implicitHeight"` warning appears, the
  text still wraps, and each dialog keeps its previous size and text.

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

For the picker start folder rules (P*), in K3:

- P2 (the reported case) — on Create, type or pick `/mnt/a/b` in Source path, then
  press its "Select the path" button: the dialog opens on `/mnt/a/b`. Change the
  field to `/mnt/c` and reopen: it opens on `/mnt/c`, not on the folder browsed
  before. Repeat on every field-backed dialog in P1. For Settings import file, with
  a file path in the field, the dialog opens on that file's folder.
- P3 — on Tags, pick a folder, tag it (the field clears), press the folder button
  again: it opens on the folder just picked. Pick a folder in Create's source path,
  then open Create's exclude dialog: the exclude dialog does not open on that
  folder, because each dialog remembers only its own pick. Pick a file in Open →
  SQLite Database, reopen it: it opens on that file's folder. Open a dialog,
  browse elsewhere, cancel, reopen: it opens where it opened before.
- P4 — just after starting K3, open each P1 dialog with its field empty. File
  mode: the `.db` file's folder. Memory mode: the collection folder. Hosted mode:
  the home folder.
- P5 — pick a folder in Tags, quit K3, restart, open Tags with an empty field: it
  opens on the browse folder of P4. The settings file has no new key.
- P6 — Settings "Export to SQLite file" still proposes `<browse folder>/export.db`,
  even after a pick in "Export to Memory mode (csv)".
- P7 — run `ninja translations_lupdate`: no new source text. `git diff core/` is
  empty. K2 pickers behave as before.
