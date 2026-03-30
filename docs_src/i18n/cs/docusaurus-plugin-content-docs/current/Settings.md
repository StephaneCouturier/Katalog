---
version: "2.11"
---
# Nastavení
![2.11](https://img.shields.io/badge/Version-2.11-blue)

## Shrnutí
Tato stránka popisuje všechny funkce obrazovky **Nastavení** a jak je používat.
* Správa dat a kolekcí
* Jazyk a téma
* O aplikaci

![Přehled obrazovky Nastavení](/img/screen_settings_01.png)

## Správa dat a kolekcí

### Kolekce {#collection}

Kolekce je jediná skupina zařízení a všech souvisejících informací, jako jsou statistiky.<br/>
Složka *Kolekce* je složka ve vašem počítači, kde jsou uložena všechna data kolekce.<br/>
Je možné mít více kolekcí.

### Datové režimy
Katalog poskytuje 3 "*Datové režimy*" nebo různé způsoby ukládání a práce s daty.

Poznámka: Změna režimu vyžaduje kliknutí na *Použít a restartovat*.

| Režim | Typ databáze | Ukládání dat | Soubory | Rychlost vyhledávání | Rychlost katalogizace |
| -------| -------------------|---|---|---|---|
| **Paměť** (výchozí) | paměť počítače | v souborech .csv oddělených tabulátorem (pro zařízení, statistiky atd.) a v souborech .idx (pro seznamy souborů katalogů) | Lepší pro pravidelnou synchronizaci souborů do cloudu | Nejvyšší rychlost vyhledávání, jakmile jsou katalogy v paměti (delší čas při prvním použití katalogu) | Mírně rychlejší |
| **Soubor** | Soubor SQLite, nízké využití paměti | vše v souboru SQLite | všechna data v jednom souboru, který může narůst na několik set MB | Rychlejší pro první vyhledávání, pomalejší pro opakované vyhledávání ve velké kolekci | Mírně pomalejší |
| **Hostovaný** | Server MySQL/MariaDB | všechna data uložena na hostovaném databázovém serveru | Data centralizována na serveru, přístupná z více počítačů v síti | Výkon dotazů na straně serveru, vhodné pro velké kolekce | Mírně pomalejší (síťová režie) |

![Diagram datových režimů](/img/settings_database-model.png)

### Paměťový režim databáze {#database-memory-mode}
![Nastavení paměťového režimu zobrazující cestu ke složce kolekce a související možnosti](/img/screen_settings_02_memory.png)

Akce složky kolekce:
* Zadat cestu ke složce Kolekce a stisknutím klávesy Enter načíst kolekci
* Vybrat cestu ke složce Kolekce a načíst kolekci
* Otevřít složku kolekce ve výchozím správci souborů systému
* *Exportovat do souboru SQLite*: exportovat kolekci do jediného souboru SQLite, pro přechod na režim *Soubor*

Nastavení pro paměťový režim:
* *Záloha*: povolit nebo zakázat (výchozí) uchování kopie katalogu před jeho aktualizací (kopie bude mít příponu `.bak`)
* *Spuštění*: povolit nebo zakázat (výchozí) předběžné načítání naposledy použitých katalogů pro rychlejší vyhledávání
* *Spuštění*: povolit nebo zakázat (výchozí) načítání naposledy otevřeného katalogu na obrazovce [Prozkoumat](Explore)

### Režim databázového souboru
![Nastavení souborového režimu zobrazující cestu k databázovému souboru a související možnosti](/img/screen_settings_03_file.png)

Akce souboru kolekce:
* Zadat cestu k souboru a stisknutím klávesy Enter načíst kolekci
* *Vybrat a otevřít databázový soubor*: vybrat a načíst existující soubor kolekce
* *Upravit*: Otevřít databázi SQLite v editoru databáze (např. [SQLite Browser](http://sqlitebrowser.org))
* *Nový*: Vytvořit nový soubor kolekce a načíst jej
* *Exportovat do paměťového režimu (csv)*: exportovat kolekci do souborů CSV a indexů, pro přechod na režim *Paměť*

### Hostovaný režim databáze
![Nastavení hostovaného režimu zobrazující pole pro připojení k serveru](/img/screen_settings_05_hosted.png)

Data kolekce jsou uložena v databázi hostované na lokálním nebo síťovém serveru (MySQL/MariaDB).

Nastavení připojení:
* **Název hostitele** — název hostitele nebo IP adresa databázového serveru (výchozí: `localhost`)
* **Název databáze** — název databáze na serveru
* **Port** — číslo portu (výchozí: `3306` pro MySQL/MariaDB)
* **Uživatelské jméno** — uživatelské jméno databáze
* **Heslo** — heslo databáze

Vyplňte všechna pole a klikněte na *Použít a restartovat* pro připojení.

#### Export
Hostovanou kolekci lze exportovat do lokálního formátu pro offline použití nebo sdílení:
* *Exportovat do souboru SQLite*: exportovat celou hostovanou kolekci do lokálního souboru SQLite (režim *Soubor*)
* *Exportovat do paměťového režimu (csv)*: exportovat kolekci do souborů CSV a indexů (režim *Paměť*)

#### Zabezpečení
* Jsou přijímány pouze **lokální** (localhost, 127.x.x.x) a **privátní síťové** (192.168.x.x, 10.x.x.x, 172.16-31.x.x) názvy hostitelů. Veřejné IP adresy a názvy domén jsou odmítnuty.
* Při připojení k privátní síťové adrese se zobrazí potvrzovací dialog.

#### Předpoklady
* Musí být spuštěn a přístupný server MySQL/MariaDB.
* Databáze musí na serveru již existovat (Katalog automaticky vytvoří potřebné tabulky).
* Musí být nainstalován odpovídající ovladač Qt SQL (`QMYSQL` pro MySQL/MariaDB).

### Přehled exportů kolekcí

Každý režim lze exportovat do jiného formátu. Exportované kolekce lze poté přímo otevřít v cílovém režimu nebo je použít jako zdroj pro import zařízení do jiné kolekce (viz [Import a aktualizace](#import-update)).

| Aktuální režim | Export do souboru SQLite | Export do paměťového režimu |
|---|---|---|
| **Paměť** | ✅ | — |
| **Soubor** | — | ✅ |
| **Hostovaný** | ✅ | ✅ |

> Import a aktualizace z hostované kolekce vyžadují mezikrok exportu: nejprve exportujte do režimu Soubor nebo Paměť, poté použijte exportovanou kolekci jako zdroj importu.

### Import a aktualizace {#import-update}

<!-- screenshot: screen_import_01.png -->

Katalog umožňuje importovat zařízení z jiné kolekce nebo obnovit obsah katalogů dříve importovaných zařízení při změně zdroje.

**Zdroj a cíl**

* **Zdrojová** kolekce je existující kolekce Katalog otevřená pouze pro čtení — nikdy není změněna.
* **Cílová** kolekce je aktuálně aktivní kolekce, do které jsou přidána zařízení a katalogy.

**Co se importuje**

Jsou přenesena zařízení, jejich katalogy, indexy souborů katalogů, statistiky, zálohovací vazby a nastavení vyloučených složek. Struktura složek zdroje je v cíli zachována vložením nezbytných nadřazených úrovní jako kontejnerových zařízení. Pokud název zařízení nebo katalogu v cíli již existuje, je automaticky přejmenován (například `Moje Jednotka (2)`), aby nedošlo ke konfliktům.

**Operace**

| Operace | Popis |
|---|---|
| *Importovat vybrané zařízení* | Importuje jedno zařízení a veškerý jeho obsah ze zdroje do cíle. Vybrat zdrojovou kolekci, zvolit zařízení ve stromu zdroje a kliknout na *Importovat vybrané zařízení*. Výběr kořene kolekce importuje všechna zařízení a zároveň zahrnuje štítky. |
| *Aktualizovat vybrané zařízení* | Obnoví obsah katalogů dříve importovaného zařízení nejnovějšími daty ze zdroje. Zdroj je znovu otevřen automaticky — není třeba ho znovu vyhledávat. Kontejnerová zařízení v cílové hierarchii nejsou dotčena. |

:::note
*Aktualizovat vybrané zařízení* je aktivní pouze tehdy, když vybrané zařízení (nebo některý z jeho potomků) bylo dříve importováno a stále má platný odkaz na zdrojovou kolekci.
:::

**Formát zdrojové kolekce**

| Formát zdroje | Jak otevřít |
|---|---|
| **Paměť** (složka CSV) | Vybrat složku kolekce |
| **Soubor** (soubor SQLite `.db`) | Vybrat soubor `.db` |
| **Hostovaný** (MySQL/MariaDB) | Nejprve exportovat do režimu *Soubor* nebo *Paměť* (viz [export hostovaného režimu](#export)), poté použít exportovanou kolekci jako zdroj |

## Jazyk a téma
* Vybrat jazyk aplikace.
* Vybrat motiv pro aplikaci a restartovat pro použití.
* Možnost použití větší velikosti ikon.
* Možnost povolit řazení souborů s rozlišením velkých a malých písmen.
* Tlačítko pro *Otevření souboru nastavení* (lokální soubor, kde jsou uloženy možnosti Katalog a poslední výběry).

### Motivy
| Motiv Katalog | Kontext pro použití |
|---|---|
| Výchozí motiv | automaticky se přizpůsobí jakémukoli OS a světlým/tmavým motivům |
| Katalog Color (světlý) | pouze pro světlá témata pracovní plochy (nevhodné pro tmavá témata) |
| Katalog Color (tmavý) | pouze pro tmavá témata pracovní plochy (nevhodné pro světlá témata) |

Příklady:

- Světlá pracovní plocha / Katalog Color (světlý)
![Světlá pracovní plocha s motivem Katalog Color světlý](/img/settings-themes-light-desktop-katalog-colors-light.png)

- Tmavá pracovní plocha / Katalog Color (tmavý)
![Tmavá pracovní plocha s motivem Katalog Color tmavý](/img/settings-themes-dark-desktop-katalog-colors-dark.png)

- Tmavá pracovní plocha / Výchozí motiv
![Tmavá pracovní plocha s výchozím motivem](/img/settings-themes-dark-desktop-default-theme.png)

## O aplikaci
* Verze a datum aplikace.
* Možnost zkontrolovat novou verzi při spuštění.
* Tlačítko pro otevření tohoto webu s dokumentací.
* Tlačítko pro otevření poznámek k vydání.
