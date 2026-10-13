---
version: "3.0"
---
# Zařízení
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Shrnutí
Tato stránka popisuje stránku **Zařízení**, kde se zobrazují a spravují zařízení kolekce:
* [model zařízení](#model): katalogy, úložiště, virtuální zařízení a skupiny,
* [zobrazení](#views): všechna zařízení, úložiště nebo katalogy, jako karty nebo tabulka,
* [akce stránky](#page-actions): aktualizace všech aktivních katalogů, snímek, nová skupina nebo úložiště,
* [místní nabídka](#context-menu) zařízení: aktualizace, prozkoumání, úprava, kontrolní součty, rozdělení, přiřazení, smazání.

Úprava zařízení (název, cesta, možnosti katalogu, údaje o úložišti) je popsána na stránce [Úprava zařízení](DevicesEdit).

![Stránka Zařízení: všechna zařízení jako strom karet](/img/devices_01_overview.png)

## Model zařízení {#model}

### Definice
* **Katalog** je rejstřík souborů jednoho adresáře: jeho **zdrojové cesty**. Katalogy se vytvářejí na stránce [Vytvořit](Create).
* **Úložné** zařízení je fyzický disk nebo jiné médium, na kterém jsou uloženy soubory. Má cestu, kde je připojeno, a fyzický prostor.
* **Virtuální** zařízení seskupuje jiná zařízení. Nemá vlastní data a sčítá údaje zařízení pod sebou.
* **Skupina** je virtuální zařízení na vrcholu stromu:
    * **Fyzická skupina** je jediná a vyhrazená pro fyzická zařízení (počítač, disk, telefon…) a jejich katalogy,
    * každá jiná je **Virtuální skupina**, ke které se přiřazují existující katalogy, například všechny katalogy fotografií do skupiny *Photos*.

Každé zařízení může mít **komentář**: volnou poznámku zobrazenou vedle jeho názvu.

### Hierarchie

![Hierarchie zařízení: skupiny, virtuální zařízení, úložná zařízení a katalogy](/img/devices_model.png)

Katalog patří k jednomu úložišti ve Fyzické skupině. Může být také přiřazen k virtuálním zařízením Virtuálních skupin: to umožňuje vyhledávat pouze v těchto katalozích a vidět jejich součty.

### Dostupná zařízení {#available}
Zařízení je **dostupné**, když je jeho cesta dosažitelná: disk je připojen a namountován. Jeho ikona je barevná; nedostupné zařízení má šedou ikonu.<br/>
V katalozích nedostupných zařízení lze stále vyhledávat a prozkoumávat je; zařízení potřebují pouze akce, které čtou disk (aktualizace, otevření, kontrolní součty…).

## Zobrazení {#views}
Lišta v horní části stránky volí, co se zobrazuje a jak.

| Tlačítko | Účinek |
|----------|--------|
| *Všechna zařízení* | Celý strom: Fyzická skupina s úložišti a katalogy a Virtuální skupiny s přiřazenými zařízeními. |
| *Úložiště* | Pouze úložná zařízení. |
| *Katalogy* | Pouze katalogy. |
| *Karty* | Každé zařízení je karta s názvem, komentářem, údaji a nabídkou akcí. |
| *Tabulka* | Každé zařízení je řádek, s jedním sloupcem pro každou hodnotu. |
| *Celá tabulka* | V tabulce zobrazí všechny sloupce, včetně interních ID a méně používaných údajů. |
| *Filtrovat z výběru* | Zobrazí pouze zařízení vybrané na stránce [Výběr](Selection) a zařízení pod ním. |

Volby se ukládají pro každou kolekci.

![Stránka Zařízení: seznam úložišť jako tabulka s lištou součtů](/img/devices_02_table.png)

### Součty
Se zobrazením *Úložiště* nebo *Katalogy* ukazuje lišta pod tlačítky součty zobrazených zařízení:
* *Katalogy*: počet katalogů, *Celková velikost souboru*, *Celkový počet souborů*,
* *Úložiště*: počet zařízení, *Celkové místo*, místo *Použitý* a *Volný*.

### Karty
Karta ukazuje ikonu a název zařízení, jeho komentář a řádek s typem, počtem souborů a velikostí, u úložiště využitým, volným a celkovým místem a datem poslední aktualizace.<br/>
Ve zobrazení *Všechna zařízení* jsou karty odsazené podle stromu a šipka vpravo na kartě sbalí nebo rozbalí zařízení pod ní.<br/>
Tlačítko nabídky vpravo na kartě, kliknutí pravým tlačítkem nebo dlouhý stisk otevře [místní nabídku](#context-menu).

### Tabulka
* **Kliknutím na nadpis sloupce** seřadíte podle tohoto sloupce; dalším kliknutím pořadí obrátíte. Čísla, velikosti a data se řadí podle hodnoty.
* **Přetažením** čáry mezi dvěma nadpisy sloupců změníte šířku sloupce; **dvojitým kliknutím** na ni přizpůsobíte sloupec obsahu. Šířky sloupců *Název* a *Cesta* se zachovají, pro každé zobrazení zvlášť.
* Ve zobrazení *Všechna zařízení* šipka před názvem sbalí nebo rozbalí zařízení pod ním.
* **Kliknutím pravým tlačítkem** na řádek (nebo dlouhým stiskem) otevřete stejnou [místní nabídku](#context-menu) jako na kartě.
* Klávesnice funguje jako ve [výsledcích vyhledávání](SearchResults#keyboard). Ve zobrazení *Všechna zařízení* `←` a `→` také sbalují a rozbalují vybraný řádek.

Hlavní sloupce:

| Zobrazení | Sloupce |
|-----------|---------|
| *Všechna zařízení* | *Název*, *Počet souborů*, *Celková velikost*, *Použité místo*, *Volný prostor*, *Celkový prostor*, *Datum aktualizováno*, *Cesta*. |
| *Úložiště* | *Název*, *ID úložiště*, *Počet souborů*, *Celková velikost*, *Použité místo*, *Volný prostor*, *Celkový prostor*, *Datum aktualizováno*, *Cesta*, *Označení*, *Souborový systém*. |
| *Katalogy* | *Název*, *Počet souborů*, *Celková velikost*, *Datum aktualizováno*, *Cesta*, *Typ souboru*, *Skrytý*, *Metadata*, *Kontrolní součet*, *Rodičovské úložiště*. |

*Celá tabulka* přidává podle zobrazení: *Typ zařízení*, *Aktivní*, ID, *Typ*, *Značka*, *Model*, *Sériové číslo*, *Datum výroby*, *Komentář 1* až *Komentář 3*, *Verze aplikace* a v režimu Paměť *Datum načtení* a *Cesta k souboru*.

## Akce stránky {#page-actions}

| Akce | Popis |
|------|-------|
| *Všechny aktivní* | Aktualizuje všechny **dostupné** katalogy pod zařízením vybraným na stránce [Výběr](Selection) (celou kolekci, když je vybráno *Všechno*). Nejprve se zobrazí otázka, zda zobrazit shrnutí pro každý katalog (*Ano*), pouze celkový výsledek (*Ne*), nebo akci zrušit. |
| *Zastavit* | Zastaví probíhající aktualizaci. |
| *Snímek* | Zaznamená aktuální údaje celé kolekce (soubory a velikost katalogů, volné a celkové místo úložišť) pro [Statistiku](Statistics). Shrnutí ukáže nové součty a co se změnilo od předchozího snímku. |
| *Vložit virtuální skupinu* | Vytvoří novou Virtuální skupinu a otevře ji v [Úpravě zařízení](DevicesEdit). |
| *Přidat úložiště* | Vytvoří nové úložné zařízení ve Fyzické skupině a otevře ho v [Úpravě zařízení](DevicesEdit). |
| *Zavřít* | Zavře stránku (také `Esc`). |

Aktualizace vyžádané během jiné probíhající aktualizace se neztratí: čekají ve frontě zobrazené v panelu činnosti (viz [Rozhraní](Interface)).

## Místní nabídka {#context-menu}

![Místní nabídka karty katalogu](/img/devices_03_context_menu.png)

Nabídka začíná názvem zařízení. Její položky závisí na typu zařízení:

| Položka | Pro | Akce |
|---------|-----|------|
| *Aktualizovat* | Úložiště, Virtuální, dostupný Katalog | Znovu načte soubory ze zařízení a aktualizuje katalog. U úložného nebo virtuálního zařízení aktualizuje všechny katalogy pod ním. |
| *Prozkoumat* | Katalog | Otevře katalog na stránce [Prozkoumat](Explore). |
| *Upravit* | Všechna | Otevře zařízení v [Úpravě zařízení](DevicesEdit). |
| *Otevřít složku* | Zařízení s cestou | Otevře cestu ve správci souborů. |
| *Filelight* | Dostupné Úložiště a Katalog | Otevře [Filelight](https://apps.kde.org/filelight/) na cestě, aby bylo vidět, co zabírá místo. Vyžaduje nainstalovaný Filelight. |
| *Ověřit kontrolní součty* | Katalog | Po potvrzení znovu vypočítá kontrolní součty všech souborů a porovná je s uloženými. Zpráva uvede celkový počet souborů a počet ověřených, neshodných a chybějících. |
| *Ověřit typy MIME* | Dostupný Katalog | Přečte obsah každého dosud neověřeného souboru, zjistí jeho skutečný typ a opraví typ odhadnutý při indexaci podle přípony. Když se typ od přípony liší, zpráva uvede počet neshod a cestu ke zprávě uložené ve složce kolekce (v režimu *Hostováno* v domovské složce), s tlačítkem *Otevřít zprávu*. Není dostupné během jiné operace se zařízením nebo během vyhledávání. |
| *Rozdělit katalog podle podadresáře* | Katalog | Viz [Rozdělení katalogu](#split). |
| *Rozdělit katalog podle typu souboru* | Katalog | Viz [Rozdělení katalogu](#split). |
| *Přidat virtuální zařízení* | Virtuální | Vytvoří pod tímto zařízením virtuální zařízení a otevře ho v [Úpravě zařízení](DevicesEdit). |
| *Přidat úložné zařízení* | Virtuální, ve Fyzické skupině | Vytvoří pod tímto zařízením úložné zařízení a otevře ho v [Úpravě zařízení](DevicesEdit). |
| *Přiřadit vybraný katalog* | Virtuální, ve Virtuální skupině | Přiřadí tomuto virtuálnímu zařízení katalog vybraný na stránce [Výběr](Selection). Dostupné, když je vybrán katalog. |
| *Zrušit přiřazení tohoto katalogu*, *Zrušit přiřazení tohoto úložiště* | Katalog nebo úložiště ve Virtuální skupině | Po potvrzení odebere zařízení z tohoto virtuálního zařízení. Samotné zařízení se zachová. |
| *Smazat tento katalog*, *Smazat toto úložiště*, *Smazat* | Všechna ostatní | Po potvrzení smaže zařízení z kolekce. Zařízení, pod kterým jsou ještě jiná zařízení, smazat nelze. Soubory na disku zůstanou nedotčeny. |

### Rozdělení katalogu {#split}
Katalog lze nahradit několika menšími katalogy. Původní katalog se odstraní: **tuto akci nelze vrátit zpět.**

* *Rozdělit katalog podle podadresáře*: vytvoří jeden katalog pro každý podadresář přímo pod zdrojovou cestou, pojmenovaný `<název katalogu>_<podadresář>`, a jeden pojmenovaný `<název katalogu>_(root)` pro soubory přímo ve zdrojové cestě. Nejprve se vyžádá potvrzení.
* *Rozdělit katalog podle typu souboru*: vytvoří jeden katalog pro každý nalezený typ souboru (Zvuk, Obraz, Text, Video, Jiné) na stejné zdrojové cestě. Dialog nabízí:
    * *Ověřit a rozdělit*: nejprve spustí *Ověřit typy MIME* pro přesný výsledek, pak rozdělí. Dostupné, když je zařízení dostupné.
    * *Rozdělit bez ověření*: použije typy souborů již uložené v katalogu.

## Vývoj
* Viz backlog [vývoje Zařízení](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=devices).
