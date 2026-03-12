---
version: "2.10"
---
# Zařízení: Úložiště
![2.10](https://img.shields.io/badge/Version-2.10-blue)

## Souhrn
Tato stránka popisuje všechny funkce zobrazení **Seznam úložiště** na obrazovce [Zařízení](Devices).

Zařízení **Úložiště** představuje fyzický disk nebo jiné úložné médium, ze kterého lze vytvořit jeden nebo více katalogů.

Data přidružená k úložnému zařízení kombinují tři typy informací:
- *Fyzická*: **volné místo**, **využité místo**, **celkový prostor**, **štítek**, **systém souborů** — čteny z disku při připojení.
- *Vypočítaná*: **celkový počet souborů** a **celková velikost souborů** — automaticky odvozeny z katalogů přidružených k tomuto úložišti.
- *Definovaná uživatelem*: **cesta**, **typ**, **značka**, **model**, **sériové číslo**, **datum výroby**, **komentáře**.

Úložná zařízení mohou být umístěna pouze ve *Fyzické skupině* a jejích podpoložkách.

![Seznam úložiště zobrazující zařízení s využitím místa a počtem přidružených katalogů](/img/devices_storage_01.png)

## Seznam a výběr
Seznam úložných zařízení lze zúžit pomocí panelu **[Výběr](Selection)** vlevo.

Pokud cesta úložného zařízení ukazuje na připojené a dostupné umístění, ikona je zobrazena barevně, což znamená, že úložiště je **aktivní**.

## Tlačítka akcí

| Tlačítko | Povoleno, když | Popis |
|----------|---------------|-------|
| *Aktualizovat* | Je vybráno úložné zařízení | Aktualizuje vybrané úložné zařízení a všechny jeho přidružené katalogy |

:::note
Tlačítko *Všechny aktivní* není dostupné v zobrazení Seznam úložiště — je povoleno pouze v zobrazení Seznam katalogů.
:::

## Kontextová nabídka {#storage-context-menu}

Kliknutím pravým tlačítkem na úložné zařízení se otevře kontextová nabídka:

![Kontextová nabídka úložného zařízení zobrazující dostupné akce](/img/devices_storage_02_context.png)

| Akce | Podmínka | Popis |
|------|----------|-------|
| *Aktualizovat* | Vždy | Aktualizuje vybrané úložné zařízení a všechny katalogy pod ním |
| *Upravit* | Vždy | Otevře [panel úprav](#edit) pro změnu polí úložného zařízení |
| *Otevřít složku* | Cesta je nastavena | Otevře cestu úložiště ve správci souborů |
| *Filelight* | Pouze aktivní úložiště | Otevře [Filelight](https://apps.kde.org/filelight/) v cestě úložiště |
| *Zrušit přiřazení tohoto úložiště* | Úložiště je v podskupině | Odebere úložiště z nadřazeného virtuálního zařízení (úložiště samotné není smazáno) |
| *Smazat toto úložiště* | Vždy | Trvale odstraní úložné zařízení (možné pouze pokud k němu nejsou přidruženy žádné katalogy) |

## Upravit {#edit}

Panel úprav umožňuje změnit všechna pole úložného zařízení:

![Panel úprav úložného zařízení zobrazující všechna konfigurovatelná pole](/img/devices_storage_03_edit.png)

| Pole | Popis |
|------|-------|
| *Název zařízení* | Zobrazovaný název úložného zařízení |
| *Typ* | Typ zařízení (např. interní disk, externí disk, USB, NAS…) |
| *Štítek* | Popisek souborového systému disku |
| *Systém souborů* | Typ souborového systému (např. ext4, NTFS, exFAT…) |
| *Značka* | Výrobce disku |
| *Model* | Název modelu disku |
| *Sériové číslo* | Sériové číslo disku |
| *Datum výroby* | Datum výroby disku |
| *Komentář 1 / 2 / 3* | Volná textová pole pro poznámky |
| *Obrázek* | Obrázek přidružený k tomuto úložišti (pouze v režimu Paměť — viz níže) |
| *Nadřazené zařízení* | Virtuální zařízení nebo skupina, do které toto úložiště patří |

## Obrázek zařízení

K úložnému zařízení lze přiřadit obrázek. Tato funkce je aktuálně dostupná pouze v [režimu databáze Paměť](Settings#database-memory-mode), protože obrázky jsou uloženy ve složce kolekce.

Postup:
1. Vytvořte složku `images` ve složce kolekce.
2. Umístěte soubor obrázku pojmenovaný podle ID úložiště (nikoli ID zařízení) — například: `/home/user/Documents/KatalogKolekce/images/3.jpg`

![Úložné zařízení s přidruženým obrázkem zobrazeným v panelu úprav](/img/devices_storage_04_picture.png)

## Vývoj
Několik nápadů na rozvoj této obrazovky:
* Viz přehled [vývoje Zařízení](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=devices).
