# Struktura kódu

## Souhrn
Tato stránka poskytuje informace o tom, jak je zdrojový kód uspořádán, a o všech běžných postupech používaných k usnadnění jeho pochopení, údržby a vývoje.

## Struktura modelu a souborů
* Pro manipulaci s daty mnoho *objektů* podporuje propojení s databází: kolekce, zařízení, úložiště, katalog, vyhledávání, značka
* Každá záložka / obrazovka Katalogu je spravována v jiném souboru cpp, který patří ke kódu hlavního okna.

## Procvičování kódu
* Komentáře, komentáře, komentáře.
* proměnné: první slovo malá písmena, všechny ostatní začínající velkým písmenem: thisIsAVariableName.
* databázová pole: pro usnadnění kompatibility mezi SQLite a Postgres jsou pole pojmenována malými písmeny, slova oddělena podtržítkem: this_is_a_fied_name
![](/img/code_structure.png)
