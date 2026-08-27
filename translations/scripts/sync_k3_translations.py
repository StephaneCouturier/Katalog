#!/usr/bin/env python3
"""
sync_k3_translations.py

Adds K3 QML qsTr() strings to all .ts translation files.
For each K3 string that already has a *finished* translation elsewhere in the
same file, that translation is copied automatically.  "Elsewhere" means any K2
context first — K2 is the canonical owner and wins any disagreement — and then
any other K3 context.  That second half matters because moving a component out
into its own .qml file renames its Qt context, which would otherwise orphan
every translation that component already had.
Strings found nowhere are left as <translation type="unfinished"/> — lrelease
with -nounfinished skips them (they stay in English) until they are translated,
which is what fill_k3_translations.py is for.

Workflow
--------
1. Run this script (from any directory):
       python3 translations/scripts/sync_k3_translations.py

2. Compile .qm files (in K2 build directory):
       ninja translations_lrelease   # or just: ninja

3. Rebuild K3 to embed the updated .qm files:
       ninja   (in qt_quick build directory)

The script is idempotent: re-running it skips .ts files that already
contain K3 contexts.
"""

import os
import re
import xml.etree.ElementTree as ET
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
QML_DIR   = REPO_ROOT / "qt_quick"
TRANS_DIR = REPO_ROOT / "translations"


def extract_qml_strings():
    """Parse every *.qml file and return {context_name: [source_strings]}."""
    result = {}
    for qml_file in sorted(QML_DIR.glob("*.qml")):
        context = qml_file.stem          # e.g. "PageSearchForm"
        content = qml_file.read_text(encoding="utf-8")
        sources = []
        seen    = set()
        for m in re.finditer(r'qsTr\("((?:[^"\\]|\\.)*)"\)', content):
            s = m.group(1)
            if s and s not in seen:
                sources.append(s)
                seen.add(s)
        if sources:
            result[context] = sources
    return result


def get_known_translations(tree_root, k3_context_names):
    """Build source -> translation map from every *finished* entry in the file.

    Harvested in two passes so precedence is explicit:

      1. K2 (non-K3) contexts — K2 is the canonical owner of Katalog's
         translations, so it wins whenever the two disagree.
      2. K3 contexts — so a string already translated on one K3 page can reach
         a sibling page that has the same source.  Without this pass, moving a
         component into its own .qml (which renames its Qt context) silently
         orphans all of its translations, and every repeated label has to be
         translated once per page.

    Everything except `unfinished` is harvested — `unfinished` is by definition
    not a translation yet, but `vanished`/`obsolete` entries are real human
    translations whose source merely left the K2 code, and a string that moved
    from K2 to K3 sits in exactly that state.  The first pass to supply a source
    wins, hence K2 before K3.

    Copying one context's wording into another is safe here because the caller
    only ever fills entries that are still `unfinished`; a context that needs
    different wording for the same English source (Qt contexts exist precisely
    to allow that) keeps whatever finished translation it already has.
    """
    known = {}
    for harvest_k3 in (False, True):
        for ctx in tree_root.findall("context"):
            name_el = ctx.find("name")
            if name_el is None:
                continue
            if (name_el.text in k3_context_names) != harvest_k3:
                continue
            for msg in ctx.findall("message"):
                src = msg.find("source")
                tr  = msg.find("translation")
                if (src is not None and src.text
                        and tr  is not None and tr.text
                        and tr.get("type") != "unfinished"
                        and src.text not in known):
                    known[src.text] = tr.text
    return known


def _esc(text):
    return (text
            .replace("&", "&amp;")
            .replace("<", "&lt;")
            .replace(">", "&gt;"))


def build_context_block(context_name, sources, known_tr):
    """Return the XML text for a new <context> element."""
    lines = [f"<context>", f"    <name>{context_name}</name>"]
    for src in sources:
        lines.append("    <message>")
        lines.append(f"        <source>{_esc(src)}</source>")
        if src in known_tr:
            lines.append(f"        <translation>{_esc(known_tr[src])}</translation>")
        else:
            lines.append('        <translation type="unfinished"></translation>')
        lines.append("    </message>")
    lines.append("</context>")
    return "\n".join(lines)


def refresh_existing_contexts(content, k3_context_names, known_tr):
    """Fill `unfinished` translations in already-present K3 contexts from the
    known-translations map.  Targeted text replacement (only unfinished entries
    that have a match) so the rest of the file is untouched.  A finished entry is
    never overwritten.  Returns (new_content, count)."""
    count = 0

    def repl_ctx(cmatch):
        nonlocal count
        block  = cmatch.group(0)
        name_m = re.search(r"<name>(.*?)</name>", block, re.S)
        if not name_m or name_m.group(1) not in k3_context_names:
            return block

        def repl_msg(mmatch):
            nonlocal count
            msg   = mmatch.group(0)
            src_m = re.search(r"<source>(.*?)</source>", msg, re.S)
            if not src_m:
                return msg
            src_plain = (src_m.group(1)
                         .replace("&amp;", "&")
                         .replace("&lt;", "<")
                         .replace("&gt;", ">"))
            tr = known_tr.get(src_plain)
            if not tr:
                return msg
            # Finish the entry whether it is empty or already carries a
            # lupdate same-text guess (type="unfinished" with text): the
            # harvested translation is a real one and outranks the guess.
            new_msg, n = re.subn(
                r'<translation type="unfinished">.*?</translation>'
                r'|<translation type="unfinished"\s*/>',
                f"<translation>{_esc(tr)}</translation>",
                msg, flags=re.S)
            count += n
            return new_msg

        return re.sub(r"<message>.*?</message>", repl_msg, block, flags=re.S)

    new_content = re.sub(r"<context>.*?</context>", repl_ctx, content, flags=re.S)
    return new_content, count


def process_ts(ts_path, k3_strings):
    """Add missing K3 contexts AND refresh unfinished entries in existing K3
    contexts.  Returns (added_contexts, refreshed_entries); (0, 0) = untouched."""
    content = ts_path.read_text(encoding="utf-8")

    tree = ET.parse(ts_path)
    root = tree.getroot()

    existing_contexts = {
        ctx.find("name").text
        for ctx in root.findall("context")
        if ctx.find("name") is not None
    }

    k3_context_names = set(k3_strings.keys())
    known_tr   = get_known_translations(root, k3_context_names)
    new_blocks = []

    for ctx_name, sources in k3_strings.items():
        if ctx_name in existing_contexts:
            continue
        block = build_context_block(ctx_name, sources, known_tr)
        new_blocks.append(block)

    # 1. Append any brand-new K3 contexts (with K2 matches pre-filled).
    if new_blocks:
        insertion = "\n" + "\n".join(new_blocks) + "\n"
        content   = content.replace("</TS>", insertion + "</TS>")

    # 2. Refresh already-present K3 contexts: fill unfinished entries that K2
    #    a translation is now known for (fixes the former add-only limitation).
    content, refreshed = refresh_existing_contexts(content, k3_context_names, known_tr)

    if not new_blocks and refreshed == 0:
        return 0, 0

    ts_path.write_text(content, encoding="utf-8")
    return len(new_blocks), refreshed


def main():
    k3_strings = extract_qml_strings()
    total = sum(len(v) for v in k3_strings.values())
    print(f"K3 QML: {len(k3_strings)} contexts, {total} strings\n")

    updated = 0
    skipped = 0
    total_added     = 0
    total_refreshed = 0
    for ts_file in sorted(TRANS_DIR.glob("Katalog_*.ts")):
        added, refreshed = process_ts(ts_file, k3_strings)
        if added or refreshed:
            print(f"  updated : {ts_file.name}  "
                  f"(+{added} context(s), {refreshed} entry(ies) filled)")
            updated += 1
            total_added     += added
            total_refreshed += refreshed
        else:
            print(f"  skipped : {ts_file.name}  (nothing left to fill)")
            skipped += 1

    print(f"\n{updated} file(s) updated, {skipped} skipped.")
    print(f"{total_added} context(s) added, {total_refreshed} entry(ies) filled "
          f"from translations already present in the same file.")
    print("Anything still unfinished has no translation anywhere yet — "
          "use fill_k3_translations.py.")
    print()
    print("Next steps:")
    print("  1. ninja translations_lrelease   (in K2 build directory)")
    print("  2. ninja                         (in K3/qt_quick build directory)")


if __name__ == "__main__":
    main()
