# Zařízení: Úložiště

## Souhrn
Tato stránka popisuje všechny funkce zobrazení **Seznam úložiště** na obrazovce [Zařízení](Devices) a jak je používat.
* **Úložiště** je skutečná fyzická jednotka, disk nebo jiný typ paměti ukládající data jako soubory, ze kterých můžete vytvořit jeden nebo více katalogů.
* Údaje spojené s tímto typem zařízení jsou kombinací 3 typů informací:
 * *fyzické úložiště* : **volné místo**, **využití místa**, **celkový prostor**. **štítek**, **systém souborů**.
 * *vypočteno*: **celkový počet souborů** a **celková velikost souboru** budou automaticky vyplněny, pokud má úložiště katalogy.
 * *uživatelské*: **cesta**, **typ**, **značka**, **model**, **sériové číslo**, **datum sestavení**, **komentář 1** , **komentář 2**, **komentář 3**.
* Jejich použití vám může pomoci vyhledávat ve více katalozích spojených s tímto konkrétním zařízením.
* Pomohou vám také zobrazit více [statistik](Statistics) o všech vašich zařízeních a o tom, co ukládají.
* Úložiště může být pouze součástí *Fyzické skupiny* a může být umístěno pod jakékoli *virtuální zařízení* v této skupině, což může být užitečné pro funkce Vyhledávání nebo Statistika.
![](/img/devices_storage_01.png)
## Seznam a výběr
Seznam katalogů lze omezit pomocí levého panelu [Výběr](Selection).

## Tlačítka akcí
* **Aktualizovat**: (povoleno, když je vybrán katalog) Aktualizujte vybraný katalog opětovným seznamem všech souborů z jeho zdrojové cesty podle jeho kritérií.
* **Všechny aktivní**: Aktualizace všech zobrazených katalogů, které jsou aktivní (cesta je dostupná).

## Kontextová nabídka (kliknutí pravým tlačítkem)
Kliknutím pravým tlačítkem na kterýkoli z uvedených katalogů se otevře kontextová nabídka pro akci s tímto aktivním katalogem.
![](/img/devices_storage_02_context.png)
 | Vstup do menu | Související akce |
 | ------------| -------------------------------------------------- |
 | **Aktualizace** | Aktualizovat vybrané úložiště: tím se aktualizuje samotné úložiště a aktualizace všech níže uvedených katalogů. |
 | **[Upravit](#edice)** | Otevřete panel pro změnu názvu, cesty atd. |
 | **Filelight** | Otevřete [Filelight](https://apps.kde.org/filelight/) v cestě k úložnému zařízení. |
 | **Smazat** | Odstraňte paměťové zařízení. Je to možné pouze v případě, že k němu není přidružen žádný katalog. Tím se neodstraní související hodnoty ve statistice. |

## Edice
Panel poskytuje přístup k úpravě všech polí úložného zařízení, kromě samotného ID zařízení.
![](/img/devices_storage_03_edit.png)

## Obrázek zařízení
Tato funkce je aktuálně dostupná pouze pro [Režim data v paměti](Settings#database-memory-mode), protože snímky jsou ukládány ve složce sbírky.

K paměťovému zařízení je možné přiřadit obrázek.<br/>
Ve složce kolekce musí být vytvořena složka *images* a obraz musí být pojmenován pomocí ID úložiště (nikoli ID zařízení, ale ID úložiště).<br/>
Příklad: /home/user/Documents/KatalogCollectionFolder/images/3.jpg


![](/img/devices_storage_04_picture.png)
