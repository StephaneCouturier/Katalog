---
version: "3.0"
---
# Výsledky vyhledávání
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Shrnutí
Tato stránka popisuje stránku **Výsledky**, která se otevře vpravo od stránky [Vyhledávání](Search) při spuštění vyhledávání:
* [shrnutí](#summary-line) toho, co bylo nalezeno,
* [seznam výsledků](#results-list), řazený a procházený myší nebo klávesnicí,
* [filtry](#filters), které zúží seznam bez nového vyhledávání,
* [místní nabídka](#file-context-menu) souboru,
* [akce se všemi výsledky](#batch-process): export, kontrolní součty, metadata, koš a smazání.

![Stránka Výsledky: řádek shrnutí, seznam nalezených souborů, filtry dole](/img/searchresults_01_overview.png)

## Řádek shrnutí {#summary-line}
Horní část stránky ukazuje:
* prohledané zařízení (například *Fyzická skupina / Photo Drive*),
* počet výsledků: nalezené soubory, složky, duplikáty nebo rozdíly,
* celkovou velikost nalezených souborů, jejich nejmenší a největší velikost a rozmezí jejich dat.

Když zařízení souboru není dostupné (odpojené nebo se změnila jeho cesta), otevření souboru se odmítne a stránka zobrazí: *Zařízení není aktivní. Může být odpojeno nebo se změnila jeho cesta.*

## Seznam výsledků {#results-list}

### Sloupce
| Sloupec | Obsah |
|---------|-------|
| *Název* | Název souboru s ikonou jeho typu. |
| *Velikost* | Velikost souboru. |
| *Datum* | Datum poslední změny. |
| *Složka* | Složka souboru. |
| *Název katalogu* | Katalog, do kterého soubor patří. |
| *Typ souboru*, *Typ MIME* | Typ souboru. |
| *Šířka*, *Výška* | Rozměry, u obrázků a videí. |
| *Délka* | Délka, u zvukových a video souborů. |
| *Umělec*, *Album*, *Název* | U zvukových souborů. |
| *Kontrolní součet (SHA256)*, *Datum kontrolního součtu* | Kontrolní součet souboru a datum jeho výpočtu. |

Sloupce metadat a kontrolního součtu jsou vyplněny pouze u katalogů vytvořených s těmito volbami (viz [Vytvořit](Create)).<br/>
Když vyhledávání zobrazuje **pouze složky** (viz [Kritéria složky](Search#folder-criteria)), sloupce *Název*, *Velikost* a *Datum* se nezobrazují a každý řádek je složka.

### Práce se seznamem
* **Kliknutím** na řádek otevřete soubor ve výchozí aplikaci.
* **Kliknutím pravým tlačítkem** na řádek (nebo dlouhým stiskem) otevřete jeho [místní nabídku](#file-context-menu).
* **Kliknutím na nadpis sloupce** seřadíte podle tohoto sloupce; dalším kliknutím pořadí obrátíte. Řazení se zachová pro další vyhledávání.
* **Přetažením** čáry mezi dvěma nadpisy sloupců změníte šířku sloupce; **dvojitým kliknutím** na ni přizpůsobíte sloupec obsahu. Šířky sloupců *Název*, *Složka* a *Název katalogu* se zachovají.

### Klávesnice {#keyboard}
Po kliknutí na řádek lze seznam ovládat klávesnicí:

| Klávesa | Akce |
|---------|------|
| `↑` `↓` | Předchozí nebo další řádek. |
| `Page Up` `Page Down` | O stránku nahoru nebo dolů. |
| `Home` `End` | První nebo poslední řádek. |
| `Enter` | Otevře soubor, stejně jako kliknutí. |
| `Menu` nebo `Shift+F10` | Otevře místní nabídku řádku. |
| `←` `→` | Posune seznam vodorovně. |
| Psaní písmen | Přejde na další řádek, jehož název začíná napsaným textem. |

## Filtry {#filters}

![Filtry dole na stránce Výsledky: rychlý filtr a seznam katalogů](/img/searchresults_02_filters.png)

Lišta dole na stránce zúží seznam **bez nového spuštění vyhledávání**:
* *Hledat…*: zobrazí pouze soubory, jejichž **název** obsahuje zadaný text, průběžně při psaní, bez ohledu na velikost písmen.
* *Katalog s výsledky*: zobrazí katalogy, ve kterých byly nalezeny výsledky. Zaškrtnutím jednoho nebo více katalogů zobrazíte pouze jejich výsledky. *Všechno* zaškrtne všechny katalogy, *Žádné* volbu zruší (zobrazí se všechny výsledky). Tlačítko ukazuje zvolený katalog nebo počet zvolených katalogů.

Oba filtry platí současně a oba se vymažou, když skončí nové vyhledávání.

## Místní nabídka {#file-context-menu}

![Místní nabídka souboru ve výsledcích](/img/searchresults_03_context_menu.png)

Kliknutí pravým tlačítkem na řádek otevře nabídku, která začíná názvem souboru:

| Položka | Akce |
|---------|------|
| *Otevřít soubor* | Otevře soubor ve výchozí aplikaci. |
| *Otevřít složku* | Otevře složku souboru ve správci souborů. |
| *Prozkoumat složku* | Otevře složku na stránce [Prozkoumat](Explore), uvnitř jejího katalogu. *Zavřít* (nebo `Esc`) na stránce Prozkoumat vrátí ke stejným výsledkům. Není dostupné pro vyhledávání v připojeném disku. |
| *Zobrazit rozšířená metadata (JSON)* | Zobrazí všechna metadata uložená pro soubor jako tabulku, s tlačítkem *Kopírovat JSON*. |
| *Kopírovat název souboru bez přípony* | Zkopíruje `report_2025` z `/home/username/Documents/report_2025.pdf`. |
| *Kopírovat název souboru s příponou* | Zkopíruje `report_2025.pdf`. |
| *Kopírovat cestu ke složce* | Zkopíruje `/home/username/Documents`. |
| *Kopírovat absolutní cestu k souboru* | Zkopíruje `/home/username/Documents/report_2025.pdf`. |
| *Vypočítat kontrolní součet (SHA-256)* | Vypočítá kontrolní součet souboru a uloží ho. Zobrazeno, když soubor ještě nemá kontrolní součet. |
| *Kopírovat kontrolní součet* | Zkopíruje uložený kontrolní součet. Zobrazeno, když ho soubor má. |
| *Ověřit kontrolní součet (SHA-256)* | Znovu vypočítá kontrolní součet a porovná ho s uloženým. Pokud se liší, zobrazí se obě hodnoty s tlačítkem *Aktualizovat kontrolní součet* pro uložení nové hodnoty. |
| *Přesunout do koše* | Přesune soubor do systémového koše. |
| *Smazat soubor* | Po potvrzení soubor **trvale** smaže. Dostupné pouze tehdy, když je v [Nastavení](Settings#behavior) zapnuto *Povolit akce, které mažou soubory (bez možnosti obnovení)*. |

Když jsou výsledkem složky, položky týkající se názvu souboru se zobrazí, ale jsou neaktivní.

## Akce se všemi výsledky {#batch-process}
Seznam vedle *Exportovat* volí akci a tlačítko vedle něj (*Spusťte proces u všech výsledků*) ji použije na **všechny výsledky**:

| Akce | Popis |
|------|-------|
| *Exportovat do CSV* | Uloží výsledky do souboru CSV ve složce *Stažené soubory*; po dokončení se zobrazí jeho cesta. |
| *Exportovat do katalogu* | Uloží výsledky jako nový katalog pod virtuální zařízení *Výsledky vyhledávání* (viz [Zařízení](Devices)). |
| *Ověřit kontrolní součty* | Po potvrzení vypočítá kontrolní součty všech výsledků a porovná je s uloženými. Zpráva uvede počet souborů shodných, neshodných, nově vypočítaných a s chybou. |
| *Extrahovat metadata* | Po potvrzení přečte metadata všech výsledků a uloží je. Zpráva uvede počet souborů aktualizovaných, přeskočených a s chybou. |
| *Přesunout do koše* | Po potvrzení přesune všechny výsledky do koše. |
| *Smazat* | Po potvrzení **trvale** smaže všechny výsledky. Dostupné pouze tehdy, když je v [Nastavení](Settings#behavior) zapnuto *Povolit akce, které mažou soubory (bez možnosti obnovení)*. |

## Vývoj
* Viz backlog [vývoje Vyhledávání](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=search).
