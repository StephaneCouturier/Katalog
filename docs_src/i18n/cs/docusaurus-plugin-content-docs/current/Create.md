---
version: "3.0"
---
# Vytvořit
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Shrnutí
Tato stránka popisuje stránku **Vytvořit**, kde se vytváří nový **katalog**: rejstřík souborů jedné složky, uložený v kolekci, aby v něm bylo možné vyhledávat a procházet ho, i když je zařízení odpojené.

Probíhá ve třech krocích:
1. [Definujte katalog](#definition): složku k indexování, její úložné zařízení a název.
1. Zvolte [možnosti obsahu](#content-options): typy souborů, podsložky, skryté soubory, metadata, kontrolní součty a vyloučení.
1. Klikněte na *Vytvořit*.

![Stránka Vytvořit: definice katalogu, možnosti obsahu a globální parametry](/img/create_01_overview.png)

## Definice katalogu {#definition}

| Pole | Popis |
|------|-------|
| *Zdrojová cesta* | Složka k indexování. Vyplní se cestou zařízení vybraného na stránce [Výběr](Selection) a lze ji zadat nebo zvolit pomocí *Vyberte cestu*. |
| *Úložiště* | Úložné zařízení, na kterém složka je: disk nebo počítač (viz [Zařízení](Devices#model)). Zobrazují se pouze úložná zařízení. Nová kolekce již jedno má, *Lokální disk*. Další přidáte pomocí *Přidat úložiště* na stránce [Zařízení](Devices#page-actions). |
| *Název katalogu* | Název nového katalogu. Každý katalog potřebuje vlastní název. *Vytvořit název z cesty* (tlačítko vedle pole) ho vytvoří ze zdrojové cesty, se `_` místo `/` a `:`. |

Když pole chybí nebo je chybné, zpráva v horní části stránky uvede, co je potřeba, například *Zadejte název pro tento nový katalog.* Zmizí, jakmile je pole opraveno. Totéž nastane, když zdrojová složka neexistuje nebo když stejný název již má jiný katalog.

Když zdrojová složka neobsahuje žádný soubor, nejprve se zobrazí otázka: složka může být prázdná, nebo zařízení nemusí být připojeno na tomto místě. *Ano* katalog přesto vytvoří, prázdný.

## Možnosti obsahu {#content-options}
Možnosti určují, co se indexuje. Lze je později změnit úpravou katalogu (viz [Úprava zařízení](DevicesEdit#content-options)); katalog pak potřebuje aktualizaci, aby se jim přizpůsobil.

### Typ souboru {#file-type}
Katalog lze omezit na jeden typ souborů.

| Typ | Popis | Definice | Příklady přípon |
|-----|-------|----------|-----------------|
| Všechno | Všechny typy souborů, bez filtrování. | | |
| Zvuk | Hudba, podcasty, zvukové nahrávky a zvukové soubory. | Podle definice typů MIME. | MP3, FLAC, AAC, M4A, OGG, WAV, AIFF, Opus, WMA, MIDI, AMR (více než 50 formátů) |
| Obraz | Fotografie, grafika, diagramy, ikony a vizuální obsah. | Podle definice typů MIME. | JPG, PNG, HEIC, WebP, TIFF, RAW, SVG, XCF, GIF, BMP (více než 100 formátů) |
| Text | Dokumenty, soubory kódu, značkovací jazyky, datové soubory a čitelný obsah. | Vlastní definice Katalogu: všechny soubory s typem MIME začínajícím `text/` a navíc soubory aplikací, jako jsou dokumenty PDF a Word. | PDF, DOCX, ODT, Markdown, HTML, JSON, soubory zdrojového kódu, e-knihy (více než 100 formátů) |
| Video | Filmy, klipy, animace a video obsah. | Podle definice typů MIME. | MP4, MKV, AVI, WebM, MOV, FLV, 3GP, OGV, M2TS (více než 40 formátů) |
| Jiné | Všechny ostatní typy souborů. | Vlastní definice Katalogu. | ZIP, RAR, EXE, DLL, ISO: spustitelné soubory, archivy, systémové soubory a soubory aplikací nezařazené jako Text. |
| Žádné | Soubory, jejichž typ nelze určit z přípony. | Soubory bez přípony nebo s neznámou příponou. | |

Typ souboru se určuje podle jeho **přípony** pomocí databáze MIME systému: rozpoznávají se stovky formátů a s aktualizacemi systému i nové. Díky tomu je indexace rychlá. Soubor s chybnou nebo chybějící příponou proto může dostat nesprávný typ.

### Zahrnout podadresáře {#include-subdirectories}
Zaškrtnuto (výchozí): indexují se soubory všech podsložek. Nezaškrtnuto: pouze soubory přímo ve zdrojové cestě.

### Zahrnout skryté soubory {#include-hidden-files}
*Žádné* (výchozí) skryté soubory a složky přeskočí; *Všechno* je zahrne.<br/>
V domovské složce nebo na systémovém disku mohou skryté složky (`.cache`, `.local`…) obsahovat mnoho souborů: vyloučením nepotřebných zůstane katalog malý.

### Zahrnout metadata {#include-metadata}
Kolik informací se čte z obsahu souborů. Indexace se tím prodlouží, ale výsledky ve [Vyhledávání](Search) (rozměry, délka, umělec…) a ve [Statistice](Statistics) jsou bohatší.

| Úroveň | Co se čte |
|--------|-----------|
| *Žádné* | Žádná metadata: nejrychlejší. |
| *Média základní* | To podstatné z obrázků, videí a zvukových souborů: rozměry a orientace obrázků; rozměry, délka, kodek a snímková frekvence videí; umělec, album, stopa, délka a datový tok zvukových souborů. |
| *Média rozšířené* | Všechna metadata obrázků, videí a zvukových souborů, včetně technických údajů. |
| *Úplné rozšířené* | Všechna metadata každého podporovaného typu souboru. |

Rozšířené úrovně používají knihovnu KFileMetaData, která určuje podporované typy a pole; jejich metadata lze zobrazit pomocí *Zobrazit rozšířená metadata (JSON)* ve výsledcích vyhledávání a na stránce Prozkoumat.<br/>
Úrovně Média čtou pouze multimediální soubory: ostatní soubory se přeskočí. Poškozené nebo nečitelné soubory se přeskočí, aniž by se vytváření katalogu zastavilo.

Podporované multimediální soubory:
* **Obrázky**: jpg, png, gif, bmp, tiff, webp, svg, heic, raw, xcf
* **Videa**: mp4, mkv, avi, mov, wmv, flv, webm, m4v, mpg, 3gp, ogv, vob
* **Zvuk**: mp3, wav, flac, ogg, m4a, aac, wma, opus, aiff, mid, amr

### Zahrnout kontrolní součet {#include-checksum}
*SHA-256* vypočítá kontrolní součet každého souboru: otisk jeho obsahu. Kontrolní součty zpřesňují [hledání duplikátů a rozdílů](Search) a umožňují později ověřit, že se soubory nezměnily.<br/>
⚠️ Trvá to mnohem déle než ostatní možnosti, protože se čtou **všechna** data každého souboru (viz [Průvodce výkonem](#performance)).

### Vyloučit složky nebo soubory {#exclude-folders-or-files}
Cesty nebo názvy, které se **v tomto katalogu** neindexují.<br/>
Zadejte cestu nebo název do *Cesta nebo název k vyloučení* nebo zvolte složku pomocí *Vyberte cestu* a klikněte na *Přidat*. Tlačítko *Odebrat* vedle položky ji ze seznamu odebere.<br/>
Seznam se ukládá spolu s katalogem.

## Globální parametry {#global-parameters}
Sekce *Globální parametry* obsahuje nastavení společná pro **všechny katalogy** kolekce. Tlačítko na řádku jejího nadpisu ji sbalí nebo rozbalí.

### Globální vyloučení složek {#exclude-directories}
Cesty vyloučené z **každého katalogu**, při jeho vytvoření i při každé aktualizaci.<br/>
Zadejte cestu do *Cesta ke globálnímu vyloučení* (nebo použijte *Vyberte cestu*) a klikněte na *Přidat*. *Odebrat* vedle položky ji odebere.

### Jak vyloučení fungují
Globální vyloučení a vyloučení katalogu se sčítají: soubor nebo složka vynechaná kterýmkoli seznamem se neindexuje.<br/>
Vyloučení je **shoda textu**: každý soubor nebo složka, jejíž úplná cesta **obsahuje** daný text, se přeskočí. Shoda rozlišuje velká a malá písmena.
* **Úplná cesta**: `/home/username/Downloads/temp` přeskočí tuto složku a celý její obsah.
* **Název složky**: `node_modules` přeskočí **každou** složku `node_modules`, ať je kdekoli.
* **Část názvu**: `.cache` přeskočí `/home/username/.cache/`, ale i `/home/username/.cachedata/`, protože obě cesty daný text obsahují.

## Vytvoření katalogu {#creating}
*Vytvořit* uloží katalog a spustí indexaci. Průběh se zobrazuje v panelu činnosti dole v okně (viz [Rozhraní](Interface)). Pokud probíhá jiná operace, vytvoření čeká ve frontě.

Indexace probíhá ve dvou fázích:
1. načte se a uloží **seznam souborů**: katalog lze používat, jakmile tato fáze skončí,
2. poté, pokud byla zvolena, se čtou **metadata** a **kontrolní součty**.

*Zastavit* ukončí vytváření:
* během první fáze se katalog nevytvoří,
* během druhé fáze se katalog **zachová** s úplným seznamem souborů; dosud nenačtená metadata nebo kontrolní součty se doplní při příští aktualizaci katalogu.

Po dokončení se nový katalog vybere na stránce [Výběr](Selection) a otevře se stránka [Zařízení](Devices) se zprávou: název a cesta katalogu, počet souborů a celková velikost a, pokud bylo aktualizováno jeho úložiště, využité, volné a celkové místo úložiště s tím, co přibylo. Pokud kontrola metadat nebo kontrolních součtů nebyla dokončena, místo toho to vysvětlí zpráva.<br/>
*Zavřít* (nebo `Esc`) stránku zavře.

## Průvodce výkonem {#performance}

### Co ovlivňuje rychlost prohledávání?


| # | Faktor | Míra dopadu | Co ho určuje | Poznámky |
|---|--------|:---:|---|---|
| 1 | **Kontrolní součet souborů** | ⚫⚫⚫⚫⚫ Kritická | **Celkový objem dat** (čte 100 % bajtů, vyrovnávací paměť 8 MB), pak rychlost disku, pak počet souborů | SHA-256 přes Qt `QCryptographicHash`. Pevný disk 2 TB při ~100 MB/s = ~5,5 hodiny jen na V/V. SSD při ~500 MB/s ≈ 1 hodina. Po přerušení lze pokračovat. |
| 2 | **Úložný hardware** | ⚫⚫⚫⚫ Velmi vysoká | Sekvenční nebo náhodný V/V, propustnost, latence | SSD ~100 000 souborů/min, pevný disk ~20–30 000 (fragmentace to zhoršuje). Síť: velmi proměnlivé. Ovlivňuje VŠECHNY ostatní faktory. |
| 3 | **Metadata: Úplné rozšířené** | ⚫⚫⚫ Vysoká | **Počet souborů** (zejména videa: 5–15 ms/soubor na analýzu kontejnerů), rozložení typů souborů | Používá KFileMetaData. Video a zvuk vyžadují přesuny ve struktuře souboru. Hlášeno ~10× zpomalení oproti Žádné. |
| 4 | **Metadata: Média základní/rozšířené** | ⚫⚫⚫ Vysoká | Pouze počet multimediálních souborů | Čte jen hlavičky (obrázky: ~2–3 ms). Nemultimediální soubory se zcela přeskočí, takže na skladbě typů souborů hodně záleží. |
| 6 | **Zahrnout skryté soubory**  | ⚫⚫  Střední | Další soubory navíc (na systémových/domovských discích může jít o obrovské množství: `.cache`, `.local`, `node_modules`…) | Násobící efekt: více souborů = více všeho výše uvedeného. Řešením jsou vyloučené složky. |
| 7 | **Režim databáze** | ⚫ Nízká až střední | Propustnost zápisu během indexace | Paměť > Soubor > Hostováno. Režim Paměť se při prohledávání vyhne zápisům do SQLite; režim Soubor je omezen zápisy. Dávkové transakce (1000 souborů/dávka) to již zmírňují. |
| 8 | **Filtr typu souboru** | ⚫ Nízká až střední | Snižuje počet souborů zpracovávaných kvůli metadatům | Filtr na Zvuk/Video/Obraz přeskočí u metadat všechny Text/Jiné: významné u disku se smíšeným obsahem. |
| 9 | **Počet souborů (celkem)** | ⚫ Nízká | Určuje dobu procházení souborového systému a počet vložení do databáze | I bez jakýchkoli možností zabere výčet 500 000 souborů čas. Nezávisí na velikostech. |
| 10 | **Vyloučené složky** | ⚫ Nízká | Snižuje skutečný počet souborů | `node_modules`, `.cache` a dočasné složky mohou tvořit 20–30 % souborů domovské složky. |
| 11 | **Zatížení systému / procesor** | ⚫ Nízká | Paralelní extrakce metadat využívá 4–8 jader | Kontrolní součet je omezen V/V, takže na procesoru tam záleží méně. Metadata jsou omezena procesorem. |

Další dlouhé procesy, které Vytvořit ani Aktualizace nepoužívají:

| # | Faktor | Míra dopadu | Co ho určuje | Poznámky |
|---|--------|:---:|---|---|
| 5 | **Ověření typů MIME** | ⚫⚫⚫⚫ Velmi vysoká | Počet souborů (čte část obsahu každého souboru) | Operace pouze při aktualizaci. „Odhad“ podle přípony nic nestojí; ověření čte bajty souborů. Dopad roste s počtem souborů, ne s velikostí. |
| 12 | **Export kolekce** | ⚫⚫⚫⚫ Velmi vysoká | Export tabulek do SQLite nebo csv || 20 minut až přes 1 hodinu pro 1 milion souborů


Testovací vzorek / sada dat: 60 GB, 40 000 souborů, z toho jeden soubor 20 GB, a směs textů, obrázků a videí.

| Test | Vzorek  | SSD 2 TB (extrapolováno) | Pevný disk 2 TB (odhad) | Základ extrapolace |
|---------|:-:|:-:|:-:|---|
| Základní indexace | 00:00:04 | **~00:02:37** | ~00:05–08:00 | Počet souborů |
| Média základní | 00:01:23 | **~00:54:00** | ~02:00–03:00 | Počet souborů (pouze multimediální) |
| Média rozšířené | 00:01:52 | **~01:13:00** | ~03:00–04:00 | Počet souborů (náročnější u videa) |
| Úplné rozšířené | 00:01:23 | **~00:54:00** | ~02:00–03:00 | Počet souborů  |
| Pouze kontrolní součet | 00:09:20 | **~06:06:00** | ~20:00–30:00 | Celkem bajtů / propustnost |
| Kontrolní součet + Média základní | 00:11:06 | **~07:15:00** | ~22:00–33:00 | Bajty (převažují) + počet |


## Vývoj
Několik nápadů na rozvoj této obrazovky:
* Katalogizace cloudové instance (například Nextcloud).
* Přizpůsobení typů souborů.
* Více viz backlog [vývoje stránky Vytvořit](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=create).
