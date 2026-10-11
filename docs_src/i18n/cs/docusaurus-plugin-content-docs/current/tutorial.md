---
sidebar_position: 1
version: "3.0"
---

# Tutoriál
![3.0](https://img.shields.io/badge/Version-3.0-blue)

Pojďme objevit **Katalog za méně než 5 minut**: vytvořit kolekci, katalogizovat složku a vyhledávat v ní.

## Stažení a spuštění
Stáhněte si Katalog pro Linux, Windows nebo macOS: dostupné balíčky a systémové požadavky viz [Podpora více platforem](Overview#multi-platform-support).

## První spuštění {#first-start}
Při prvním spuštění uvítací zpráva uvede použitý jazyk a téma (lze je později změnit v [Nastavení](Settings#application)) a nabídne:
* *Vytvořit nový...*: vytvoří novou **kolekci**, místo, kde Katalog uchovává všechna svá data. Dialog navrhne, kam ji uložit, jako databázový soubor (`.db`). Vhodným místem je složka ve vašem domovském adresáři.
* *Otevřít existující...*: otevře dříve vytvořenou databázi kolekce (`.db`), například z Katalogu 2.

Dialogy lze zavřít i bez volby: Katalog se pak spustí bez kolekce a kolekci lze otevřít nebo vytvořit později z postranního panelu (viz [Rozhraní](Interface)).

![První spuštění: uvítací zpráva](/img/tutorial_01_welcome.png)

Po *Vytvořit nový...* zpráva vypíše tři kroky k vytvoření prvního katalogu. *OK* otevře stránku [Vytvořit](Create).

## Vytvoření katalogu {#create}
**Katalog** je rejstřík souborů jedné složky.
1. *Zdrojová cesta*: zadejte složku ke katalogizaci nebo ji zvolte pomocí *Vyberte cestu*. Například svou složku *Dokumenty*.
1. *Úložiště*: ponechte *Lokální disk*, úložné zařízení, které Katalog vytvořil pro váš počítač.
1. *Název katalogu*: zadejte název nebo použijte *Vytvořit název z cesty*.
1. Klikněte na *Vytvořit* v horní části stránky.

![Stránka Vytvořit se zvolenou složkou](/img/tutorial_02_create.png)

Průběh se zobrazuje dole v okně. Po dokončení zpráva potvrdí, že byl katalog vytvořen, a ten se objeví ve stromu zařízení na stránce [Výběr](Selection), pod *Lokální disk*.

## Vyhledávání souborů {#search}
1. Na stránce [Výběr](Selection) klikněte pravým tlačítkem na nový katalog a zvolte *Vyhledávání*: katalog se vybere a vedle se otevře stránka [Vyhledávání](Search).
1. Zadejte text do pole názvu souboru, například `report`.
1. Klikněte na *Vyhledávání* (nebo stiskněte `Enter`).

Vpravo se otevře stránka [Výsledky](SearchResults) s nalezenými soubory. Kliknutím na soubor ho otevřete, kliknutím pravým tlačítkem získáte další akce.

![Vyhledávání a výsledky](/img/interface_01_overview.png)

V katalogu lze vyhledávat, i když složka nebo disk nejsou připojeny: právě k tomu Katalog slouží. Stejným způsobem katalogizujte své externí disky, každý pod vlastním úložným zařízením (viz [Zařízení](Devices)), a prohledávejte je všechny najednou výběrem *Fyzická skupina* na stránce Výběr.

## Další kroky
* Použijte další [kritéria vyhledávání](Search): velikost, data, typy souborů, duplikáty, rozdíly.
* Procházejte katalog na stránce [Prozkoumat](Explore).
* Uspořádejte své disky na stránce [Zařízení](Devices).
* Zálohujte složku pomocí stránky [Záloha](BackUp).
* Všechny části okna jsou popsány na stránce [Rozhraní](Interface).
