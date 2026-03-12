---
version: "2.10"
---
# Statistika
![2.10](https://img.shields.io/badge/Version-2.10-blue)

## Souhrn
Tato stránka popisuje všechny funkce obrazovky **Statistiky** a jak je používat.

Všechna data pocházejí ze záznamů různých aktualizací nebo *snapshotů*.

Tato obrazovka poskytuje pohledy na obsah a vývoj kolekce:
1. Pro katalogová zařízení: počet souborů nebo celková velikost souboru.
1. Pro úložná zařízení: použitý a celkový prostor a celková velikost souboru souvisejících katalogů nebo počet souborů.
1. Pro virtuální zařízení: související zařízení počet souborů nebo celkový prostor a celková velikost souborů souvisejících katalogů.

![Obrazovka Statistiky zobrazující graf vývoje kolekce](/img/screen_statistics_01.png)

## Funkce
### Možnost dat
* Údaje jsou založeny na vybraném zařízení z panelu [Výběr](Selection).
* *Zdroj*: zvolte, zda mají být použita všechna data, nebo aktualizace a snímky současně.
* *Typ dat*: zvolte zobrazení celkové velikosti souboru (která může zahrnovat použité zařízení a celkový prostor) nebo pouze počet souborů.
* *Zobrazit každou hodnotu*: zvolte zobrazení malého kosočtverce pro každý datový bod

### Upravit soubor
* k dispozici pouze v [Režim paměti](Settings#database-memory-mode).
* Tlačítko *Upravit statistiku*: Může být užitečné upravit soubor statistik pro opravu některých čísel, tlačítko otevře soubor v aplikaci spojené se soubory csv. Dejte pozor, aby to byl **soubor oddělený tabulátory**
* Tlačítko *Reload*: data se znovu načtou a graf se obnoví.

### Graf
* Kliknutím levým tlačítkem a podržením přiblížíte část grafu.
* klikněte pravým tlačítkem pro oddálení.
* kliknutím na tlačítko *Znovu načíst* se vrátíte k původnímu přiblížení.

## Vývoj
* Podívejte se na nevyřízené položky [Vývoj statistik](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=statistics).
