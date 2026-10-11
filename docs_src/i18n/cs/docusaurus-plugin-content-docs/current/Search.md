---
version: "3.0"
---
# Vyhledávání
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Shrnutí
Tato stránka popisuje stránku **Vyhledávání**: kritéria vyhledávání, spuštění vyhledávání a historii vyhledávání.

* Vyhledávejte soubory v **katalozích** vybraného zařízení nebo přímo na **připojeném disku**.
* Kombinujte **kritéria** pro zúžení výsledků:
  * [název souboru](#search-text-criteria), s jedním nebo více hledanými texty,
  * [atributy souboru](#file-criteria): typ, velikost a datum,
  * [metadata souboru](#file-metadata-criteria): text, rozměry obrázku nebo videa, délka,
  * [kritéria složky](#folder-criteria): zobrazit pouze složky nebo pouze složky se štítkem,
  * hledání [duplikátů](#duplicates-on) nebo [rozdílů](#differences-on) mezi 2 zařízeními.
* Znovu použijte kritéria předchozího vyhledávání z [historie](#search-history).

Výsledky se otevírají na stránce **Výsledky**, vpravo od stránky Vyhledávání: viz [Výsledky vyhledávání](SearchResults).

![Stránka Vyhledávání se slovem „sunset“ v kritériích názvu souboru, vedle stránky Výsledky](/img/search_01_overview.png)

## Kde vyhledávat {#search-source}

| Možnost | Popis |
|---------|-------|
| *Katalogy* | Vyhledává v katalozích zařízení vybraného na stránce [Výběr](Selection): v katalogu, ve všech katalozích úložného zařízení nebo virtuálního zařízení. Když není vybráno žádné zařízení (*Všechno*), vyhledává ve všech katalozích. |
| *Připojený disk* | Vyhledává přímo ve složce počítače nebo připojeného disku, bez katalogu. Zadejte cestu nebo použijte *Vyberte cestu*. Užitečné pro vyhledávání ve složce, která nemá katalog, nebo pro zobrazení nedávných změn bez předchozí aktualizace katalogu. |

## Kritéria vyhledávání
Kritéria jsou seskupena do sekcí. Zaškrtávací políčko před nadpisem sekce zapne nebo vypne celou sekci, **aniž by se ztratilo, co v ní bylo zadáno**. Totéž platí pro zaškrtávací políčka před jednotlivým kritériem (*Typ*, *Velikost*, *Datum*...).

### Název souboru {#search-text-criteria}

![Kritéria názvu souboru se dvěma hledanými texty a jedním vylučovacím textem](/img/search_02_file_name.png)

#### text
Hledaný text. Může jít o **jeden nebo více hledaných textů**, jeden na řádek:
* *+* přidá řádek, *−* ho odebere (poslední řádek odebrat nelze),
* soubor je nalezen, když odpovídá **kterémukoli** z řádků,
* křížek v řádku ho vyprázdní.

Tlačítka vedle řádků:
* *Vložte text ze schránky*: nahradí řádky obsahem schránky, jeden řádek na každý řádek textu. Užitečné pro hledání seznamu zkopírovaného z tabulkového procesoru, textového editoru nebo správce souborů.
* *Vyčistěte hledaný text od znaků jako _ - . ,*: nahradí tyto znaky mezerami v každém řádku.

Vložení více řádků pomocí `Ctrl+V` do řádku je přidá jako nové řádky. `Enter` v kterémkoli řádku spustí vyhledávání.

#### s
Jak se hledají slova každého řádku:

| Možnost | Chování |
|---------|---------|
| *Všechna slova* | Nalezena jsou všechna slova řádku, v libovolném pořadí (výchozí). |
| *Přesná fráze* | Řádek je nalezen tak, jak byl zadán, slova ve stejném pořadí. |
| *Začíná s* | Název souboru začíná řádkem. Pouze s volbou *Pouze názvy souborů*. |
| *Jakékoli slovo* | Nalezeno je alespoň jedno slovo řádku. |
| *Regex* | Řádek je regulární výraz (syntaxe PCRE2). Neplatný výraz nenajde nic. |

„Slovo“ je skupina znaků oddělená od jiné mezerou.

#### v
Kde se text hledá:

| Možnost | Chování |
|---------|---------|
| *Pouze názvy souborů* | V názvech souborů (výchozí). |
| *Názvy souborů nebo cesty ke složkám* | V názvech souborů a v cestách ke složkám. |
| *Pouze cesta ke složce* | Pouze v cestách ke složkám. |

#### citlivý na velká písmena
Velká a malá písmena se považují za odlišná.

#### vyloučit
Soubory jsou z výsledků vynechány, když odpovídají **kterémukoli** z vylučovacích řádků. Vylučovací řádky fungují jako řádky vyhledávání: *+*, *−*, vložení a vyčištění.

### Atributy souboru {#file-criteria}

#### Typ
Pouze soubory jednoho typu. Když *Typ* není zaškrtnut, prohledávají se všechny typy.

| Typ | Popis |
|-----|-------|
| *Všechno* | Bez filtru typu. |
| *Zvuk* | Zvukové soubory (mp3, ogg, wav, flac...). |
| *Obraz* | Obrázkové soubory (jpg, png, gif, raw...). |
| *Text* | Dokumenty a textové soubory (pdf, docx, odt, epub...). |
| *Video* | Video soubory (mp4, mkv, avi, mov...). |
| *Jiné* | Soubory žádného z výše uvedených typů. |
| *Žádné* | Soubory bez rozpoznaného typu. |

Typy souborů se rozpoznávají podle systémové databáze typů souborů, takže přesný seznam přípon se řídí systémem.

#### Velikost
Minimální a maximální velikost, každá se svou jednotkou: Bajty, KiB, MiB, GiB nebo TiB.

#### Datum
Minimální a maximální datum změny. *Vyberte datum* otevře kalendář se zkratkami: *Nyní*, *Před 1 dnem*, *Před 1 týdnem*, *Před 1 měsícem*, *Před 1 rokem*.

### Metadata souboru {#file-metadata-criteria}
Kritéria pro informace uložené uvnitř souborů. Vyžadují katalogy vytvořené nebo aktualizované s volbou *Zahrnout metadata* (viz [Vytvořit](Create)).

| Kritérium | Popis |
|-----------|-------|
| *Text* | Text v metadatech, například umělec, album, název nebo autor. |
| *Velikost* | *Šířka* a *Výška* v pixelech, každá s minimem a maximem (obrázky a videa). |
| *Délka* | Minimální a maximální délka ve formátu hodiny:minuty:sekundy (zvuk a video). |

### Kritéria složky {#folder-criteria}

#### ve výsledcích zobrazí pouze složky
Výsledkem jsou složky, které obsahují odpovídající soubory, místo samotných souborů.

#### Štítek {#tags-criteria}
Pouze soubory ve složkách, které mají tento štítek. Štítky se přiřazují na stránce [Štítky](Tags).

### Duplikáty {#duplicates-on}
Najde soubory, které jsou pravděpodobně tentýž soubor na více místech.

*Na*: co musí mít dva soubory společné, jedno nebo více z:

| Pole | Popis |
|------|-------|
| *Název* | Stejný název souboru. |
| *Velikost* | Stejná velikost. |
| *Datum* | Stejné datum změny. |
| *Kontrolní součet* | Se `=` stejný kontrolní součet: obsah je totožný. Se `≠` odlišný kontrolní součet: stejný název nebo velikost, ale jiný obsah. |

Kontrolní součty jsou k dispozici pouze pro katalogy vytvořené nebo aktualizované s volbou *Zahrnout kontrolní součet* (viz [Vytvořit](Create)).

*Rozsah*:
* *V rámci vybraného zařízení*: duplikáty uvnitř vybraného zařízení,
* *Porovnat dvě zařízení*: duplikáty mezi *Zařízení 1* a *Zařízení 2*, například mezi počítačem a starým notebookem.

### Rozdíly {#differences-on}
Porovná dvě zařízení, *Mezi* a *A*, například složku a její zálohu. Najde soubory, které jsou jen v jednom z nich, nebo jsou v obou, ale s odlišnou hodnotou v polích zvolených v *Na*: *Název*, *Velikost*, *Datum*, *Kontrolní součet* (`=` nebo `≠`, jako u Duplikátů).

Rozdíly jsou k dispozici pouze při vyhledávání v *Katalogy*.

:::note
*Duplikáty* a *Rozdíly* nelze používat současně.
:::

## Spuštění vyhledávání {#running}
Tlačítka v horní části stránky:

| Tlačítko | Akce |
|----------|------|
| *Vyhledávání* | Spustí vyhledávání. Totéž udělá `Enter` v textovém poli. |
| *Pauza* / *Obnovit* | Pozastaví probíhající vyhledávání a obnoví ho. |
| *Zastavit* | Zastaví probíhající vyhledávání. Dosud nalezené výsledky se zachovají. |
| *Resetovat* | Vrátí všechna kritéria na výchozí hodnoty. |
| *Historie* | Otevře [historii vyhledávání](#search-history). |
| *Zavřít* | Zavře stránku Vyhledávání. |

Průběh vyhledávání se zobrazuje v [panelu činnosti](Interface#activity-panel).<br/>
Vyhledávání nelze spustit, dokud se vytváří katalog nebo aktualizuje zařízení: *Vyhledávání* je mezitím neaktivní.

## Historie vyhledávání {#search-history}

![Historie vyhledávání: minulá vyhledávání s datem, zařízením a kritérii](/img/search_03_history.png)

Každé vyhledávání se ukládá do historie kolekce, s kritérii a zařízením, ve kterém bylo spuštěno.

*Historie* otevře seznam minulých vyhledávání. Každý řádek ukazuje datum, prohledané zařízení (nebo *Všechno*) a shrnutí kritérií.<br/>
Kliknutí na řádek **obnoví všechna jeho kritéria a vybrané zařízení**. Klikněte na *Vyhledávání* pro opětovné spuštění.

| Tlačítko | Akce |
|----------|------|
| *Ponechat posledních 10* | Po potvrzení smaže všechna vyhledávání kromě 10 nejnovějších. |
| *Vymazat* | Po potvrzení smaže celou historii. |
| *Zrušit* | Zavře historii. |

## Vývoj
* Viz backlog [vývoje Vyhledávání](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=search).
