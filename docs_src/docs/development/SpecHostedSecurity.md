---
id: SpecHostedSecurity
title: Hosted Security — Only Local or Private-Network Servers
description: Which database servers a Hosted collection may use, in K2 and K3
---

# HOSTED SECURITY — ONLY LOCAL OR PRIVATE-NETWORK SERVERS

![Status](https://img.shields.io/badge/Status-Approved-brightgreen) ![Implementation](https://img.shields.io/badge/Implementation-K2%20complete%2C%20K3%20planned-lightgrey) ![Verification](https://img.shields.io/badge/Retest-pending-orange)

## Context

K2 restricts which database server a Hosted collection may use. When the Hosted
settings are saved, the host name is classified as localhost, private network, or
anything else, and anything else is refused, so that collection data is not sent to a
public server. K3 had no such check: any host could be used from the Settings
*Connect* button and from the drawer's recent-collections list. The gap was recorded in
`SpecK3ParityGaps.md` on 2026-10-04.

On 2026-10-10 the maintainer decided: "K3 MUST implement the same limitation as K2". The
rows below record K2's existing behaviour as `[Implemented]` and its K3 port as
`[Planned]`. Approved by the maintainer on 2026-10-10, including the decisions on the
recent-collections entry (`HST-F5`), the K3 backstop (`HST-C4`), startup (`HST-F6`),
dialog titles and the six strings (`HST-C3`).

## Scope at a glance

| In scope | Not in scope |
|----------|--------------|
| Classifying the host name of a Hosted collection when the Hosted settings are applied: in K2 on save (apply and restart), in K3 on *Connect* in Settings › Collection and when a Hosted entry of the drawer's recent-collections list is opened. What happens for each class, and the messages shown. | Startup reconnection with the saved settings (not re-validated, `HST-F6`). There is no auto-connect option: K3's *Connect automatically on startup* checkbox, which had no K2 equivalent and was never honoured, was removed on 2026-10-10. Port, user, password and database name. Any change to the classification ranges or to `Database::validateHostname()`. |

---

## Operational requirements — *why / for whom*

| ID | Requirement | Status |
|----|-------------|--------|
| HST-O1 | When a user sets up or opens a Hosted collection, the collection data is only sent to a server on the user's own machine or on their private network. It is never sent to a public server. | K2 [Implemented] / K3 [Planned] |

## Functional requirements — *what the system does*

| ID | Requirement | Status |
|----|-------------|--------|
| HST-F1 | When the Hosted settings are applied (K2: save; K3: Settings *Connect*), leading and trailing spaces are removed from the host name. If the host name is empty, `localhost` is used and the host field shows `localhost`. | K2 [Implemented] / K3 [Planned] |
| HST-F2 | A localhost host (`localhost`, `::1`, `127.x.x.x`) is accepted with no message, and the action continues. | K2 [Implemented] / K3 [Planned] |
| HST-F3 | A private-network host (`10.x.x.x`, `172.16-31.x.x`, `192.168.x.x`) needs a Yes/No confirmation, with *No* as the default. The text is "Private network connection" + ": " + host. The informative text is "Your data will be sent to this network database." + a blank line + "Continue?". *Yes* continues. *No* cancels: nothing connects and nothing is written. K2: a message box. K3: a modal dialog whose title reuses "Private network connection". | K2 [Implemented] / K3 [Planned] |
| HST-F4 | Any other host (public address, domain name, value containing `://`) is refused and the action is cancelled: nothing connects and nothing is written. The message is "Invalid hostname." followed by "Only local or private network allowed." and "Valid:" + " localhost, 127.x.x.x, 192.168.x.x, 10.x.x.x, 172.16-31.x.x". K2: a warning message box. K3: a `Kirigami.InlineMessage` (`MessageType.Warning`) at the top of the hosted form, with a close button, hidden again when the host field is edited (`SpecValidationRules.md`). | K2 [Implemented] / K3 [Planned] |
| HST-F5 | K3: opening a Hosted entry of the drawer's recent-collections list (`SpecCollectionOpen.md` OPN-F7) applies `HST-F2` to `HST-F4` to the entry's host before connecting. For a private host, the `HST-F3` dialog comes first. For a refused host, nothing connects and nothing is written: Settings › Collection opens, the hosted form is pre-filled from the entry, and the `HST-F4` message is shown there. | K3 [Planned] |
| HST-F6 | At startup, the application reconnects with the saved Hosted settings and does not re-validate the host, the same in K2 and K3. | K2 [Implemented] / K3 [Implemented] |

## Constructional requirements — *how it is built / limits / MUST-NOTs*

| ID | Requirement | Status |
|----|-------------|--------|
| HST-C1 | The host is classified only by the existing `Database::validateHostname()` (`core/database.cpp`). No change to `core/`. K3 reaches it through the K3 UI layer (`AppManager`). | K2 [Implemented] / K3 [Planned] |
| HST-C2 | A refused host, or a private host the user declined, MUST NOT be written to the settings file or to the K3 recent-collections list. In K2 the check runs before `settings.setValue`. In K3, `AppManager::openCollectionHosted()` writes the settings, so the check runs before that write. | K2 [Implemented] / K3 [Planned] |
| HST-C3 | The messages use the existing K2 strings byte-for-byte, approved for their K3 contexts on 2026-10-10: "Invalid hostname.", "Only local or private network allowed.", "Valid:", "Private network connection", "Your data will be sent to this network database.", "Continue?". The literal list ` localhost, 127.x.x.x, 192.168.x.x, 10.x.x.x, 172.16-31.x.x` is not translated, the same as in K2. No other string is added, including for dialog titles. | K2 [Implemented] / K3 [Planned] |
| HST-C4 | K3 backstop: `AppManager::openCollectionHosted()` refuses a host classified as public or invalid before it writes any setting, so no K3 path connects to a public server. Startup does not call it, so `HST-F6` is unaffected. | K3 [Planned] |
| HST-C5 | No new settings key. Existing beta1/beta2 `katalog3_prerelease_settings.ini` files are read unchanged. A public host already saved there still connects at startup (`HST-F6`), but *Connect* and recent entries pointing to it are refused. | K3 [Planned] |

---

## Manual test charter

- **HST-F1** — Settings › Collection, Hosted form: leave the host empty, then *Connect*. The field shows `localhost` and the connection is attempted on localhost. Repeat with `  localhost  `: the spaces are removed.
- **HST-F2** — Host `127.0.0.1`, then `::1`: the connection is attempted with no message.
- **HST-F3** — Host `192.168.1.10`: the private-network dialog shows the host, and *No* is the default. Press *No*: there is no connection, and neither the settings file nor the recent list changes. Repeat and press *Yes*: the connection is attempted.
- **HST-F4** — Host `example.com`, then `8.8.8.8`, then `http://localhost`: the warning appears inline at the top of the hosted form, with nothing connected or written. Edit the host field: the warning disappears. Its close button also hides it. K2: the same hosts show the warning box and the save is cancelled.
- **HST-F5** — Put a Hosted recent entry with host `192.168.1.10` in the settings file: opening it shows the `HST-F3` dialog first. Put one with a public host: opening it shows Settings › Collection with the form pre-filled and the `HST-F4` warning, and nothing connects.
- **HST-F6** — Save a public host directly in the settings file and start the application: it reconnects without a message (same as K2).
- **HST-C2 / HST-C4 / HST-C5** — After each refusal or *No* above, the settings file and the recent list are unchanged, and no new key is present.
- **HST-C3** — Switch the interface language: all six messages are translated, and the range list stays as written.
