---
version: "3.0"
---
# Úprava zařízení
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Shrnutí
Tato stránka popisuje stránku **Upravit zařízení**, kde se mění definice zařízení:
* [pole společná všem zařízením](#device): název, nadřazené zařízení, komentář,
* [zdrojová cesta](#location) katalogu nebo úložiště a [co se stane, když se změní](#path-change),
* [možnosti obsahu](#content-options) katalogu,
* [podrobnosti úložiště](#storage-details) a [obrázek](#picture) úložného zařízení.

Typy zařízení jsou popsány na stránce [Zařízení](Devices#model).

![Stránka Upravit zařízení pro katalog](/img/devicesedit_01_catalog.png)

## Otevření a zavření {#opening}
Stránka se otevře:
* volbou *Upravit* v místní nabídce zařízení na stránce [Výběr](Selection#context-menu) nebo [Zařízení](Devices#context-menu),
* po vytvoření zařízení pomocí *Vložit virtuální skupinu*, *Přidat úložiště*, *Přidat virtuální zařízení* nebo *Přidat úložné zařízení* na stránce [Zařízení](Devices).

| Akce | Popis |
|------|-------|
| *Uložit* | Uloží změny, po potvrzeních popsaných níže, a zavře stránku. |
| *Smazat* | Po potvrzení smaže zařízení z kolekce. Zařízení, pod kterým jsou ještě jiná zařízení, smazat nelze. |
| *Zrušit* | Zavře stránku bez uložení (také `Esc`). |

Pokud byla stránka otevřena ze stránky Zařízení, vrátí se na stránku Zařízení.

## Zařízení {#device}
Pro všechna zařízení:

| Pole | Popis |
|------|-------|
| *Typ* | Katalog, Úložiště nebo Virtuální. Pouze se zobrazuje, nelze ho změnit. |
| *Název* | Název zařízení, nutný pro uložení. |
| *Nadřazené zařízení* | Zařízení, ke kterému ve stromu patří. Katalog patří k úložišti; úložiště k virtuálnímu zařízení Fyzické skupiny. Samotná Fyzická skupina nemá nadřazené zařízení. |
| *Komentář* | Volná poznámka zobrazená vedle názvu na kartách zařízení. |

## Umístění {#location}
Pro katalogy a úložná zařízení:
* *Zdrojová cesta*: složka indexovaná katalogem nebo složka, kde je úložné zařízení připojeno (například `/run/media/username/Photos` v Linuxu, `E:\` ve Windows). *Vyberte cestu* otevře dialog pro výběr složky.

U nového úložného zařízení se po prvním nastavení cesty a uložení zařízení ihned aktualizuje, čímž se vyplní jeho údaje o místě.

### Když se cesta změní {#path-change}
Když se cesta katalogu nebo úložiště změní a uloží, dialog zobrazí starou a novou cestu a zeptá se, jak aktualizovat rejstříky katalogů:

| Tlačítko | Účinek |
|----------|--------|
| *Nahradit kořen cesty* | Nahradí starou cestu novou ve všech indexovaných souborech a složkách: v katalogu nebo ve všech katalozích pod úložištěm. Z disku se nic nečte, takže je to okamžité a funguje to i s odpojeným diskem. Panel činnosti uvede, kolik katalogů, souborů a složek bylo změněno. |
| *Úplné přeindexování* | Znovu načte soubory z nové cesty. Vyžaduje dostupné zařízení. |
| *Přeskočit* | Uloží novou cestu beze změny rejstříků. |

*Nahradit kořen cesty* je správná volba, když se soubory nezměnily a změnilo se jen místo, kde je disk připojen, například `/mnt/drive1` na `/mnt/drive01` nebo `F:\` na `G:\`.

## Možnosti obsahu {#content-options}
Pro katalogy stejné možnosti jako při vytvoření katalogu (viz [Vytvořit](Create)):

| Pole | Popis |
|------|-------|
| *Typ souboru* | Všechno, Zvuk, Obraz, Text, Video, Jiné nebo Žádné. |
| *Zahrnout podadresáře* | Indexuje i soubory podsložek, nebo pouze soubory přímo ve zdrojové cestě. |
| *Zahrnout skryté soubory* | Žádné nebo Všechno. |
| *Zahrnout metadata* | Žádné, Média základní, Média rozšířené nebo Úplné rozšířené. |
| *Zahrnout kontrolní součet* | Žádné nebo SHA-256. |
| *Vyloučit složky nebo soubory* | Cesty nebo názvy, které se neindexují. Zadejte cestu nebo název (nebo použijte *Vyberte cestu*) a klikněte na *Přidat*; *Odebrat* jednu položku ze seznamu odebere. |

Když se změní některá možnost nebo zdrojová cesta, uložení vypíše změny a vyžádá potvrzení, protože katalog již neodpovídá tomu, co bylo indexováno. Druhá otázka pak nabídne aktualizovat obsah katalogu podle nových kritérií hned (*Ano*), nebo později (*Ne*).

## Podrobnosti úložiště {#storage-details}
Pro úložná zařízení:

| Pole | Popis |
|------|-------|
| *ID úložiště* | Číslo úložiště, například číslo napsané na fyzickém disku. Každé úložiště by mělo mít vlastní: když stejné číslo již používá jiné úložiště, stránka zobrazí *Toto ID již používá jiné úložiště.* Číslo se přesto uloží a lze ho opravit. |
| *Typ*, *Označení*, *Souborový systém* | Druh zařízení, název disku a jeho souborový systém (ext4, NTFS, exFAT…). |
| *Celkové místo (bajty)*, *Volné místo (bajty)* | Místo na disku, vedle hodnota v čitelné jednotce. |
| *Obnovit z disku* | Načte z disku *Typ*, *Označení*, *Souborový systém* a hodnoty místa. Vyžaduje dostupný disk a předem uloženou cestu. |
| *Značka*, *Model*, *Sériové číslo*, *Datum sestavení* | Údaje o disku pro informaci. |
| *Komentář 1*, *Komentář 2*, *Komentář 3* | Volné poznámky. |

### Obrázek {#picture}

![Podrobnosti úložiště s obrázkem disku](/img/devicesedit_02_storage.png)

Obrázek pomáhá disk poznat. Postup přidání:
1. Umístěte obrázek do **složky obrázků** kolekce (viz [Nastavení](Settings#images-folder)).
2. Zvolte ho v seznamu *Obrázek*. Tlačítko obnovení vedle seznamu (*Znovu načíst obrázky*) po přidání souboru složku znovu načte.
3. *Uložit*.

Zvolený obrázek se zobrazí pod seznamem.<br/>
Když není zvolen žádný obrázek, Katalog zobrazí soubor pojmenovaný podle **ID úložiště**, pokud ve složce obrázků existuje: například `12.jpg` pro ID úložiště 12. Pojmenování obrázku podle čísla napsaného na disku stačí k jeho zobrazení, bez volby v seznamu. Stejný název funguje i v Katalogu 2.

## Vývoj
* Viz backlog [vývoje Zařízení](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=devices).
