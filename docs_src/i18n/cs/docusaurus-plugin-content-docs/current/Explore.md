---
version: "2.12"
---
# Prozkoumat
![2.12](https://img.shields.io/badge/Version-2.12-blue)

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
