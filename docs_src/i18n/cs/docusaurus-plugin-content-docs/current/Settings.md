---
version: "3.0"
---
# Nastavení
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Shrnutí
Tato stránka popisuje všechny funkce obrazovky **Nastavení** a jak je používat. Obrazovka má dvě karty:
* *Aplikace*: [verze, jazyk, téma, velikost textu, velikost okna a možnosti chování](#application),
* *Sbírka*:
  * [Sbírka a databáze](#collection-database): otevřená kolekce, její datový režim, [export do jiného datového režimu](#export), verze databáze a kontrola kvality a složka obrázků,
  * [Import a synchronizace sbírky](#import-update): import zařízení z jiné kolekce nebo aktualizace dříve importovaných zařízení.

*Nastavení* se otevírá z postranního panelu, nad ostatními stránkami, na kartě *Aplikace*. *Otevřít... > Hostovaná databáze...* v postranním panelu je otevře na kartě *Sbírka* s formulářem pro hostované připojení. *Zavřít* (nebo `Esc`) se vrátí k ostatním stránkám (viz [Rozhraní](Interface)).

## Aplikace {#application}

![Karta Aplikace: verze, jazyk, téma, velikost textu, obnovení velikosti okna a možnosti chování](/img/settings_05_application.png)

### Verze
* Verze a datum vydání Katalogu a *Poznámky k vydání*, kde si lze přečíst, co se změnilo.
* *Zkontrolovat novou verzi při spuštění*: je-li zapnuto, Katalog při spuštění zkontroluje, zda je k dispozici novější verze.

### Jazyk {#language}
Jazyk rozhraní. Při prvním spuštění Katalog použije jazyk systému, pokud je k dispozici, jinak angličtinu (US).

### Téma {#theme}
| Téma | Popis |
|------|-------|
| *Téma plochy* | Řídí se barvami plochy. |
| *Téma plochy (šedé)* | Řídí se barvami plochy, s šedými plochami a zvýrazněním. |
| *Barvy Katalogu* | Vlastní modré barvy Katalogu. |

S každým tématem se Katalog řídí **světlým nebo tmavým režimem** systému, v Linuxu (KDE Plasma i jiná prostředí), Windows a macOS, a mění se s ním bez restartu. Mimo KDE Plasma používá Katalog světlé a tmavé barvy Breeze místo vlastního barevného schématu prostředí.

:::note
V některých linuxových prostředích (například Linux Mint s Cinnamonem) se tmavý režim při spuštění Katalogu nerozpozná: okno se otevře světlé a pak se řídí další změnou světlého nebo tmavého režimu.
:::

Na stejném řádku jsou:
* *Použijte větší velikost ikony*: zobrazí v seznamech větší ikony.
* *Velikost textu*: zmenší nebo zvětší veškerý text Katalogu, od 80 % do 120 %, posuvníkem nebo jeho dvěma tlačítky. Velikost vychází z velikosti písma nastavené v systému, takže se řídí i změnou systémového písma. Název kolekce v horní části postranního panelu a nadpisy stránek si ponechávají systémovou velikost.
* *Obnovit velikost okna*: vrátí okno na výchozí velikost **1280×800** (nebo na velikost obrazovky, je-li menší). Maximalizované okno se nejprve obnoví. Poloha okna se nemění. Stejnou velikost má okno při úplně prvním spuštění Katalogu.

### Chování {#behavior}
| Možnost | Popis |
|---------|-------|
| *Třídění souborů rozlišuje malá a velká písmena* | Je-li zapnuto, třídění rozlišuje velikost písmen a dává toto pořadí: AA, AB, AC, Aa, Ab, Ac. |
| *Obnovit stav zařízení při návratu do aplikace* | Je-li zapnuto, stav zařízení (dostupné nebo ne) se znovu zkontroluje pokaždé, když se okno Katalogu stane aktivním, takže disk mezitím připojený nebo odpojený se zobrazí správně. Ve výchozím stavu vypnuto. |
| *Povolit akce, které mažou soubory (bez možnosti obnovení)* | Je-li zapnuto, lze soubory **trvale smazat** z výsledků [vyhledávání](Search) a z obrazovky [Prozkoumat](Explore). Ve výchozím stavu vypnuto: tyto akce jsou pak zašedlé. *Přesunout do koše*, které lze vrátit zpět, je vždy dostupné. |

### Soubor s nastavením
Cesta k souboru, ve kterém Katalog uchovává svá nastavení a poslední volby. *Otevřít* ho otevře v přidružené aplikaci.

## Sbírka a databáze {#collection-database}

![Stránka Nastavení, karta Sbírka: sekce Sbírka a databáze, ukázková kolekce v režimu Soubor](/img/settings_01_overview.png)

### Kolekce {#collection}
**Kolekce** je jediná skupina zařízení a všech souvisejících informací: katalogů, statistik, značek, historie vyhledávání a záložních odkazů.<br/>
Je možné mít více kolekcí a přepínat mezi nimi.

Kolekce se otevírají a vytvářejí z postranního panelu pomocí *Otevřít...* a *Nový...* (viz [Rozhraní](Interface#drawer)). Přepnutí na jinou kolekci Katalog nerestartuje.

Sekce zobrazuje:
* *Režim databáze*: datový režim otevřené kolekce (viz níže),
* *Sbírka*: kde je kolekce uložena (složka, databázový soubor nebo server a název databáze), s tlačítkem *Upravit*:
  * v režimu *Soubor* otevře databázový soubor v přidružené aplikaci (například [DB Browser for SQLite](https://sqlitebrowser.org)),
  * v režimu *Paměť* otevře složku kolekce ve správci souborů,
  * v režimu *Hostováno* otevře databázi v phpMyAdmin, na adrese *URL adresa phpMyAdmin* nastavené pro připojení.

### Datové režimy {#database-memory-mode}
Katalog nabízí 3 *datové režimy*, tedy způsoby ukládání a zpracování dat kolekce.

| Režim | Typ databáze | Ukládání dat | Soubory | Rychlost vyhledávání | Rychlost katalogizace |
|-------|--------------|--------------|---------|----------------------|-----------------------|
| **Paměť** | paměť počítače | v souborech `.csv` oddělených tabulátory (pro zařízení, statistiky atd.) a v souborech `.idx` (pro seznamy souborů katalogů) | lepší pro pravidelnou synchronizaci souborů do cloudu | nejrychlejší vyhledávání, jakmile jsou katalogy v paměti (déle při prvním použití katalogu) | mírně rychlejší |
| **Soubor** (výchozí) | soubor SQLite, nízké využití paměti | vše v souboru SQLite | všechna data v 1 souboru, který může narůst na několik set MB | rychlejší pro první vyhledávání, pomalejší pro opakovaná vyhledávání ve velké kolekci | mírně pomalejší |
| **Hostováno** | server MySQL/MariaDB | vše na databázovém serveru | data centralizovaná na serveru, dostupná z více počítačů v síti | výkon dotazů na straně serveru, vhodné pro velké kolekce | mírně pomalejší (síť) |

### Hostovaný režim {#hosted-mode}

![Stránka Nastavení s formulářem pro hostované připojení](/img/settings_02_hosted.png)

V režimu *Hostováno* je kolekce uložena v databázi MySQL/MariaDB na místním nebo síťovém serveru.<br/>
Formulář pro připojení se zobrazí, když je otevřená kolekce v režimu *Hostováno*, nebo po volbě *Otevřít...* > *Hostovaná databáze...* v postranním panelu.

| Pole | Popis |
|------|-------|
| *Název hostitele* | `localhost` nebo IP adresa databázového serveru, na tomto počítači nebo v místní síti (viz [Zabezpečení](#hosted-security)). Prázdné pole znamená `localhost`. |
| *Jméno databáze* | Název databáze na serveru. |
| *Port* | Port serveru (`3306` pro MySQL/MariaDB). |
| *Uživatelské jméno* | Uživatelské jméno databáze. |
| *Heslo* | Heslo databáze. |
| *URL adresa phpMyAdmin* | Adresa phpMyAdmin, kterou používá tlačítko *Upravit*. |

*Připojit* otevře kolekci na serveru.

Předpoklady:
* Server MySQL/MariaDB musí běžet a být dosažitelný.
* Databáze musí na serveru již existovat; potřebné tabulky Katalog vytvoří.

#### Zabezpečení {#hosted-security}
Aby data kolekce zůstala na počítačích, které má uživatel pod kontrolou, připojuje se Katalog pouze k databázovému serveru na tomto počítači nebo v místní síti. Hostitel se kontroluje při *Připojit* a při otevření kolekce v režimu *Hostováno* ze seznamu *Nedávné sbírky* v postranním panelu:

| Hostitel | Co se stane |
|----------|-------------|
| Tento počítač: `localhost`, `127.x.x.x` | Katalog se připojí. |
| Místní síť: `192.168.x.x`, `10.x.x.x`, `172.16.x.x` až `172.31.x.x` | Katalog si nejprve vyžádá potvrzení (*Připojení k soukromé síti*, *Vaše data budou odeslána do této síťové databáze.*). Výchozí volba je *Ne*; při jejím zvolení se nic nezmění. |
| Cokoli jiného: veřejná adresa nebo název serveru (například `mydb.example.com` nebo `nas`) | Odmítnuto: v horní části formuláře se zobrazí *Neplatný název hostitele.* spolu s povolenými adresami. Nic se nepřipojí ani neuloží. Odmítnutá kolekce otevřená ze seznamu *Nedávné sbírky* zobrazí vyplněný formulář se stejnou zprávou. |

Server v místní síti se zadává IP adresou, nikoli názvem.

### Export do jiného datového režimu {#export}
Otevřenou kolekci lze zkopírovat do jiného datového režimu, například pro přesun kolekce ze serveru do jediného souboru. Otevřená kolekce se nemění a zůstává otevřená.

| Datový režim otevřené kolekce | *Export do souboru SQLite* | *Export do režimu paměti (csv)* |
|---|:---:|:---:|
| *Paměť* | ano | — |
| *Soubor* | — | ano |
| *Hostováno* | ano | ano |

* *Export do souboru SQLite* se zeptá na soubor, který se má vytvořit, ve výchozím stavu `export.db` ve složce kolekce (v režimu *Hostováno* v domovské složce). Pokud soubor již existuje, nejprve se přesune do koše. V režimu *Paměť* se nejprve načtou soubory a složky všech katalogů, což u velké kolekce může chvíli trvat.
* *Export do režimu paměti (csv)* se zeptá na složku, která **musí být prázdná**. Katalog do ní zapíše soubory `.csv` kolekce a soubory `.idx` jednotlivých katalogů.

Průběh se zobrazuje dole na stránce, s tlačítkem *Zrušit*. Zrušení odstraní, co export již zapsal. Po dokončení zpráva ukáže cestu k exportu. Tlačítka exportu nejsou dostupná během vytváření nebo aktualizace katalogu ani během vyhledávání.

Exportovanou kolekci lze poté otevřít z postranního panelu pomocí *Otevřít...*.

:::note
Pokud bylo v režimu *Paměť* před exportem spuštěno vyhledávání, mohou v souboru SQLite u prohledaných katalogů chybět složky, které neobsahují žádné soubory. Chcete-li je mít všechny, exportujte hned po otevření kolekce.
:::

### Verze databáze a kontrola kvality {#quality-check}

![Zpráva kontroly kvality ukázkové kolekce](/img/settings_03_quality_check.png)

*Verze databáze* ukazuje verzi struktury databáze otevřené kolekce.

*Kontrola kvality* provede řadu kontrol pouze pro čtení na otevřené kolekci a zobrazí zprávu. Hledá nesrovnalosti mezi zařízeními, úložišti a katalogy:
* zařízení úložiště propojená s chybějícím úložištěm,
* úložiště nepropojená s žádným zařízením,
* ID úložiště používaná více než jedním úložištěm,
* zařízení katalogu propojená s chybějícím katalogem,
* katalogy nepropojené s žádným zařízením,
* zařízení s chybějícím nadřazeným zařízením.

U každé kontroly zpráva uvádí, co bylo nalezeno, nebo *Nic nenalezeno*. *Kopírovat do schránky* zprávu zkopíruje, například pro přiložení k hlášení chyby.<br/>
Kontrola kvality **nic neopravuje**: pouze informuje.

### Složka obrázků {#images-folder}
*Složka obrázků* je složka, ve které jsou uloženy obrázky úložných zařízení. Jde o nastavení každé kolekce zvlášť.
* Zadejte cestu do pole nebo klikněte na *Vybrat* a zvolte složku.
* Pokud není nastavena, použije se složka `images` uvnitř složky kolekce.

Obrázky umístěné v této složce lze poté zvolit pro úložné zařízení při jeho úpravě na obrazovce [Zařízení](Devices).

## Import a synchronizace sbírky {#import-update}

![Sekce importu se zvolenou zdrojovou kolekcí, zařízením vybraným ve zdrojovém stromu a zprávou o importu](/img/settings_04_import.png)

Katalog umí importovat zařízení z jiné kolekce do otevřené kolekce a později je aktualizovat, když se zdroj změní.
* **Zdroj** je ta druhá kolekce. Pouze se čte, nikdy se nemění.
* **Cíl** je otevřená kolekce, do které se zařízení a katalogy přidávají.

| Pole | Použití |
|------|---------|
| *Datový režim* | Typ zdroje: *Katalog / Soubor* (soubor `.db`), *Katalog / Paměť* (složka kolekce) nebo *VVV / Hodnoty oddělené tabulátory* (exportní soubor z VVV). |
| *Zdroj* | Zdrojová kolekce nebo soubor, zvolené pomocí *Vybrat*. |
| *Zařízení* | Zařízení k importu, zvolené ve stromu zařízení zdroje. *Import* ho importuje s veškerým obsahem. |
| *Aktualizovat* | Zdrojové kolekce, ze kterých byla zařízení importována. *Aktualizovat* obnoví ze zvoleného zdroje katalogy všech zařízení z něj importovaných. |

**Co se importuje**<br/>
Zařízení, jejich katalogy a seznamy souborů, statistiky, záložní odkazy a vyloučené složky. Hierarchie zdroje se v cíli zachová přidáním potřebných nadřazených zařízení. Pokud název zařízení nebo katalogu v cíli již existuje, importovaný se přejmenuje (například `My Drive (2)`).

**Zpráva**<br/>
Po skončení aktualizace zpráva shrne, co se změnilo: počet aktualizovaných katalogů a počet a celkovou velikost souborů před a po.

:::note
Zdrojová kolekce musí mít stejnou verzi databáze jako otevřená kolekce. Pokud byla vytvořena starší verzí Katalogu, otevřete ji nejprve jednou touto verzí: její databáze se tím aktualizuje.
:::

**Import z VVV**<br/>
[VVV (Virtual Volumes View)](https://www.fsoft.it/VVV/) je jiná katalogizační aplikace. S volbou *VVV / Hodnoty oddělené tabulátory* vyberte soubor exportovaný z VVV a klikněte na *Import*: jeho katalogy se přidají pod novou virtuální skupinu.

## Vývoj
* Viz backlog [vývoje Nastavení](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=settings).
