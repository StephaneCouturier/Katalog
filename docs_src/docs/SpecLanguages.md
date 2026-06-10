# Languages & Translation

![Draft](https://img.shields.io/badge/Status-Draft-orange) ![Partial](https://img.shields.io/badge/Implementation-partial-yellow)

## Context

Katalog ships in ~30 languages. Translations are maintained as Qt Linguist `.ts`
files (one per language) compiled to `.qm` and bundled into the binary via a
Qt resource. This document is the single reference for how languages are
detected, loaded, matched, and maintained across **K2 (Qt Widgets)** and
**K3 (Qt Quick / QML)**, and defines the **target K3 architecture** for when K2
is retired.

> **The cardinal rule — do not edit user-visible strings.** Translation matching
> is **byte-for-byte**. Changing a single character of any `tr()` / `qsTr()`
> source string silently drops it to `unfinished` (English) across all 30
> languages. No user-visible string may be added, changed, or removed without
> explicit per-string approval (see `CLAUDE.md` → *CRITICAL — User-visible text*).

---

## Source of truth: the `Language` core class

`core/language.cpp` / `core/language.h` is the **single source of truth** for the
supported language set. It is UI-agnostic and shared by both K2 and K3.

| Field | Meaning | Example |
|-------|---------|---------|
| `code` | Standard locale name (`language_COUNTRY`) | `fr_FR`, `cs_CZ`, `en_US` |
| `displayName` | English name shown in the selector | `French`, `Czech` |
| `flagPath` | Qt resource path to the flag image | `:/images/flags/fr.png` |

Helpers: `getSupportedLanguages()`, `isLanguageSupported(code)`,
`getSystemLanguage()`, `getDisplayName(code)`, `getFlagPath(code)`,
`getFlagIcon(code)`.

**Code convention:** `code` MUST be the standard Qt/ISO locale name returned by
`QLocale::system().name()`, because detection compares against it directly.
The flag image basename (e.g. `cz.png`) is cosmetic and may differ from the
locale code (e.g. `cs_CZ` uses `cz.png`).

---

## Translation contexts — the K2 / K3 divergence

This is the heart of the matter.

| | Context model | Why |
|---|---|---|
| **K2** | **One context: `MainWindow`** | Every `tr()` lives in the `MainWindow` class, so `lupdate` files them all under that single context. |
| **K3** | **One context per QML file** (`PageSearchForm`, `PageSettings`, `Main`, …) | Qt's QML `qsTr()` uses the file's base name as the translation context. This is not a choice — it is how QML translation works. |

Consequence: a fully-translated K2 string under `MainWindow` does **not**
automatically satisfy a K3 `qsTr()` lookup under, say, `PageSettings`. The two
must be bridged (today) or generated independently (target).

---

## Current workflow (while K2 is still maintained)

K2 remains the canonical owner of the translated strings. K3 reuses them through
a bridging script.

### `packaging/sync_k3_translations.py`

1. Scans `qt_quick/*.qml`, collecting every `qsTr("…")` grouped by file → context.
2. For each `translations/Katalog_*.ts`, builds a `source → translation` map from
   the **K2 contexts** (chiefly `MainWindow`).
3. Appends a new `<context>` per K3 page, **auto-filling the translation where the
   source string matches a K2 string byte-for-byte**; K3-only strings are written
   as `<translation type="unfinished"></translation>`.

Then compile and embed:

```
python3 packaging/sync_k3_translations.py     # bridge K2 → K3 contexts
ninja translations_lrelease                   # K2 build dir → recompile .qm
ninja                                          # qt_quick build dir → re-embed .qm
```

`lrelease` runs with `-nounfinished` (see `qt_widgets/CMakeLists.txt`), so
`unfinished` entries are omitted from the `.qm` and render in English.

### Refresh behaviour

The script both **adds** brand-new K3 contexts and **refreshes** existing ones:
for any `unfinished` entry (empty or with an lupdate suggestion) whose source has
a K2 translation, it fills the K2 (authoritative, human) translation and clears
`type="unfinished"`. So re-running it is safe and idempotent, and it picks up
newly added `qsTr` strings as well as strings K2 has since gained. Edits are
targeted text replacements scoped to K3 contexts — the rest of each `.ts` is
untouched, and **no source string is ever modified**.

> This replaced an earlier add-only version that skipped existing contexts.

---

## New & reworded strings — the fill step

The bridging script only **matches** existing translations. It does nothing for
strings that have no match — and those are the strings that stay English. There
are exactly two sources, and they are identical in nature:

1. **New feature strings** — no K2 equivalent ever existed.
2. **Reworded / simplified strings** — a deliberate change to the wording.

> **Rewording a string destroys its translation in all 30 languages — K2 included.**
> To Qt, a reworded source is a brand-new string with zero translations. Example:
> the K3 Settings page was heavily reworded, so it is ~0% translated — every string
> diverges from the old source and matches nothing. Treat each reword as "throw away
> N existing translations and create N untranslated strings". **Reword deliberately
> and in batches**, never casually.

### The pipeline has a collect step but no fill step

`translations_lupdate` (or, during transition, `sync_k3_translations.py`) only
**collects** strings and marks new ones `type="unfinished"`. Because `lrelease`
runs with `-nounfinished`, every `unfinished` entry is dropped from the `.qm` and
renders in English. Something must **fill** those entries. That is the step below.

### Fill mechanism: AI batch-fill (chosen workflow)

New/reworded strings are translated by an **AI batch-fill** pass, using
`packaging/fill_k3_translations.py`, regularly:

1. **Collect** — run `sync_k3_translations.py` (adds new K3 contexts and fills every
   K2-matchable string with the real K2 translation). Only the genuine new/reworded
   strings remain `unfinished` afterwards.
2. **Extract** — `python3 packaging/fill_k3_translations.py --extract` writes
   `packaging/k3_unfinished.json`: `{ lang: { source: "" } }` for every remaining
   unfinished K3 string (en_US skipped; English source = displayed text).
3. **Translate** — fill the JSON values, AI-translating **from the authoritative
   English (`en_US`) source only** — never relay through another language. Keep
   placeholders (`%1`, `%2`), HTML/rich-text tags, and `&` accelerators intact.
4. **Write back** — `python3 packaging/fill_k3_translations.py --apply` replaces the
   matching `unfinished` entries (scoped to K3 contexts) with finished translations.
5. **Compile & embed** — `ninja translations_lrelease` → `ninja translations_copy`
   (K2 build dir) → `ninja` (qt_quick build dir).
6. **Spot-check** — review a sample; machine output is acceptable for an alpha but
   should be reviewable (optionally tracked for later human revision).

> `packaging/k3_unfinished.json` is a transient working file (regenerated by
> `--extract`); it need not be committed.

### Cadence (the "regular basis")

- Run the full collect → fill → compile pass **before every release**, and after
  any batch of rewording.
- **Batch rewording** decisions so a single fill pass covers them, rather than
  leaving strings English release after release.
- `en_US` is always the authoritative source; all other languages derive from it.

> Status: tooling implemented (`sync_k3_translations.py` matcher + refresh,
> `fill_k3_translations.py` extract/apply). What remains is ongoing *content* —
> translating the genuine new/reworded strings per language.

---

## First-run & language detection

Identical logic in K2 (`qt_widgets/main.cpp`) and K3 (`qt_quick/main.cpp`),
applied at startup before any UI/QML string is evaluated:

1. Read `Settings/Language` from the shared settings `.ini`.
2. **Legacy migration:** `cz_CZ` → `cs_CZ` (rewrite + persist).
3. **First run** (empty value): `userLanguage = Language::getSystemLanguage()`,
   which returns `QLocale::system().name()` only if supported, else `en_US`;
   guard with `isLanguageSupported()`; persist.
4. **Sanitize** an already-stored but unsupported value (e.g. `en_GB` from an
   earlier bug) → `en_US`; persist.
5. Load `Katalog_<code>` from the `:translations` resource; install the translator.

**Fallback target is always `en_US`** (English source strings = `en_US`).
A UK system (`en_GB`) is not in the list → resolves to `en_US`.

> The settings key `Settings/Language` is intentionally shared between K2 and K3
> so a single settings file works for both. K3 resolves the settings path the
> same portable-aware way `AppManager` does (executable dir if a settings file
> is present there, else `~/.config/katalog_settings.ini`).

---

## Runtime application of a language change

| | Apply mechanism |
|---|---|
| **K2** | Save `Settings/Language`; **requires app restart** (translator is loaded once at startup). |
| **K3** | Save via `AppManager::setLanguage()` → emits `languageChanged` → `main.cpp` reloads the translator and calls `engine.retranslate()` → **live, no restart.** |

K3 language selector: `PageSettings.qml` combo, populated from
`AppManager::getLanguageList()`, current value from `getCurrentLanguage()`,
applied via `setLanguage(code)`. (Custom `contentItem` combos must set
`displayText: ""` or the selected label renders twice under the desktop style.)

---

## Resource bundling

Both UIs bundle the prebuilt `translations/Katalog_*.qm` via a manual
`translations.qrc` (`qt_widgets/translations.qrc`, `qt_quick/translations.qrc`)
at resource path `:/translations/…`, loaded with
`translator->load("Katalog_<code>", ":translations")`. When a language is
added/removed/renamed, **all** of these must stay in sync: the `Language` list,
both `.qrc` files, `qt_widgets/CMakeLists.txt` `TS_FILES`, and the `.ts`/`.qm`
file names.

---

## Target K3 architecture (when K2 is retired)

Once K2 is no longer maintained, K3 becomes the sole owner of the strings and the
K2-bridging machinery is removed. Target design:

1. **K3 owns the `.ts` files directly.** Maintain them with `lupdate` run over
   `qt_quick/*.qml` (per-QML-file contexts), driven by `qt_add_translations`
   (or an explicit `lupdate`/`lrelease` target) in `qt_quick/CMakeLists.txt`.
   This replaces `sync_k3_translations.py`, which is retired.
2. **Per-QML-file contexts are the permanent model** — embraced, not bridged.
   The byte-for-byte rule still governs edits (it now protects K3's own
   translations rather than K2↔K3 matching).
3. **Live `retranslate()` is the only runtime model** — no restart path.
4. **`Language` core class stays the single source of truth** for codes
   (standard locale names), display names, and flags.
5. **First-run detection + `en_US` fallback chain stays as specified above.**
6. **One-time migration of legacy translations:** before deleting the K2
   `MainWindow` context, run a final `sync_k3_translations.py` (ideally the
   *refreshing* version) so every still-matching K2 translation is carried into
   the K3 contexts; only then drop the K2 `.ts` contexts.

### Migration checklist (K2 → K3-only)

- [ ] Extend the sync script to refresh existing contexts (add new strings,
      re-fill now-matching translations) for a clean final carry-over.
- [ ] Run the final sync; verify no regression in finished-string counts.
- [ ] Add a K3-native `lupdate`/`lrelease` flow in `qt_quick/CMakeLists.txt`.
- [ ] Remove K2 `TS_FILES` / `MainWindow`-context generation.
- [ ] Retire `packaging/sync_k3_translations.py`.
- [ ] Translate remaining K3-only `unfinished` strings across all languages.

---

## Rules of engagement (summary)

- **Never** add/change/remove a user-visible string without explicit per-string
  approval — byte-for-byte matching makes every edit a 30-language risk.
- When porting K2 → K3, **copy** the K2 `tr()` source verbatim into `qsTr()`.
- Adding/removing a language touches: `Language` list, both `.qrc` files,
  `qt_widgets/CMakeLists.txt` `TS_FILES`, and the `.ts`/`.qm` files.
- Progress/status messages follow `SpecProgressReport.md`.
- This is a Specification page: **English only**, no FR/CS translation.
