#!/usr/bin/env python3
"""One-off: mark the 8 Search combo strings as finished.

lupdate's same-text heuristic copied the K2 (MainWindow) translations into the
PageSearchForm context but left them type="unfinished", so lrelease -nounfinished
skips them. These sources are byte-identical to their finished K2 counterparts,
so promoting the heuristic text to finished is correct. Scoped strictly to the
PageSearchForm context and to these 8 sources.
"""
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TRANS = ROOT / "translations"
COMBO = {
    "All Words", "Exact Phrase", "Begins With", "Any Word", "Regex",
    "File names only", "File names or Folder paths", "Folder path only",
}

ctx_re = re.compile(r"(<context>\s*<name>PageSearchForm</name>.*?</context>)", re.S)

def finish_in_context(block):
    count = 0
    for src in COMBO:
        pat = re.compile(
            r"(<source>" + re.escape(src) + r"</source>\s*)"
            r"<translation type=\"unfinished\">([^<][^<]*?)</translation>")
        def repl(m):
            nonlocal count
            count += 1
            return m.group(1) + "<translation>" + m.group(2) + "</translation>"
        block = pat.sub(repl, block)
    return block, count

total = 0
for ts in sorted(TRANS.glob("Katalog_*.ts")):
    text = ts.read_text(encoding="utf-8")
    m = ctx_re.search(text)
    if not m:
        continue
    new_block, n = finish_in_context(m.group(1))
    if n:
        text = text[:m.start(1)] + new_block + text[m.end(1):]
        ts.write_text(text, encoding="utf-8")
        total += n
    print(f"{ts.name}: finished {n}")
print("Total finished:", total)
