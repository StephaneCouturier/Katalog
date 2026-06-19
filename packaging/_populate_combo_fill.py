#!/usr/bin/env python3
"""One-off: populate k3_unfinished.json for the Search-page combo strings.

The 8 combo strings already have verbatim translations elsewhere in each .ts
(K2 MainWindow context); copy those so K3 stays byte-identical with K2.
The 2 genuinely-new strings (Now, Select a date) get explicit translations.
"""
import json, re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TRANS = ROOT / "translations"
JSON = ROOT / "packaging" / "k3_unfinished.json"

COMBO = [
    "All Words", "Exact Phrase", "Begins With", "Any Word", "Regex",
    "File names only", "File names or Folder paths", "Folder path only",
]

NEW = {
    "Now": {
        "fr_FR":"Maintenant","cs_CZ":"Nyní","de_DE":"Jetzt","es_ES":"Ahora",
        "it_IT":"Ora","pt_PT":"Agora","ja_JP":"今","nl_NL":"Nu","ro_RO":"Acum",
        "pl_PL":"Teraz","hi_IN":"अभी","zh_CN":"现在","da_DK":"Nu","hu_HU":"Most",
        "sv_SE":"Nu","nb_NO":"Nå","sk_SK":"Teraz","fi_FI":"Nyt","si_SI":"Zdaj",
        "sr_RS":"Сада","bg_BG":"Сега","el_GR":"Τώρα","et_EE":"Nüüd","lt_LT":"Dabar",
        "lv_LV":"Tagad","uk_UA":"Зараз","hr_HR":"Sada","id_ID":"Sekarang",
    },
    "Select a date": {
        "fr_FR":"Sélectionner une date","cs_CZ":"Vyberte datum","de_DE":"Datum auswählen",
        "es_ES":"Seleccionar una fecha","it_IT":"Seleziona una data","pt_PT":"Selecionar uma data",
        "ja_JP":"日付を選択","nl_NL":"Selecteer een datum","ro_RO":"Selectați o dată",
        "pl_PL":"Wybierz datę","hi_IN":"एक तिथि चुनें","zh_CN":"选择日期","da_DK":"Vælg en dato",
        "hu_HU":"Válasszon dátumot","sv_SE":"Välj ett datum","nb_NO":"Velg en dato",
        "sk_SK":"Vyberte dátum","fi_FI":"Valitse päivämäärä","si_SI":"Izberite datum",
        "sr_RS":"Изаберите датум","bg_BG":"Изберете дата","el_GR":"Επιλέξτε ημερομηνία",
        "et_EE":"Valige kuupäev","lt_LT":"Pasirinkite datą","lv_LV":"Atlasiet datumu",
        "uk_UA":"Виберіть дату","hr_HR":"Odaberite datum","id_ID":"Pilih tanggal",
    },
}

def finished_translation(ts_text, source):
    """Return the first FINISHED translation for `source` in the .ts text."""
    for m in re.finditer(
        r"<source>" + re.escape(source) + r"</source>\s*"
        r"(?:<comment>.*?</comment>\s*)?"
        r"<translation([^>]*)>(.*?)</translation>", ts_text, re.S):
        attrs, body = m.group(1), m.group(2)
        if "unfinished" not in attrs and body.strip():
            return body
    return None

data = json.loads(JSON.read_text(encoding="utf-8"))
report = {}
for lang, entries in data.items():
    if lang == "en_US":
        continue
    ts_text = (TRANS / f"Katalog_{lang}.ts").read_text(encoding="utf-8")
    filled = 0
    for s in COMBO:
        if s in entries:
            t = finished_translation(ts_text, s)
            if t:
                entries[s] = t
                filled += 1
    for s, table in NEW.items():
        if s in entries and lang in table:
            entries[s] = table[lang]
            filled += 1
    report[lang] = filled

JSON.write_text(json.dumps(data, ensure_ascii=False, indent=1), encoding="utf-8")
print("Populated languages:", len(report))
print("Sample fr_FR filled count:", report.get("fr_FR"))
miss = [l for l,c in report.items() if c < 10]
print("Languages with <10 filled:", miss)
