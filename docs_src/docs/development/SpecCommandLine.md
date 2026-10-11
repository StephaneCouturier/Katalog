---
id: SpecCommandLine
title: Command Line — Running Katalog Actions from a Terminal
description: Requirements for the command-line actions (list, update, search) shared by K2 and K3, the settings file a command-line run uses, the reported version, and the K3 wiring needed before the 3.0 release.
version: "2.13"
---

# COMMAND LINE — RUNNING KATALOG ACTIONS FROM A TERMINAL

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Version](https://img.shields.io/badge/Version-2.13-blue) ![Implementation](https://img.shields.io/badge/Implementation-K2%20%2B%20K3-brightgreen) ![K2](https://img.shields.io/badge/K2-minimal%20fix-lightgrey) ![K3](https://img.shields.io/badge/K3-3.0-blue)

> **Approved 2026-10-11.** The maintainer approved this requirement area and
> answered all open questions the same day (see the decisions below).

## Context

Katalog can run some tasks from a terminal, without the graphical interface, so
they can be scripted (list the catalogs, update one or all active catalogs, run a
search). The handler is shared core code, so K2 and K3 get the same actions and
options. The user documentation is the *Command Lines* page (K3 page, still a K2
copy, and its `k2/` copy).

State on 2026-10-11:

- **K2** detects a command-line launch in its `main` and hands it to the shared
  handler; when the handler reports "no action", K2 continues to the window.
- **K3 does not detect command-line launches at all.** Every invocation, including
  `search` or `--help`, opens the window. This is a defect to fix before the 3.0
  release (`CLI-F1`).
- The handler resolves the settings file itself, with a hard-coded
  `katalog_settings.ini`. K3 resolves its own file elsewhere, and pre-release K3
  builds use `katalog3_prerelease_settings.ini`. A K3 command-line run would
  therefore read a different file from the K3 window. The maintainer decided on
  2026-10-11 that this path rule lives **once**, in core (`CLI-C2`).
- `--version` reports `2.6` in both versions: the value is hard-coded in the core
  handler and in K2's `main` (`CLI-F7`, `CLI-C3`).

---

## Scope at a glance

**In scope:** the four actions and their options; detecting a command-line launch
and leaving it without opening a window; the settings file a command-line run
reads; the version `--help` and `--version` report; the K3 wiring; the K3 user
page.

**Out of scope (non-goals):** new actions or options; console output on Windows
(see *Known limitations*); the K2-only `restart` action (K3 reconnects in place and
has no use for it); translating command-line output; the deactivated `--report`
option; any change to K2 beyond the minimal fixes named here.

---

## Operational requirements — *why / for whom*

| ID | Requirement | Status |
|----|-------------|--------|
| CLI-O1 | A user can list, update and search the catalogs of a collection from a terminal, without the graphical interface, so these tasks can be automated (scripts, scheduled jobs). | [Implemented] |
| CLI-O2 | A K3 user has the same command-line actions and options as a K2 user, and a command-line run works on the same collection and settings as the K3 window they would open. | [Implemented] |

## Functional requirements — *what the system does*

| ID | Requirement | Status |
|----|-------------|--------|
| CLI-F1 | When Katalog (K2 or K3) is launched with one of the actions `list_catalogs`, `update_catalog`, `update_all_active`, `search`, or with `-h`/`--help` or `-v`/`--version`, it runs in command-line mode: it performs the request, prints its output to the terminal and exits with the result code of `CLI-F8`. **No window opens.** Launched without any of these, it opens the window as usual. *(K2 meets this. K3 does not: it always opens the window — the 3.0 defect this page exists for.)* | [Implemented] |
| CLI-F2 | `list_catalogs` lists the catalogs of the collection with their ID, active state and name. | [Implemented] |
| CLI-F3 | `update_catalog <deviceID>` updates the catalog of that device. A missing or non-numeric device ID is an error (`CLI-F8`). | [Implemented] |
| CLI-F4 | `update_all_active` updates every active catalog of the collection. | [Implemented] |
| CLI-F5 | `search` runs a search with the criteria of the last search in the collection's search history, each overridable by an option: `--text`, `--type` (`all`, `audio`, `image`, `text`, `video`), `--size-min`, `--size-max` (e.g. `1MB`, `5GB`), `--date-after`, `--date-before` (`YYYY-MM-DD`), `--case-sensitive`, `--search-in` (`filenames`, `files-and-folders`, `folder-paths`), `--text-criteria` (`all-words`, `exact-phrase`, `begins-with`, `any-word`), `--exclude`. `--no-history` starts from default criteria instead of the history. `--selectedDeviceID` limits the search to one device (`0` = all). `--limit` caps the number of files printed. An invalid value for any of these is an error (`CLI-F8`). | [Implemented] |
| CLI-F6 | Without `-c`/`--collection`, every action uses the collection recorded in the settings file of `CLI-F9`. With `--collection <path>`, it uses that collection instead, its mode detected from the folder contents (Memory: `device.csv` and `storage.csv`; File: a `.db` file or the folder holding it); an undetectable path is an error (`CLI-F8`). `--verbose` adds progress detail to the output. | [Implemented] |
| CLI-F7 | `-v`/`--version`, and the header of `-h`/`--help`, report the version of the **running** application: the K2 build version (2.13 in this cycle) for K2, the K3 build version (e.g. `3.0.beta2`, `3.0` at release) for K3 — never a fixed number. *(Both report `2.6` on 2026-10-11.)* | [Implemented] |
| CLI-F8 | A successful run exits with code `0`. An error (invalid option value, unknown action, collection not found or not openable) prints a message on standard error and exits with code `1`. `update_catalog` and `update_all_active` also exit with code `1` when a catalog is not updated (update error or cancellation, device not a catalog or not active; for `update_all_active`, any catalog failing), so a script or scheduled task can detect the failure. | [Implemented] |
| CLI-F9 | A command-line run reads the **same settings file** as the window of the same application on the same machine: for K3, `katalog3_prerelease_settings.ini` in a pre-release build (alpha/beta) and `katalog_settings.ini` in a release build; for K2, `katalog_settings.ini`. In both, the file beside the executable is used when it exists (portable install), otherwise the one in the user's `.config` folder. *(K3 command-line runs do not exist yet; when wired as is, they would read `katalog_settings.ini`.)* | [Implemented] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| CLI-C1 | The command-line actions, options, parsing and output live **only** in the shared core handler (`core/commandline.h/.cpp`). K3's `main` only detects a command-line launch and delegates to it, as K2's does. There MUST NOT be a second command-line implementation in `qt_quick/` or a K3-specific copy of the handler. | [Implemented] |
| CLI-C2 | The settings-file rule of `CLI-F9` (which file name, then "beside the executable, else `.config`") is defined **once**, in a core helper, and used by both K3's start-up (`AppManager::initiateApp`) and the core command-line handler. Neither keeps its own copy of the rule. K2's window start-up (`qt_widgets/mainwindow.cpp`, same "beside the executable, else `.config`" rule with `katalog_settings.ini`) may use the same helper as a minimal K2 change. *Design note: the helper's name, signature and the way it learns which file name applies (pre-release flag, file name, or version string) are left to implementation; it is one of the two core changes approved for this page (`CLI-C8`).* | [Implemented] |
| CLI-C3 | No version number is hard-coded for `CLI-F7`. The core handler takes the version from the running application (K2: its generated version header; K3: `KATALOG_VERSION_STRING`). The literal `"2.6"` is removed from `core/commandline.cpp` (`collection->appVersion`) and from `qt_widgets/main.cpp` (`setApplicationVersion`). In K3 the application version is set **before** the command line is parsed, since `--version` is answered during parsing. | [Implemented] |
| CLI-C4 | In K3, a command-line run MUST NOT create the QML engine, `AppManager`, any window, or the K3 translator, and MUST NOT write to the settings file (in particular, the first-run language detection that K3's `main` performs before the window MUST NOT run in command-line mode). Detection and delegation happen before any of that. | [Implemented] |
| CLI-C5 | Only the actions and options **defined** by the handler trigger command-line mode (`CLI-F1`). K3 MUST NOT treat `--csv`, `--report`, `--search` or `-s` as triggers: they are in K2's detection list but are not defined options (`--report` is commented out in the parser). Whether K2's list is corrected is a separate minimal K2 fix (open question 2). | [Implemented] |
| CLI-C6 | Command-line output is English only, written with `qInfo()` / standard output and standard error as described in [Dev-CodePractice](Dev-CodePractice.md) ("Special case: commandline.cpp"), never `tr()`/`qsTr()`. This work adds **no** translatable string and changes no existing command-line output text other than the version value of `CLI-F7`. | [Implemented] |
| CLI-C7 | A command-line run opens the collection through the normal core path (`Database::initialize`), so the 3.0-cycle migration (`runMigration_3_0`) runs exactly as on a window open. This work makes **no schema change**, adds **no persisted setting** and changes no settings key; existing beta1/beta2 `katalog3_prerelease_settings.ini` files are read unchanged. | [Implemented] |
| CLI-C8 | Core changes for this page are limited to those approved by the maintainer on 2026-10-11: the shared settings-file helper (`CLI-C2`), the removal of the hard-coded version (`CLI-C3`), the update actions returning their outcome for the exit code (`CLI-F8`), the removal of the undefined `--report` check, the `Katalog` command name in the `--help` examples, and the double-free fix in `update_catalog`. Nothing else in `core/` is changed. | [Implemented] |
| CLI-C9 | Documentation: the K3 *Command Lines* page is rewritten for K3 in English, French and Czech (`Katalog` command, Flatpak and AppImage forms, version badge 3.0), and its screenshots are retaken (added to the shot list). The `k2/` copy stays as is. Text done 2026-10-11; screenshots pending. | [Planned] |

---

## Known limitations

- **Windows: no console output.** K3 (like K2) is built as a Windows GUI
  application (`WIN32_EXECUTABLE`), so it has no console: command-line output does
  not appear in the terminal. Not fixed for 3.0; the user page keeps saying the
  commands run under Linux only.
- **macOS: untested.** The executable is inside the application bundle
  (`Katalog.app/Contents/MacOS/Katalog`). It may work from there but has not been
  tested; it is not a requirement for 3.0.
- **Linux AppImage and the portable settings file.** In an AppImage the executable
  runs from the mounted image, so no settings file is ever found "beside the
  executable": the `.config` file is always used. This is the same for the window
  and the command line, so `CLI-F9` holds.

---

## Decisions (2026-10-11)

1. **K3 launcher name for the user page.** *Answered 2026-10-11:* no portable
   version is provided any more, so `Katalog.sh` is obsolete. The Linux page shows
   the AppImage and the Flatpak forms:
   `flatpak run io.github.stephanecouturier.Katalog <action> [options]`, or the
   exported `io.github.stephanecouturier.Katalog` launcher, or a shell alias.
   The command line in the Flatpak relies on `--filesystem=host` in the manifest
   (`--collection` paths, catalog folders, and the same `~/.config` settings file
   as the window); removing that permission breaks the command line.
   Verified 2026-10-11 with the installed K2 2.12 Flatpak: `list_catalogs --verbose`
   runs and reads `~/.config/katalog_settings.ini`, the same file as the window.
2. **K2 detection list.** `--csv`, `--report`, `--search` and `-s` are removed
   from K2's command-line triggers (minimal K2 fix): they are not defined options,
   so they made K2 exit instead of opening the window.
3. **`--report` read while undefined.** The check is removed; the update actions
   no longer print Qt's "option not defined" warning. A real `--report` option
   stays a backlog idea on the user page.
4. **K2 language lookup.** Out of scope, unchanged: no portable version is
   provided any more, so the settings file beside the executable is not used.
5. **Ratifying existing behaviour.** `CLI-F2` to `CLI-F6` and `CLI-F8` are
   confirmed as they work today.
6. **Exit code of a failed update.** A failed update exits with code `1`
   (added to `CLI-F8`).

---

## Manual test charter

- **CLI-F1 / CLI-C4** — K3, Linux: `<launcher> search`, `<launcher> list_catalogs`,
  `<launcher> --help`, `<launcher> --version` each print to the terminal and exit;
  no window appears. `<launcher>` alone opens the window. Repeat with K2.
- **CLI-C5** — K3: `<launcher> --csv` and `<launcher> --report` open the window
  (or are ignored), they do not enter command-line mode.
- **CLI-F7 / CLI-C3** — `--version` prints the build version (K3 `3.0.beta2` or
  current; K2 2.13), not `2.6`. Same in the `--help` header.
- **CLI-F9 / CLI-C2** — K3 pre-release build with different collections recorded
  in `katalog3_prerelease_settings.ini` and `katalog_settings.ini`:
  `list_catalogs` lists the collection of the pre-release file, the one the K3
  window opens. With a settings file beside the executable (portable), that file
  is used. K2 still reads `katalog_settings.ini`.
- **CLI-C4** — Delete or rename the K3 settings file, run `<launcher> --version`:
  no settings file is created.
- **CLI-F2 to CLI-F6, CLI-F8** — Run each action with a valid and an invalid
  argument (bad device ID, bad `--size-min`, bad date, unknown action, bad
  `--collection` path) in File and Memory collections; check output, standard
  error and the exit code (`echo $?`).
- **CLI-C7** — Run a command-line `search` on a beta1 collection and settings
  file, then open the K3 window: same collection, settings unchanged.
- **CLI-C9** — The K3 *Command Lines* page in EN, FR and CS shows the K3 launcher
  name, badge 3.0 and the new screenshots.
