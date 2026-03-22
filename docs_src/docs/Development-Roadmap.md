# Roadmap
![Katalog2](https://img.shields.io/badge/Version-Katalog2-green) ![Katalog3](https://img.shields.io/badge/Version-Katalog3-orange)

## Vision

Katalog aims to provide the richest set of features for device and file management across as many platforms as possible — starting from KDE Plasma on Linux, extending toward Windows, mobile, and web.

## Ideas

```mermaid
mindmap
  root((Katalog))
    File Management & Search
      More metadata
      Archive support
      Richer statistics
      Fuzzy matching
      File backup
    UI & Multi-platform
      Mordern UI (QtQuick / Kirigami)
      Mobile support
      Web features
    Quality & Scale
      Nextcloud as source
```

## Backlog

Active and planned work items are tracked in the [GitHub Backlog — Major Features](https://github.com/users/StephaneCouturier/projects/7/views/1?sliceBy[value]=1_Major_feature).

---
## Katalog 3

### Introduction

**Katalog 3** is a new major version of Katalog, transitionning to a more modern UI, and fitting future tablets & smartphones use. It relies on QtQuick and Kirigami from KDE.

**Katalog 2** remains the current major version, until Katalog 3 covers all features. It may be maintained beyond Katalog 3 release, as long as UI work is light.

**Katalog 3.0.0** will therefore be first complete release and will become then the main version. <br/>Until then, the release will still be 2.x but will include binaries **Katalog 3.alpha.x**.

This is enabled by having done a full split of UI and backend, and the common use of `/core` (backend) means that **Collections will remain compatible to both versions**.


![Preview of Katalog 3 interface](/img/K3_Search_Results_3Pages.png)
 
### Development phases

#### Code management:
see [Development-Repository](http://localhost:3000/Katalog/docs/Development-Repository)

#### Phases
Legend: ✅ Done · 🚧 Partial · 🔲 Not started

| Phase | Feature Scope                                  | Status | K3 Notes |
|-------|------------------------------------------------|--------|----------|
| 1     | READ ONLY, Search features, English Only       |   🚧   |          |
| 2     | Theme & Translations                           |   🔲   |          |
| 3     | READ ONLY advanced / graphical features        |   🔲   |          |
| 4     | CREATE / EDIT features                         |   🔲   |          |

| Platform |  Status | Notes                       |
|----------|---------|-----------------------------|
| Linux    |    🚧   | Primary dev environment     |
| Windows  |    🚧   | Build works with Craft      |
| macOS    |    🔲   | Never tried                 |
| Android  |    🔲   | Never tried                 |

### Detailed status by Feature 

| Screen / Feature      | K2 | K3 | Remaining | New vs K2 |
|-----------------------|----|----|-----------|-----------|
| **Screen/tabs**       | ✅ | ✅ | | - Access via a "Drawer" now, which can be hidden or pinned" |
| **Open Collection**   | ✅ | 🚧 | dialog to open is not well readable in light theme | - Open recent collections|
| **Selection**         | ✅ | ✅ | | - Card like entries with file & storage statistics<br/> - Filter option to limit the list of entries
| **Search**            | ✅ | 🚧 | |
| — Search Criteria     | ✅ | ✅ | closing Serach also coses Resuts | - Paste/Clean buttons for all text input fields|
| — Search Results      | ✅ | ✅ | | - Select Device path is displayed
| — Search History      | ✅ | 🔲 | |
| — Search in Connected | ✅ | 🔲 | |
| — Search Pause/Stop   | ✅ | 🔲 | review thread mechanism, compared to BackUp |
| — Search Progress     | ✅ | 🔲 | |
| **Devices**           | ✅ | 🔲 | |
| — View                | ✅ | 🔲 | |
| — Create/Edit         | ✅ | 🔲 | |
| — Update progress     | ✅ | 🔲 | |
| **Explore**           | ✅ | 🔲 | |
| **Create**            | ✅ | 🔲 | |
| **Statistics**        | ✅ | 🔲 | |
| **Tags**              | ✅ | 🔲 | |
| **Backup**            | ✅ | 🔲 | |
| — View                | ✅ | 🔲 | |
| — Create              | ✅ | 🔲 | |
| — Execute/progress    | ✅ | 🔲 | |
| **Settings**          | ✅ | 🚧 | glitch: showing search page when transition to or from About|
| — SettingsFile        | ✅ | ✅ | |
| — Version             | ✅ | ✅ | |
| — Themes              | ✅ | 🔲 | |
| — Language            | ✅ | 🔲 | |
| **About**             | ✅ | 🚧 | link to release notes |
