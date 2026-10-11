---
version: "3.0"
---
# Statistika
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Shrnutí
Tato stránka popisuje stránku **Statistika**: graf vývoje zařízení v čase, pro zařízení vybrané na stránce [Výběr](Selection).

![Stránka Statistika: graf úložného zařízení s celkovou velikostí souborů, využitým místem a celkovým místem](/img/statistics_01_overview.png)

## Odkud data pocházejí {#data}
Pokaždé, když se zařízení aktualizuje, a pokaždé, když se zaznamená **snímek** (*Snímek* na stránce [Zařízení](Devices#page-actions)), Katalog uloží záznam jeho údajů: počet souborů, celkovou velikost souborů a u úložných zařízení volné a celkové místo.<br/>
Graf tyto záznamy vykresluje. Zařízení, které nikdy nebylo aktualizováno a nikdy nebylo součástí snímku, nemá data: graf zobrazí *Žádná data*.

## Co se zobrazuje {#chart}
Nadpis grafu uvádí název zařízení, zobrazený údaj a jeho jednotku.

| Vybrané zařízení | *Celková velikost souboru* | *Počet souborů* |
|------------------|----------------------------|-----------------|
| Katalog | Celková velikost jeho souborů. | Počet jeho souborů. |
| Úložiště, Virtuální | Celková velikost souborů jeho katalogů a navíc **využité** a **celkové místo** úložiště. | Počet souborů jeho katalogů. |

Velikosti se zobrazují v jednotce odpovídající hodnotám (KiB, MiB, GiB nebo TiB).

## Možnosti {#options}

| Možnost | Účinek |
|---------|--------|
| *Zdroj* | *všechny záznamy*, *pouze aktualizace* nebo *pouze snímky*. Snímky dávají pravidelné body pro celou kolekci; aktualizace přidávají bod při každé aktualizaci zařízení. |
| *Data* | *Celková velikost souboru* nebo *Počet souborů*. |
| *Od data* | Zobrazí pouze záznamy od tohoto data. *Vyberte datum* otevře dialog s rychlými volbami (*Před 1 dnem*, *Před 1 týdnem*, *Před 1 měsícem*, *Před 1 rokem*); *Vymazat* znovu zobrazí všechny záznamy. |
| *Zobrazte každou hodnotu* | Označí každý záznam bodem na čarách. |

Možnosti se ukládají pro kolekci.

## Přiblížení {#zoom}
* **Kliknutím a tažením** přes graf přiblížíte danou oblast.
* **Kliknutím pravým tlačítkem** nebo **dvojitým kliknutím** se vrátíte k celému grafu.

## Vývoj
* Viz backlog [vývoje Statistiky](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=statistics).
