---
version: "2.11"
---
# Vyhledávání
![2.11](https://img.shields.io/badge/Version-2.11-blue)

## Shrnutí
Tato stránka popisuje všechny funkce obrazovky **Vyhledávání** a jak je používat.

Hlavní funkce této obrazovky jsou:

* Vyhledávání souborů z katalogů nebo z připojených jednotek
* Použití více **kritérií** k upřesnění a snížení počtu výsledků:
  * filtrovat podle [textu v názvu souboru nebo cestě](Search#search-text-criteria),
  * filtrovat podle [atributů souboru](Search#file-criteria), jako je velikost, typ nebo datum,
  * filtrovat podle [metadat souboru](Search#file-metadata-criteria), jako jsou rozměry obrázku nebo délka audia/videa,
  * zvýraznit [duplicitní soubory](Search#duplicates-on),
  * nebo zvýraznit [rozdíly](Search#differences-on) mezi 2 katalogy.
* Použít **Výsledky** s [kontextovou nabídkou](Search#file-context-menu) (kliknutí pravým tlačítkem) nebo [dávkově zpracovat výsledky](Search#batch-process),
* Znovu použít předchozí kritéria z historie vyhledávání.
* Všechna kritéria vyhledávání jsou uložena a dostupná při příštím otevření Katalogu, kromě samotných výsledků.

![Obrazovka Vyhledávání zobrazující panel kritérií vlevo a výsledky vpravo](/img/screen_search_01.png)

## Zdroj vyhledávání

### Vyhledávání v katalozích souborů
Panel **[Výběr](Selection)** na levé straně poskytuje volitelné filtry pro omezení katalogů, ve kterých se má hledat:
- *Umístění*: omezí katalogy na ty odpovídající vybranému umístění definovanému na obrazovce [Úložiště](DevicesStorage).
- *Úložiště*: omezí katalogy na ty pod konkrétním úložným zařízením.
- *Katalog*: zúží vyhledávání na jediný konkrétní katalog.

### Vyhledávání v připojených jednotkách
Vyhledávejte přímo v počítači a na připojených jednotkách bez potřeby katalogu.
To je užitečné pro hledání v konkrétní složce nebo pro rychlý přehled po nedávných změnách bez nutnosti aktualizace katalogu.

## Kritéria vyhledávání
Tento panel seskupuje všechna kritéria, která upřesňují a snižují počet výsledků. Panel lze skrýt, aby se ušetřilo místo.

Vztahují se na cesty a názvy souborů, atributy souborů, metadata, štítky a mohou identifikovat duplikáty nebo rozdíly mezi 2 katalogy.
Výsledky mohou být seznam souborů nebo seznam složek obsahujících odpovídající soubory.

![Panel kritérií vyhledávání zobrazující sekce textu, souboru, metadat, duplikátů a rozdílů](/img/screen_search_02.png)

### Kritéria textu {#search-text-criteria}

#### Název souboru
Zadat text pro vyhledávání v názvech souborů a/nebo cestách složek.

„Slovo" je skupina znaků oddělená od jiné skupiny mezerou. To lze použít k vyhledání složek, souborů nebo souborů v určitých složkách.

Tlačítka vedle textového pole:
- *Vložit ze schránky* — vloží obsah schránky do vyhledávacího pole.
- *Vyčistit text vyhledávání* — odstraní speciální znaky (`.  ,  _  -  (  )  [  ]  {  }  /  \  '  "`).

Pokud je před kritériem zaškrtávací políčko, lze ho zapnout nebo vypnout bez ztráty zadané hodnoty.

#### S
Určuje, jak mají být slova v poli *Text* porovnávána:

| Možnost | Chování |
|---------|---------|
| *Všechna slova* | Vrátí výsledky pouze pokud jsou nalezena všechna slova (výchozí) |
| *Přesná fráze* | Vrátí výsledky, kde je nalezena přesná fráze (včetně pořadí slov a mezer) |
| *Začíná na* | Název souboru musí začínat textem — dostupné pouze s *Pouze názvy souborů* |
| *Jakékoli slovo* | Vrátí výsledky, pokud je nalezeno alespoň jedno ze slov |

#### V
Určuje, ve které části cesty souboru se má hledat:

| Možnost | Chování |
|---------|---------|
| *Pouze názvy souborů* | Hledá pouze v názvech souborů (výchozí) |
| *Názvy souborů nebo cesty složek* | Hledá v názvech souborů i cestách složek |
| *Pouze cesta složky* | Hledá pouze v cestách složek (nedostupné s *Začíná na*) |

#### Rozlišovat velikost písmen
Vynutí přesnou shodu znaků (velká a malá písmena jsou rozlišována).

#### Vyloučit
Vyloučí výsledky, pokud je v cestě nebo názvu souboru nalezeno *jakékoli* z uvedených slov.

### Kritéria souboru {#file-criteria}

Celou sekci kritérií souborů lze zapnout nebo vypnout pomocí zaškrtávacího políčka.

#### Velikost
Nastavte minimální a/nebo maximální velikost souboru. Každá hranice přijímá číslo a jednotku.

Dostupné jednotky: **Bajty**, **KiB**, **MiB**, **GiB**, **TiB**

#### Typ souboru
Filtruje výsledky podle typu souboru. Dostupné typy:

| Typ | Popis |
|-----|-------|
| Vše | Bez filtru typu (výchozí) |
| Audio | Zvukové soubory (např. mp3, ogg, wav, flac…) |
| Obrázek | Obrazové soubory (např. jpg, png, gif, raw…) |
| Text | Dokumenty a textové soubory (např. pdf, docx, odt, epub…) |
| Video | Video soubory (např. mp4, mkv, avi, mov…) |
| Ostatní | Soubory neodpovídající žádnému z výše uvedených typů |
| Žádný | Soubory bez rozpoznaného typu |

Typy souborů jsou dynamicky detekovány z databáze MIME systému (KFileMetadata), takže přesný seznam přípon se přizpůsobuje systému.

#### Datum
Nastavte minimální a/nebo maximální datum změny souboru.

### Kritéria metadat souboru {#file-metadata-criteria}

Sekce metadat umožňuje vyhledávání v obsahu vloženém do souborů (vyžaduje, aby byl katalog indexován s rozšířenými metadaty).

![Sekce kritérií metadat zobrazující pole pro text, rozměry obrázku a délku](/img/screen_search_metadata_criteria.png)

Celou sekci metadat lze zapnout nebo vypnout pomocí zaškrtávacího políčka.

#### Text metadat
Vyhledávání textu v polích metadat, jako je jméno interpreta, album, název, autor nebo jiný vložený textový obsah.

#### Rozměry metadat (obrázky a videa)
Filtrovat podle rozměrů v pixelech:
- *Min / Max výška* — rozsah výšky v pixelech
- *Min / Max šířka* — rozsah šířky v pixelech

#### Délka metadat (audio a video)
Filtrovat podle délky přehrávání pomocí časového rozsahu.

### Kritéria složky

#### Zobrazit pouze složky
Zobrazí cesty složek jako výsledky místo jednotlivých souborů.

:::note
*Zobrazit pouze složky* nelze kombinovat s *Najít rozdíly*.
:::

#### Štítky {#tags-criteria}
Filtruje výsledky na soubory nebo složky, kterým je přiřazen konkrétní štítek.
Štítky jsou definovány na obrazovce [Štítky](Tags).

### Duplikáty {#duplicates-on}

Najde potenciálně duplicitní soubory na základě jednoho nebo kombinace:

| Pole | Popis |
|------|-------|
| *Název souboru* | Stejný název souboru |
| *Velikost souboru* | Stejná velikost souboru |
| *Datum změny* | Stejné datum změny |
| *Kontrolní součet (SHA-256)* | Stejná nebo odlišná hodnota kontrolního součtu |

**Operátor kontrolního součtu** (= nebo ≠) umožňuje najít soubory se *stejným* kontrolním součtem (skutečné duplikáty) nebo soubory s *odlišnými* kontrolními součty přes jinak shodné atributy.

**Možnosti rozsahu:**
- *V rámci vybraného zařízení/katalogu* — najde duplikáty uvnitř aktuálně vybraného zařízení nebo katalogu.
- *Porovnat dvě zařízení* — porovná soubory mezi dvěma vybranými zařízeními nebo katalogy pro nalezení duplikátů mezi nimi.

:::note
Pro spuštění vyhledávání duplikátů musí být vybráno alespoň jedno pole.
:::

### Rozdíly {#differences-on}

Najde soubory přítomné v jednom zařízení, ale ne v druhém, nebo se soubory s odlišnými hodnotami atributů — užitečné pro porovnání zdroje se zálohou.

![Sekce kritérií rozdílů zobrazující výběrové prvky zařízení a zaškrtávací políčka atributů](/img/screen_search_03_diff.png)

Vyberte dvě zařízení (Virtuální, Úložiště nebo Katalog) k porovnání a zvolte, které atributy definují rozdíl:

| Pole | Popis |
|------|-------|
| *Název souboru* | Soubory s odlišnými názvy |
| *Velikost souboru* | Soubory s odlišnými velikostmi |
| *Datum změny* | Soubory s odlišnými daty změny |
| *Kontrolní součet (SHA-256)* | Soubory se stejným nebo odlišným kontrolním součtem |

**Operátor kontrolního součtu** (= nebo ≠) funguje stejně jako u Duplikátů.

:::note
Musí být vybráno alespoň jedno pole. *Najít rozdíly* nelze kombinovat s *Zobrazit pouze složky* ani s *Najít duplikáty*.
:::

## Spuštění vyhledávání

### Tlačítko Vyhledat
Kliknutím na *Vyhledat* spustíte vyhledávání. Tlačítko mění stav podle operace:

| Stav | Popisek tlačítka | Tlačítko Zastavit |
|------|-----------------|-------------------|
| Nečinný | *Vyhledat* (zelené) | Zakázáno |
| Probíhá | *Pauza* | Povoleno |
| Pozastaveno | *Pokračovat* | Povoleno |

![Stavový řádek zobrazující stav pauzy během operace vyhledávání](/img/screen_search_statusbar_paused.png)

- **Pauza / Pokračovat** — pozastavit a pokračovat ve vyhledávání bez ztráty průběhu. Není dostupné v paměťovém databázovém režimu.
- **Zastavit** — zrušit aktuální vyhledávání. Zobrazí se částečné výsledky, které již byly nalezeny.

### Resetovat
Tlačítko *Resetovat vše* obnoví všechna kritéria na výchozí hodnoty.

## Výsledky

### Katalogy s výsledky
Levý panel zobrazuje seznam katalogů, ve kterých byly nalezeny odpovídající soubory.
Kliknutím na katalog v tomto seznamu se vyhledávání spustí znovu, ale omezeno pouze na tento katalog.
Tento panel lze skrýt, aby se ušetřilo místo.

### Nalezené soubory
Pravý panel zobrazuje soubory nebo složky odpovídající kritériím vyhledávání.

Záhlaví ukazuje počet nalezených souborů nebo duplikátů a celkovou velikost.
Kliknutím na ikonu statistiky se otevře **dialog s podrobnými statistikami**:

| Statistika | Popis |
|------------|-------|
| Nalezené soubory | Celkový počet odpovídajících výsledků |
| Zpracované soubory | Celkový počet zkoumaných souborů |
| Dokončení | Procento zpracovaných souborů (pokud bylo vyhledávání zastaveno, výsledky jsou označeny jako neúplné) |
| Celková velikost | Součet velikostí souborů |
| Min / Max / Průměrná velikost | Distribuce velikostí |
| Min / Max datum | Rozsah dat výsledků |
| Zpracované katalogy | Počet prohledaných katalogů z celkového počtu |

### Kontextová nabídka {#file-context-menu}

Kliknutím pravým tlačítkem na řádek výsledku se otevře kontextová nabídka:

**Navigace:**
| Akce | Popis |
|------|-------|
| *Otevřít soubor* | Otevře soubor výchozí aplikací systému |
| *Otevřít složku* | Otevře nadřazenou složku souboru ve správci souborů |
| *Prozkoumat složku* | Přejde do složky souboru na obrazovce [Prozkoumat](Explore) v Katalogu |

**Metadata:**
| Akce | Popis |
|------|-------|
| *Zobrazit rozšířená metadata (JSON)* | Zobrazí vložená metadata souboru (dostupné pokud byl katalog indexován s rozšířenými metadaty, nebo pokud typ souboru je podporuje) |

**Kopírovat do schránky:** *(příklad: `/home/uzivatel/dokumenty/soubor.txt`)*
| Akce | Kopírovaná hodnota |
|------|------------------|
| *Kopírovat cestu složky* | `/home/uzivatel/dokumenty` |
| *Kopírovat absolutní cestu souboru* | `/home/uzivatel/dokumenty/soubor.txt` |
| *Kopírovat název souboru s příponou* | `soubor.txt` |
| *Kopírovat název souboru bez přípony* | `soubor` |

**Kontrolní součet:**
| Akce | Podmínka | Popis |
|------|----------|-------|
| *Vypočítat kontrolní součet (SHA-256)* | Žádný kontrolní součet uložen | Vypočítá a uloží SHA-256 hash |
| *Kopírovat kontrolní součet* | Kontrolní součet uložen | Zkopíruje hash do schránky |
| *Ověřit kontrolní součet (SHA-256)* | Kontrolní součet uložen | Přepočítá a porovná s uloženou hodnotou |

**Operace se soubory:**
| Akce | Popis |
|------|-------|
| *Přesunout do koše* | Přesune soubor do systémového koše (s potvrzením) |
| *Smazat soubor* | Trvale smaže soubor (s potvrzením) |

### Dávkové zpracování {#batch-process}

Tlačítko *Zpracovat výsledky* otevře nabídku dávkových operací aplikovaných na všechny soubory ve výsledcích:

| Akce | Popis |
|------|-------|
| *Exportovat výsledky* | Exportovat do nového katalogu (pro další přesnější vyhledávání) **nebo** do souboru CSV pojmenovaného s datem a uloženého ve [složce Kolekce](Settings#database-memory-mode) |
| *Přejmenovat (KRename)* | Otevře všechny soubory výsledků v [KRename](https://apps.kde.org/krename/) pro dávkové přejmenování |
| *Ověřit kontrolní součty* | Pro každý soubor výsledků: pokud není kontrolní součet dosud uložen, vypočítá a uloží hash SHA-256; pokud je kontrolní součet již uložen, porovná skutečnou hodnotu s uloženou (shoda / neshoda). Průběh je zobrazen ve stavovém řádku. |
| *Zahrnout metadata* | Extrahuje rozšířená metadata (rozměry obrázků, délka audia/videa atd.) pro každý soubor výsledků a uloží je do katalogu. Užitečné pro soubory původně indexované bez metadat. Průběh je zobrazen ve stavovém řádku. |
| ⚠ *Přesunout do koše* | Přesune všechny soubory výsledků do systémového koše (s potvrzením zobrazujícím počet a celkovou velikost) |
| ⚠ *Smazat* | Trvale smaže všechny soubory výsledků (s potvrzením — bez možnosti obnovy) |

## Historie vyhledávání
Pokaždé, když je spuštěno vyhledávání, jsou kritéria a výběr zařízení uloženy do souboru historie ve složce Kolekce.

Tato historie je zobrazena v tabulce ve spodní části obrazovky. Kliknutím na řádek se obnoví všechna kritéria a okamžitě se spustí toto vyhledávání znovu.

Tento panel lze skrýt, aby se ušetřilo místo.

![Panel historie vyhledávání zobrazující předchozí vyhledávání s časovými razítky a kritérii](/img/screen_search_04_search_history.png)

## Vývoj
Některé nápady na vývoj této obrazovky:
* Podívejte se na nevyřízené položky [vývoje Vyhledávání](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=search).
