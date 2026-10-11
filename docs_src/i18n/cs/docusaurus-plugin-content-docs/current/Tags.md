---
version: "3.0"
---
# Štítky
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Shrnutí
Tato stránka popisuje stránku **Štítky**, kde se **složkám přiřazují štítky**: například *Dovolená* složkám s fotografiemi z dovolené, ať jsou kdekoli.<br/>
Štítek pak lze použít ve [Vyhledávání](Search#tags-criteria) k nalezení pouze souborů ve složkách s tímto štítkem.

![Stránka Štítky: přidání štítku a seznam aktuálních složek a štítků](/img/tags_01_overview.png)

Štítek se přiřazuje **cestě ke složce**, nikoli katalogu ani zařízení: platí pro každý katalog, který tuto cestu obsahuje.<br/>
Složka může mít více štítků a štítek může být na více složkách, ale složka má každý štítek jen jednou.

## Přidání štítku {#add}
1. *Složka*: zadejte cestu ke složce nebo ji zvolte pomocí *Vyberte složku*. Je již vyplněna, pokud byla stránka Štítky otevřena volbou *Označit tuto složku štítkem* na stránce [Prozkoumat](Explore#tag-folder).
1. *Štítek*: zvolte existující štítek ze seznamu nebo zadejte nový název.
1. Klikněte na *Označit složku štítkem*.

Když chybí složka nebo název štítku, nebo když složka již tento štítek má, oznámí to zpráva v horní části stránky.<br/>
Nové přiřazení se objeví v seznamu níže.

## Aktuální složky a štítky {#list}
Seznam ukazuje každou složku s jejím štítkem.
* Seznam nad sloupcem *Štítek* filtruje podle jednoho štítku; *Všechno* zobrazí všechny.
* Tlačítko na konci řádku (*Odebrat tento štítek*) toto přiřazení odebere. Složky ani jejích souborů se to nedotkne.

## Vývoj
Několik nápadů na rozvoj této obrazovky:
* Štítky pro zařízení.
* Štítky pro složky v konkrétních katalozích.
* Více viz backlog [vývoje Štítků](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=tag).
