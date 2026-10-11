---
version: "3.0"
---
# Výběr
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Shrnutí

Tato stránka popisuje stránku **Výběr**: seznam všech zařízení kolekce zobrazený jako strom karet. Je to první stránka okna a ostatní obrazovky se otevírají vedle ní (viz [Rozhraní](Interface#pages)).

Výběr zařízení určuje **rozsah** ostatních obrazovek:
* [Vyhledávání](Search): vyhledávání probíhá v souborech vybraného zařízení,
* [Zařízení](Devices): s volbou *Filtrovat z výběru* se zobrazí pouze vybrané zařízení a jeho podřízená zařízení,
* [Vytvořit](Create): vybrané úložiště se nabídne pro nový katalog,
* [Záloha](BackUp): záložní odkazy se filtrují podle vybraného zařízení,
* [Statistika](Statistics): grafy zobrazují vybrané zařízení.

## Vybrané zařízení {#selected-device}
Horní část stránky vždy ukazuje **vybrané zařízení** s jeho ikonou a názvem. Když není vybráno žádné zařízení, zobrazuje *Všechno*: celou kolekci.

Kliknutím na něj se seznam posune na vybranou kartu, což se hodí v dlouhém seznamu. Výběr se tím nemění.

## Karty zařízení {#device-cards}
<div className="screenshot-side">
![Stránka Výběr: vybrané zařízení nahoře, pole filtru, strom karet s vybranou fyzickou skupinou a zobrazenými informacemi o zařízeních](/img/selection_01_overview.png)
</div>

Každé zařízení kolekce je zobrazeno jako karta, odsazená pod svým nadřazeným zařízením.
* **Kliknutím** na kartu zařízení vyberete. Vybraná karta je zvýrazněna.
* **Kliknutím pravým tlačítkem** na kartu (nebo dlouhým stiskem na dotykové obrazovce) otevřete její [místní nabídku](#context-menu).
* Šipka vpravo na kartě skryje nebo zobrazí její podřízená zařízení.

Typ zařízení poznáte podle ikony a stylu názvu:

| Zařízení | Ikona | Název |
|----------|-------|-------|
| **Virtuální** zařízení nebo skupina | několik disků | tučná kurzíva, ztlumený |
| **Úložné** zařízení | pevný disk | tučný, mírně ztlumený |
| **Katalog** | disk (jedna ikona, když je katalog dostupný, jiná, když není) | normální |

Co jsou zařízení Virtuální, Úložiště a Katalog, viz [Zařízení](Devices).

Když jsou zobrazeny informace o zařízení, druhý řádek pod názvem uvádí:
* u **Katalogu**: počet souborů a jejich celkovou velikost,
* u **Úložného** nebo **Virtuálního** zařízení: počet souborů a celkovou velikost jeho katalogů, poté využité místo.

<div className="screenshot-side-end" />

## Filtr a tlačítka stromu {#filter}

<div className="screenshot-side">

![Filtr zařízení se zadaným „back“: zobrazí se pouze odpovídající zařízení](/img/selection_02_filter.png)

</div>

Nad kartami:

| Ovládací prvek | Použití |
|----------------|---------|
| Pole *Název...* | Filtruje karty **podle názvu zařízení**: zobrazí se pouze zařízení, jejichž název obsahuje zadaný text, bez ohledu na velikost písmen. Filtruje zařízení, nikoli soubory. Tlačítko s křížkem pole vymaže. `Ctrl+F` umístí kurzor do tohoto pole. |
| *Zobrazit informace o zařízení* | Zobrazí nebo skryje druhý řádek karet. |
| *Sbalit o jednu úroveň* | Skryje nejhlubší aktuálně zobrazenou úroveň zařízení. |
| *Rozbalit o jednu úroveň* | Zobrazí o jednu úroveň zařízení více. |

<div className="screenshot-side-end" />

## Místní nabídka {#context-menu}

<div className="screenshot-side">

![Místní nabídka karty Katalog](/img/selection_03_context_menu.png)

</div>

Místní nabídka začíná názvem zařízení, za kterým následuje:

| Položka | Zobrazeno pro | Akce |
|---------|---------------|------|
| *Vyhledávání* | všechna zařízení | Vybere zařízení a otevře stránku [Vyhledávání](Search). |
| *Aktualizovat* | virtuální a úložná zařízení a dostupné katalogy | Aktualizuje zařízení: znovu prohledá soubory jeho katalogů. Pokud je vyžádáno během jiné probíhající operace, přidá se do [fronty](Interface#queue). |
| *Prozkoumat* | katalogy | Vybere katalog a otevře ho na stránce [Prozkoumat](Explore). |
| *Otevřít složku* | všechna zařízení | Otevře složku zařízení ve správci souborů. |
| *Upravit* | všechna zařízení | Otevře stránku úprav zařízení (viz [Zařízení](Devices)). |

*Vyhledávání* a *Prozkoumat* nejprve zařízení vyberou, takže zvýrazněná karta vždy odpovídá tomu, co se prohledává nebo prozkoumává. *Aktualizovat*, *Otevřít složku* a *Upravit* působí na zařízení, na které bylo kliknuto pravým tlačítkem, **beze změny výběru**.

<div className="screenshot-side-end" />

## Vyhledávání v připojeném disku
Přímé vyhledávání ve složce připojeného disku, bez katalogu, se volí přímo na stránce [Vyhledávání](Search) (*Připojený disk*), nikoli ve Výběru.

## Vývoj
* Viz backlog [vývoje Výběru](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=selection).
