---
id: SpecCollectionIdentity
title: Collection Identity — Naming Collections in the K3 Drawer
description: Requirements for how the K3 global drawer names the collection that is currently open and the collections listed in its Open menu, in Memory, File and Hosted database modes
---

# COLLECTION IDENTITY — NAMING COLLECTIONS IN THE K3 DRAWER

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Implementation](https://img.shields.io/badge/Implementation-planned-lightgrey) ![2.13](https://img.shields.io/badge/Version-2.13-blue)

## Context

K3 can switch collection without restarting: the user opens another collection
and the application reconnects in place. That makes *"which collection am I
looking at?"* a question the interface has to keep answering, and the global
drawer header — an icon plus a label — is where it answers it.

Two defects prompted this page. In **Memory** mode the label was blank: the name
was read from the in-memory collection folder, which is not always populated at
the moment the header is rendered. And the label only refreshed when a
collection was written to the recent list, so a connection made through a path
that does not touch the recent list left the previous collection's name on
screen.

Nothing in the requirement base covered the drawer header at all. The File-mode
and Hosted-mode behaviour below therefore **ratifies what the code already did**;
only the Memory-mode name and the refresh are changes. The user reviewed and
approved these rows, including the explicit decision to keep the host name in
the Hosted label rather than showing the database name alone.

The same blank name then turned up in the drawer's **Open** menu, which lists
the last five collections. That list has a second, worse property: its label is
frozen at write time. It is stored alongside the path when the collection is
opened and read back verbatim, so an entry once written blank stays blank for
ever — which is why the user saw names that *"were working some time ago"*
disappear and never come back. Fixing the write side alone would leave every
already-damaged entry unreadable, so the rows below require the label to be
derived when the stored one is empty, and the stored one never to be written
empty again. The user asked for this directly: *"the names should appear when
clicking open for all types of collections"*.

This page covers naming collections: the one already open, and the ones offered
in the Open menu. It does **not** cover choosing, creating or opening one — that
is `SpecCollectionOpen.md` — nor importing or updating across collections
(`SpecCollection.md`).

> **Reading the requirement IDs.** Each requirement has a permanent ID.
> IDs are never renumbered or reused; a retired requirement is marked
> `[Removed]`, not deleted. Status is one of
> `[Implemented] / [Planned] / [Backlog] / [Removed]`.

---

## Scope at a glance

**In scope:** the text shown in the K3 global drawer header for the collection
currently open, in Memory, File and Hosted modes, and when that text is
refreshed; the labels of the recent collections listed in the drawer's **Open**
menu, both as they are written and as they are read back.

**Out of scope (non-goals):** the drawer icon; how many entries the Open menu
holds, their order, and the act of opening one; the Settings page connection
status; K2, which has no equivalent drawer; and any disclosure of connection
details outside the drawer — the About block is bound by `ABT-C1` in
`SpecAbout.md` and is unaffected by these rows.

---

## Operational requirements — *why / for whom*

Goals in real use, independent of how they are built.

| ID | Requirement | Status |
|----|-------------|--------|
| CID-O1 | The user can tell at a glance which collection is currently open, in every database mode. | [Planned] |
| CID-O2 | The user can recognise each collection offered in the Open menu by name, in every database mode, without having to reopen it first to repair its entry. | [Planned] |

## Functional requirements — *what the system does*

Observable behaviour that can be triggered and watched.

| ID | Requirement | Status |
|----|-------------|--------|
| CID-F1 | In **Memory** mode the drawer header shows the collection folder's own name — the last segment of its path — independent of whether the path carries a trailing separator. | [Planned] |
| CID-F2 | In **File** mode the drawer header shows the database file name. *(Ratifies behaviour that already existed but had never been authorised.)* | [Planned] |
| CID-F3 | In **Hosted** mode the drawer header shows the host name, a `/`, then the database name. The host is deliberately kept: it is what distinguishes two servers carrying a database of the same name. It MUST NOT be reduced to the database name alone. *(Ratifies behaviour that already existed but had never been authorised.)* | [Planned] |
| CID-F4 | The drawer header updates on every successful connection and reconnection, including one made from Settings, without restarting the application. It MUST NOT depend on the collection being written to the recent-collections list: connection paths that do not touch that list must refresh the header just the same. | [Planned] |
| CID-F5 | Every entry of the drawer's **Open** menu carries a non-empty name, in all three modes: the folder's own name in Memory, the database file name in File, and host + `/` + database name in Hosted — the same values `CID-F1` to `CID-F3` define for the header. | [Planned] |
| CID-F6 | When the name stored with a recent entry is empty, it is derived on read from that entry's own stored data, falling back to the stored path when nothing better is available. Entries already written blank therefore recover by themselves, with no action from the user and no need to reopen each collection. An entry MUST NOT be shown as an empty menu line. | [Planned] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

Boundaries and implementation constraints, not user-visible behaviour.

| ID | Requirement | Status |
|----|-------------|--------|
| CID-C1 | The header renders a data value only. These rows add **no** new user-visible string, and MUST NOT introduce a label, prefix or separator that needs translating. | [Planned] |
| CID-C2 | The Memory, File and Hosted values are read from the in-memory collection fields that the database initialisation repopulates on every connect (collection folder, hosted host name, hosted database name), with the persisted settings as fallback. For Memory mode the fallback key is the flat `LastCollectionFolder` entry of the collection settings file — written by K2 (`qt_widgets/mainwindow_tab_settings.cpp`) and K3 (`qt_quick/appmanager.cpp`) with no `Settings/` group, so it MUST be read flat. When the in-memory field is unset the header falls back rather than rendering empty. | [Planned] |
| CID-C3 | The change-notification for the header is emitted from the successful-connection paths themselves, not only from the recent-collections write. This is what makes `CID-F4` hold; tying the notification to the recent list again would reintroduce the stale-name defect. | [Planned] |
| CID-C4 | The name written with a recent entry is computed where the entry is written, from the entry's own mode, path and hosted fields — not taken on trust from the caller that requested the write. A caller passing an empty or wrong name MUST NOT be able to store it. This is the write-side half of `CID-F6`: derivation on read repairs the past, this prevents the future. | [Planned] |
| CID-C5 | The header and the Open menu MUST derive a name the same way for a given mode. The two MUST NOT drift into separate naming rules — the menu naming a collection differently from the header it produces once opened is the confusion these rows exist to remove. | [Planned] |

---

## Manual test charter

Each line below is a case that must hold after any change to the drawer header
or to the Open menu, or to the connection paths that feed them. All cases
are (K3).

- **CID-F1** — Open a Memory-mode collection. The drawer header shows the
  collection folder's name, not a blank label and not the full path. Repeat with
  a folder path entered with a trailing separator: the same name appears, with
  no empty segment.
- **CID-F1 / CID-C2** — Open a Memory-mode collection, then reach the drawer
  through a path where the in-memory collection folder is not populated (for
  instance immediately at startup against the last-used collection). The name
  still appears, taken from `LastCollectionFolder`.
- **CID-F2** — Open a File-mode collection. The header shows the `.db` file
  name, without its folder path.
- **CID-F3** — Open a Hosted collection. The header shows `host/database`. Open
  a second Hosted collection with the same database name on a different host:
  the two are distinguishable.
- **CID-F4** — With a collection open, switch to another collection of a
  *different* mode from Settings. The header text and icon change immediately,
  with no restart. Repeat switching between two collections of the *same* mode.
- **CID-F4 / CID-C3** — Reconnect through a path that does not write to the
  recent-collections list (saving the connection settings from the Settings
  page). The header still updates; it does not keep the previous collection's
  name.
- **CID-F5** — Open a collection of each mode in turn, then open the drawer's
  **Open** menu. All three appear by name: the Memory folder name, the File
  `.db` name, and `host/database` for the Hosted one. No line is blank.
- **CID-F6** — Blank the stored name of a recent entry in the collection
  settings file, restart, and open the **Open** menu. The entry shows its
  derived name, not an empty line, without that collection having been reopened.
  Repeat for one entry of each mode.
- **CID-F6** — Blank both the stored name and the hosted fields of an entry.
  The menu line shows the stored path rather than nothing.
- **CID-C4** — Open a Memory-mode collection whose folder path ends with a
  separator. Inspect the collection settings file: the recent entry's stored
  name is the folder name, not empty.
- **CID-C5** — Open each collection from the **Open** menu in turn. The drawer
  header then shows exactly the text the menu line showed.
- **CID-C1** — Run `ninja translations_lupdate`. No new untranslated string
  appears for the drawer header or the Open menu.
