# SEARCH List as Input

## Context

The Search tab originally accepted a **single text string** as the file name criterion. This forced users who needed to find multiple specific files, or to apply several unrelated patterns, to run repeated searches and combine results (mentally or via exports). This spec explores scenarios where the text input becomes a **list of terms**, covering the need, each scenario's UI and backend impact, and a suggested phasing.

Multiple terms are already supported as one term per line (S1, S6), but entering
a second line requires **Shift+Enter**, which no affordance advertises. The
[K3 UI Specification — Term rows](#k3-ui-specification--term-rows) replaces that
hidden gesture with an explicit row list, without making the single-term case —
by far the most common — any harder.


---

## Current State

| Aspect | Current behaviour |
|--------|-------------------|
| Text input | Multi-line text area; one term per line, entered with **Shift+Enter** — not discoverable, which is what the term-rows phase fixes |
| Matching modes | All Words, Exact Phrase, Begins With, Any Word, Regex — applied per term |
| Exclude input | Single-line field; already multi-term, but split on **spaces** |
| SQL generation | Single `LIKE`/`GLOB`/`REGEXP` clause per criterion; terms OR'd inside one regex |
| Core field | `Search::searchText` — `QString`, split on `\n` in `prepareSearchPatterns()` |

---

## Scenarios

### S1 — OR list: find files matching any of N terms
![implemented](https://img.shields.io/badge/K2-2.12-brightgreen) ![implemented](https://img.shields.io/badge/K3-2.12-brightgreen)

**Need:** A list search is equivalent to running one search per term with the same criteria, then combining all results. User wants to locate files whose name or path matches at least one item in the list.

**Current workaround:** Run one search per term, collect results manually.

**Example:**

Input list (mode: *All Words*, search in: *File names and folder paths*):
```
holidays france
trip europe
weekend mountain
```
Results — a file appears if it matches **any** of the three terms:
```
/holidays/france/photo1.jpg          ← matched "holidays france"
/holidays/photo_france1.jpg          ← matched "holidays france"
/trip_europe.jpg                     ← matched "trip europe"
/photos/trip/europe.jpg              ← matched "trip europe"
/europe/trip1/IMG001.jpg             ← matched "trip europe"
/weekend/mountain/IMG001.jpg         ← matched "weekend mountain"
```

| Impact | Detail |
|--------|--------|
| UI | Text row becomes an expandable list; each entry is a term; one row always shown |
| Backend | `searchText` changes from `QString` to `QStringList` |
| SQL | `AND (filename LIKE '%t1%' OR filename LIKE '%t2%' OR ...)` |
| Matching modes | All existing modes (*All Words*, *Any Word*, *Exact Phrase*, *Begins With*, *Regex*) apply per-term independently |

---

### S2 — AND list: file name must contain all N terms
![not implemented](https://img.shields.io/badge/status-not%20implemented-lightgrey)

**Need:** User wants to narrow results to files whose name/path contains every term simultaneously. Currently partially achievable with *All Words* when terms are space-separated within a single string, but not for *Exact Phrase* or *Regex* per-term.

**Example:**

Input list (mode: *Any Word*, list logic: **AND**):
```
2024
france
holiday
```
Results — a file appears only if it matches **all three** terms:
```
/2024/france/holiday_001.jpg         ← all three present
/holiday_france_2024.mp4             ← all three present
```
Not matched:
```
/2024/france/photo.jpg               ← missing "holiday"
/holiday/france/2023.jpg             ← missing "2024"
```

| Impact | Detail |
|--------|--------|
| UI | Same list as S1 + a global mode switch: *Match any* (OR) / *Match all* (AND) |
| Backend | Two SQL generation paths depending on AND/OR mode |
| SQL | `AND filename LIKE '%t1%' AND filename LIKE '%t2%' ...` |

---

### S3 — Exact filename list (lookup mode)
![not implemented](https://img.shields.io/badge/status-not%20implemented-lightgrey)

**Need:** User has a manifest of specific filenames (from a colleague, a backup log, or a delivery list) and wants to find every item across all catalogs in one operation. The list may be large (dozens of filenames); the matching mode is always *Exact Phrase*.

**Current workaround:** Not achievable without external scripting.

**Example:**

Input list (mode: *Exact Phrase*):
```
invoice_2024_01.pdf
contract_acme_corp.docx
photo_passport_john.jpg
```
Results grouped by search term:
```
invoice_2024_01.pdf    → /documents/accounting/invoice_2024_01.pdf
contract_acme_corp.docx→ /documents/legal/contract_acme_corp.docx
                         /backup/contracts/contract_acme_corp.docx
photo_passport_john.jpg→ (not found)
```

| Impact | Detail |
|--------|--------|
| UI | Import button: *Paste list* and/or *Load from file* |
| Backend | Large `IN (…)` or repeated `OR` clauses; may need batching for very large lists |
| SQL | `AND filename IN ('f1','f2',…)` when mode is Exact, otherwise `OR LIKE` |
| Result display | Each result row should indicate which search term matched it |

---

### S4 — Multiple exclude terms
![specified](https://img.shields.io/badge/K3-3.0%20specified-blue)

**Need:** The *exclude* field has the same single-string limitation. Users want to exclude several unrelated patterns in one search.

> **Note:** exclude is *already* multi-term today, but split on **spaces** inside a
> single line. What S4 adds is a row-based editor plus newline tolerance in the
> splitter. See [K3 UI Specification — Term rows](#k3-ui-specification--term-rows).

**Example:**

Main search text: `photo`
Exclude list:
```
thumb
cache
tmp
```
Results: files containing `photo` anywhere, but **none** of the three excluded patterns in their path or filename.

| Impact | Detail |
|--------|--------|
| UI | Exclude row also becomes a list (same widget as S1) |
| Backend | `Search::selectedSearchExclude` changes from `QString` to `QStringList` |
| SQL | `AND filename NOT LIKE '%e1%' AND filename NOT LIKE '%e2%' ...` |
| Scope | Independent of S1/S2; can be shipped separately |

---

### S5 — Per-term AND/OR flag ![not implemented](https://img.shields.io/badge/status-not%20implemented-lightgrey)

**Need:** Advanced users want mixed logic — some terms required, others optional — within a single search.

**Example:**

Input list with per-term flags:
```
invoice  [AND — required]
2024     [AND — required]
receipt  [OR  — optional]
```
Results:
```
/accounting/invoice_2024_jan.pdf     ← matches AND group (invoice + 2024)
/expenses/receipt_hotel.pdf          ← matches OR term (receipt alone)
```
Not matched:
```
/accounting/invoice_2023.pdf         ← has "invoice" but not "2024", no "receipt"
```

| Impact | Detail |
|--------|--------|
| UI | Each list entry has an AND/OR badge or toggle — significantly more complex |
| Backend | SQL generation must build grouped `AND (… OR …)` expressions |
| SQL | `AND (filename LIKE '%invoice%' AND filename LIKE '%2024%') OR filename LIKE '%receipt%'` |
| Note | Most powerful but most complex; do not attempt before S1/S2 are stable |

---

### S6 — Clipboard import
![implemented](https://img.shields.io/badge/K2-2.12-brightgreen) ![implemented](https://img.shields.io/badge/K3-2.12-brightgreen)

**Need:** User copies a column from a spreadsheet, a selection from a text editor, or output from a file manager and pastes it into the search form. Each line becomes one search term.

**Example:**

User selects in a text editor and copies:
```
holidays france
trip europe
weekend mountain
```
Clicks *Paste list* (or simply pastes into the textarea) → the three terms fill the list → search runs as S1.

> **Extended by the term-rows phase:** with single-line rows a plain `Ctrl+V`
> would silently drop the newlines, so `Ctrl+V` is intercepted and expands into
> rows. See [Paste](#paste).

| Impact | Detail |
|--------|--------|
| UI | Textarea or *Paste list* button splits on newlines automatically |
| Backend | Pure UI pre-processing — no new core logic beyond S1/S3 |
| Scope | Depends on S1 or S3 being implemented first |

---

### S7 — Load list from file
![not implemented](https://img.shields.io/badge/status-not%20implemented-lightgrey)

**Need:** User maintains a text file of search terms that they reuse across sessions (licensed asset list, backup manifest, project file inventory).

**Example:**

File `search_list.txt`:
```
holidays france
trip europe
weekend mountain
```
User clicks *Load from file*, picks the file → the three terms fill the list → search runs as S1.

| Impact | Detail |
|--------|--------|
| UI | *Load from file* button opens a file picker; one term per line |
| Backend | `QFile` + `QTextStream` reads lines into the same list as S6 |
| Scope | Additive to S6; depends on S1 or S3 |

---

## Implementation Strategy

K2 is in UI maintenance mode: the existing feature set is being migrated to K3, and new major UI features will be designed and built in K3 only. For this feature, that means:

- **K2 gets minimal UI changes** — only what is needed to expose the backend capability without redesigning the search form.
- **K3 gets the full UI design** — once K2 migration is complete, the K3 search form can be designed with richer interaction patterns (chips, live list editing, etc.).

### K2 implementation path

| Step | Description |
|------|-------------|
| **Step 1 — Textarea** | Replace the single text input with a small multi-line textarea; one term per line; existing single-line input behaviour preserved when only one line is entered |
| **Step 2 — File path** | Add a single path field or *Browse* button below the textarea; the file is read at search time, lines appended to the textarea terms |

No additional buttons, no chips, no per-term mode controls in K2.

### K3 implementation path

The three options originally considered:

| Option | Description | Pros | Cons |
|--------|-------------|------|------|
| **A — Tag chips** | Terms appear as removable chips; type + Enter to add | Compact, visually clear | Harder to edit long terms |
| **B — Editable list rows** | Each term on its own row with a × button | Consistent with Katalog's exclude-folders widget | Taller UI with many terms |
| **C — Textarea (one per line)** | Same as K2 Step 1 — reuse as-is | Zero migration cost | Multi-line entry needs Shift+Enter, which is not discoverable |

**Decision: Option B — editable list rows.** C is what K3 ships today and the
Shift+Enter requirement is the exact problem this phase removes. A is rejected
because search terms are frequently long phrases (`holidays france 2024`) that
are awkward to edit inside a chip. B also aligns with Kirigami list patterns and
with the existing exclude-folders widget.

The full design is specified below.

---

## K3 UI Specification — Term rows

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Implementation](https://img.shields.io/badge/K3-3.0-blue) ![K2](https://img.shields.io/badge/K2-no%20change-lightgrey)

> **Approved for implementation.** All three `core/` changes (SRL-C3, SRL-C4,
> SRL-C5) and design points P1–P10 were approved by the user.

Covers **S1** (OR list), **S4** (multiple exclude terms) and **S6** (clipboard
import) for the K3 search form. Replaces the current `TextArea` +
Shift+Enter interaction.

### Design intent

One search term is the overwhelmingly common case and **must not cost the user
anything**. A single-term search therefore looks and behaves exactly as it does
today, plus one small `+` affordance. Everything about multiple terms is
progressive disclosure: nothing appears until a second row exists.

### Wire format — the compatibility contract

**`Search::searchText` and `Search::selectedSearchExclude` remain `QString`.
Rows are joined with `\n`.**

This single decision is what keeps the feature cheap and safe:

| Consequence | Detail |
|-------------|--------|
| No core signature change | `prepareSearchPatterns()` already splits `searchText` on `\n` into OR'd terms |
| No DB schema change | `search.text_phrase` / `search.text_exclude` are TEXT columns; newlines round-trip in both SQLite and MySQL |
| No history migration | **File/Hosted mode only.** Existing rows restore unchanged. **Memory mode is broken today — see [Memory mode — blocking defect](#memory-mode--blocking-defect)** |
| K2 ↔ K3 interoperable | Both write the same format to the same `search` table; a multi-term search saved in K3 restores in K2 as a multi-line `QPlainTextEdit`, and vice versa |
| Term list is UI-side only | Splitting and joining happens in `PageSearchForm.qml`; the core sees the string it sees today |

The `QStringList` migration sketched in *Backend Impact Summary* is therefore
**not** required for this phase and is deferred to S2/S5, which genuinely need
per-term structure.

### Layout

Single term — the common case. Identical to today except the `+` line:

```
text     [ holidays france                    ]  [✕] [📋] [⌫]
         [+]
```

Three terms:

```
text     [ holidays france                    ]  [✕] [📋] [⌫]
         [ trip europe                        ]  [−]
         [ weekend mountain                   ]  [−]
         [+]
```

- The `✕` Clear / `📋` Paste / `⌫` Clean button group stays anchored to the
  **first** row and acts on the **whole list** — they are list-level actions.
- `−` (`list-remove`) appears on every row **only when the list has more than one
  row**. A single-row list never shows it, so a one-term search is visually
  unchanged.
- `+` (`list-add`) sits on its own line under the list, flat/transparent, left
  aligned with the fields.
- Each row is a single-line `Controls.TextField`, replacing today's
  `Controls.TextArea` (`PageSearchForm.qml:555`).
- The same widget is used for the `exclude` field, which keeps its own
  `📋` / `⌫` buttons. It has no `✕` button today; instead it carries an **inline
  clear icon inside the field** (`edit-clear-locationbar`, shown when the field
  is non-empty — `PageSearchForm.qml:648-660`). **Decision: that inline icon
  stays per-row**, clearing only the row it sits in, and no list-level `✕` is
  added to exclude. Rationale: it is the standard Kirigami single-field
  affordance and a per-row clear is the natural reading of an icon drawn inside
  a row.

### Controls

| Control | Icon | Scope | Behaviour |
|---------|------|-------|-----------|
| `+` | `list-add` | list | Appends an empty row at the end and moves focus into it |
| `−` | `list-remove` | row | Deletes that row. Hidden when only one row exists, so the last row can never be deleted |
| `✕` Clear | `edit-clear` | list | Resets the list to a **single empty row** |
| `📋` Paste | `edit-paste` | list | **Replaces** the whole list with the clipboard, one row per line (current behaviour, extended to rows) |
| `⌫` Clean | `edit-clear-history` | list | Applies `returnCleanedText()` to **every** row. The helper only substitutes punctuation for spaces and never touches `\n`, so per-row and whole-string application are equivalent |

### Row count and height

There is no hard cap on the number of rows. To stop a long list from pushing the
rest of the form off screen, the row list has a **maximum visible height of six
rows** and scrolls beyond that; the `+` line stays visible below the scrolling
area. Below six rows the list grows naturally with no scrollbar.

### Keyboard

| Key | Behaviour |
|-----|-----------|
| `Enter` | Runs the search — unchanged from today and from K2 (`mainwindow_tab_search_ui.cpp:48`) |
| `Shift+Enter` | **Adds a new empty row below the focused row and moves focus into it.** Preserves the existing muscle memory: the gesture that used to insert a newline now creates the row that newline represented |
| `Ctrl+V` | See *Paste* below |
| `Backspace` at position 0 of an **empty, non-first** row | Deletes the row and moves focus to the end of the previous row |
| `Down` / `Up` | Moves focus to the next / previous row. Free with single-line fields and expected by keyboard users |
| `Tab` | Focus order is: row 1 field → row 1 `−` (when visible) → row 2 field → … → last row → `+` → the list-level `✕` / `📋` / `⌫` group → the `with` combo. The list-level group comes after the rows despite being drawn beside row 1, so tabbing walks the terms without interruption |

### Paste

Multi-line clipboard content is the main import path (S6) and must never be
silently lost. A single-line `TextField` drops newlines on a plain paste, so
`Ctrl+V` is intercepted.

| Source | Clipboard | Behaviour |
|--------|-----------|-----------|
| `📋` button | any | Replaces the entire list; one row per non-empty line |
| `Ctrl+V` | single line | Normal insertion at the cursor in the focused row |
| `Ctrl+V` | multiple lines | First line is inserted **at the cursor** in the focused row; remaining lines become new rows **immediately after** it, pushing existing rows down. Focus moves to the end of the last inserted row |
| `Ctrl+C` / `Ctrl+X` | — | Unchanged, per row |

The same rules apply to the **exclude** list, with its own clipboard read; the
text list and the exclude list never share content.

Line splitting rules, applied to both sources and both lists:

- Split on `\n`, tolerating `\r\n`.
- Empty and whitespace-only lines are skipped.
- A trailing newline never produces an empty final row.
- If the result is empty, the list falls back to a single empty row.

**Paste size cap.** Terms are OR'd into a single alternation regex
(`termPatterns.join("|")`, `core/search.cpp:330`) which is evaluated per file, so
a very large paste degrades search speed sharply. A paste producing more than
**100 rows** is truncated to the first 100 and reports the truncation. *(Cap
value awaiting approval — see below. An uncapped paste of a 5000-line clipboard
is a plausible way to make Katalog appear to hang.)*

### Normalization on search launch

Applied when the criteria string is built in `getCriteria()`:

1. Each row is `trimmed()`.
2. Blank rows are excluded from the joined string.
3. Blank rows are also pruned from the UI, always keeping at least one row.
4. If every row is blank, `searchText` is `""` — identical to an empty field
   today; existing validation applies unchanged.
5. Row order is preserved. Duplicate rows are **not** removed: they are harmless
   in an OR list, and silently dropping user input is worse than a redundant term.

### Per-term semantics

Unchanged from S1, restated here because the row UI makes it visible:

- The `with` mode combo (*All Words*, *Exact Phrase*, *Begins With*, *Any Word*,
  *Regex*) applies **per row, independently**.
- Rows are combined with **OR** — a file matches if it matches any row.
- In *Regex* mode a row may itself contain `|`; it is OR'd into the group as-is.
- Exclude rows are combined with **AND NOT** — a file is rejected if it matches
  any exclude term.

### Exclude field — approved core change

Exclude terms are split on **spaces**, not newlines
(`core/search.cpp:353`), so rows are not free here. Two changes to
`Search::prepareSearchPatterns()` are **approved** for this phase:

| # | Change | Rationale |
|---|--------|-----------|
| 1 | `split(" ", Qt::SkipEmptyParts)` → `split(QRegularExpression("\\s+"), Qt::SkipEmptyParts)` | Treats a newline like a space. Behaviour is identical for any existing input whose only whitespace is the **space character**. `\s` also matches `\t`, `\r`, `\v`, `\f`: an existing exclude containing a literal tab currently means the single term `a<TAB>b` and would become two terms. Judged acceptable — a tab cannot be typed into the field and would only arrive via paste |
| 2 | Guard `lineFieldList.isEmpty()` before `lineFieldList[0]` at `core/search.cpp:357` | Pre-existing latent out-of-bounds: a whitespace-only exclude string passes the `!= ""` test but splits to an empty list. Blank rows make this easy to reach |

**Documented asymmetry:** because the split stays whitespace-based, an *exclude*
row containing spaces still yields several independent terms — `thumb cache` in
one row means "exclude thumb OR cache", not "exclude the phrase thumb cache". A
*text* row, by contrast, is one term interpreted by the mode combo. Making
exclude rows newline-only would be more consistent but would silently change the
meaning of every existing single-line exclude and of every saved search that uses
one, so it is rejected. Revisit under S5 if per-term control is ever added.

### Search history

| Aspect | Rule |
|--------|------|
| Save | **No change.** `text_phrase` and `text_exclude` receive the `\n`-joined, already-normalized strings (`core/search.cpp:709`, `:745`) |
| Restore | `applyHistoryCriteria()` (`PageSearchForm.qml:224`, `:230`) splits the stored string on `\n` using **the same rules as paste** — `Qt::SkipEmptyParts` plus `trimmed()` per term — so restore and launch normalization can never disagree. Always at least one row; an empty stored value yields one empty row |
| Empty rows | Cannot reach history — normalization strips them before the search runs, and history is written from the executed criteria |
| K2 interop | A K3 multi-row search restores in K2 as a multi-line `QPlainTextEdit`, and a K2 multi-line search restores in K3 as multiple rows |
| Command line | `core/commandline.cpp:754` prints `Search text: "<phrase>"` on one line when listing history. It applies the same rule as the summary: terms quoted individually, joined with `", "`, so the output stays one line per entry |

### Memory mode — blocking defect

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Approval](https://img.shields.io/badge/Approval-granted-brightgreen)

The "no history migration" claim holds only for File/Hosted mode. In **Memory
mode** search history is persisted as an **unescaped, tab-separated,
newline-delimited flat file**:

- `Collection::saveSearchHistoryTableToFile()` (`core/collection.cpp:1598-1606`)
  writes `out << record.value(i).toString()` with `'\t'` between fields and
  `'\n'` after the record. **No escaping of either delimiter.**
- `Collection::loadSearchHistoryFileToTable()` (`core/collection.cpp:866+`) reads
  with `readLine()`, then `line.split('\t')`, then pads short records to 43
  fields with `""`.

A `\n` inside `text_phrase` therefore splits one record across several physical
lines. On reload the first line is short and gets silently padded — **losing
every criterion stored after the phrase** — and each continuation line is
imported as a bogus extra history entry. K3 reaches this path:
`qt_quick/appmanager.cpp:238` calls `saveSearchHistoryTableToFile()`.

This is **pre-existing drift, not caused by term rows** — K2's `QPlainTextEdit`
can already write a multi-line phrase today. But term rows make multi-line the
*normal* case, converting a rare latent corruption into a routine one. It is a
**third core change**, approved as SRL-C5.

Resolution — escape on write, unescape on read, inside those two
`Collection` methods only:

| Aspect | Proposal |
|--------|----------|
| Escape | On write, `\` → `\\`, then `\t` → `\t`, `\n` → `\n` (literal two-character sequences). On read, reverse in the opposite order |
| Scope | The two Memory-mode methods above; no change to the DB path, to `Search`, or to the K2/K3 UI |
| Backward compatibility | Old files contain no escape sequences. A stored phrase containing a literal backslash-n would be misread once, on first load of a pre-fix file. Judged acceptable: `\n` is not a plausible search term, and the alternative (a version marker in the file header) costs more than the risk |
| Benefit | Fixes an existing 2.13 defect that already affects K2 users, independently of term rows |

The alternative — blocking multi-line phrases in Memory mode — was rejected: it
would make the feature mode-dependent and leave the existing K2 defect unfixed.

### History summary rendering

The history list delegate is a single line, so an embedded
newline must never reach it. In `AppManager::getSearchHistory()`
(`appmanager.cpp:1277`), the phrase is currently wrapped in one pair of quotes.
The rule becomes: **split the stored phrase on `\n`, quote each term
individually, and join the quoted terms with `", "`.** The criteria suffix is
appended once, after the list. The same applies to the exclude terms passed into
the existing `tr("Exclude: %1")`.

Stored `text_phrase`:

```
holidays france
trip europe
weekend mountain
```

Rendered summary:

```
"holidays france", "trip europe", "weekend mountain" (Exact Phrase)
```

A single-term search renders exactly as it does today: `"holidays france"`.

### Translation impact

**Zero new translatable strings.** This is a hard requirement of the design:

- `+` and `−` are icon-only (`list-add` / `list-remove`), matching the adjacent
  `✕` / `📋` / `⌫` buttons, which carry no tooltip in K3 today.
- The history summary joins terms with quotes and `", "` — punctuation only, no
  `tr()`.
- No placeholder text is added to the rows.

If tooltips are ever wanted on `+` / `−`, they must be added to all five buttons
in the row together, with per-string approval, and must reuse the existing K2
tooltip strings where one fits (`mainwindow.ui:945`).

### Validation

No change, and **no rule exists to change**. `SpecValidationRules.md` covers only
the Tags page (T1–T2) and the Create page (C1–C4); it states nothing about the
search form, and K3's search form has no empty-text guard today (no
`InlineMessage` in `PageSearchForm.qml`). An all-blank list produces
`searchText == ""`, which is exactly what an empty field produces today — no
validation rule exists for either case, and the term-rows phase adds none.

### Approved design points

The user first approved three things: the `+` / `−` row layout, `Ctrl+V`
interception, and the exclude field becoming a row list. Everything below was
proposed while writing this spec and was **subsequently approved as a whole**,
including the two additional `core/` changes P8 and P10.

| # | Decision | Why it was proposed | Alternative that was rejected |
|---|-----------|---------------------|-------------|
| P1 | `Shift+Enter` repurposed to "add row" | Preserves existing muscle memory rather than making the gesture dead | Shift+Enter does nothing; `+` is the only way to add a row |
| P2 | `Backspace` at position 0 of an empty row deletes it | Standard list-editing behaviour | Rows are removed only with `−` |
| P3 | `Up` / `Down` move focus between rows | Free with single-line fields; expected by keyboard users | Only `Tab` traverses rows |
| P4 | Max **six** visible rows, then scroll | Stops a long list pushing the form off screen. The number is arbitrary | List grows unbounded |
| P5 | Duplicate rows are **not** deduplicated | Silently dropping user input is worse than a redundant term | Dedupe on launch |
| P6 | Blank rows pruned from the **UI** on launch | Keeps the form tidy | Blank rows stay visible; they are still excluded from the criteria |
| P7 | Paste cap of **100 rows** | An uncapped paste builds a huge alternation regex evaluated per file | No cap; accept the performance risk |
| P8 | Empty-list guard (core change #2) | Fixes a real pre-existing out-of-bounds | Crash remains reachable |
| P9 | K2 ↔ K3 history interoperability | Both already write the same column; stated so it is not broken by accident | Not tested for |
| P10 | Memory-mode TSV escaping (core change #3) | **Was blocking** — see above | Term rows cannot ship in Memory mode |

P8 and P10 are defect fixes rather than features; they are recorded separately
because CLAUDE.md requires prior approval for any `core/` change and neither was
covered by the original exclude-row approval. Both are now approved.

### Out of scope for this phase

| Excluded | Reason |
|----------|--------|
| Reordering rows (drag handles) | Term order does not affect an OR list |
| Per-row mode combo, per-row AND/OR | S5 — needs the `QStringList` core migration |
| AND across rows | S2 |
| Load list from file | S7 |
| Any K2 UI change | K2 is in maintenance mode and keeps its `QPlainTextEdit` |
| Grouping results by matched term | S3 |

---

## Scope at a glance

| | |
|---|---|
| **In scope** | K3 search form: text term list, exclude term list, clipboard import into rows, history save/restore/summary of multi-term searches |
| **Out of scope** | K2 UI (keeps `QPlainTextEdit`), AND logic across terms (S2), per-term mode or AND/OR (S5), load from file (S7), grouping results by matched term (S3) |
| **Applies to** | K3 3.0 |
| **Depends on** | S1 and S6, already implemented |

## Operational requirements — *why / for whom*

| ID | Requirement | Status |
|----|-------------|--------|
| SRL-O1 | A user searches for several unrelated terms in one operation instead of running one search per term and combining results by hand. | [Implemented] |
| SRL-O2 | A user discovers that multiple terms are possible without being told — the affordance is visible on the form. | [Planned] |
| SRL-O3 | A user searching for a single term — the common case — does no more work than before. | [Planned] |
| SRL-O4 | A user imports a list of terms copied from a spreadsheet, text editor or file manager. | [Implemented] |
| SRL-O5 | A user excludes several unrelated patterns from one search. | [Implemented] |
| SRL-O6 | A user re-runs an earlier multi-term search from history and gets every term back. | [Planned] |

## Functional requirements — *what the system does*

| ID | Requirement | Status |
|----|-------------|--------|
| SRL-F1 | The text criterion is a list of rows, one term per row, showing exactly one row when empty. | [Planned] |
| SRL-F2 | `+` appends an empty row and focuses it. `−` deletes its row and is hidden when only one row exists, so the last row cannot be deleted. | [Planned] |
| SRL-F3 | Terms are combined with **OR**: a file matches if it matches any row. The `with` mode applies to each row independently. | [Implemented] |
| SRL-F4 | The exclude criterion is the same row list. Exclude terms are combined with **AND NOT**. | [Planned] |
| SRL-F5 | `Enter` runs the search from any row — unchanged from today and from K2. | [Implemented] |
| SRL-F6 | The `📋` Paste button replaces the whole list with the clipboard, one row per non-empty line. | [Planned] |
| SRL-F7 | `Ctrl+V` of multi-line clipboard content splices the lines in at the cursor as new rows; single-line content pastes normally. Newlines are never silently dropped. | [Planned] |
| SRL-F8 | `✕` Clear resets the list to one empty row. `⌫` Clean applies `returnCleanedText()` to every row. | [Planned] |
| SRL-F9 | On launch, rows are trimmed and blank rows are excluded from the criteria; an all-blank list is equivalent to an empty field. | [Planned] |
| SRL-F10 | A multi-term search is saved to history and restored from it with every term, in order, alongside all other criteria. | [Planned] |
| SRL-F11 | The history summary renders each term quoted and joined with `", "`, on a single line, with the criteria suffix appended once. | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| SRL-C1 | `Search::searchText` and `Search::selectedSearchExclude` remain `QString`, with rows joined by `\n`. The row list is **UI-side only**; the core sees the string it sees today. | [Planned] |
| SRL-C2 | The term-rows phase MUST NOT change the database schema and MUST NOT require a history migration. | [Planned] |
| SRL-C3 | **Approved core change.** The exclude splitter in `Search::prepareSearchPatterns()` becomes `split(QRegularExpression("\\s+"), Qt::SkipEmptyParts)` so a newline is treated as a separator. Behaviour is unchanged for any input whose only whitespace is the space character. | [Planned] |
| SRL-C4 | `prepareSearchPatterns()` MUST guard against an empty term list before indexing `lineFieldList[0]` (`core/search.cpp:357`). **Approved core change.** | [Planned] |
| SRL-C5 | Search history MUST survive a multi-line `text_phrase` in **Memory** mode. `Collection::saveSearchHistoryTableToFile()` / `loadSearchHistoryFileToTable()` escape and unescape the `\t` and `\n` delimiters. **Approved core change.** | [Planned] |
| SRL-C6 | The phase MUST add **zero** new translatable strings. `+` / `−` are icon-only; the history separator is punctuation. | [Planned] |
| SRL-C10 | A term pattern MUST NOT contain a **top-level alternation**: the exclude clause is prefixed to it and `\|` binds looser than concatenation, so a bare `a\|b` leaves the exclude guarding only `a`. *Any Word* and *Regex* term patterns, and the multi-term join, are wrapped in `(?:…)`. | [Planned] |
| SRL-C11 | The exclude clause is anchored with `^`, so the search pattern MUST be combined with it as a **lookahead** (`^(?!…)(?=.*PATTERN)`), never by concatenation. Concatenating forces the search to match at position 0, turning *Exact Phrase*, *Any Word* and *Regex* into "starts with" searches whenever an exclude term is present. | [Planned] |
| SRL-C7 | K2 MUST NOT change. Both versions keep writing the same `\n`-joined format to the same `search` table, and each restores the other's saved searches. | [Planned] |
| SRL-C8 | The UI layer MUST NOT contain raw SQL for this feature; splitting and joining is string handling in QML. | [Planned] |
| SRL-C9 | Any single-line consumer of `text_phrase` MUST render terms joined on one line — the history delegate and `core/commandline.cpp:754`. | [Planned] |

---

## Manual test charter

For each row: set up the stated condition, run the operation, confirm the result.

- **SRL-O3 / SRL-F1** — Open a fresh search form. Exactly one text row and one exclude row are shown, each with a `+` beneath; no `−` is visible anywhere.
- **SRL-F5** — Type one term, press `Enter`. The search runs exactly as it did before the change.
- **SRL-F2** — Click `+`. An empty row appears, focused, and `−` appears on every row. Remove rows until one remains: `−` disappears again and the remaining row cannot be deleted.
- **SRL-F3** — Enter rows `a` and `b` with mode *Exact Phrase*. The result set is the union of a search for `a` alone and a search for `b` alone.
- **SRL-F4** — Enter exclude rows `thumb` and `cache`. Every file matching either is absent from the results.
- **SRL-C3** — Enter the single-line exclude `thumb cache tmp`. Results are identical before and after the core change.
- **SRL-C4** — Enter an exclude list of only blank rows. The search runs, applies no exclusion, and does not crash.
- **SRL-C10** — Search two text terms with an exclude term that appears in files matching the **second** term. Those files are absent from the results, exactly as for the first term.
- **SRL-C11** — Search one term in *Exact Phrase* mode with any exclude term, where the term appears in the **middle** of the file name. The file is found. Repeat in *Any Word* mode.
- **SRL-F6** — Copy three lines, click `📋`. Exactly three rows appear and any previous content is gone.
- **SRL-F7** — With three rows and the cursor mid-text in row 2, `Ctrl+V` three lines. Rows 1 and 3 are intact, the three lines are spliced in at the cursor, and the last inserted row has focus. Repeat with single-line clipboard content: it inserts at the cursor and creates no row.
- **SRL-F8** — With three rows, click `⌫`: all three are cleaned. Click `✕`: the list returns to one empty row.
- **SRL-F9** — Run a search with rows `a`, blank, `b`. Results match a search with rows `a`, `b`.
- **SRL-F11** — Run a three-term search. The history entry is one line reading `"a", "b", "c"` with no newline and no wrapping.
- **SRL-F10** — Click that history entry. All three text rows are restored in order, plus the exclude rows, plus every other criterion.
- **SRL-C5** — **In Memory mode**, run a three-term search, close and reopen the collection. The history entry survives intact, with all criteria, and no bogus extra entries appear.
- **SRL-C7** — Save a multi-term search in K3, open the same collection in K2: the phrase appears as multiple lines. Save a multi-line search in K2, open in K3: it appears as multiple rows.
- **SRL-C9** — List search history from the command line with `--verbose`. Each entry prints on one line.
- **SRL-C6** — Run `ninja translations_lupdate`. No new untranslated string appears for the search form.
- **P4** — Enter 10 rows. The list scrolls within a bounded height instead of pushing the rest of the form off screen.

---

## Backend Impact Summary

### Term rows phase (S1 / S4 / S6) — what is actually needed

| Change | Scope | Notes |
|--------|-------|-------|
| Row list widget, join/split on `\n` | `PageSearchForm.qml` | Replaces the `TextArea`; `getCriteria()` joins, `applyHistoryCriteria()` splits |
| Exclude splitter tolerates newlines | Core — `Search::prepareSearchPatterns()` | **Approved** (SRL-C3). `split(" ")` → `split(QRegularExpression("\\s+"))`; unchanged for space-only input |
| Empty-list guard | Core — `core/search.cpp:357` | **Approved** (SRL-C4 / P8). Fixes a pre-existing out-of-bounds on a whitespace-only exclude |
| Memory-mode history escaping | Core — `Collection::save/loadSearchHistory*` | **Approved** (SRL-C5 / P10). Without it, multi-line phrases corrupt Memory-mode history |
| History summary joins quoted terms | `AppManager::getSearchHistory()` | Keeps the delegate on one line; no new translatable string |
| Database schema | — | **None** |
| `Search::searchText` type | — | **Unchanged** — stays `QString` |
| K2 UI | — | **None** — keeps `QPlainTextEdit` |

### Deferred to S2 / S5 — only if per-term structure is ever required

| Change | Scope | Notes |
|--------|-------|-------|
| `Search::searchText` → `QStringList` | Core | Breaking change for all callers; `selectedTextCriteria` mode applies per-term |
| `Search::selectedSearchExclude` → `QStringList` | Core | Same approach |
| SQL query builder | Core | New `buildTextClause(QStringList, mode, andOr)` helper |
| Search history serialization | Core / `collection` | Only if the stored format stops being a `\n`-joined string |

---

## Suggested Priority

| Scenario | Value | Complexity | K2 phase | K3 phase |
|----------|-------|------------|----------|----------|
| S1 — OR list | High | Medium | Step 1 — textarea (done) | Term rows |
| S6 — Clipboard import | High | Low | Step 1 (paste into textarea, done) | Term rows |
| S4 — Multiple exclude terms | High | Low | No K2 change | Term rows |
| S7 — Load from file | Medium | Low | Step 2 — file path field | Phase 2 |
| S2 — AND list | Medium | Low (after S1) | No new K2 UI needed | Phase 2 |
| S3 — Exact filename list | Medium | Medium | Covered by textarea + Exact mode | Phase 2 |
| S5 — Per-term AND/OR flag | Low | High | Not in K2 | Backlog |
