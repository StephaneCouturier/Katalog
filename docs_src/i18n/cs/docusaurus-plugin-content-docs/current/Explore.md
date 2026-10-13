---
version: "3.0"
---
# Prozkoumat
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Shrnutí
Tato stránka popisuje stránku **Prozkoumat**, která zobrazuje obsah **jednoho katalogu** jako strom adresářů a seznam souborů, **i když jeho zařízení není připojeno**:
* [strom adresářů](#directory-tree) katalogu, vlevo,
* [seznam souborů](#file-list) vybrané složky, vpravo, s velikostí každé podsložky,
* [místní nabídka](#file-context-menu) souboru nebo složky,
* [označení adresáře](#tag-folder) stromu štítkem.

![Stránka Prozkoumat: strom adresářů vlevo, seznam souborů vybrané složky vpravo](/img/explore_01_overview.png)

## Otevření katalogu {#opening}
Katalog se na stránce Prozkoumat otevře volbou *Prozkoumat* v místní nabídce katalogu:
* na stránce [Výběr](Selection#context-menu): katalog se zároveň vybere,
* na stránce [Zařízení](Devices),
* z výsledku vyhledávání volbou *Prozkoumat složku* (viz [Výsledky vyhledávání](SearchResults#file-context-menu)): Prozkoumat se otevře přímo ve složce souboru.

Dokud je stránka Prozkoumat otevřená, **kliknutím na katalog** na stránce [Výběr](Selection) se katalog otevře také v ní.

V režimu *Paměť* se soubory katalogu nejprve načtou do paměti. U velkého katalogu to může chvíli trvat: průběh se zobrazuje v panelu aktivity v dolní části okna a *Zastavit* načítání zruší (Prozkoumat pak dál zobrazuje předchozí katalog). Otevření jiného katalogu během načítání ho nahradí. Již načtený katalog se otevře okamžitě.

*Prozkoumat* v postranním panelu znovu zobrazí stránku s posledním katalogem prozkoumaným od spuštění Katalogu.<br/>
*Zavřít* (nebo `Esc`) stránku zavře. Pokud byla otevřena z výsledku vyhledávání, vrátí se ke stejným výsledkům.

## Strom adresářů {#directory-tree}
Levý panel ukazuje počet adresářů katalogu a jeho **celkovou velikost**, pod tím strom jeho adresářů.<br/>
Tažením čáry mezi stromem a seznamem souborů strom rozšíříte nebo zúžíte; šířka se zachová pro příště a sdílí se s Katalogem 2.

Při otevření katalogu strom ukazuje složku katalogu a první dvě úrovně adresářů pod ní; hlubší úrovně jsou zpočátku sbalené.<br/>
Adresář, který obsahuje podadresáře, má šipku: kliknutím na ni tuto větev sbalíte nebo rozbalíte.<br/>
Kliknutím na adresář zobrazíte jeho obsah v seznamu souborů.

Čtyři tlačítka nad stromem mění, jak velká část stromu je zobrazena najednou:

| Tlačítko | Účinek |
|----------|--------|
| *Sbalit o jednu úroveň* | Sbalí nejhlubší aktuálně zobrazenou úroveň. |
| *Rozbalit o jednu úroveň* | Rozbalí o jednu úroveň více. |
| *Sbalit vše* | Sbalí vše zpět na složku katalogu. |
| *Rozbalit vše* | Rozbalí všechny adresáře katalogu. |

Tlačítko je zašedlé, když už není co sbalit nebo rozbalit.<br/>
Otevření podsložky ze seznamu souborů rozbalí její větev, takže vybraný adresář je ve stromu vždy vidět.<br/>
Sbalené a rozbalené větve se neukládají: strom začíná znovu od prvních dvou úrovní při každém otevření katalogu.

### Označit tuto složku štítkem {#tag-folder}
**Kliknutím pravým tlačítkem** na adresář stromu (nebo dlouhým stiskem) otevřete jeho nabídku s jedinou položkou: *Označit tuto složku štítkem*.<br/>
Ta otevře stránku [Štítky](Tags) přes celou stránku Prozkoumat, s již vyplněnou cestou k tomuto adresáři; tam zvolte štítek a potvrďte. Adresář vybraný ve stromu a seznam souborů se nemění.<br/>
*Zavřít* (nebo `Esc`) na stránce Štítky pak vrátí na stejnou stránku Prozkoumat a stejný adresář.

## Seznam souborů {#file-list}
Horní část seznamu souborů ukazuje cestu k vybrané složce a možnosti zobrazení:

| Možnost | Účinek |
|---------|--------|
| *Zobrazit složky* | Zobrazí podsložky spolu se soubory. |
| *a všechny podsložky* | Zobrazí obsah všech podsložek v libovolné hloubce. Dostupné se *Zobrazit složky*. |
| *Řadit složky jako první* | Seřadí seznam se složkami na začátku, potom soubory. Dostupné se *Zobrazit složky*. |

Pod tím *Počet souborů* a *Celková velikost* udávají soubory **přímo** ve vybrané složce, bez jejích podsložek.

### Velikosti složek
Každá zobrazená složka ukazuje ve sloupci *Velikost* celkovou velikost **všeho, co obsahuje**, v libovolné hloubce. Složka bez souborů má velikost 0.<br/>
Největší složky tak lze najít bez připojení zařízení.

Táž složka může ukazovat dvě různá čísla: svou velikost v seznamu nadřazené složky (vše, co obsahuje) a svou *Celková velikost*, když je vybrána (pouze soubory přímo v ní).<br/>
S volbou *a všechny podsložky* se složka započítává i do velikosti každé složky nad ní: velikosti v seznamu se nesčítají do celku.

### Sloupce
| Sloupec | Obsah |
|---------|-------|
| *Název* | Název souboru nebo složky s ikonou jeho typu. |
| *Velikost* | Velikost souboru nebo celková velikost složky. |
| *Datum* | Datum poslední změny. |
| *Typ souboru*, *Typ MIME* | Typ souboru. |
| *Šířka*, *Výška*, *Délka* | Rozměry obrázků a videí, délka zvukových a video souborů. |
| *Umělec*, *Album*, *Název* | U zvukových souborů. |
| *Kontrolní součet (SHA256)*, *Datum kontrolního součtu* | Kontrolní součet souboru a datum jeho výpočtu. |

Sloupce *Šířka* až *Název* se zobrazují pouze u katalogů, které obsahují metadata, a sloupce kontrolního součtu jsou vyplněny pouze u katalogů vytvořených s kontrolními součty (viz [Vytvořit](Create)).

### Práce se seznamem
* **Kliknutím** na soubor ho otevřete ve výchozí aplikaci. **Kliknutím** na složku ji otevřete na stránce Prozkoumat.
* **Kliknutím pravým tlačítkem** na řádek (nebo dlouhým stiskem) otevřete jeho [místní nabídku](#file-context-menu).
* **Kliknutím na nadpis sloupce** seřadíte podle tohoto sloupce; dalším kliknutím pořadí obrátíte. Řazení se zachová pro další návštěvy.
* **Přetažením** čáry mezi dvěma nadpisy sloupců změníte šířku sloupce; **dvojitým kliknutím** na ni přizpůsobíte sloupec obsahu. Šířka sloupce *Název* se zachová.
* Klávesnice funguje jako ve [výsledcích vyhledávání](SearchResults#keyboard): šipky, `Page Up` `Page Down`, `Home` `End`, `Enter` pro otevření, `Menu` nebo `Shift+F10` pro místní nabídku, psaní písmen pro přechod na název.

Když zařízení katalogu není dostupné (odpojené nebo se změnila jeho cesta), katalog lze stále procházet, ale otevření souboru nebo složky ve správci souborů se odmítne a v dolní části okna se krátce zobrazí zpráva: *Zařízení není aktivní. Může být odpojeno nebo se změnila jeho cesta.*

Když je zařízení dostupné, ale soubor nebo složka už neexistuje (například když je potřeba katalog aktualizovat), otevření se také odmítne a zpráva zní *Soubor nebyl nalezen nebo jej nelze přečíst* nebo *Složka nebyla nalezena.*

## Místní nabídka {#file-context-menu}

![Místní nabídka souboru na stránce Prozkoumat](/img/explore_02_context_menu.png)

Kliknutí pravým tlačítkem na řádek otevře nabídku přizpůsobenou typu řádku.

### U souboru
| Položka | Akce |
|---------|------|
| *Otevřít soubor* | Otevře soubor ve výchozí aplikaci. |
| *Otevřít složku* | Otevře složku souboru ve správci souborů. |
| *Zobrazit rozšířená metadata (JSON)* | Zobrazí všechna metadata uložená pro soubor jako tabulku, s tlačítkem *Kopírovat JSON*. Zobrazeno, když katalog obsahuje rozšířená metadata. |
| *Kopírovat název souboru bez přípony* | Zkopíruje `report_2025` z `/home/username/Documents/report_2025.pdf`. |
| *Kopírovat název souboru s příponou* | Zkopíruje `report_2025.pdf`. |
| *Kopírovat cestu ke složce* | Zkopíruje `/home/username/Documents`. |
| *Kopírovat absolutní cestu k souboru* | Zkopíruje `/home/username/Documents/report_2025.pdf`. |
| *Kopírovat kontrolní součet* | Zkopíruje uložený kontrolní součet. Zobrazeno, když ho soubor má. |
| *Vypočítat kontrolní součet (SHA-256)* | Vypočítá kontrolní součet souboru a uloží ho. Zobrazeno, když soubor ještě nemá kontrolní součet. |
| *Ověřit kontrolní součet (SHA-256)* | Znovu vypočítá kontrolní součet a porovná ho s uloženým. Pokud se liší, zobrazí se obě hodnoty s tlačítkem *Aktualizovat kontrolní součet* pro uložení nové hodnoty. |
| *Přesunout soubor do koše* | Přesune soubor do systémového koše. |
| *Smazat soubor* | Po potvrzení soubor **trvale** smaže. Dostupné pouze tehdy, když je v [Nastavení](Settings#behavior) zapnuto *Povolit akce, které mažou soubory (bez možnosti obnovení)*. |

### U složky
| Položka | Akce |
|---------|------|
| *Otevřít složku* | Otevře složku ve správci souborů. |
| *Kopírovat název složky* | Zkopíruje `Documents` z `/home/username/Documents`. |
| *Kopírovat cestu ke složce* | Zkopíruje `/home/username/Documents`. |
| *Přesunout složku do koše* | Přesune složku a její obsah do systémového koše. |

## Vývoj
* Viz backlog [vývoje stránky Prozkoumat](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=explore).
