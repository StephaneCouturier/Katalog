---
version: "3.0"
---

# Rozhraní
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Shrnutí
Tato stránka popisuje části okna Katalogu, které jsou společné všem obrazovkám: postranní panel, otevírání stránek vedle sebe, klávesové zkratky, panel činnosti, možnosti vzhledu a stránku *O aplikaci*.

![Hlavní okno: připnutý panel vlevo, stránky Vyhledávání a Výsledky vedle sebe](/img/interface_01_overview.png)

Okno má tři oblasti:
* **postranní panel** vlevo, pro otevření kolekce a přechod na libovolnou obrazovku,
* **stránky**, zobrazené vedle sebe, počínaje stránkou [Výběr](Selection),
* **panel činnosti** dole, během vytváření katalogu, aktualizace zařízení nebo vyhledávání.

## Stránky {#pages}
Stránka [Výběr](Selection) je výchozím bodem: zobrazuje zařízení kolekce a je vždy prvním sloupcem.

Otevření obrazovky z postranního panelu (například *Vyhledávání*, *Zařízení* nebo *Prozkoumat*) zobrazí její stránku vedle Výběru. Současně je otevřena jen jedna obrazovka: otevření jiné ji nahradí.<br/>
Spuštění vyhledávání otevře stránku *Výsledky* vpravo od stránky *Vyhledávání*. Z výsledku otevře *Prozkoumat složku* stránku [Prozkoumat](Explore) na této složce.

Každá stránka má na panelu nástrojů tlačítko *Zavřít*, které vrací o jeden krok zpět. *Nastavení* a *O aplikaci* se otevírají nad ostatními stránkami a zavírají se také svým tlačítkem *Zavřít*.

### Zobrazit nebo skrýt Výběr {#show-hide-selection}
Když je vedle Výběru otevřena obrazovka, lze Výběr skrýt, aby tato obrazovka měla více místa.<br/>
Použijte tlačítko dole v postranním panelu: zobrazuje *Výběr zobrazen* nebo *Výběr skryt*. Je dostupné pouze tehdy, když je vedle Výběru otevřena obrazovka. Volba se zachová pro příští spuštění.

### Velikost a poloha okna
Velikost a poloha okna a to, zda je maximalizované, se obnoví při příštím spuštění.

## Postranní panel {#drawer}

<div className="screenshot-side">

![Připnutý panel: otevřená kolekce nahoře, pod ní nabídky a obrazovky](/img/interface_02_drawer.png)

</div>

Horní část postranního panelu ukazuje **otevřenou kolekci**: ikonu jejího režimu (Paměť, Soubor nebo Hostováno) a její název:
* v režimu *Paměť* název složky kolekce,
* v režimu *Soubor* název databázového souboru,
* v režimu *Hostováno* název serveru, znak `/` a název databáze.

Pod ní:

| Položka | Použití |
|---------|---------|
| *Otevřít...* | Otevře kolekci nebo znovu otevře nedávnou kolekci: viz [Otevřít kolekci](#open). |
| *Nový...* | Vytvoří novou kolekci: *Databáze SQLite...* vytvoří nový databázový soubor a otevře ho. |
| *Výběr* … *Štítky* | Otevřou obrazovku: [Výběr](Selection), [Vyhledávání](Search), [Zařízení](Devices), [Prozkoumat](Explore), [Vytvořit](Create), [Záloha](BackUp), [Statistika](Statistics), [Štítky](Tags). |
| *Nastavení* | Otevře [Nastavení](Settings). |
| *Dokumentace* | Otevře tuto dokumentaci ve webovém prohlížeči. |
| *O aplikaci* | Otevře stránku [O aplikaci](#about). |
| *Ukončit* | Zavře Katalog. |

<div className="screenshot-side-end" />

### Otevřít kolekci {#open}

<div className="screenshot-side">

![Postranní panel s rozbalenou nabídkou Otevřít...: tři způsoby otevření kolekce, poté nedávné kolekce a dvě tlačítka dole](/img/interface_02_drawer_open.png)

</div>

*Otevřít...* rozbalí nabídku s položkami:
* *Databáze SQLite...*: otevře databázový soubor (`.db`) v režimu *Soubor*,
* *Složka sbírky...*: otevře složku kolekce v režimu *Paměť*,
* *Hostovaná databáze...*: otevře databázi na serveru v režimu *Hostováno*.

<div className="screenshot-side-gap" style={{height: '6rem'}} />

Pod nadpisem *Nedávné sbírky* nabídka dále zobrazuje **10 naposledy otevřených kolekcí**, nejnovější jako první: kliknutím na jednu ji přímo znovu otevřete.

Přepnutí na jinou kolekci Katalog nerestartuje: kolekce se otevře přímo a všechny obrazovky se obnoví.

<div className="screenshot-side-gap" style={{height: '9rem'}} />

### Připnutý nebo plovoucí panel
První tlačítko dole v postranním panelu zobrazuje *Panel připnut* nebo *Panel plovoucí*:
* **připnutý**: panel zůstává otevřený vlevo od stránek,
* **plovoucí**: panel se otevírá nad stránkami a po výběru se zavře.

Volba se zachová pro příští spuštění.

Druhé tlačítko zobrazuje *Výběr zobrazen* nebo *Výběr skryt*: viz [Zobrazit nebo skrýt Výběr](#show-hide-selection).

<div className="screenshot-side-end" />

## Klávesové zkratky {#keyboard-shortcuts}

| Klávesa | Akce |
|---------|------|
| `Esc` | Vrátí se o jeden krok zpět: stejně jako tlačítko *Zavřít* poslední otevřené stránky (nebo *Zrušit* na stránce úprav zařízení). |
| `Ctrl+F` | Umístí kurzor do filtru zařízení na stránce [Výběr](Selection). |
| `Ctrl+Q` | Ukončí Katalog v Linuxu (`Cmd+Q` v macOS). |

Jak funguje `Esc`:
* Nejprve se zavře otevřené dialogové okno, nabídka nebo rozbalovací seznam; stránka pod nimi zůstane otevřená.
* `Esc` zavře stránku, i když je kurzor v textovém poli.
* Na stránce úprav zařízení a na stránce [Vytvořit](Create) `Esc` zahodí, co bylo zadáno, stejně jako *Zrušit* a *Zavřít*. Bez potvrzení.

Ovládání výsledků vyhledávání klávesnicí je popsáno na stránce [Výsledky vyhledávání](SearchResults#keyboard).

## Panel činnosti {#activity-panel}

![Panel činnosti dole: probíhající aktualizace zařízení, fronta ukazuje jednu čekající aktualizaci](/img/interface_03_activity_panel.png)

Panel činnosti se zobrazí dole pod stránkami, když probíhá **vytváření katalogu**, **aktualizace zařízení** nebo **vyhledávání**. Je to jediné místo, kde lze tyto operace sledovat, ať je otevřena kterákoli stránka:
* stavová zpráva ukazuje, co se zpracovává a jak daleko operace pokročila,
* *Zastavit* zastaví probíhající operaci.

Když operace skončí (dokončena, zastavena nebo neúspěšná), panel zůstane několik sekund viditelný s konečným stavem a pak se skryje.

### Fronta {#queue}
Katalog provádí najednou jen jedno vytváření katalogu nebo jednu aktualizaci zařízení. Vytvoření nebo aktualizace vyžádané během jiné probíhající operace jsou **přidány do fronty** místo odmítnutí:
* panel zobrazí *Fronta* a počet čekajících operací,
* každá čekající operace je uvedena (*Vytvořit* nebo *Aktualizovat*, s názvem zařízení) a lze ji odebrat tlačítkem *Odebrat*,
* *Vymazat* frontu vyprázdní,
* operace ve frontě se provádějí jedna po druhé, v pořadí, v jakém byly vyžádány.

*Zastavit* ukončí pouze probíhající operaci: poté se spustí další operace ve frontě. Zařízení, které již probíhá nebo čeká, se nepřidá podruhé.

Vyhledávání nelze spustit během vytváření nebo aktualizace (tlačítko *Vyhledávání* je neaktivní) a vytváření nebo aktualizaci nelze spustit během vyhledávání.

## Vzhled {#appearance}
Vzhled Katalogu lze upravit v sekci *Aplikace* v [Nastavení](Settings#application):
* **Téma**: barvy plochy nebo vlastní barvy Katalogu, vždy podle světlého nebo tmavého režimu systému.
* **Velikost ikon a textu**: větší ikony a menší nebo větší text.
* **Jazyk**: jazyk rozhraní.

Tlačítka, která zobrazují pouze ikonu, ukážou svůj název, když nad nimi ukazatel myši chvíli zůstane.

## O aplikaci {#about}

![Stránka O aplikaci s akcí „Kopírovat informace o verzi a systému“](/img/interface_04_about.png)

Stránka *O aplikaci* zobrazuje verzi Katalogu, jeho licenci a autora.

*Kopírovat informace o verzi a systému* zkopíruje do schránky krátký text, připravený k vložení do hlášení chyby. Obsahuje:
* verzi Katalogu a datum vydání,
* verze Qt a KDE Frameworks,
* operační systém, jádro a architekturu procesoru,
* jazyk rozhraní,
* režim a verzi databáze,
* zobrazovací server a desktopové prostředí.

Neobsahuje **žádné osobní údaje**: žádné jméno uživatele ani počítače, žádnou cestu k souborům nebo kolekcím, žádný název kolekce, adresu serveru ani heslo.

## Vývoj
* Viz backlog [vývoje rozhraní](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=Interface).
