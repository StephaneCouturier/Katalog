# Vyhledávání
## Souhrn
Tato stránka popisuje všechny funkce obrazovky **Vyhledávání** a jak je používat.<br/>
Hlavní vlastnosti této obrazovky jsou:

* Vyhledávání souborů z katalogů nebo z připojených jednotek
* Použijte více **kritérií** k upřesnění a snížení počtu výsledků:
 * Vyhledávání bude ve výchozím nastavení filtrovat výsledky na základě textu, který jste zadali do řádku _Text_.
 * a lze jej upřesnit pomocí dalších [možností vyhledávacího textu](Search#search-text-criteria),
 * poskytnutí [atributů souboru](Search#file-criteria), jako je velikost nebo typ,
 * zvýraznění [duplicitních souborů](Search#duplicates-on),
 * nebo zvýrazněním [rozdílů](Search#differences-on) mezi 2 katalogy.
* Použijte **Výsledky** s [kontextovou nabídkou](Search#file-context-menu) (kliknutím pravým tlačítkem) nebo [Export výsledků](Search#batch-process),
* Znovu použijte předchozí kritéria z historie vyhledávání.
* Veškerá historie vyhledávání a kritéria jsou uložena a dostupná při příštím otevření Katalogu, kromě samotných výsledků.
![](/img/screen_search_01.png)
## Vyhledávejte z katalogů nebo z připojených jednotek
Zatímco Katalog je speciálně navržen pro správu katalogů souborů pro vyhledávání offline, je stále možné vyhledávat přímo na připojených (online) discích.
V levém panelu je záložka "Filtry" rozdělena na 2 části, aktivovat lze pouze jednu:

### Vyhledávání v katalozích souborů
Použijte 3 volitelné filtry k omezení seznamu katalogů, ve kterých se mají hledat soubory:
- _Umístění_: toto je seznam odlišných umístění zadaných na obrazovce "Úložiště". Výběrem jednoho umístění se seznam úložných zařízení a katalogů zmenší na odpovídající.
- _Storage_: toto je seznam různých úložných zařízení, jak je zadáno na obrazovce "Storage". Výběrem jednoho úložného zařízení se seznam zmenší na odpovídající katalogy.
- _Katalogy_: toto zužuje vyhledávání na 1 konkrétní katalog.

### Vyhledávání v připojených jednotkách
Vyhledávejte přímo v počítači a připojených jednotkách.
To může ušetřit čas na hledání v konkrétní složce, nikoli v celé jednotce nebo katalogu, nebo to může pomoci získat rychlý náhled po nedávných změnách, aniž byste museli katalog aktualizovat.

## Kritéria vyhledávání
Tento panel seskupuje všechna kritéria, která mohou zpřesnit a snížit počet výsledků. Může být skrytý.<br/>
Vztahují se na cestu a názvy souborů, informace o souborech a značky a mohou identifikovat duplikáty nebo pomoci vidět rozdíly mezi dvěma katalogy<br/>
Výsledky mohou být seznam souborů nebo seznam složek obsahujících odpovídající soubory.<br/>
![](/img/screen_search_02.png)
### Kritéria vyhledávacího textu {#search-text-criteria}
#### Název souboru
Tato oblast je místo pro zadání textu, který se má použít pro hledání názvu souboru, který může zahrnovat cestu k souboru.

Definice: Zde je "slovo" skupina znaků oddělená od jiné skupiny mezerou.<br/>
To lze použít k vyhledání složek, souborů nebo souborů v určitých složkách.<br/>
Ve výchozím nastavení je toto jediné vybrané kritérium<br/>
Všechna kritéria lze vymazat/resetovat pomocí tlačítka Reset.

Pro usnadnění vyhledávání klíčových slov z kopírovaného textu:
- Tento text lze vložit ze schránky,
- a lze jej vyčistit od speciálních znaků: odstraněno touto funkcí: . , _ - ( ) [ ] { }

Je-li k dispozici, kliknutím na zaškrtávací políčko umístěné před kritériem povolíte/zakážete kritéria, aniž byste ztratili vybrané hodnoty.<br/>

#### S
Určete, jak mají být slova zadaná v "Text" použita:
* _Všechna slova_: soubor nebo složka je vrácena pouze v případě, že jsou nalezena všechna slova v textu.
* _Začít na_: pouze u volby "Názvy souborů" musí název souboru začínat textem (včetně mezer mezi slovy).
* _Jakékoli slovo_: soubor nebo složka je vrácena, pokud je nalezeno alespoň jedno ze slov v textu.

#### V
Určete, ve které části absolutní cesty k souboru má být použit Text.
* _Pouze názvy souborů_: textová slova budou použita pouze k vyhledání názvů souborů.
* _Pouze cesta ke složce_: textová slova budou použita pouze k nahlédnutí do cest ke složce.
* _Názvy složek a souborů_: textová slova budou použita k nahlédnutí do cest složek a názvů souborů.

#### citlivý na velká písmena
Vynutit vyhledávání, aby odpovídalo přesným znakům (malá nebo velká písmena).

#### vyloučit
Na rozdíl od "Text" to vyloučí výsledky, pokud je v cestě nebo názvu souboru nalezeno _jakékoli_ z poskytnutých slov.

### Kritéria souboru {#file-criteria}

#### Velikost
Nastavte rozsah velikosti souboru zadáním čísla a jednotky pro minimální a maximální velikost souboru.

#### Typy souborů
 | Typ | Rozšíření |
 | ------| -------------------------------------------------- |
 | Audio | aif, mp3, ogg, wav |
 | Obrázek | png, jpg, gif, xcf, tif, bmp, raw |
 | Text | txt, pdf, odt, idx, html, rtf, doc, docx, epub |
 | Video | wmv, avi, mp4, mkv, flv, webm, m4v, vob, ogv, mov |

#### Termíny
Nastavte rozsah data změny souboru.

#### Duplikáty zapnuty {#duplicates-on}
Možnost upřesnit výsledky a zobrazit pouze soubory, které by mohly být duplicitní.<br/>
Pojem duplikáty lze definovat jako jednu nebo jako kombinaci několika částí:
název, velikost, datum změny.
Alespoň jedna z těchto možností je nezbytná pro zvážení toho, co by mohly být duplikáty.

#### Rozdíly na {#differences-on}
Najděte rozdíly mezi libovolnými 2 zařízeními (virtuální, úložiště, katalogy) (užitečné pro srovnání například se zálohou).<br/>
Pokud jde o duplikáty, kombinuje se s dalšími kritérii vyhledávání a pojem rozdílu lze použít na název, velikost nebo datum změny.
![](/img/screen_search_03_diff.png)

### Kritéria složky

#### Zobrazit pouze složky
Možnost zobrazit složky pouze jako výsledky místo souborů.

#### Tagy
Možnost upřesnit výsledky na vybrané _Tag_.
Tagy jsou definovány na obrazovce _Tags_. V současné době lze označit pouze složky.

## Výsledek
V této oblasti se zobrazí výsledky vyhledávání.<br/>

### Katalogy s výsledky
Levý panel zobrazuje seznam katalogů, ve kterých byly nalezeny výsledky.<br/>
Kliknutí na jeden z těchto katalogů spustí stejné vyhledávání, ale upřesní u vybraného katalogu.<br/>
Tento panel lze skrýt, abyste ušetřili místo a zobrazili více informací o výsledcích souboru.<br/>

### Nalezeny soubory (nebo duplikáty).
Pravý panel zobrazuje seznam souborů nebo složek, které byly nalezeny podle kritérií vyhledávání.<br/>
V horní části je uveden počet nalezených souborů nebo počet duplikátů (počet jedinečných výsledků) a celková velikost nalezených souborů. <br/>
Ikona dále otevře okno s dalšími statistikami o výsledcích:<br/>
Celková velikost, min., max., průměrná velikost, min., max. datum.<br/><br/>

### Dávkový proces {#batch-process}
| Případ | Vstup do menu |
| ------------------| --------------------|
| Exportovat výsledky | Exportovat výsledky do nového katalogu (se kterým lze provést přesnější vyhledávání) nebo exportovat do souboru csv, pojmenovaného s datem a umístěného ve [složce kolekce] (Settings#collection). |
| Přejmenovat (KRename) | Otevřete všechny soubory uvedené ve výsledcích pomocí [KRename](https://apps.kde.org/krename/) |
| &#9888; Přesunout do koše | přesunout všechny soubory uvedené ve výsledcích do koše. |
| &#9888; Smazat | odstranit všechny soubory uvedené ve výsledcích (bez obnovení).|

### Kontextová nabídka Soubor {#file-context-menu}
Kliknutím pravým tlačítkem na řádek výsledku se otevře kontextová nabídka s následujícími možnostmi:
* _Otevřít soubor_: k otevření souboru bude použita výchozí aplikace v počítači, pokud je dostupná/online.
* _Otevřít složku_: k otevření složky a zobrazení jejího obsahu bude použit výchozí správce souborů v počítači, pokud je dostupný/online.

Zkopírujte do schránky některé informace o souboru, s příkladem úplné cesty k souboru: _/home/user/documents/filename.txt_
* _Kopírovat cestu ke složce_: _/home/user/documents_
* _Kopírovat soubor absolutní cesta_: _/home/user/documents/filename.txt_
* _Kopírovat název souboru s příponou_: _název_souboru.txt_
* _Kopírovat název souboru bez přípony_: _název_souboru_

Operace se soubory:
* _Přesunout do koše_
* _Smazat soubor_

## Historie vyhledávání
Při každém spuštění vyhledávání se kritéria vyhledávání a vybrané katalogy uloží do souboru historie umístěného ve složce Collection.<br/>
Tento soubor je poté načten a zobrazen v této tabulce.<br/>
Kliknutím na řádek tabulky obnovíte všechny hodnoty kritérií do Kritéria vyhledávání a umožníte znovu spustit přesně stejné vyhledávání jako dříve nebo usnadní některé úpravy.<br/>
Tento panel lze skrýt, abyste ušetřili místo a zobrazili více informací o výsledcích souborů.<br/>
![](/img/screen_search_04_search_history.png)
