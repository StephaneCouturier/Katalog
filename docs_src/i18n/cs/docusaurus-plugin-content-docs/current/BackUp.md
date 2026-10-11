---
version: "3.0"
---
# Záloha
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Shrnutí
Tato stránka popisuje stránku **Záloha**, kde se soubory kopírují nebo přesouvají z jednoho katalogu do druhého, přičemž se z dat katalogů zjišťuje, co chybí:
* [pojmy](#concepts): odkazy, záloha a archiv, konflikty, zdrojový režim,
* [seznam odkazů](#links) s jejich pokrytím a posledním spuštěním,
* [vytvoření a úprava odkazu](#link-form),
* [náhled](#preview) a [spuštění](#run) odkazu nebo [všech uvedených odkazů](#run-listed),
* [další akce](#more-actions) s odkazem: duplikovat, replikovat adresáře, invertovat, exportovat, smazat, LuckyBackup.

![Stránka Záloha: seznam odkazů se zdrojem, cílem, rozdílem a poslední zálohou](/img/backup_01_overview.png)

## Pojmy {#concepts}

### Odkazy
**Odkaz** spojuje **zdrojový** katalog s **cílovým** katalogem, do kterého se jeho soubory zálohují. Oba jsou katalogy kolekce (viz [Zařízení](Devices#model)): chcete-li zálohovat složku na externí disk, musí mít složka i záložní složka na disku nejprve každá svůj katalog (viz [Vytvořit](Create)).

Katalog porovná oba katalogy, najde soubory zdroje, které **v cíli chybí**, a pak je zkopíruje nebo přesune. Nepotřebuje žádný externí nástroj.

### Záloha nebo Archiv {#type}

| Typ | Operace | Účel |
|-----|---------|------|
| **Záloha** | **Zkopíruje** chybějící soubory do cíle. | Zdroj se nikdy nemění. Cílem je redundance a obnova. |
| **Archiv** | **Přesune** soubory do cíle. | Každý soubor se ze zdroje odstraní, jakmile je potvrzena jeho kopie. Cílem je uvolnit zdroj pro dlouhodobé uložení. Prázdné složky, které ve zdroji zůstanou, se **nemažou**. |

Záloha:
* znovu vytvoří strukturu složek zdroje v cíli a pak zkopíruje chybějící soubory,
* **nepřepíše** soubor, který již v cíli je, i když se liší (viz [konflikty](#conflicts)),
* **nemaže** soubory cíle, které ve zdroji nejsou.

Archivace na stejném souborovém systému přesune soubory okamžitě; mezi různými souborovými systémy se každý soubor zkopíruje a po dokončení kopie ze zdroje smaže. Pokud kopírování selže, zdrojový soubor zůstane nedotčen.

### Konflikty {#conflicts}
**Konflikt** je soubor, který je na stejném místě ve zdroji i v cíli, ale s jiným datem, velikostí nebo kontrolním součtem. Co se stane, určuje volba odkazu *Při konfliktu*:

| Situace | *Přeskočit – ponechat cíl beze změny* | *Přejmenovat nejstarší – přejmenovat cíl, kopírovat zdroj* |
|---------|------|---------------|
| Zdroj novější než cíl | konflikt nahlášen | cíl přejmenován, pak zdroj zkopírován |
| Cíl novější než zdroj | konflikt nahlášen | přeskočeno, pro ochranu novějšího cíle |
| Stejné datum, jiná velikost | konflikt nahlášen | přeskočeno (není jasný vítěz) |
| Zdrojový soubor na disku chybí | chyba | chyba |

S volbou *Přejmenovat nejstarší* se starý cílový soubor přejmenuje na `název_RRRRMMDD-HHmmss.ext` (například `report_20260225-102559.docx`): datum se vloží před příponu, aby soubor šel stále otevřít. Pokud pak kopírování zdroje selže, přejmenovaný soubor dostane zpět svůj název: nic se neztratí.<br/>
Tyto přejmenované soubory zůstávají v cíli a zabírají místo: odstraňují se ručně.

### Zdrojový režim {#source-mode}
* Ve výchozím stavu pochází seznam zdrojových souborů z **katalogu**: porovnání funguje i s odpojeným zdrojem a složky vyloučené z katalogu se nezálohují.
* S volbou *Skenovat zdrojový disk přímo (vyžaduje připojený zdroj)* se zdrojová složka čte z **disku**: zahrne se každý soubor bez ohledu na katalog a jeho vyloučení. Zdroj musí být připojen.

### Místo v cíli {#space}

| Operace | Místo potřebné v cíli | Účinek na zdroj |
|---------|-----------------------|-----------------|
| **Záloha** | Všechny soubory ke kopírování. | Žádný. |
| **Archiv, stejný souborový systém** | Žádné: soubory se pouze přesouvají. | Místo uvolněno. |
| **Archiv, jiný souborový systém** | Všechny soubory k přesunutí. | Místo uvolněno po smazání každého souboru. |
| **Konflikty** ***Přejmenovat nejstarší*** | Přejmenované soubory se zachovají: o jednu kopii více u každého. | Žádný. |

[Náhled](#preview) zkontroluje volné místo v cíli a upozorní, když je nedostatečné nebo malé (méně než 512 MB zbývajících po operaci). Spuštění se před startem znovu zkontroluje: při nedostatku místa se odmítne a zpráva uvede potřebné, dostupné a chybějící místo. Když nelze místo v cíli zjistit (síťové nebo virtuální disky), kontrola neproběhne.

## Seznam odkazů {#links}
Stránka zobrazuje odkazy jako karty.

### Filtry
| Filtr | Účinek |
|-------|--------|
| *Typ* | *Všechno*, *Záloha* nebo *Archiv*. |
| *Filtr* | *Všechno*: všechny odkazy, nebo pouze ty, u nichž je zařízení vybrané na stránce [Výběr](Selection) *Zdroj* nebo *Cíl*. |

Řádek nad kartami uvádí pro vybrané zařízení počet odkazů, velikost jejich zdrojů a u záloh procento **pokrytí**: kolik ze zdrojů už je v cílech.

### Karty
Každá karta ukazuje:
* název odkazu a jeho typ,
* katalogy **Zdroj** a **Cíl** s jejich velikostí a počtem souborů,
* *Rozdíl* u zálohy: co v cíli chybí, nebo *Aktuální*; *K přesunutí* u archivu: co je ještě ve zdroji, nebo *Není co přesouvat*,
* *Poslední záloha* nebo *Poslední archivace*: datum a velikost posledního spuštění, nebo *nikdy*,
* tlačítka *Spustit zálohu* / *Spustit archivaci*, *Náhled zálohy* / *Náhled archivu* a *Další akce*.

Když ještě žádný odkaz neexistuje, stránka to oznámí a *Přidat* vytvoří první.

## Vytvoření nebo úprava odkazu {#link-form}
*Přidat* na panelu nástrojů stránky otevře formulář *Přidat odkaz*; *Upravit* v *Další akce* otevře stejný formulář, *Upravit odkaz*, pro existující odkaz.

![Formulář Přidat odkaz](/img/backup_02_link_form.png)

| Pole | Popis |
|------|-------|
| *Zdrojový katalog* | Katalog, jehož soubory se zálohují. |
| *Cílový katalog* | Katalog, do kterého se zapisují. |
| *Název* | Název odkazu. Tlačítko vedle (*Automaticky vygenerovat název*) vytvoří `<zdroj> -> <cíl>`. |
| *Typ* | *Záloha* (kopírování) nebo *Archiv* (přesun), viz [výše](#type). |
| *Složky* | *Zahrnout prázdné* (ve výchozím stavu zaškrtnuto): složky, které nic neobsahují, žádný soubor ani podsložku, se v cíli také vytvoří. Nezaškrtnuto: vytvoří se pouze složky, které něco obsahují. Nikdy neodstraní složku, která již v cíli je. |
| *Přesné kopírování* | *Přesně zrcadlit strukturu složek (výchozí)*: každý soubor se zkopíruje na své vlastní místo, i když stejný soubor už je jinde v cíli. Nezaškrtnuto: soubor se přeskočí, když soubor se stejným názvem a velikostí už je kdekoli v cíli: žádné duplikáty. U archivu vždy vypnuto. |
| *Při konfliktu* | Viz [konflikty](#conflicts). |
| *Zdrojový režim* | Viz [zdrojový režim](#source-mode). |

*Uložit* vytvoří nebo aktualizuje odkaz; *Zrušit* zavře formulář. Zpráva v horní části formuláře uvede, co chybí.

## Náhled {#preview}
*Náhled zálohy* / *Náhled archivu* vypočítá, co by spuštění udělalo, **aniž by cokoli kopíroval**. Když je zaškrtnuto *Aktualizovat katalogy*, oba katalogy se nejprve aktualizují. Výsledek se otevře na vlastní stránce:
* shrnutí: soubory ke kopírování nebo přesunutí, konflikty, soubory, které už v cíli jsou, a upozornění, když je zdroj nebo cíl offline,
* [kontrola místa](#space) v cíli,
* seznam souborů s jejich *Stav* (*Kopírovat*, *Přesunout* nebo *Konflikt*), *Název souboru*, *Cesta* a *Velikost*,
* *Exportovat do CSV* uloží seznam do složky kolekce (v režimu *Hostováno* do domovské složky) a ukáže cestu k souboru.

![Náhled zálohy: shrnutí, kontrola místa a seznam souborů](/img/backup_03_preview.png)

## Spuštění {#run}
*Spustit zálohu* / *Spustit archivaci* spustí odkaz. Zdroj i cíl musí být dostupné.

* Se zaškrtnutým *Aktualizovat katalogy* (doporučeno) se oba katalogy aktualizují **před** porovnáním, takže záloha pracuje s aktuálními daty. Po spuštění, které nebylo zrušeno, se cíl znovu aktualizuje (u archivu i zdroj), aby katalogy ukazovaly výsledek.
* Během běhu karta ukazuje průběh, právě kopírovaný soubor, rychlost a zbývající čas (*Zbývá*). *Pauza* běh pozastaví po aktuálním souboru, *Obnovit* pokračuje, *Zrušit* ho zastaví: právě kopírovaný soubor se z cíle odstraní, takže nezůstane žádný neúplný soubor.
* Na konci řádek v horní části stránky uvede výsledek: zkopírované nebo přesunuté soubory, archivované a zkopírované (*Přejmenovat nejstarší*), konflikty a chyby. Na kartě se aktualizuje *Poslední záloha*.
* *Sestava* na tomto řádku otevře seznam souborů spuštění, jeden řádek na soubor s jeho stavem (*Zkopírováno*, *Přesunuto*, *Archivováno & Zkopírováno*, *Konflikt*, *Chyba*), názvem, cestou a velikostí, a součty. Je to sestava posledního spuštění (posledního odkazu, po *Spustit uvedené odkazy*); uchovává se do dalšího spuštění nebo do zavření Katalogu.

### Spustit uvedené odkazy {#run-listed}
*Spustit uvedené odkazy* spustí **odkazy uvedené na stránce**, jeden po druhém, takže sadu záloh lze spustit najednou a nechat doběhnout.
* Co se spustí, určují filtry: chcete-li spustit jinou sadu, nejprve změňte filtry.
* Spustí se pouze odkazy, jejichž zdroj **i** cíl jsou dostupné; ostatní se přeskočí.
* Nejprve se zobrazí potvrzení: počet odkazů, které se spustí, objem archivů (horní mez) a záloh (přibližně) a počet přeskočených odkazů. Jde o odhady ze součtů katalogů; přesné číslo dá *Náhled* u odkazu.
* Každý odkaz běží, jako by byl spuštěn ze své vlastní karty, včetně *Aktualizovat katalogy*.
* *Zrušit* zastaví **celé** spuštění: dosud čekající odkazy se nespustí. *Pauza* a *Obnovit* působí na právě běžící odkaz.

## Další akce {#more-actions}

| Akce | Popis |
|------|-------|
| *Upravit* | Otevře odkaz ve [formuláři](#link-form). |
| *Duplikovat* | Vytvoří nový odkaz se stejným nastavením k úpravě. |
| *Replikovat adresáře* | Vytvoří v cíli strukturu složek zdroje, bez souborů. Výsledek uvede vytvořené složky, již existující složky a chyby. |
| *Invertovat (prohodit zdroj a cíl)* | Prohodí zdroj a cíl, například pro obnovu. Když název obsahuje šipku, prohodí se i jeho dvě poloviny. |
| *Exportovat do CSV* | Vypočítá náhled tohoto odkazu, pak uloží jeho seznam souborů do souboru CSV ve složce kolekce (v režimu *Hostováno* do domovské složky) a ukáže cestu k souboru. |
| *Smazat* | Po potvrzení smaže odkaz. Žádného souboru ve zdroji ani v cíli se to nedotkne. |

### Profil LuckyBackup {#luckybackup}
*LuckyBackup* na panelu nástrojů stránky vytvoří profil [LuckyBackup](https://luckybackup.sourceforge.net) z **odkazů uvedených** na stránce: každý odkaz se stane úlohou. Profil se uloží do `~/.luckyBackup/profiles/`. Viz [Profil LuckyBackup](BackUp_luckybackup_profile).

## Vývoj
Několik nápadů na rozvoj této obrazovky:

### Budoucí funkce
- [ ] Správa snímků
- [ ] Vzory vyloučení/zahrnutí
- [ ] Plánování (cron/systemd/Plánovač úloh)
- [ ] Funkce obnovy
- [ ] Možnosti komprese
- [ ] Vzdálené zálohy (ssh)

### Budoucí režimy konfliktů

| Režim | Chování |
|-------|---------|
| **Přepsat** | Zdroj vždy vítězí: cíl se bez upozornění přepíše. Pro uživatele, kteří chtějí, aby zdroj platil bez ohledu na datum. |
| **Vždy přejmenovat** | Vždy přejmenuje cíl a zkopíruje zdroj, i když je cíl novější: důsledná, výslovná archivace. |

### Budoucí možnosti

- **Úklid zdroje po archivaci**: volitelné mazání prázdných složek, které ve zdroji zůstanou po operaci Archiv (přesun).
- **Režim mazání**: volitelné odstranění souborů cíle, které ve zdroji nejsou.
- **Porovnání kontrolním součtem**: odhalení změn obsahu, i když se shoduje název, velikost a datum.
- **Plánovaná/automatická záloha**: spuštění časovačem nebo při aktualizaci katalogu.
- **Historie záloh**: záznam minulých spuštění s daty a statistikami.

### Budoucí funkce pro místo na disku
- **Minimální volné místo pro každý odkaz**: nastavitelná spodní hranice (například vždy ponechat 5 GB volných).
- **Předčasné ukončení při vyčerpání místa za běhu**: zachytit chyby ENOSPC během kopírování a okamžitě skončit, místo aby selhával soubor za souborem.
- **Ověření zdroje po archivaci**: potvrdit, že se po dokončení Archivu místo ve zdroji skutečně uvolnilo.
- **Zobrazení vývoje místa**: zobrazit místo v cíli v čase na kartě Statistika.
- **Nástroj pro úklid archivovaných souborů**: vypsat a hromadně smazat soubory `název_RRRRMMDD-HHmmss.ext` vytvořené režimem Přejmenovat nejstarší.

* Více viz backlog [vývoje Zálohy](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=BackUp).
