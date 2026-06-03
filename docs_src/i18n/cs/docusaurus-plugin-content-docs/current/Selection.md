---
version: "2.12"
---
# Výběr
![2.12](https://img.shields.io/badge/Version-2.12-blue)

## Shrnutí
Tato stránka popisuje všechny funkce panelu **Výběr**, levé části uživatelského rozhraní.
* Tento panel se používá k upřesnění výběru pro různé obrazovky a funkce.
* V režimu *Hledat v katalozích souborů* bude výběr filtrovat informace pro obrazovky [Vyhledávání](Search), [Zařízení](Devices), [Vytvořit](Create) a [Statistika](Statistics).
* V režimu *Hledat v připojených jednotkách* lze vybrat adresář přímo z připojených zařízení. Toto se používá pouze pro obrazovku [Vyhledávání](Search).

![Panel Výběr zobrazující stromové zobrazení zařízení a ovládací prvky výběru](/img/selection_01.png)

## Rozhraní
Horní tlačítka:
* *Zobrazit/Skrýt*: tlačítko vlevo nahoře lze použít ke skrytí a opětovnému zobrazení panelu.
* *Resetovat*: toto tlačítko s ikonou smetáku resetuje aktuální výběr, takže jsou vybrána všechna data/katalogy.
* *Znovu načíst*: znovu načte data celé kolekce, což pomáhá aktualizovat aplikaci pro změny dat provedené mimo ni.

## Hledat v katalozích
Informace o výběru:
Tato část zobrazuje aktuální výběr virtuálního, úložného nebo katalogového zařízení.

Dvě tlačítka vedle štítku stromu zařízení umožňují sbalit nebo rozbalit strom o jednu úroveň.

### Kontextová nabídka (kliknutí pravým tlačítkem)

Kontextová nabídka se liší podle typu vybraného zařízení.

**Pro úložná a virtuální zařízení:**
* *Hledat*: vybere položku a přejde na obrazovku Vyhledávání (zobrazí se pouze pokud obrazovka Vyhledávání není již aktivní)
* *Aktualizovat*: spustí aktualizaci (skenování souborů) vybraného zařízení a všech jeho katalogů

**Pro katalogová zařízení:**
* *Hledat*: vybere položku a přejde na obrazovku Vyhledávání (zobrazí se pouze pokud obrazovka Vyhledávání není již aktivní)
* *Aktualizovat*: spustí aktualizaci (skenování souborů) vybraného katalogu (dostupné pouze pokud je katalog aktivní)
* *Prozkoumat*: otevře vybraný katalog na obrazovce [Prozkoumat](Explore)
* *Otevřít složku*: otevře zdrojovou složku katalogu ve správci souborů systému (zobrazí se pouze pokud je definována zdrojová cesta)

## Hledat v připojených discích
Pomocí této možnosti lze vyhledávat přímo v libovolném adresáři připojeného disku, bez potřeby katalogu.

Pro výběr složky pro vyhledávání použijte tlačítko *Vybrat cestu* nebo stromové zobrazení adresářů.

![Panel Výběr v režimu připojených disků zobrazující stromové zobrazení adresářů a výběr cesty](/img/selection_02.png)
