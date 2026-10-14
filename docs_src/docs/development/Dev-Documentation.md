---
id: Dev-Documentation
title: Documentation
description: "How Katalog's documentation is organised, written, translated and built"
---

# Documentation

## Summary
This page describes the management of Katalog's Documentation.

Katalog's user documentation is published as a static website built with [**Docusaurus**](https://docusaurus.io/) (React/TypeScript).

- **Source repository:** `docs_src/` inside the main Katalog source tree
- **Default language:** English — source files live in `docs/`
- **Translations:** French (`i18n/fr/`) and Czech (`i18n/cs/`)
- **Sidebar:** manually defined in `sidebars.ts`

Commands, run from `docs_src/`:
- `npm run start` to start a local server for viewing and testing pages in English
- `npm run start -- --locale fr` to start it in another language (`fr` or `cs`)
- `npm run build` to check the production build of all languages; it reports front matter and broken-link errors the local server may not

---

## Specification and test pages

`Spec*.md` and `Test*.md` live in **`docs_src/docs/development/`**, not in the
docs root, which is reserved for user-facing feature pages and the
`k2/` copies. The `Dev-*` pages are in `development/` too.

**They are not listed in the sidebar.** The sidebar carries two index pages —
`development/Specifications` and `development/Tests` — and each lists every page
of its kind in a table. Adding a specification or a test plan means adding a row
to the relevant index; `sidebars.ts` does not change.

### Test plan pages

A specification may have a test plan beside it. **The file name is the
specification's name with `Spec` replaced by `Test`**: `SpecStorageIdentity.md`
is tested by `TestStorageIdentity.md`. One specification, one test plan, same
subject.

Rules:

* **Conventions are stated once, in the `Tests` index — a test plan does not
  repeat them.** A plan opens with a link to its specification and goes straight
  to its cases.
* **Every test case cites the requirement it verifies**, by its ID
  (`STI-F3`, `DSR-C8`…). A test that cites no requirement is testing something
  nobody asked for; a requirement no test cites is untested — both are visible
  at a glance when the IDs are in the table.
* Test case IDs mirror the prefix with a `T`: `STI-T1`, `STI-T2`… They are
  permanent, like requirement IDs: never renumbered, never reused, retired as
  `[Removed]` rather than deleted.
* A test case states **what to set up, what to do, and what must be true
  afterwards** — enough for someone who did not write the code to run it and
  judge the result without reading the source.
* Test plans are **English only**, like every other Development and
  Specification page.
* The specification keeps its short *Manual test charter*: one line per
  requirement saying what proving it looks like. The test plan is where that
  becomes runnable steps. The charter says *what*, the test plan says *how*.
* Result columns (`Pass` / `Fail` / date) belong in the test plan, not in the
  specification. A specification says what must be true; a test plan records
  whether it was.

---
## Page Design

* The page shall start with a minimal frontmatter block containing only `version`. This is machine-readable and more reliable than the badge for automation:
```
---
version: "2.10"
---
```

* The page shall have next the markdown header1 `#` followed by the page title.

* The page shall show a **Version badge** just after the header1 using shields.io (human-readable). Example:

![2.10](https://img.shields.io/badge/Version-2.10-blue)

* Workflow rule: when updating a translated page to match an English change, update both the `version` frontmatter field and the `Version` badge to the current release version.

* Badge labels (`Version-2.10`, etc.) are always in **English** across all languages.

* Katalog 2 pages are kept as copies in `docs/k2/` (and the same folder under each translation), stamped `2.13`, with `displayed_sidebar: tutorialSidebar` in the front matter and an info box pointing to the current documentation.

* **Development and Specification pages** use a different header: a front matter block with `id`, `title` and `description`, then the `#` title, then a **Status** badge and a **Version** badge. They are English only.

* Front matter values containing a colon (`:`) must be in double quotes, e.g. `description: "Requirements for X: what it does"`. An unquoted colon stops the documentation build with a YAML error.

* Features pages are written for end-users: **no code** allowed (no method names, variable names, source filenames, or internal identifiers).

### Section structure

Every feature page follows this structure:

```markdown
---
version: "2.10"
---
# PageName
![2.10](https://img.shields.io/badge/Version-2.10-blue)

## Summary
Short description of the screen + main screenshot.

## FeatureArea1
…

## FeatureArea2
…

## Development          ← optional, links to GitHub backlog
```

### Formatting rules

| Element | Syntax | Example |
|---------|--------|---------|
| Cross-page link | `[Label](PageID)` | `[Storage](../DevicesStorage)` |
| Deep link (anchor) | `[Label](PageID#anchor-id)` | `[duplicates](../Search#duplicates-on)` |
| Custom anchor on heading | `{#anchor-name}` after heading | `## Duplicates {#duplicates-on}` |
| Screenshot | `![Alt text describing the screenshot](/img/filename.png)` | `![BackUp links list](/img/k2/screen_backup_01.png)` |
| Narrow screenshot, text beside it | `<div className="screenshot-side">`, blank line, the screenshot, blank line, `</div>`; then `<div className="screenshot-side-end" />` where the text continues below the picture | drawer in `Interface.md`. Below 600 px wide the picture stacks above the text. To line a paragraph up with a lower part of the picture, put `<div className="screenshot-side-gap" style={{height: '10rem'}} />` before it (height measured on a render; hidden on narrow screens) |
| Screenshot not taken yet | `![Alt text describing the screenshot](/img/filename.png)` | written exactly like a screenshot, with a grey placeholder PNG saved under the final name in `static/img/` (`Screenshot to take: <name>`). Saving the real screenshot over it updates the page at once. Never leave the file missing: a page compiled while its image is missing keeps a broken link until `npm run clear`. List the placeholders left (they carry a PNG text tag): `grep -la screenshot-placeholder static/img/*.png` |
| UI button / label | `*Name*` | click *Create Catalog* |
| Inline code / path | `` `text` `` | path `/home/user/…` |
| Line break | `<br/>` | end of short sentence |
| Important callout | `:::note` … `:::` | for warnings or caveats |
| Option table | Markdown table | see Create.md for reference |
| Translation note | `<!-- translation note: … -->` | only where a term needs translator guidance (not systematic) |

**Screenshot files:** current (K3) screenshots are at the root of `static/img/`; the Katalog 2 screenshots are in `static/img/k2/` and are used only by the Katalog 2 copies.

**Screenshots:** always include descriptive alt text — it helps translators understand what the screenshot shows without seeing it, and makes missing images self-describing.

**Explicit anchors:** add `{#anchor-id}` to any heading that is cross-linked (from within the same page or from another page). This decouples the link target from the heading text — the heading can be reworded freely without breaking links. Not needed on headings that are never referenced directly.

**Planned / future features:** do **not** mention them inline in the feature content. All future ideas belong exclusively in the `## Development` section at the end of the page.

**UI names in translations:** tab names, button labels, and menu items must use the **exact translated form used in the application UI** — never a noun or gerund derived from it. For example, the French tab name for *Create* is *Créer* (not *Création*), and for *Search* is *Rechercher* (not *Recherche*). When referencing a UI name in running text, use italic (`*Name*`) to make clear it is a UI label, not a translated word.

### Development section (optional, end of page)

Feature pages may end with a `## Development` section. This is the **only** place where future or planned features are mentioned — never inline in the main content.

```
## Development
Some ideas of developments for this screen:
* idea 1
* For more, see the backlog of [PageName development](https://github.com/…?filterQuery=pagename).
```

---
## Docusaurus rendering

This project uses **Docusaurus `^3.10.0`** (configured in `package.json`).

Published site: [https://stephanecouturier.github.io/Katalog/](https://stephanecouturier.github.io/Katalog/)



---

## Known limitations & related Improvements

### Translation coverage

The French and Czech feature pages carry the same version as their English page.

### Language coverage
- continue to design this documentation management to automate for more Language
- cover the same languages as the app

### Establish ongoing process

For each release cycle:
1. Update English source pages first (single source of truth)
2. **Before translating: verify the English page fully complies with the Page Design guidelines** (frontmatter, status/version badges, no inline future features, alt text on screenshots, etc.) — fix any gaps first
3. Identify which translated pages are affected (diff or manual review)
4. Update FR/CS translations and bump `version`
5. For new pages: create the French and Czech files together with the English page

---
