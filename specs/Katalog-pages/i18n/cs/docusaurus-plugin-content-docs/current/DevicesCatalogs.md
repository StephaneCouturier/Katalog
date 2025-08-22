# Zařízení: Katalogy

## Souhrn
Tato stránka popisuje všechny funkce zobrazení **Seznam katalogů** na obrazovce [Zařízení](Devices) a jak je používat.
* *Katalog* je index souborů z daného adresáře označovaný jako katalogová **cesta**.
* **Seznam katalogů** zobrazuje všechny katalogy [Kolekce](Settings#kolekce), filtrované podle [Výběr](Selection).

![](/img/devices_catalogs_01.png)

## Seznam a výběr
Seznam katalogů lze omezit pomocí levého panelu [Výběr](Selection).
Pokud zdrojová cesta katalogu ukazuje na cestu, která je aktivní/připojená/připojená, ikona katalogu je zbarvena modře.

## Tlačítka akcí
* **Aktualizovat**: (povoleno, když je vybrán katalog) Aktualizujte vybraný katalog opětovným seznamem všech souborů z jeho zdrojové cesty podle jeho kritérií.
* **Všechny aktivní**: Aktualizace všech zobrazených katalogů, které jsou aktivní (cesta je dostupná).
* **[Import](#import)**: Importuje katalog z jiného nástroje. V současné době je podporován import z exportu VVV (csv, oddělené tabulátory).

## Kontextová nabídka (kliknutí pravým tlačítkem)

Kliknutím pravým tlačítkem na kterýkoli z uvedených katalogů se otevře kontextová nabídka pro akci s tímto aktivním katalogem.

![](/img/devices_catalogs_02_context2.png)
 | Vstup do menu | Související akce |
 | ------------| -------------------------------------------------- |
 | **Aktualizace** | Aktualizujte vybraný katalog opětovným výpisem všech souborů z jeho zdrojové cesty podle jeho kritérií (typ souboru, skryté soubory atd.) |
 | **Prozkoumat** | Otevřete vybraný soubor katalogu na obrazovce [Prozkoumat](Explore) a zobrazte složky a soubory katalogu. |
 | **[Upravit](#edice)** | Otevřete panel a změňte název, cestu, úložné zařízení atd. Před úpravou libovolného pole si přečtěte podrobnosti níže, abyste pochopili důsledky. |
 | **Filelight** | Otevřete [Filelight](https://apps.kde.org/filelight/) v cestě katalogu. |
 | **Smazat** | Smazat katalog. (Tímto se nesmaže záložní soubor ani související hodnoty ve statistikách) |

## Edice
Obecně se doporučuje vytvořit nový katalog se správným počátečním nastavením:<br/>
Jinak panel umožňuje upravit následující pole:
* **Název zařízení**
* **Název rodiče (ID)** (jiné úložné zařízení).
* **Cesta zdroje**
* **Typ souboru**
* **Zahrnout skryté soubory**
![](/img/devices_catalogs_03_edit.png)
## Import
Nyní je možné importovat všechny fyzické svazky VVV z jednoho exportního souboru vytvořeného pomocí tabelace jako oddělovače.<br/>
Každý fyzický svazek VVV se stane 1 samostatným katalogem. <br/>
Limit: zdrojová cesta a další informace o svazku nejsou k dispozici v exportu z VVV.
- Ve VVV zvolte Soubor / Exportovat... a jako oddělovač vyberte TAB
- Z Katalogu na obrazovce Katalogy: klikněte na tlačítko Importovat a vyberte dříve vytvořený soubor.
- Pomocí tlačítka Upravit je možné přidat zdrojovou cestu katalogu pro pozdější povolení aktualizací.
