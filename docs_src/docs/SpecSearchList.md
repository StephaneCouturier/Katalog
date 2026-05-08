# SEARCH List as Input

## Context

The current Search tab accepts a **single text string** as the file name criterion. This forces users who need to find multiple specific files or apply several unrelated patterns to run repeated searches and mentally combine results. This spec explores scenarios where the text input becomes a **list of terms**, covering the need, each scenario's UI and backend impact, and a suggested phasing.


---

## Current State

| Aspect | Current behaviour |
|--------|-------------------|
| Text input | Single `QString` — one search string |
| Matching modes | All Words, Exact Phrase, Begins With, Any Word, Regex |
| Exclude input | Single `QString` — one exclusion string |
| SQL generation | Single `LIKE`/`GLOB`/`REGEXP` clause per criterion |
| Core field | `Search::searchText` — `QString` |

---

## Scenarios

### S1 — OR list: find files matching any of N terms
![implemented](https://img.shields.io/badge/K2-2.12-brightgreen) ![implemented](https://img.shields.io/badge/K3-pending-lightgrey)

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
![not implemented](https://img.shields.io/badge/status-not%20implemented-lightgrey)

**Need:** The *exclude* field has the same single-string limitation. Users want to exclude several unrelated patterns in one search.

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
![implemented](https://img.shields.io/badge/K2-2.12-brightgreen) ![implemented](https://img.shields.io/badge/K3-pending-lightgrey)

**Need:** User copies a column from a spreadsheet, a selection from a text editor, or output from a file manager and pastes it into the search form. Each line becomes one search term.

**Example:**

User selects in a text editor and copies:
```
holidays france
trip europe
weekend mountain
```
Clicks *Paste list* (or simply pastes into the textarea) → the three terms fill the list → search runs as S1.

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

### K3 implementation path (after K2 migration is complete)

All richer UI options remain open:

| Option | Description | Pros | Cons |
|--------|-------------|------|------|
| **A — Tag chips** | Terms appear as removable chips; type + Enter to add | Compact, visually clear | Harder to edit long terms |
| **B — Editable list rows** | Each term on its own row with a × button | Consistent with Katalog's exclude-folders widget | Taller UI with many terms |
| **C — Textarea (one per line)** | Same as K2 Step 1 — reuse as-is | Zero migration cost | No per-term mode control |

Option B or a hybrid A/B is preferred for K3 to align with Kirigami list patterns.

---

## Backend Impact Summary

| Change | Scope | Notes |
|--------|-------|-------|
| `Search::searchText` → `QStringList` | Core | Breaking change for all callers; `selectedTextCriteria` mode applies per-term |
| `Search::selectedSearchExclude` → `QStringList` | Core (S4) | Same approach |
| SQL query builder | Core | New `buildTextClause(QStringList, mode, andOr)` helper |
| K2 UI wiring | `mainwindow_tab_search_ui.cpp` | Replace single-field read with list read |
| K3 QML wiring | `PageSearchForm.qml` → `AppManager` | Replace `searchText` string property with list |
| Search history | `collection` | History serialization of multi-term queries |

---

## Suggested Priority

| Scenario | Value | Complexity | K2 phase | K3 phase |
|----------|-------|------------|----------|----------|
| S1 — OR list | High | Medium | Step 1 — textarea | Phase 1 |
| S6 — Clipboard import | High | Low | Step 1 (paste into textarea) | Phase 1 |
| S4 — Multiple exclude terms | High | Low | Step 1 (same textarea pattern) | Phase 1 |
| S7 — Load from file | Medium | Low | Step 2 — file path field | Phase 1 |
| S2 — AND list | Medium | Low (after S1) | No new K2 UI needed | Phase 2 |
| S3 — Exact filename list | Medium | Medium | Covered by textarea + Exact mode | Phase 2 |
| S5 — Per-term AND/OR flag | Low | High | Not in K2 | Backlog |
