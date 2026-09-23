---
id: SpecLanguages
title: Languages — interface language selection
description: Requirements for how Katalog chooses the interface language on a new install, how a stored language is honoured, and why no behaviour may depend on the position of a language in the supported list.
version: "2.13"
---

# LANGUAGES — INTERFACE LANGUAGE SELECTION

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Version](https://img.shields.io/badge/Version-2.13-blue) ![Implementation](https://img.shields.io/badge/Implementation-complete-brightgreen) ![Verification](https://img.shields.io/badge/Retest-pending-orange)

## Context

Katalog ships one translation per supported language and picks one of them at
startup. This page owns **which language the application selects and stores** —
first run, a stored value, and what the Settings control displays. It does not
own how translations are produced: the `.ts` / `.qm` pipeline, the K2 `MainWindow`
context versus the per-QML-file contexts of K3, and the byte-exact K2→K3 string
bridge are the translation agent's subject (`.claude/agents/translations.md`).
Pages that cite this spec for the byte-exact bridge are citing that agent file's
subject, not a requirement below.

The page exists because of a defect reported on the app defaulting to the first language in the list.
Two independent paths can produce a position-0 language, and both were measured
rather than assumed:

- **Resolution reads the wrong locale.** System-language detection used the
  *format / region* locale. On macOS the Preferred-Languages list and the Region
  are independent settings, so a user reading the interface in English while
  formatting dates for another region resolves to that other region's language —
  or, when it is unsupported, falls through. `LNG-F4` closes this.
- **The Settings control falls back to its own default index.** A language
  control populated from the list and then asked to select a stored code that is
  not in the list keeps its default index — index 0, Bulgarian. One click or one
  press of Enter then stores Bulgarian permanently. This happens in **both**
  versions: in K3 the restore loop only assigns when a code matches, and in K2
  `setCurrentText` on a non-editable combo silently does nothing for a value it
  cannot find. `LNG-F5`, `LNG-C1` and `LNG-C2` close this.

> **K2 is in scope here.** The maintainer decided on 2026-09-21 that both
> versions are to be hardened ("we need to robustify both K2 and K3"), despite K2
> being in maintenance mode. `LNG-F4` reaches K2 automatically because detection
> lives in shared core (`LNG-C3`); `LNG-F5` and `LNG-C2` are separate repairs in
> each version's Settings control. The two versions do **not** share one settings
> file while K3 is a pre-release (`LNG-C5`), so neither repair covers the other.

> **Diagnostics were deliberately not requested.** The maintainer chose to harden
> every path that can produce a position-0 language rather than ask the tester
> which one bit. The consequence is recorded honestly: **it is not known which
> path produced this report.** The macOS mechanism behind `LNG-F4` is *inferred*,
> because the UI-language-versus-region split cannot be reproduced on Linux,
> where both derive from the same environment variables. What is measured is the
> improvement, not the cause — see *Measured evidence* below.

> **Reading the requirement IDs.** Each requirement has a permanent ID. IDs are
> never renumbered or reused; a retired requirement is marked `[Removed]`, not
> deleted. Status is one of `[Implemented] / [Planned] / [Backlog] / [Removed]`.

> **On the statuses below.** Every row is `[Implemented]`, and in this project
> that means **built, not retested**: the status vocabulary has no separate
> *verified* value. The three repairs of `LNG-F4`, `LNG-F5` and `LNG-C2` were
> applied on 2026-09-21 and both versions compile and link; the resolution of
> `LNG-F4` was measured by compiling the real `core/language.cpp` into a harness
> (see *Measured evidence*). What has **not** happened is that nobody has yet
> opened either version's Settings page and looked at the language control, and
> the macOS report itself was never confirmed — diagnostics were deliberately not
> requested. The charter at the foot of this page is the instrument for that, and
> the *Retest* shield above stays orange until it has been walked through.

---

## Scope at a glance

**In scope:** resolving the interface language on a new install, migrating and
sanitising a stored language code, what is allowed to write the stored language,
what the Settings language control displays, and the ban on depending on the
order of the supported-language list. K2 and K3 equally.

**Out of scope (non-goals):** the translation pipeline, `.ts` / `.qm` files, the
K2→K3 context bridge and AI batch-fill (`.claude/agents/translations.md`); adding
or removing a supported language; the documentation site's own languages; live
retranslation versus restart, which is an existing difference between the two
versions and is not changed here.

---

## Operational requirements — *why / for whom*

Goals in real use, independent of how they are built.

| ID | Requirement | Status |
|----|-------------|--------|
| LNG-O1 | A new install starts in **the language the user reads their operating system in**, when Katalog supports it, and in English otherwise. *This intent was ratified by the maintainer on 2026-09-21*, in preference to the narrower reading "Katalog follows the system region". It is the reading that makes `LNG-F4` a repair of an unmet goal rather than a change of behaviour — so if the intent is ever disowned, `LNG-F4` must be re-judged as a new requirement, not quietly kept. Before `LNG-F4` the goal held only where the operating system's region and reading language happened to agree. | [Implemented] |
| LNG-O2 | After first run the interface language is only ever the one the user chose. Katalog MUST NOT change it on the user's behalf, and MUST NOT store a language the user never selected. | [Implemented] |

## Functional requirements — *what the system does*

Observable behaviour that can be triggered and watched.

| ID | Requirement | Status |
|----|-------------|--------|
| LNG-F1 | On first run — no interface language stored — Katalog resolves the system language, uses it, and stores it. A resolved language that Katalog does not support becomes English (US). | [Implemented] |
| LNG-F2 | A stored legacy Czech code `cz_CZ` is migrated to the standard `cs_CZ` and rewritten. | [Implemented] |
| LNG-F3 | A stored language code that Katalog does not support is replaced by English (US) and rewritten, so that what is stored is always a language the application can actually load. | [Implemented] |
| LNG-F4 | System-language resolution reads the operating system's **reading-language list** (its UI languages), not its format/region locale. Each tag is normalised to Katalog's code form (`en-US` → `en_US`) and matched in this order: exact match against a supported code; then match on the language part alone, resolving to that language's supported variant (`en` → `en_US`, `pt` → `pt_PT`); then the system format locale, as before; then English (US). The first match wins. **This is the fix for the reported defect's detection path** — see Context. It reaches K2 and K3 together through shared core (`LNG-C3`). | [Implemented] |
| LNG-F5 | The Settings language control shows the language actually in effect as its selection. When that code is absent from the supported list it shows **English (US)** — which is what `LNG-F3` has already stored, so the control reports the truth rather than substituting a guess. It MUST NOT be left on whatever entry happens to be first. Were English (US) itself ever absent from the supported list, the control shows **no selection** — still never the first entry. Applies to the K2 combo box and the K3 combo box alike. | [Implemented] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

Boundaries and implementation constraints, not user-visible behaviour.

| ID | Requirement | Status |
|----|-------------|--------|
| LNG-C1 | **No behaviour may depend on the order of the supported-language list, and no control may use list position 0 as an implicit fallback.** A language is always resolved by code lookup. The order is arbitrary: it is roughly alphabetical by code but not consistently so (Greek sits after French), and the entry that happens to be first — Bulgarian — is what the reporter saw as "Russian". Reordering the list, or inserting a language ahead of the first, MUST NOT be able to change which language a user gets. **One consequence to watch when a language is added:** `LNG-F4`'s match on the language part alone is unambiguous only because each supported language appears exactly once in the list. Adding a second variant of a language already present — a `pt_BR` beside `pt_PT` — would make that match resolve to whichever of the two comes first in the list, which is precisely the order dependence this row forbids. Adding such a variant therefore requires the tie to be decided explicitly, not left to the list. | [Implemented] |
| LNG-C2 | The stored interface language is written **only** as the result of an explicit user choice, or by the first-run, migration and sanitise rules of `LNG-F1`–`LNG-F3`. **Constructing or populating a control MUST NOT write it.** In K2 the language combo's change signal is connected by the generated UI and writes the setting, so adding the first item moves the index from −1 to 0 and stores Bulgarian before the stored value is restored: the population must not emit. This is the mechanism by which a user who never opened the language control can still end up with one stored. | [Implemented] |
| LNG-C3 | System-language resolution lives in **one** place in shared core (`core/language.cpp`, `Language::getSystemLanguage()`) and is called by both K2 (`qt_widgets/main.cpp`, `qt_widgets/mainwindow_setup.cpp`) and K3 (`qt_quick/main.cpp`). A change to its body changes both versions. K2 being in maintenance mode does not exempt it, and a change here MUST be stated as affecting K2 as well. The repair of `LNG-F4` changes this method's body only — no new core method, no header change. | [Implemented] |
| LNG-C4 | Supported-language codes are standard locale names (`cs_CZ`, never `cz_CZ`). `LNG-F2` exists because an earlier non-standard code was once stored. | [Implemented] |
| LNG-C5 | The interface language is stored in the **single application settings file**, not per collection: one file for the whole installation, chosen once at startup between the portable location next to the executable and the user's configuration directory. Opening a different collection does not change which file it is, and does not re-run `LNG-F1`–`LNG-F3`. While K3 is a pre-release it uses its own file name, so K2 and K3 currently store their language **separately** — which is why hardening one version does not harden the other, and why `LNG-F5` and `LNG-C2` are required in both. | [Implemented] |
| LNG-C6 | These repairs introduce **no** new user-visible string, and change and remove none. Nothing in the translation files is touched. | [Implemented] |

---

## Measured evidence for `LNG-F4`

Obtained by compiling the shipped `core/language.cpp` into a standalone harness
and running it under seven locale setups. Recorded because the reporter's own path
is unknown, so the evidence for the fix is the improvement it produces, not a
reproduction of the report.

| Operating system reading language | Before | After |
|---|---|---|
| `en_GB` | Bulgarian (first entry of the list) | English (US) |
| `pt_BR` | Bulgarian (first entry of the list) | Portuguese |
| `ko_KR` (unsupported) | English (US) | English (US) |
| `bg_BG` (genuine) | Bulgarian | Bulgarian |
| `fr_FR` (genuine) | French | French |
| `en_US` (genuine) | English (US) | English (US) |
| `cs_CZ` (genuine) | Czech | Czech |

The two *Before* rows reading "Bulgarian" are the defect itself: a supported
language existed for both cases — English for `en_GB`, Portuguese for `pt_BR` —
and the old resolution reached neither.

**Limitation.** The macOS reading-language-versus-region split cannot be
reproduced on Linux, where both derive from the same environment variables: with
the reading language set to English and the region to Bulgaria, Linux reports
English. The macOS mechanism therefore remains **inferred**. This is also why the
defect never appeared on the development machine.

---

## Decisions recorded, not open items

| Item | Detail |
|------|--------|
| The list holds 29 languages, the documentation says 30 | Noted by the maintainer on 2026-09-21 and **deliberately left as it is** until further languages are added. This is a recorded decision, not drift and not a task. |
| The same unmatched-selection pattern elsewhere in K2 | The K2 Settings restore path applies the stored language to the combo a second time, with the same silent no-match behaviour. Nobody has reported it and it is **not authorised** here: `LNG-F5` covers the population path that the report points at. Recorded so a future reader does not mistake its absence for an oversight, and does not carry a sweep in behind this fix. |

---

## Manual test charter

Each line is a case that must hold after any change to language resolution or to
either version's language control. Cases marked (K2) apply to the Qt Widgets
version, (K3) to the Qt Quick version.

**None of these has been walked through yet.** The 2026-09-21 repairs were built,
compiled and — for `LNG-F4` — measured in a harness, but no one has opened either
Settings page to look at the language control, and the macOS case can only be run
by a macOS tester. Until this list has been walked, the rows above mean *built*.

- **LNG-F1 / LNG-O1** (K2, K3) — Remove the stored interface language, set the
  operating system's reading language to a supported one, and start. The
  interface is in that language and the value is stored.
- **LNG-F4** (K2, K3) — Set the operating system's reading language to `en_GB`,
  remove the stored value, and start. The interface is **English**, not
  Bulgarian. Repeat with `pt_BR`: the interface is Portuguese. Repeat with an
  unsupported language such as Korean: the interface is English.
- **LNG-F4** (K3, macOS) — Set Preferred Languages to English and the Region to a
  country whose language Katalog supports. Remove the stored value and start. The
  interface is English. This is the reporter's case and the only test that
  exercises the inferred mechanism; it cannot be run on Linux.
- **LNG-F2** (K2, K3) — Store `cz_CZ` by hand and start. The interface is Czech
  and the stored value has become `cs_CZ`.
- **LNG-F3 / LNG-F5 / LNG-C1** (K2, K3) — Store a junk code by hand, start, then
  open Settings. The interface is English, the language control shows **English**,
  and no Bulgarian appears anywhere. Click nothing; close the application and
  inspect the stored value: it is English, never Bulgarian.
- **LNG-C2** (K2) — Start, go to Settings, and touch nothing. Quit and inspect
  the stored interface language: it is unchanged. Run this with a stored value
  that is in the list and again with one that is not — neither may be overwritten
  by the control being built.
- **LNG-C2** (K3) — The same: open the Settings page, touch nothing, quit, and
  confirm the stored value is unchanged.
- **LNG-O2** (K2, K3) — Select a language deliberately, restart, and confirm it
  is still in effect. No launch may change it back or aside.
- **LNG-C1** (K2, K3) — Code review case: after any change, no code selects a
  language by index, and none treats the first entry of the supported list as a
  default.
- **LNG-C6** (K2, K3) — No entry in the translation files is added, changed or
  removed by these repairs.
