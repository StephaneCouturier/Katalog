# Nastavení
## Souhrn
Tato stránka popisuje všechny funkce obrazovky **Nastavení** a jak je používat.
* Správa dat
* Jazyk a téma
* O
![](/img/screen_settings_01.png)
## Správa dat
### Kolekce

Sbírka je jediná skupina zařízení a všech souvisejících informací, jako jsou statistiky.<br/>
Složka *Collection* je složka ve vašem počítači, kde jsou uložena všechna data kolekce.<br/>
Je možné mít několik sbírek.

### Datové režimy
Katalog poskytuje 2 "*Datové režimy*" nebo různé způsoby ukládání a manipulace s daty.

Poznámka 1: zatím neexistuje žádná funkce pro převod sbírky v jednom režimu do režimu druhého.

Poznámka 2: Změna režimu vyžaduje kliknutí na *Použít a restartovat*
 | Režim | Typ databáze | Ukládání dat |Soubory | Rychlost vyhledávání | Rychlost katalogizace |
 | -------| -------------------|---|---|---|---|
 | **Paměť** (výchozí)| paměť počítače | v souborech .csv oddělených záložkou (pro zařízení, statistiky atd.) a v souborech .idx (pro seznamy souborů katalogů)|Lepší pro pravidelnou synchronizaci souborů do cloudu|Nejvyšší rychlost vyhledávání, jakmile jsou katalogy v paměti (delší doba pro při prvním použití katalogu) | Mírně rychlejší|
 | **Soubor** | Soubor SQLite, nízké využití paměti | vše v souboru SQLite|všechna data se spojila do 1 souboru, který může narůst až do velikosti několika set Mb |Rychlejší pro 1. vyhledávání, pomalejší pro opakované vyhledávání ve velké sbírce|O něco pomalejší|

![](/img/settings_database-model.png)

### Režim paměti databáze {#database-memory-mode}
![](/img/screen_settings_02_memory.png)
Akce složky kolekce:
* Zadejte cestu ke složce Collection a stisknutím klávesy Enter načtěte kolekci
* Vyberte cestu ke složce Collection a načtěte kolekci
* Otevřete složku kolekce ve výchozím správci souborů systému.

Nastavení pro režim paměti
* Zálohovat: Povolí nebo zakáže (výchozí) uchování kopie katalogu před jeho aktualizací (kopie bude mít příponu .bak)
* Spuštění: Povolte nebo zakažte (výchozí) předběžné načítání naposledy použitých katalogů (poslední výběr), abyste získali rychlejší vyhledávání.
* Spuštění: Povolí nebo zakáže (výchozí) načítání naposledy otevřeného katalogu na obrazovce [Prozkoumat](Explore).


### Režim databázového souboru
![](/img/screen_settings_03_file.png)
Akce složky kolekce:
* Zadejte cestu k souboru a stisknutím klávesy Enter načtěte kolekci
* *Vybrat a otevřít databázový soubor* poskytuje způsob, jak vybrat a načíst kolekci
* *Upravit*: Otevřete databázi SQLite v editoru databáze (např. [SQLite Browser](http://sqlitebrowser.org)).
* *Nový*: Vytvořte nový soubor sbírky a načtěte jej.

## Jazyk a téma
* Vyberte jazyk aplikace.
* Vyberte motiv pro aplikaci a restartujte jej, abyste jej použili.
* Možnost použití větší velikosti ikony.
* Tlačítko pro *Otevření souboru nastavení* (místní soubor, kde jsou uloženy možnosti Katalop a poslední výběry).

### Motivy
|Téma katalogu|Kontext pro použití|
|---|---|
|Výchozí motiv|automaticky se přizpůsobí jakémukoli OS a světlým/tmavým motivům|
|Katalogová barva (světlá)|pouze pro světlá témata pracovní plochy (nevhodná pro tmavá témata pracovní plochy)|
|Katalogová barva (tmavá)|pouze pro tmavé motivy plochy (nevhodné pro světlá témata plochy)|

Příklady:
- Světlá pracovní plocha / Katalog barev (světlá)
![](/img/settings-themes-light-desktop-katalog-colors-light.png)

- Tmavá plocha / barvy katalogu (tmavé)
![](/img/settings-themes-dark-desktop-katalog-colors-dark.png)

- Tmavá plocha / výchozí motiv
![](/img/settings-themes-dark-desktop-default-theme.png)

## O
* Verze a datum aplikace.
* Možnost zkontrolovat novou verzi při spuštění.
* Tlačítko pro otevření této stránky dokumentace.
* Tlačítko pro otevření poznámek k verzi.
