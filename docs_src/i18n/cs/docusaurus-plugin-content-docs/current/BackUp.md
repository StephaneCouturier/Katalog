---
version: "2.13"
---
# Záloha
![2.13](https://img.shields.io/badge/Version-2.13-blue)

## Shrnutí
Tato stránka popisuje všechny funkce obrazovky **BackUp** a jejich použití.<br/>
Z této obrazovky může uživatel **spravovat zálohy katalogů (kopírování) nebo archivaci (přesun)**.<br/>

![Hlavní obrazovka BackUp](/img/screen_backup_01.png)

## Hlavní koncepty

### Zálohovací propojení

Obrazovka umožňuje <b>přiřadit katalog</b>, považovaný za <b><i>zdroj</i></b>, k jinému katalogu, považovanému za zálohovací <b><i>cíl</i></b>.<br/>Toto je základní prvek všech funkcí této obrazovky.

### Porovnání katalogů

Obrazovka zobrazuje seznam propojení a <b>pokrytí záloh</b> pro celý výběr i jednotlivá propojení, přičemž porovnává velikost, počet souborů a datum aktualizace zdroje a cíle.

### Replikace adresářů

V průběhu zálohovacího procesu je možné samostatně spustit kopírování hierarchie složek bez souborů.

## BackUp nebo Archive

| Operace | Zpracování souborů | Účel |
|---------|-------------------|------|
| **BackUp** | Operace **kopírování** ze zdrojového katalogu do cílového katalogu. | Zdroj není nikdy změněn. Cílem je redundance a obnova. <br/> Postupem času budou k dispozici různé strategie (úplná, přírůstková, synchronizace). |
| **Archive** | Operace **přesunu** ze zdrojového katalogu do cílového katalogu. | Soubory jsou přesunuty a po ověření kopie odstraněny ze zdroje. Cílem je dlouhodobé přesouvání a organizace úložiště. Prázdné adresáře zanechané ve zdroji **nejsou** odstraněny. |



### Přírůstková záloha (kopírování souborů)

Záloha Katalog kopíruje soubory ze **zdrojového katalogu** do **cílového katalogu** s použitím indexovaných dat Katalog.
<br/>Aby to fungovalo správně, budou katalogy aktualizovány před zálohou i po ní (volitelné, ale velmi doporučené).
<br/>Není vyžadován žádný externí nástroj.
<br/><br/>Pořadí akcí:
- Porovná zdrojový a cílový katalog a nalezne **soubory chybějící v cíli**.
- Zkopíruje chybějící soubory do cíle a znovu vytvoří strukturu složek.
- **Nepřepisuje** existující soubory v cíli (ani pokud jsou odlišné).
- **Neodstraňuje** soubory z cíle, které nejsou ve zdroji.

Možnosti „Přísné kopírování"
- <i>Přísné kopírování</i> (výchozí): Katalog zkopíruje soubory, i když jsou již v cíli přítomny. Pokud není zaškrtnuto, soubory již přítomné v cíli (podle názvu, velikosti a data) nebudou znovu kopírovány.

Možnosti „Adresáře"
- <i>Zahrnout prázdné</i> (výchozí: zaškrtnuto): struktura adresářů zdroje je v cíli vytvořena před zkopírováním jakéhokoli souboru. Pokud je zaškrtnuto, je vytvořen i adresář, který neobsahuje vůbec nic — žádný soubor ani podadresář. Pokud není zaškrtnuto, takový adresář vytvořen není, zatímco každý adresář, který něco obsahuje, vytvořen je. Zrušení zaškrtnutí nikdy neodstraní adresář, který je již v cíli přítomen.

Možnosti řešení konfliktů
- <i>Při konfliktu</i> (výchozí: <i>Přejmenovat nejstarší</i>)

Katalog může řešit konflikty různými způsoby, pokud soubor existuje v cíli, ale datum, velikost nebo kontrolní součet se liší.

**Konflikt** nastane, pokud soubor existuje ve zdroji i v cíli na stejné cestě, ale datum, velikost nebo kontrolní součet se liší. Režim určuje, jak se v takovém případě postupuje.

#### Dostupné režimy {#available-modes}

| Režim | Chování |
|-------|---------|
| **Přeskočit** (výchozí) | Žádná operace — zdroj není zkopírován, cíl není změněn. Konflikt je zaznamenán pro kontrolu uživatelem. |
| **Přejmenovat nejstarší** | Pokud je zdroj novější: přejmenuje starší cílový soubor (přidá časové razítko), poté zkopíruje zdroj. Pokud je cíl novější nebo stejného data: přeskočí (chrání novější cíl). |

#### Úplná tabulka scénářů

| # | Situace | Přeskočit | Přejmenovat nejstarší |
|---|---------|-----------|----------------------|
| A | Zdroj novější než cíl | konflikt zaznamenán | přejmenuje cíl → zkopíruje zdroj ✓ |
| B | Cíl novější než zdroj | konflikt zaznamenán | přeskočí (chrání novější cíl) |
| C | Stejné datum, různá velikost | konflikt zaznamenán | přeskočí (není jasný vítěz) |
| D | Zdrojový soubor na disku chybí | chyba | chyba |

> **Přejmenovat nejstarší — formát archivovaného názvu**: starý cílový soubor je přejmenován na `původnínázev_RRRRMMDD-HHmmss.ext` (např. `zpráva_20260225-102559.docx`). Časové razítko je vloženo před příponu, aby soubor zůstal otevíratelný. Tyto soubory se hromadí v cíli a je nutné je ručně odstranit pro uvolnění místa.

> **Přejmenovat nejstarší — bezpečnostní záruka**: pokud kopírování zdrojového souboru selže poté, co byl cíl již přejmenován, přejmenovaný soubor je automaticky obnoven na původní název. Žádná data nejsou ztracena.


### Archive (přesun souborů)

Operace Archive **přesouvá** soubory ze zdroje do cíle místo jejich kopírování. Zdrojové soubory jsou odstraněny po úspěšném a potvrzeném přenosu — pokud přenos selže, zdrojový soubor zůstane nedotčen.

- Na **stejném souborovém systému**: přesun je okamžitý — žádná data nejsou fyzicky kopírována; mění se pouze umístění souboru.
- **Mezi různými souborovými systémy**: soubor je nejprve zkopírován do cíle, poté odstraněn ze zdroje po potvrzení kopírování.

### Zdrojový režim {#source-mode}

Každé zálohovací propojení má **Zdrojový režim**, který určuje, co se používá jako zdroj při porovnávání a kopírování souborů.

| Režim | Popis |
|-------|-------|
| **Katalog** (výchozí) | Používá index katalogu. Funguje offline — zdrojové zařízení nemusí být připojeno. Jsou uplatňována pravidla pro vyloučení složek katalogu: vyloučené složky nejsou zálohovány. |
| **Disk** | Prochází přímo zdrojový souborový systém. Zdrojové zařízení **musí být připojeno a namontováno**. Jsou zahrnuty všechny soubory pod zdrojovou cestou — index katalogu a pravidla pro vyloučení složek jsou zcela ignorovány. |

**Rozhraní**: zaškrtávací políčko *Prohledat zdrojový disk přímo* na panelu Vytvořit propojení ovládá toto nastavení. Nezaškrtnuto = Katalog (výchozí), zaškrtnuto = Disk.

### Profil LuckyBackup

Je možné exportovat zálohovací propojení do profilu [LuckyBackup](https://luckybackup.sourceforge.net).
Viz samostatná stránka: [Profil LuckyBackup](BackUp_luckybackup_profile)


## Správa zálohovacích propojení

Pro přehledné zobrazení a porovnání zdrojových adresářů a jejich záloh umožňuje Katalog propojovat katalogy.

### Vytvoření zálohovacího propojení

Panel *Vytvořit propojení* lze sbalit nebo rozbalit pomocí přepínacího tlačítka v horní části panelu.

#### Pole propojení

| Pole | Popis |
|------|-------|
| Název | Označení propojení. Lze automaticky vygenerovat ve formátu `"<název zdroje> -> <název cíle>"`. |
| Typ | `BackUp` (kopírování) nebo `Archive` (přesun). Určuje, zda jsou zdrojové soubory po přenosu odstraněny. |
| Zdrojové zařízení | Zdrojový katalog. Soubory jsou čteny z jeho indexované cesty. |
| Cílové zařízení | Cílový katalog zálohy. Soubory jsou zapisovány do jeho cesty. |
| Datum poslední zálohy | Datum posledního dokončeného zálohovacího běhu. Aktualizováno automaticky. |
| Velikost poslední zálohy | Celkový počet bajtů přenesených při posledním zálohovacím běhu. Aktualizováno automaticky. |
| Přísné kopírování | Pokud je povoleno (výchozí), kopíruje soubory podle cesty — i když soubor již existuje jinde v cíli. Pokud je zakázáno, přeskočí soubory již přítomné v cíli (režim deduplikace). Nelze použít pro propojení *Archive* (automaticky zakázáno). |
| Při konfliktu | Co dělat, pokud soubor existuje na stejné cestě ve zdroji i cíli, ale liší se. Výchozí: `PřejmenovatNejstarší`. Viz [Dostupné režimy](#available-modes). |
| Zdrojový režim | `Katalog` (výchozí) nebo `Disk`. Určuje, zda je zdroj čten z indexu katalogu nebo přímým procházením souborového systému. Viz [Zdrojový režim](#source-mode). |
| Adresáře - Zahrnout prázdné | Pokud je povoleno (výchozí), adresáře, které neobsahují žádný soubor ani podadresář, jsou vytvořeny v cíli. Pokud je zakázáno, jsou vytvořeny pouze adresáře, které něco obsahují. |

#### Příklad a katalogy
Cíl: vytvořit propojení mezi zdrojem na místním disku a cílem na externím disku.
![Příklad zdrojového a cílového zařízení](/img/screen_backup_1_devices.png)

#### Výběr zdroje a cíle
- S panelem Výběr a 2 tlačítky „Načíst katalogy" pro zdroj a cíl získejte seznam katalogů k výběru.
- Tlačítko „bez propojení" pomáhá omezit počet zobrazených katalogů — zobrazí pouze katalogy, které ještě nemají propojení jako zdroj (nebo cíl).
- Vyberte zdroj a cíl.

![Výběr zdroje a cíle](/img/screen_backup_2_select_source_target.png)


#### Nastavení názvu, možností a vytvoření
- Vygenerovat název: název zdrojového katalogu + „ -> " + název cílového katalogu
- Nastavit možnost „Přísné kopírování"
- Nastavit možnost „Adresáře"
- Nastavit chování při detekci konfliktu
- Vytvořit propojení

![Vytvoření propojení](/img/screen_backup_4_create_mapping.png)

#### Porovnání zdroje a zálohovacího cíle
- Propojení se zobrazí v seznamu a pokrytí je vypočteno.
![Porovnání zdroje a cíle](/img/screen_backup_5_comparison.png)

### Filtry seznamu propojení

Seznam propojení lze filtrovat pro zobrazení pouze relevantních propojení:
- Přepínače **Zdroj / Cíl** — zobrazí pouze propojení, ve kterých vybrané zařízení funguje jako zdroj nebo cíl.
- Rozbalovací nabídka **Typ** — filtruje podle *BackUp* nebo *Archive*.
- Zaškrtávací políčko **Zobrazit celou tabulku** — přepíná zobrazení dalších sloupců s podrobnostmi.

### Kontextové menu propojení

Kliknutím pravým tlačítkem na propojení v seznamu se otevře kontextové menu s následujícími akcemi:

| Akce | Popis |
|------|-------|
| *Spustit zálohu* / *Spustit archivaci* | Spustí zálohovací nebo archivační operaci pro toto propojení. |
| *Náhled zálohy* / *Náhled archivace* | Spustí náhled (simulaci) bez kopírování souborů. |
| *Replikovat adresáře* | Zkopíruje pouze strukturu složek bez souborů. |
| *Invertovat (prohodit zdroj a cíl)* | Prohodí zdroj a cíl propojení jedním kliknutím — užitečné pro obrácení směru zálohy. |
| *Smazat* | Odstraní propojení (soubory na disku nejsou ovlivněny). |

**Vytvoření profilu LuckyBackUp**
Katalog může vygenerovat profil LuckyBackUp připravený k použití na základě zálohovacích propojení:
- Uloženo do adresáře `~/.luckyBackup/profiles/`
- Každé zálohovací propojení se stane jedním úkolem v profilu
- Ve výchozím nastavení jsou zahrnuta **všechna** propojení
- Zaškrtnutím *Pouze vybrané odkazy* zahrnete pouze propojení aktuálně viditelná ve filtrovaném seznamu (filtrovaném podle zařízení Zdroj/Cíl a/nebo Typu)

## Spuštění zálohy nebo archivace

### Předpoklady
- Je vybráno propojení BackUp/Archive
- Oba katalogy musí patřit zařízením s platnými a přístupnými cestami.
- Na cíli musí být dostatek místa pro kopírování/přesun souborů.
- Ačkoli je to volitelné, doporučuje se ponechat zaškrtnutou možnost „Aktualizovat katalogy" pro aktualizaci před zahájením a po dokončení procesu.

### Náhled
- Náhled (simulace) lze spustit pro otestování efektu zálohy nebo archivace a vygenerování protokolu.
- Výsledek náhledu lze **exportovat** pomocí tlačítka *Exportovat*, čímž se seznam plánovaných operací uloží do souboru.

### Pozastavení, obnovení a zrušení

Během provádění se tlačítko **Spustit zálohu** mění na jiný popis a funkci:
- Při **spuštění** → kliknutím **Pozastavit** (pozastaví po dokončení aktuálního souboru)
- Při **pozastavení** → kliknutím **Pokračovat**

Tlačítko **Zrušit** je vždy dostupné během provádění nebo pozastavení. Zrušení operaci čistě zastaví — jakýkoli právě kopírovaný soubor je z cíle odstraněn (nezůstanou žádné neúplné soubory).

### Aktualizace katalogu po provedení

Po úspěšné záloze bude cílový katalog automaticky aktualizován, aby odrážel nově zkopírované soubory, bez nutnosti úplného přeindexování. *(Plánováno — zatím není k dispozici.)*

## Základní chování: přírůstkové kopírování nebo archivace

Kritéria porovnání:
- Shoda podle **názvu souboru + relativní cesty složky** (stejný soubor na stejném relativním místě).
- Soubor je „chybějící", pokud v cílovém katalogu neexistuje žádná shoda.

**Správa konfliktů**
| Rozhodnutí | Volba | Zdůvodnění |
|------------|-------|------------|
| Smazat soubory v cíli? | Ne (v1). | Začít bezpečně — pouze přírůstkově. |
| Přepsat konflikty? | Ne (v1). Zaznamenat je. | Zabránit ztrátě dat. |
| Vytvořit chybějící adresáře? | Ano, vždy. | Nutné pro jakékoli kopírování souborů. |

### Správa místa na disku

Místo na disku je kritickým omezením pro operace zálohy i archivace. Nedostatek místa v průběhu operace ponechá cíl v částečném stavu.

Požadavky na místo podle operace:

| Operace | Co spotřebovává místo na cíli | Čistý efekt na zdroj |
|---------|------------------------------|---------------------|
| **BackUp** | Všechny soubory ke kopírování | Žádný |
| **Archive (stejný FS)** | Nula — přesun pouze metadat | Místo uvolněno ve zdroji |
| **Archive (různé FS)** | Soubory zkopírovány před odstraněním zdroje | Místo uvolněno po odstranění |
| **Konflikt PřejmenovatNejstarší** | Přejmenovaný cílový soubor zachován (+1 kopie) | Žádný do ručního vyčištění |

### Kontrola místa (implementováno)

Vypočteno před spuštěním a zobrazeno v náhledu.

| Podmínka | Práh | Akce |
|----------|------|------|
| **Nedostatečné** | dostupné < požadované | Blokováno: varování, operace nespuštěna |
| **Nízké** | dostupné − požadované < 512 MB | Žádost o potvrzení (Ano/Ne pro pokračování) |
| **OK** | dostupné − požadované ≥ 512 MB | Pokračuje bez oznámení |

V náhledu je stav místa přidán k souhrnu:
- **Nedostatečné** → červené varování: `⚠ Místo na cíli: X dostupné, Y potřebné`
- **Nízké** → oranžové varování: `⚠ Nízké místo na cíli: Z zbývající po operaci`
- **OK** → bez anotace

> **Poznámka**: Režim *PřejmenovatNejstarší* trvale přidává přejmenované kopie na cíl. Tyto soubory (`název_RRRRMMDD-HHmmss.ext`) je nutné ručně odstranit pro uvolnění místa.

## Protokol
Po provedení je tabulka náhledu nahrazena **zálohovacím protokolem** — tabulkou se čtyřmi sloupci: **Stav**, **Název souboru**, **Cesta** a **Velikost**.

Každý řádek odpovídá jednomu souboru s následujícími hodnotami stavu:

| Stav | Význam |
|------|--------|
| *Zkopírováno* | Soubor byl úspěšně zkopírován do cíle. |
| *Přesunuto* | Soubor byl úspěšně přesunut do cíle (operace Archive). |
| *Archivováno & Zkopírováno* | Konfliktní cílový soubor byl přejmenován (režim PřejmenovatNejstarší), poté byl zkopírován zdroj. |
| *Konflikt* | Soubor existuje na stejné cestě ve zdroji i cíli, ale liší se — nepřepsán, zaznamenán pro kontrolu. |
| *Chyba* | Soubor nebylo možné zkopírovat nebo přesunout (přístup odepřen, disk plný atd.). |

Souhrnný řádek nad tabulkou zobrazuje celkové počty: zkopírované soubory, archivované & zkopírované, konflikty a chyby.

---

## Vývoj
Několik nápadů na rozvoj této obrazovky:

### Budoucí funkce
- [ ] Správa snímků
- [ ] Filtry zahrnutí/vyloučení
- [ ] Plánování (cron/systemd/Plánovač úloh)
- [ ] Funkce obnovení
- [ ] Možnosti komprese
- [ ] Vzdálené zálohy (ssh)

### Budoucí režimy konfliktů

| Režim | Chování |
|-------|---------|
| **Přepsat** | Zdroj vždy vyhraje — přepíše cíl bez upozornění. Pro uživatele, kteří chtějí, aby zdroj byl autoritativní bez ohledu na datum. |
| **Vždy přejmenovat** | Vždy přejmenuje cíl a zkopíruje zdroj, i když je cíl novější — agresivní, explicitní archivace. |

### Budoucí možnosti

- **Vyčištění zdroje po archivaci**: volitelné odstranění prázdných adresářů zanechaných ve zdroji po operaci Archive.
- **Režim mazání**: volitelné odstranění cílových souborů nepřítomných ve zdroji.
- **Porovnání pomocí kontrolního součtu**: detekce změn obsahu i při shodě názvu, velikosti a data.
- **Plánovaná/automatická záloha**: spuštění podle časovače nebo při aktualizaci katalogu.
- **Historie záloh**: protokol minulých zálohovacích běhů s daty a statistikami.

### Budoucí možnosti — Místo na disku
- **Minimální volné místo pro propojení**: uživatelsky konfigurovatelná spodní hranice (např. vždy ponechat 5 GB volných).
- **Předčasné přerušení při zaplnění disku**: detekce chyb při kopírování a okamžité zastavení.
- **Ověření po archivaci**: potvrzení, že místo na zdroji bylo po archivaci skutečně uvolněno.
- **Zobrazení trendu místa**: sledování místa na cíli v průběhu času na kartě Statistiky.
- **Nástroj pro čištění archivovaných souborů**: výpis a hromadné odstranění souborů `název_RRRRMMDD-HHmmss.ext` vytvořených režimem PřejmenovatNejstarší.

* Další informace naleznete v backlogu [vývoje BackUp](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=BackUp).
