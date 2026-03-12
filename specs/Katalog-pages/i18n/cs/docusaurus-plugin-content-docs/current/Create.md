---
version: "2.10"
---
# Vytvořit
![2.10](https://img.shields.io/badge/Version-2.10-blue)

## Souhrn
Tato stránka popisuje všechny funkce obrazovky **Vytvořit** a jak je používat.<br/>
Na této obrazovce může uživatel **vytvořit katalog souborů**.<br/>
Provádí se ve 3 hlavních krocích:
1. Vybrat cestu ke zdroji: zařízení nebo adresář se soubory, které mají být zahrnuty do nového katalogu.
1. Vybrat možnosti pro zahrnutí nebo vyloučení některých souborů.
1. Vybrat [Úložiště](DevicesStorage), definovat název katalogu a vytvořit katalog.

![Přehled obrazovky Vytvořit](/img/screen_create_01.png)

## Vybrat cestu ke zdroji
Existují 3 způsoby, jak vybrat zdrojovou cestu k adresáři se soubory, které mají být zahrnuty do nového [Katalogu](DevicesCatalogs):
1. zadáním cesty do textového pole
1. pomocí stromového zobrazení systému souborů — stačí rozbalit a kliknout na správné zařízení nebo adresář
1. nebo kliknutím na tlačítko *Vybrat*, které otevře dialogové okno pro výběr složky.

Vybraná cesta se vždy zobrazí v textovém poli a aplikace ji použije k procházení a katalogizaci obsahu.

## Vybrat možnosti pro zahrnutí/vyloučení souborů {#select-options-to-includeexclude-files}
Zvolte, které typy souborů mají být zahrnuty do katalogu.

### Filtrování typů souborů {#enhanced-file-type-filtering}
Katalog používá inteligentní detekci typů souborů podle přípony, která automaticky podporuje stovky formátů.

**Jak detekce typů souborů funguje:**
- **Počáteční detekce**: typy souborů jsou určeny analýzou přípon pomocí inteligentní mezipaměti přípon sestavené z databáze MIME systému
- **Rozsáhlá podpora**: podporuje stovky formátů souborů
- **Perspektivní**: nové formáty jsou automaticky rozpoznány při aktualizaci systému

**Poznámka k přesnosti**:
- Typy souborů jsou při vytváření katalogu určeny z analýzy přípon, aby byla maximalizována rychlost indexování.
- Mohou však nastat chyby přípon nebo chybějící přípony
- Pro maximální přesnost je možné spustit ověření MIME na existujících katalozích přes obrazovku Zařízení a opravit soubory s zavádějícími příponami.

### Kategorie typů souborů
Obsah katalogu může být omezen na konkrétní typ souborů.
Tato možnost se bude vztahovat na katalog dopředu. Lze ji později změnit úpravou [Katalogu](DevicesCatalogs).

| Typ   | Popis | Definice | Příklady přípon |
| ------| ------|----------|-----------------|
| Vše   | Všechny typy souborů bez filtrování | | |
| Audio | Hudba, podcasty, zvukové nahrávky a zvukové soubory | (dle typů MIME) | MP3, FLAC, AAC, M4A, OGG, WAV, AIFF, Opus, WMA, MIDI, AMR (50+ formátů) |
| Obrázek | Fotografie, grafika, diagramy, ikony a vizuální obsah | (dle typů MIME) | JPG, PNG, HEIC, WebP, TIFF, RAW, SVG, XCF, GIF, BMP (100+ formátů) |
| Text  | Dokumenty, zdrojové soubory, značkovací jazyky, datové soubory a čitelný obsah | (specifická definice Katalog)<br/>Zahrnuje všechny soubory MIME začínající na „text/" a také aplikační soubory jako PDF, Word atd. | PDF, DOCX, ODT, Markdown, HTML, JSON, zdrojový kód, e-knihy (100+ formátů) |
| Video | Filmy, klipy, animace a video obsah | (dle typů MIME) | MP4, MKV, AVI, WebM, MOV, FLV, 3GP, OGV, M2TS (40+ formátů) |
| Ostatní | Všechny ostatní typy nepokryté výše uvedenými kategoriemi | (specifická definice Katalog) | ZIP, RAR, EXE, DLL, ISO, aplikační soubory neklasifikované jako Text |
| Žádný | Soubory, jejichž typ nelze určit podle přípony | Soubory bez přípony nebo s neznámými příponami | |

### Extrakce metadat {#metadata-extraction}
Zvolte, kolik metadat se má z vašich souborů extrahovat při vytváření katalogu.
<br/>To ovlivňuje rychlost katalogizace nebo velikost kolekce, ale poskytuje bohatší informace o souborech pro vyhledávání a statistiky.

**Dostupné možnosti:**
- **None**: žádná extrakce metadat (nejrychlejší katalogizace)
- **Media Basic**: extrakce základních metadat z obrázků, videí a zvukových souborů
- **Media Extended**: extrakce komplexních metadat včetně technických detailů
- **Full Extended**: maximální extrakce metadat pro všechny podporované typy souborů

**Co extrahuje Media Basic:**
- **Audio**: interpret, album, detaily stopy, délka, datový tok
- **Obrázky**: rozměry, orientace
- **Videa**: rozměry, délka, kodek a snímková frekvence

**Co extrahuje Extended:**
Tento mechanismus je postaven na knihovně KFileMetaData, která určuje podporované typy souborů a metadata.

**Vliv na výkon:**
- **None**: nejrychlejší možnost, vhodná pro velké adresáře nebo když metadata nejsou potřeba
- **Media Basic/Extended**: mírný dopad, zpracovává pouze multimediální soubory
- **Full Extended**: pomalejší, ale nejkompletnější — extrahuje ze všech podporovaných formátů

**Podporované typy souborů pro metadata:**
- **Obrázky**: jpg, png, gif, bmp, tiff, webp, svg, heic, raw, xcf
- **Videa**: mp4, mkv, avi, mov, wmv, flv, webm, m4v, mpg, 3gp, ogv, vob
- **Audio**: mp3, wav, flac, ogg, m4a, aac, wma, opus, aiff, mid, amr

Toto nastavení se vztahuje pouze na tento katalog a lze je později změnit úpravou [Katalogu](DevicesCatalogs).

**Poznámka:** Extrakce metadat vyžaduje čitelné soubory. Poškozené soubory nebo soubory s omezeným přístupem budou přeskočeny bez vlivu na proces katalogizace.

### Kontrolní součet souboru {#file-checksum}
Kontrolní součet SHA256 lze vypočítat během indexování pro vyhledávání duplicit nebo rozdílů.
⚠️ Je to výrazně delší proces než ostatní možnosti indexování, protože čte VŠECHNA data pro výpočet kontrolních součtů.
Stejně jako metadata lze tuto možnost vybrat při vytváření katalogu nebo změnit později; při přerušení procesu jsou již vypočítané kontrolní součty uloženy a příští aktualizace bude pokračovat pro zbývající soubory.
Kontrolní součty souborů lze použít jako možnost vyhledávání duplicit nebo vyhledávání rozdílů.

### Zahrnout skryté soubory {#other-options}
Skryté soubory nejsou ve výchozím nastavení zahrnuty, ale tato možnost je umožňuje zahrnout.<br/>
Tato možnost se bude vztahovat na katalog dopředu.<br/>
Lze ji později změnit úpravou [Katalogu](DevicesCatalogs).

### Panel Globální parametry

Panel *Globální parametry* sdružuje nastavení, která se vztahují na všechny katalogy. Lze jej sbalit nebo rozbalit pomocí přepínacího tlačítka v horní části panelu.

### Vyloučit adresáře (globálně) {#exclude-directories}

:::note
Tato vyloučení jsou **globální**: platí pro **všechny** katalogy, jak při vytváření nových katalogů, tak při aktualizaci stávajících.
:::

Z katalogizace je možné vyloučit adresáře, a to jak při vytváření, tak při aktualizacích.<br/>
Zadejte cestu nebo textový vzor a klikněte na *Přidat*.<br/>
Záznam se pak zobrazí v seznamu níže.<br/>
Libovolný záznam lze odebrat kliknutím pravým tlačítkem a výběrem *Odebrat*.<br/>

**Jak vyloučení funguje:**

Vyloučení používá **textovou shodu**: jakýkoli soubor nebo složka, jejichž celá cesta **obsahuje** text vyloučení, bude přeskočen. To znamená:

- **Celá cesta**: zadání `/home/user/Downloads/temp` vyloučí tento konkrétní adresář a veškerý jeho obsah.
- **Název složky**: zadání `node_modules` vyloučí **každý** adresář `node_modules` ve všech katalozích.
- **Částečná cesta**: zadání `.cache` vyloučí adresáře jako `/home/user/.cache/`, ale také `/home/user/.cachedata/`, protože shoda je založena na obsahu textu.

Shoda rozlišuje **velká a malá písmena**.

![Seznam globálně vyloučených adresářů s ukázkovými záznamy](/img/screen_create_04_exclude.png)

### Vyloučit složky (pro každý katalog)

Kromě globálních vyloučení je možné definovat vyloučené složky, které platí pouze pro katalog, který se právě vytváří.

- Zadejte cestu ke složce ručně nebo ji vyhledejte pomocí tlačítka *Vybrat*.
- Klikněte na *Přidat* pro přidání do čekajícího seznamu.
- Odeberte libovolný záznam kliknutím pravým tlačítkem a výběrem *Odebrat*.

Vyloučení pro každý katalog jsou uložena spolu s katalogem po dokončení jeho vytváření. Uplatňují se navíc ke globálním vyloučením — složka přeskočená libovolným pravidlem nebude indexována.

Tuto možnost lze později změnit úpravou [Katalogu](DevicesCatalogs).

## Definovat a vytvořit katalog
#### Vybrat úložné zařízení
Katalog musí být spojen s fyzickým zařízením [Úložiště](DevicesStorage), aby se usnadnilo pozdější vyhledávání nebo statistiky.<br/>
Ve výchozím nastavení Katalog předvytváří výchozí úložné zařízení, místní disk.<br/>
Toto lze později aktualizovat na obrazovce virtuálního stromu [Zařízení](DevicesTree).<br/>
Pokud pro tento katalog potřebujete jiné úložiště, klikněte na *Přidat úložiště* a přidejte jej pomocí obrazovek [Zařízení](DevicesTree) nebo [Úložiště](DevicesStorage).

Tato volba se bude vztahovat na katalog dopředu.<br/>
Lze ji později změnit úpravou [Katalogu](DevicesCatalogs).

#### Zadat název
Zadejte název katalogu.<br/>
Duplicitní názvy v současné době nejsou povoleny.

Tlačítko *Generovat* může vytvořit název na základě cesty ke složce a nahradit lomítka <code>/</code> podtržítkem <code>_</code>.

#### Vytvořit katalog
Až budete připraveni, klikněte na tlačítko *Vytvořit katalog* pro uložení katalogu a zahájení procesu katalogizace obsahu cesty rekurzivně (budou zahrnuty všechny podadresáře).

Jakmile je proces dokončen,
- Zpráva potvrdí vytvoření a poskytne počet souborů a celkovou velikost souboru vybrané složky pro tento katalog.
- váš místní disk (úložné zařízení přidané automaticky) byl také aktualizován a zpráva uvádí volné, použité a celkové místo:

![Potvrzení vytvoření katalogu s počtem souborů a přehledem úložného prostoru](/img/screen_create_02.png)

zobrazí se obrazovka [Zařízení](DevicesTree) pro zobrazení katalogu ve stromu zařízení.

Nový katalog je automaticky vybrán na panelu [Výběr](Selection) a připraven k použití pro [Vyhledávání](Search) obsahu.

## Průvodce výkonem

### Co ovlivňuje rychlost skenování?

#### 1. Extrakce metadat (největší dopad: ~10× zpomalení)
- Metadata obrázků: ~2–3 ms/soubor (čtení hlavičky)
- Metadata videí: ~5–15 ms/soubor (prohledávání a analýza kontejneru)
- Řešení: používat pouze „Media Basic", ne „Full Extended"

#### 2. Režim databáze
- Paměťový režim: rychlejší, používá RAM
- Souborový režim SQLite: pomalejší, omezený vstupem/výstupem

#### 3. Typ úložiště
- SSD: ~100 000 souborů/min
- HDD: ~20 000–30 000 souborů/min (fragmentace má vliv)
- Síťové úložiště: velmi proměnlivé

#### 4. Vyloučené složky
- Více vyloučení = rychlejší skenování
- Příklad: vyloučit .cache, node_modules atd.

#### 5. Zátěž systému
- Jiné náročné procesy mohou zasahovat

### Benchmarky výkonu

| Soubory | Úložiště | Metadata | Čas |
|---------|----------|----------|-----|
| 5 000 | SSD | None | 10 s |
| 5 000 | SSD | Basic | 50 s |
| 95 000 | HDD | Basic | 47 s (1. spuštění) / 10 s (z mezipaměti) |

## Vývoj
Některé nápady na vývoj této obrazovky:
* Přizpůsobení typů souborů a/nebo použití MIME typů
* Další informace najdete v nevyřízeném záznamu [vývoj Vytvořit](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=create).
