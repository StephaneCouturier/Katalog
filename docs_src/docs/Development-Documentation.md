# Documentation
![Draft](https://img.shields.io/badge/Status-Draft-orange) ![All](https://img.shields.io/badge/Version-All-green)

## Summary
This page describes the management of Katalog's Documentation.

Katalog's user documentation is published as a static website built with [**Docusaurus**](https://docusaurus.io/) (React/TypeScript).

- **Source repository:** `docs_src/` inside the main Katalog source tree
- **Default language:** English — source files live in `docs/`
- **Translations:** French (`i18n/fr/`) and Czech (`i18n/cs/`)
- **Sidebar:** manually defined in `sidebars.ts`

Documentation is generated and published following this sequence
- `npm run build` to generate the structure
- `npm run start` to start a local server for visualizing & testing pages in English
- `npm run start -- --locale=fr` to generate to start a local server with page in a different language

---

## Pages Inventory

Sections:
* Introduction (Overview and Tutorial)
* Features: 
    - Related to Katalog's user interface items: panel or tabs, in alphabetical order
    - Translated
* Development: 
    - Information on the project and content management
    - English only, not translated

### Introduction & Features section

| Page | EN | FR | CS | Notes |
|------|:--:|:--:|:--:|-------|
| Overview | 2.10 | 2.10 | 2.10 | |
| tutorial | 2.10 | 2.10 | 2.10 | |
| BackUp | 2.10 | 2.10 | 2.10 | |
| Create | 2.10 | 2.10 | 2.10 | |
| Devices | 2.10 | 2.10 | 2.10 | |
| DevicesCatalogs | 2.10 | 2.10 | 2.10 | |
| DevicesStorage | 2.10 | 2.10 | 2.10 | |
| DevicesTree | 2.10 | 2.10 | 2.10 | |
| Explore | 2.10 | 2.10 | 2.10 | |
| Search | 2.10 | 2.10 | 2.10 | |
| Selection | 2.10 | 2.10 | 2.10 | |
| Settings | 2.10 | 2.10 | 2.10 | |
| Statistics | 2.10 | 2.10 | 2.10 | |
| Tags | 2.10 | 2.10 | 2.10 | |
| CommandLines | 2.10 | 2.10 | 2.10 | |

### Development section (English only)

#### Development

| Page | Notes |
|------|-------|
| Development-Overview | |
| Development-Roadmap | |
| Development-Build-from-source | |
| Development-Release | |
| Development-Documentation | This page |
| Development-CodePractice | Coding conventions (diagnostic output, etc.) |

#### Specification pages

| Page | Notes |
|------|-------|
| BackUp_luckybackup_profile | |
| DevicesCatalogChecksum | |
| DevicesCatalogMetadata | |
| SpecCatalogIncludeExclude | |
| SpecProgressReport | |
| SpecVersions | |


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
| Cross-page link | `[Label](PageID)` | `[Storage](DevicesStorage)` |
| Deep link (anchor) | `[Label](PageID#anchor-id)` | `[duplicates](Search#duplicates-on)` |
| Custom anchor on heading | `{#anchor-name}` after heading | `## Duplicates {#duplicates-on}` |
| Screenshot | `![Alt text describing the screenshot](/img/filename.png)` | `![BackUp links list](/img/screen_backup_01.png)` |
| Screenshot placeholder | `<!-- screenshot: filename.png -->` | marks where a screenshot is needed but not yet available |
| UI button / label | `*Name*` | click *Create Catalog* |
| Inline code / path | `` `text` `` | path `/home/user/…` |
| Line break | `<br/>` | end of short sentence |
| Important callout | `:::note` … `:::` | for warnings or caveats |
| Option table | Markdown table | see Create.md for reference |
| Translation note | `<!-- translation note: … -->` | only where a term needs translator guidance (not systematic) |

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

This project uses **Docusaurus `^3.9.2`** (configured in `package.json`).

Published site: [https://stephanecouturier.github.io/Katalog/](https://stephanecouturier.github.io/Katalog/)



---

## Known limitations & related Improvements

### Outdated and Uneven translation coverage

FR and CZ pages are lagging behind the English pages, themselves lagging in Features added recently.

Approach:
- page by page
- Verify coverage of English against code
- Propose a list of additions (ex: function name)
- Add selected
- Update translated pages

### Language coverage
- continue to design this documentation management to automate for more Language
- cover the same languages as the app

### Establish ongoing process

For each release cycle:
1. Update English source pages first (single source of truth)
2. **Before translating: verify the English page fully complies with the Page Design guidelines** (frontmatter, status/version badges, no inline future features, alt text on screenshots, etc.) — fix any gaps first
3. Identify which translated pages are affected (diff or manual review)
4. Update FR/CS translations and bump `version`
5. For new pages: create translated files at release time (not during development)

---
