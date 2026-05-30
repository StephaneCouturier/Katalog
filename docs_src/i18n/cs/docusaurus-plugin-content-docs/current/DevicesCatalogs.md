---
version: "2.12"
---
# Zařízení: Katalogy
![2.12](https://img.shields.io/badge/Version-2.12-blue)

## Souhrn
Tato stránka popisuje všechny funkce zobrazení **Seznam katalogů** na obrazovce [Zařízení](Devices).

*Katalog* je index souborů z daného adresáře označovaného jako **cesta** katalogu.<br/>
**Seznam katalogů** zobrazuje všechny katalogy [Kolekce](Settings#collection), filtrované podle panelu [Výběr](Selection).

![Seznam katalogů zobrazující názvy, cesty, počty souborů a přidružená úložná zařízení](/img/devices_catalogs_01.png)

## Seznam a výběr
Seznam katalogů lze zúžit pomocí panelu **[Výběr](Selection)** vlevo.

Pokud zdrojová cesta katalogu ukazuje na připojené a dostupné umístění, ikona katalogu je zobrazena barevně (modře), což znamená, že katalog je **aktivní**.

## Tlačítka akcí

| Tlačítko | Povoleno, když | Popis |
|----------|---------------|-------|
| *Aktualizovat* | Je vybrán katalog | Znovu prohledá vybraný katalog ze zdrojové cesty podle jeho kritérií (typ souboru, skryté soubory atd.) |
| *Všechny aktivní* | Vždy (pouze zobrazení Seznam katalogů) | Aktualizuje všechny zobrazené katalogy, jejichž zdrojová cesta je dostupná |
| *Zastavit* | Probíhá aktualizace | Zruší probíhající operaci aktualizace |
| *Ověřit typy MIME* | Je vybrán aktivní katalog | Znovu zkontroluje typy souborů všech souborů v katalogu pomocí systémové databáze MIME |
| *Importovat* | Vždy | Importuje katalogy ze souboru exportu VVV — viz [Import](#import) níže |

:::note
Tlačítko *Všechny aktivní* je dostupné pouze v zobrazení **Seznam katalogů**. Je zakázáno, pokud je vybráno zobrazení Strom zařízení nebo Seznam úložiště.
:::

## Kontextová nabídka {#catalog-context-menu}

Kliknutím pravým tlačítkem na katalog se otevře kontextová nabídka:

![Kontextová nabídka katalogu zobrazující dostupné akce](/img/devices_catalogs_02_context2.png)

| Akce | Podmínka | Popis |
|------|----------|-------|
| *Aktualizovat* | Pouze aktivní katalog | Znovu prohledá katalog ze zdrojové cesty |
| *Prozkoumat* | Vždy | Otevře katalog na obrazovce [Prozkoumat](Explore) pro procházení složek a souborů |
| *Upravit* | Vždy | Otevře [panel úprav](#edit) pro změnu nastavení katalogu |
| *Otevřít složku* | Cesta je nastavena a není export | Otevře zdrojovou složku katalogu ve správci souborů |
| *Ověřit kontrolní součty* | Vždy | Přepočítá a porovná kontrolní součty všech souborů v katalogu |
| *Filelight* | Pouze aktivní katalog | Otevře [Filelight](https://apps.kde.org/filelight/) ve zdrojové cestě katalogu |
| *Zrušit přiřazení tohoto katalogu* | Katalog přiřazen do virtuální skupiny | Odebere katalog z jeho virtuální skupiny (katalog samotný není smazán) |
| *Smazat tento katalog* | Katalogy fyzické skupiny a exporty | Trvale odstraní katalog z kolekce |
| *Rozdělit katalog podle podadresáře* | Zařízení katalog | Rozdělí katalog na jeden katalog na každý bezprostřední podadresář plus jeden pro soubory v kořenové složce — viz [Rozdělení katalogu](#split-catalog) |
| *Rozdělit katalog podle typu souboru* | Zařízení katalog | Rozdělí katalog na jeden katalog na každý typ souboru — viz [Rozdělení katalogu](#split-catalog) |

## Rozdělení katalogu {#split-catalog}

Dvě operace rozdělení jsou dostupné z kontextové nabídky (kliknutí pravým tlačítkem) na libovolném zařízení typu Katalog. Obě operace odstraní původní katalog a nahradí ho sadou menších katalogů. **Tuto operaci nelze vrátit zpět.**

### Rozdělit podle podadresáře

Vytvoří jeden nový katalog pro každý bezprostřední podadresář zdrojové cesty katalogu. Soubory umístěné přímo v kořenové složce (nikoli v žádném podadresáři) jsou shromážděny do dedikovaného katalogu pojmenovaného `[NázevKatalogu]_(root)`.

Nové katalogy se pojmenovávají podle vzoru `[NázevKatalogu]_NázevPodadresáře`.

Před pokračováním se zobrazí potvrzovací dialog s počtem katalogů, které budou vytvořeny.

:::note
Pokud katalog nemá žádné bezprostřední podadresáře, operace je zrušena a původní katalog zůstane nezměněn.
:::

### Rozdělit podle typu souboru

Vytvoří jeden nový katalog pro každý typ souboru nalezený v katalogu (Audio, Obrázek, Text, Video, Ostatní), použijíce stejnou zdrojovou cestu jako originál. Je to ekvivalent ručního vytvoření těchto katalogů s filtrem typu souboru při vytváření.

Před rozdělením nabídne dialog tyto možnosti:
- *Ověřit a pak rozdělit* — znovu načte každý soubor z disku pomocí systémové databáze MIME pro přesnou detekci typu, poté rozdělí. Vyžaduje připojené zařízení.
- *Rozdělit bez ověření* — rozdělí okamžitě pomocí typů souborů již uložených v indexu katalogu.

## Upravit {#edit}

Panel úprav umožňuje změnit následující pole:

![Panel úprav katalogu zobrazující všechna konfigurovatelná pole](/img/devices_catalogs_03_edit.png)

| Pole | Popis |
|------|-------|
| *Název zařízení* | Zobrazovaný název katalogu |
| *Nadřazené zařízení* | Úložné zařízení, ke kterému tento katalog patří |
| *Zdrojová cesta* | Cesta ke složce, ze které je katalog vytvořen |
| *Typ souboru* | Omezí katalog na konkrétní typ souboru (Vše, Audio, Obrázek, Text, Video) |
| *Zahrnout skryté soubory* | Zda jsou při prohledávání zahrnuty skryté soubory a složky |
| *Metadata* | Úroveň indexování metadat: *Žádná*, *Standardní* nebo *Rozšířená* |
| *Kontrolní součet* | Zda jsou vypočítány kontrolní součty souborů: *Žádný* nebo *SHA-256* |
| *Vyloučené složky* | Seznam podsložek, které mají být vyloučeny z prohledávání katalogu |

Obecně se doporučuje nastavit správné možnosti při **vytváření** katalogu, nikoli je upravovat později.

### Změna zdrojové cesty {#catalog-path-change}

Když je *Zdrojová cesta* změněna a uložena, Katalog detekuje změnu a nabídne tři možnosti:

| Možnost | Popis |
|---------|-------|
| *Nahradit kořen cesty* | Okamžitě aktualizuje všechny indexované cesty souborů a složek nahrazením starého prefixu novým — bez nutnosti opětovného skenování, funguje bez připojeného zařízení |
| *Úplná re-indexace* | Znovu prohledá katalog z nové zdrojové cesty |
| *Přeskočit* | Uloží novou cestu bez změny indexu katalogu |

*Nahradit kořen cesty* je nejrychlejší možností, pokud se soubory nepřesunuly a změnil se pouze přípojný bod nebo písmeno jednotky.

## Import {#import}

Katalogy lze importovat ze souboru exportu **VVV** (Virtual Volumes View) s tabulátorem jako oddělovačem.

Každý fyzický svazek VVV se stane samostatným katalogem Katalogu.

Postup:
1. Ve VVV zvolte *Soubor / Exportovat…* a jako oddělovač vyberte TAB.
2. V Katalogu přejděte na seznam katalogů, klikněte na *Importovat* a vyberte dříve vytvořený soubor.

:::note
Zdrojová cesta a další informace o svazcích VVV nejsou v exportu dostupné. Pomocí panelu *Upravit* přidejte zdrojovou cestu, pokud chcete moci katalog v budoucnu aktualizovat.
:::

## Vývoj
Několik nápadů na rozvoj této obrazovky:
* Viz přehled [vývoje Zařízení](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=devices).
