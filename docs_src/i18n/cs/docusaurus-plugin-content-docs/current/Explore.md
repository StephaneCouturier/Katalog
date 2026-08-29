---
version: "2.13"
---
# Prozkoumat
![2.13](https://img.shields.io/badge/Version-2.13-blue)

## Shrnutí
Tato stránka popisuje všechny funkce obrazovky **Prozkoumat** a jak je používat.

Cílem této obrazovky je **prozkoumat obsah katalogu souborů**, i když fyzické zařízení není připojeno.

Toto zobrazení načte data po kliknutí pravým tlačítkem na katalog z panelu **[Výběr](Selection)** nebo zobrazení seznamu **[Katalogy](DevicesCatalogs)** a výběru položky *Prozkoumat* z kontextové nabídky.

![Obrazovka Prozkoumat zobrazující stromové zobrazení adresářů vlevo a seznam souborů vpravo](/img/explore_01.png)

## Funkce

Obrazovka je rozdělena do dvou panelů:

- **Levý panel** — stromové zobrazení adresářů katalogu. Kliknutím na adresář se zobrazí jeho soubory v pravém panelu.
- **Pravý panel** — seznam souborů pro vybraný adresář.

Kliknutím na soubor v pravém panelu se aplikace pokusí otevřít jej výchozí aplikací systému, pokud je zařízení připojeno. Kliknutím na položku složky se přejde do této složky.

Jsou-li složky zobrazeny, u každé se uvádí její celková velikost — soubory v ní obsažené plus vše ve složkách pod ní — takže lze největší složky odhalit bez připojení zařízení. Celková velikost celého katalogu se zobrazuje nahoře vedle cesty ke složce.

### Stromové zobrazení adresářů

Levý panel se otevře na kořenu katalogu a jeho prvních dvou úrovních adresářů; hlubší úrovně jsou zpočátku sbalené.

Adresář, který obsahuje podadresáře, nese malou šipku: kliknutím na ni se daná větev sbalí nebo rozbalí. Adresáře bez podadresářů šipku nemají a všechny řádky zůstávají zarovnané.

Čtyři tlačítka nad stromem mění, kolik hierarchie je zobrazeno najednou:

| Tlačítko | Účinek |
|----------|--------|
| *Sbalit o jednu úroveň* | Sbalí nejhlubší aktuálně zobrazenou úroveň |
| *Rozbalit o jednu úroveň* | Rozbalí o jednu úroveň stromu více |
| *Sbalit vše* | Sbalí vše zpět na kořen katalogu |
| *Rozbalit vše* | Rozbalí každý adresář katalogu |

Tlačítko je zašedlé, pokud už nemá co sbalit nebo rozbalit.

Přechod do složky z pravého panelu rozbalí větev, do které složka patří, takže vybraný adresář je ve stromu vždy viditelný.

Strom se vždy znovu otevře na kořenu katalogu a jeho prvních dvou úrovních: sbalený nebo rozbalený stav se mezi návštěvami neuchovává.

### Možnosti zobrazení

Tři možnosti ovládají, co se zobrazuje v seznamu souborů:

- **Zobrazit složky** — pokud je povoleno, zobrazují se položky složek spolu se soubory. Povolením této možnosti se aktivují i dvě níže uvedené možnosti.
- **Zobrazit podsložky** — pokud je povoleno, soubory ze všech podsložek jsou uvedeny společně v seznamu souborů.
- **Složky napřed** — tlačítko, které přeřadí seznam tak, aby se nejprve zobrazily složky (abecedně), pak soubory (abecedně).

## Kontextová nabídka adresářů (kliknutí pravým tlačítkem) {#directory-context-menu}

Kliknutí pravým tlačítkem na adresář v levém panelu zobrazí:

![Kontextová nabídka adresáře s možností označit složku](/img/explore_02_context.png)

- *Označit tuto složku* — otevře obrazovku [Štítky](Tags) s předvyplněnou touto složkou pro přiřazení štítku.

## Kontextová nabídka souborů a složek (kliknutí pravým tlačítkem) {#file-context-menu}

Kliknutí pravým tlačítkem na položku v pravém panelu zobrazí kontextovou nabídku přizpůsobenou typu vybrané položky.

![Kontextová nabídka souborů zobrazující operace: otevřít, kopírovat, kontrolní součet a smazat](/img/explore_03_context.png)

### Pro soubory

| Akce | Popis |
|------|-------|
| *Otevřít soubor* | Otevře soubor výchozí aplikací systému |
| *Otevřít složku* | Otevře nadřazenou složku souboru ve správci souborů |
| *Zobrazit rozšířená metadata (JSON)* | Zobrazí podrobná metadata (k dispozici pouze pokud byl katalog indexován s rozšířenými metadaty) |
| *Kopírovat cestu složky* | Zkopíruje cestu nadřazené složky do schránky |
| *Kopírovat absolutní cestu souboru* | Zkopíruje úplnou cestu souboru do schránky |
| *Kopírovat název souboru s příponou* | Zkopíruje název souboru (s příponou) do schránky |
| *Kopírovat název souboru bez přípony* | Zkopíruje název souboru (bez přípony) do schránky |
| *Kopírovat kontrolní součet* | Zkopíruje uložený kontrolní součet do schránky (zobrazí se pouze pokud je kontrolní součet uložen) |
| *Vypočítat kontrolní součet (SHA-256)* | Vypočítá a uloží kontrolní součet SHA-256 souboru (zobrazí se pouze pokud ještě není kontrolní součet uložen) |
| *Ověřit kontrolní součet (SHA-256)* | Přepočítá kontrolní součet a porovná ho s uloženou hodnotou (zobrazí se pouze pokud je kontrolní součet uložen) |
| *Přesunout soubor do koše* | Přesune soubor do systémového koše |
| *Smazat soubor* | Trvale smaže soubor |

### Pro složky

| Akce | Popis |
|------|-------|
| *Otevřít složku* | Otevře složku ve správci souborů systému |
| *Kopírovat cestu složky* | Zkopíruje cestu složky do schránky |
| *Kopírovat název složky* | Zkopíruje název složky do schránky |
| *Přesunout složku do koše* | Přesune složku do systémového koše |

## Vývoj
Některé nápady na vývoj této obrazovky:
* Podívejte se na nevyřízené položky [vývoje Prozkoumat](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=explore).
