---
version: "2.10"
---
# Nastavení
![2.10](https://img.shields.io/badge/Version-2.10-blue)

## Shrnutí
Tato stránka popisuje všechny funkce obrazovky **Nastavení** a jak je používat.
* Správa dat
* Jazyk a téma
* O aplikaci

![Přehled obrazovky Nastavení](/img/screen_settings_01.png)

## Správa dat

### Kolekce {#collection}

Kolekce je jediná skupina zařízení a všech souvisejících informací, jako jsou statistiky.<br/>
Složka *Kolekce* je složka ve vašem počítači, kde jsou uložena všechna data kolekce.<br/>
Je možné mít více kolekcí.

### Datové režimy
Katalog poskytuje 3 "*Datové režimy*" nebo různé způsoby ukládání a práce s daty.

Poznámka 1: zatím neexistuje žádná funkce pro převod kolekce z jednoho režimu do jiného.

Poznámka 2: Změna režimu vyžaduje kliknutí na *Použít a restartovat*

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
* Exportovat kolekci do souboru databáze SQLite, pro přechod na režim *Soubor*

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
* *Exportovat do paměťového režimu*: exportovat kolekci do souborů CSV a indexů, pro přechod na režim *Paměť*

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

#### Zabezpečení
* Jsou přijímány pouze **lokální** (localhost, 127.x.x.x) a **privátní síťové** (192.168.x.x, 10.x.x.x, 172.16-31.x.x) názvy hostitelů. Veřejné IP adresy a názvy domén jsou odmítnuty.
* Při připojení k privátní síťové adrese se zobrazí potvrzovací dialog.

#### Předpoklady
* Musí být spuštěn a přístupný server MySQL/MariaDB.
* Databáze musí na serveru již existovat (Katalog automaticky vytvoří potřebné tabulky).
* Musí být nainstalován odpovídající ovladač Qt SQL (`QMYSQL` pro MySQL/MariaDB).

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
