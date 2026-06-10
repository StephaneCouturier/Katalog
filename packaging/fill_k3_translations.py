#!/usr/bin/env python3
"""
fill_k3_translations.py

AI batch-fill of the *unfinished* K3 (QML) translation strings — the strings
that have no K2 match (new features or reworded text) and therefore render in
English. See docs_src/docs/SpecLanguages.md.

This complements sync_k3_translations.py (which only *matches* existing K2
translations). This script handles the *fill* step for everything left over.

Two modes:

  1. Extract a template of all unfinished K3 strings, one entry per language:
         python3 packaging/fill_k3_translations.py --extract
     -> writes packaging/k3_unfinished.json  (values empty, ready to fill)
        and prints a per-language / per-context count summary.

  2. Apply a filled template back into the .ts files:
         python3 packaging/fill_k3_translations.py --apply
     -> for every non-empty translation, replaces the matching
        <translation type="unfinished"></translation> inside the relevant K3
        context with a finished <translation>…</translation>.

Then compile & embed (K2 build dir, then K3 build dir):
       ninja translations_lrelease
       ninja translations_copy        # if your flow copies .qm to source
       ninja                          # qt_quick build dir

Safety:
  * Only touches messages whose translation is type="unfinished".
  * Only inside contexts that are K3 QML files (qt_quick/*.qml stems).
  * en_US is skipped (English source = the displayed text already).
  * Edits are targeted text replacements — the rest of each .ts is untouched.
"""

import argparse
import json
import re
import xml.etree.ElementTree as ET
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
QML_DIR   = REPO_ROOT / "qt_quick"
TRANS_DIR = REPO_ROOT / "translations"
JSON_PATH = REPO_ROOT / "packaging" / "k3_unfinished.json"

SKIP_LANGS = {"en_US"}  # English source already equals displayed text


def k3_contexts():
    """Context names that belong to K3 = stems of qt_quick/*.qml."""
    return {p.stem for p in QML_DIR.glob("*.qml")}


def lang_of(ts_path):
    # Katalog_fr_FR.ts -> fr_FR
    return ts_path.stem.replace("Katalog_", "", 1)


def _esc(text):
    return (text.replace("&", "&amp;")
                .replace("<", "&lt;")
                .replace(">", "&gt;"))


def extract():
    contexts = k3_contexts()
    template = {}      # { lang: { source: "" } }
    summary  = {}      # { lang: { context: count } }

    for ts_path in sorted(TRANS_DIR.glob("Katalog_*.ts")):
        lang = lang_of(ts_path)
        if lang in SKIP_LANGS:
            continue
        root = ET.parse(ts_path).getroot()
        per_lang = {}
        per_ctx  = {}
        for ctx in root.findall("context"):
            name_el = ctx.find("name")
            if name_el is None or name_el.text not in contexts:
                continue
            for msg in ctx.findall("message"):
                src = msg.find("source")
                tr  = msg.find("translation")
                if src is None or not src.text:
                    continue
                if tr is not None and tr.get("type") == "unfinished":
                    per_lang[src.text] = ""
                    per_ctx[name_el.text] = per_ctx.get(name_el.text, 0) + 1
        if per_lang:
            template[lang] = per_lang
            summary[lang]  = per_ctx

    JSON_PATH.write_text(json.dumps(template, ensure_ascii=False, indent=2),
                         encoding="utf-8")

    print(f"Wrote {JSON_PATH.relative_to(REPO_ROOT)}")
    print(f"Languages with unfinished K3 strings: {len(template)}")
    total = sum(len(v) for v in template.values())
    print(f"Total entries to fill (across languages): {total}\n")
    # Per-context totals (union of sources across languages, for a sense of size)
    all_ctx = {}
    for lang, per_ctx in summary.items():
        for c, n in per_ctx.items():
            all_ctx[c] = max(all_ctx.get(c, 0), n)
    print("Unfinished strings per K3 context (max across languages):")
    for c in sorted(all_ctx, key=lambda x: -all_ctx[x]):
        print(f"  {c:32s} {all_ctx[c]}")


def _apply_to_text(content, contexts, src_to_tr):
    """Replace unfinished translations inside K3 contexts only. Returns (text, n)."""
    count = 0

    def replace_context(cmatch):
        nonlocal count
        block = cmatch.group(0)
        name_m = re.search(r"<name>(.*?)</name>", block, re.S)
        if not name_m or name_m.group(1) not in contexts:
            return block

        def replace_msg(mmatch):
            nonlocal count
            msg = mmatch.group(0)
            src_m = re.search(r"<source>(.*?)</source>", msg, re.S)
            if not src_m:
                return msg
            # Unescape the source to look it up in the (plain) JSON keys
            src_plain = (src_m.group(1)
                         .replace("&amp;", "&")
                         .replace("&lt;", "<")
                         .replace("&gt;", ">"))
            tr = src_to_tr.get(src_plain)
            if not tr:
                return msg
            new_msg, n = re.subn(
                r'<translation type="unfinished">\s*</translation>',
                f"<translation>{_esc(tr)}</translation>",
                msg)
            count += n
            return new_msg

        return re.sub(r"<message>.*?</message>", replace_msg, block, flags=re.S)

    new_content = re.sub(r"<context>.*?</context>", replace_context,
                         content, flags=re.S)
    return new_content, count


def apply():
    if not JSON_PATH.exists():
        print(f"ERROR: {JSON_PATH} not found. Run --extract first and fill it.")
        return
    data = json.loads(JSON_PATH.read_text(encoding="utf-8"))
    contexts = k3_contexts()

    total = 0
    for ts_path in sorted(TRANS_DIR.glob("Katalog_*.ts")):
        lang = lang_of(ts_path)
        src_to_tr = {s: t for s, t in data.get(lang, {}).items() if t}
        if not src_to_tr:
            continue
        content = ts_path.read_text(encoding="utf-8")
        new_content, n = _apply_to_text(content, contexts, src_to_tr)
        if n:
            ts_path.write_text(new_content, encoding="utf-8")
            print(f"  {ts_path.name}: filled {n}")
            total += n
    print(f"\nFilled {total} translation(s).")
    print("Next: ninja translations_lrelease -> translations_copy -> rebuild K3")


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    g = ap.add_mutually_exclusive_group(required=True)
    g.add_argument("--extract", action="store_true",
                   help="dump unfinished K3 strings to packaging/k3_unfinished.json")
    g.add_argument("--apply", action="store_true",
                   help="write filled translations from the json back into .ts files")
    args = ap.parse_args()
    if args.extract:
        extract()
    else:
        apply()


if __name__ == "__main__":
    main()
