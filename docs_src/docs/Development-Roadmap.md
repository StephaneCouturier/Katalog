# Roadmap
![Katalog2](https://img.shields.io/badge/Version-Katalog2-green) ![Katalog3](https://img.shields.io/badge/Version-Katalog3-orange)

## Vision

Katalog aims to provide the richest set of features for device and file management across as many platforms as possible starting from KDE Plasma on Linux, extending toward Windows, mobile, and web.

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

Active and planned work items are tracked in the [GitHub Backlog Major Features](https://github.com/users/StephaneCouturier/projects/7/views/1?sliceBy[value]=1_Major_feature).

---
## Katalog 3

### Introduction

**Katalog 3** is the new major version of Katalog, transitionning to a more modern UI. It relies on QtQuick and Kirigami from KDE (instead of QtWidgets).

**Katalog 2** will be maintained for some time: critical bugs & light features (when it uses the same core classes as Katalog 3).

This is enabled by having done a full split of UI and backend, and the common use of `/core` (backend) means that **Collections will remain compatible to both versions**.


![Preview of Katalog 3 interface](/img/K3_Search_Results_3Pages.png)
 
### Development phases

#### Code management:
see [Development-Repository](http://localhost:3000/Katalog/docs/Development-Repository)

#### Phases
Legend: ✅ Done · 🚧 Partial · 🔲 Not started

| Phase | Feature Scope                                                      | Status | Katalog 3 Version |
|-------|--------------------------------------------------------------------|--------|-------------------|
| 1     | READ ONLY, Search features, English Only                           |   ✅   | Katalog 3.alpha1  |
| 2     | Full Featured except Themes & Translations                         |   ✅   | Katalog 3.alpha2  |
| 3     | Translations, Documentation, Tests                                 |   ✅   | Katalog 3.0       |

| Platform |  Status | Notes                           |
|----------|---------|---------------------------------|
| Linux    |    ✅   | Primary dev environment         |
| Windows  |    ✅   | Build works with Craft          |
| macOS    |    ✅   | Build works in GitHub Actions   |

### Detailed status by Feature 

| Screen / Feature    | K2 | K3 | Remaining | New feature vs K2 |
|---------------------|----|----|-----------|-----------|
| **Screen/tabs**     | ✅ | ✅ | | - Access via a "Drawer" now, which can be hidden or pinned" |
| **Open Collection** | ✅ | ✅ | | - Clearer selection of mode and Collection. Open recent collections (last 5 entries). Create collection (File mode)|
| **Selection**       | ✅ | ✅ | | - Card like entries with file & storage statistics<br/> - Filter option to limit the list of Devices
| **Search**          | ✅ | ✅ | |
| Search Criteria     | ✅ | ✅ | | - Paste/Clean buttons for all text input fields|
| Search Results      | ✅ | ✅ | | - Select Device path is displayed
| Search History      | ✅ | ✅ | |
| Search in Connected | ✅ | ✅ | |
| Search Progress     | ✅ | ✅ | result throttler| |
| Search Pause/Stop   | ✅ | ✅ | Review thread mechanism compared to BackUp | |
| **Create**          | ✅ | ✅ | Review thread mechanism compared to BackUp, Add Paste button, Add Paste from selected path & name | |
| **Devices**         | ✅ | 🚧 | |
| View list           | ✅ | 🚧 | Storage and Catalog lists are card like display, to be reviewed and maybe transformed in actual tables |
| Edit                | ✅ | ✅ | |
| Update progress     | ✅ | ✅ | |
| Update Stop         | ✅ | ✅ | |
| **Explore**         | ✅ | ✅ | |
| **Statistics**      | ✅ | ✅ | zoom in feature |
| **Tags**            | ✅ | ✅ | |
| **Backup**          | ✅ | ✅ | |
| View                | ✅ | ✅ | |
| Create              | ✅ | ✅ | |
| Execute/progress    | ✅ | ✅ | |
| **Settings**        | ✅ | 🚧 | |
| SettingsFile        | ✅ | ✅ | |
| Version             | ✅ | ✅ | |
| Image folder        | ✅ | ✅ | |
| New & Import        | ✅ | ✅ | |
| Themes              | ✅ | 🔲 | optional |
| Languages           | ✅ | ✅ | |
| **About**           | ✅ | ✅ | |


### Feedback & Support

Bugs: https://github.com/StephaneCouturier/Katalog/issues

Feedback (UI, ideas, etc.): https://github.com/StephaneCouturier/Katalog/discussions

