---
version: "2.12"
---
# Strom zařízení
![2.12](https://img.shields.io/badge/Version-2.12-blue)

## Souhrn
Tato stránka popisuje všechny funkce zobrazení **Strom zařízení** na obrazovce [Zařízení](Devices).

Strom zařízení zobrazuje úplnou hierarchii všech zařízení — Fyzickou skupinu s jejími úložnými zařízeními a katalogy a všechny Virtuální skupiny s přiřazenými katalogy.

![Strom zařízení zobrazující úplnou hierarchii fyzických a virtuálních zařízení](/img/devices_tree_01.png)

## Možnosti zobrazení

Horní lišta ovládá, které části stromu jsou zobrazeny:

| Možnost | Popis |
|---------|-------|
| *Fyzická skupina* | Zobrazí nebo skryje Fyzickou skupinu a všechna její zařízení |
| *Virtuální skupiny* | Zobrazí nebo skryje všechny Virtuální skupiny a jejich přiřazená zařízení |
| *Úložiště* | Zobrazí nebo skryje úložná zařízení (skrytí úložiště skryje i katalogy pod nimi) |
| *Katalogy* | Zobrazí nebo skryje katalogová zařízení |

Tlačítko *Použít na Výběr* uloží aktuální možnosti zobrazení a použije je na strom zařízení zobrazený v panelu [Výběr](Selection), aby obě zobrazení zůstala konzistentní.

## Tlačítka akcí

| Tlačítko | Popis |
|----------|-------|
| *Vložit virtuální skupinu* | Vytvoří novou Virtuální skupinu na nejvyšší úrovni a otevře panel úprav |
| *Přidat virtuální* | Vytvoří nové virtuální zařízení pod vybraným zařízením a otevře panel úprav |
| *Rozbalit strom* | Rozbalí všechny uzly stromu |
| *Sbalit strom* | Sbalí všechny uzly stromu |

## Kontextová nabídka {#tree-context-menu}

Kliknutím pravým tlačítkem na libovolné zařízení ve stromu se otevře kontextová nabídka, jejíž položky závisí na typu vybraného zařízení.

### Katalogová zařízení

![Kontextová nabídka katalogu ve Fyzické skupině](/img/devices_tree_02_context_phy_virt.png)

| Akce | Podmínka | Popis |
|------|----------|-------|
| *Aktualizovat* | Pouze aktivní katalog | Znovu prohledá katalog ze zdrojové cesty |
| *Prozkoumat* | Vždy | Otevře katalog na obrazovce [Prozkoumat](Explore) |
| *Upravit* | Vždy | Otevře panel úprav pro změnu nastavení katalogu |
| *Otevřít složku* | Cesta je nastavena a není export | Otevře zdrojovou složku katalogu ve správci souborů |
| *Ověřit kontrolní součty* | Vždy | Přepočítá a porovná kontrolní součty všech souborů v katalogu |
| *Filelight* | Pouze aktivní katalog | Otevře [Filelight](https://apps.kde.org/filelight/) ve zdrojové cestě |
| *Zrušit přiřazení tohoto katalogu* | Katalog je ve Virtuální skupině | Odebere katalog z virtuální skupiny (katalog samotný není smazán) |
| *Smazat tento katalog* | Katalogy fyzické skupiny a exporty | Trvale odstraní katalog z kolekce |

### Úložná zařízení

![Kontextová nabídka úložného zařízení ve stromu](/img/devices_tree_03_context_phy_storage.png)

| Akce | Podmínka | Popis |
|------|----------|-------|
| *Aktualizovat* | Vždy | Aktualizuje úložné zařízení a všechny jeho katalogy |
| *Upravit* | Vždy | Otevře panel úprav pro změnu polí úložného zařízení |
| *Otevřít složku* | Cesta je nastavena | Otevře cestu úložiště ve správci souborů |
| *Filelight* | Pouze aktivní úložiště | Otevře [Filelight](https://apps.kde.org/filelight/) v cestě úložiště |
| *Zrušit přiřazení tohoto úložiště* | Úložiště je v podskupině | Odebere úložiště z nadřazeného virtuálního zařízení |
| *Smazat toto úložiště* | Vždy | Trvale odstraní úložné zařízení |

### Virtuální zařízení a skupiny

Virtuální skupiny a virtuální zařízení sdílejí stejnou kontextovou nabídku s drobnými odchylkami:

![Kontextová nabídka virtuálního zařízení ve Fyzické skupině](/img/devices_tree_03_context_vir_virtual.png)
![Kontextová nabídka virtuálního zařízení ve Virtuální skupině s přiřazením katalogu](/img/devices_tree_03_context_vir_catalog.png)

| Akce | Podmínka | Popis |
|------|----------|-------|
| *Aktualizovat* | Vždy | Aktualizuje všechny katalogy a úložná zařízení pod tímto virtuálním zařízením |
| *Upravit* | Vždy | Otevře panel úprav pro přejmenování virtuálního zařízení |
| *Otevřít složku* | Cesta je nastavena | Otevře cestu ve správci souborů |
| *Přidat virtuální zařízení* | Vždy | Vytvoří nové virtuální zařízení pod tímto zařízením |
| *Přidat úložné zařízení* | Pouze položky Fyzické skupiny | Vytvoří nové úložné zařízení pod tímto zařízením |
| *Přiřadit vybraný katalog* | Pouze Virtuální skupiny (v panelu [Výběr](Selection) musí být vybrán katalog) | Přiřadí aktuálně vybraný katalog k tomuto virtuálnímu zařízení |
| *Smazat* | Vždy (kromě kořenové Fyzické skupiny) | Smaže virtuální zařízení (možné pouze pokud nemá žádné podpoložky ani přiřazené katalogy) |

Přiřazení a zrušení přiřazení katalogů ve virtuálních skupinách:

![Přiřazení katalogu k virtuálnímu zařízení](/img/devices_tree_03_context_vir_assign.png)
![Zrušení přiřazení katalogu z virtuálního zařízení](/img/devices_tree_03_context_vir_unassign.png)

- *Přiřadit vybraný katalog*: přiřadí katalog aktuálně vybraný v panelu [Výběr](Selection) k vybranému virtuálnímu zařízení.
- *Zrušit přiřazení tohoto katalogu*: dostupné v kontextové nabídce katalogu, pokud je katalog ve Virtuální skupině — odebere přiřazení bez smazání katalogu.

## Vývoj
Několik nápadů na rozvoj této obrazovky:
* Viz přehled [vývoje Zařízení](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=devices).
