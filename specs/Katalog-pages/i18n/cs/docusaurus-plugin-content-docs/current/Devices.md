# Zařízení
## Souhrn
Tato stránka popisuje koncept **Zařízení** v Katalogu a horní část obrazovky **Zařízení**.

![](/img/devices_example1_cut.png)

V tomto příkladu bylo vytvořeno několik katalogů ze 2 fyzických jednotek.<br/>
Katalogy s fotkami/obrázky/obrázky byly přiřazeny k virtuálnímu zařízení *Photos*.<br/>
To umožňuje vyhledávat pouze v nich a poskytuje celkový počet a velikost souborů fotografií.<br/>

## Modelka


### Definice

* Zařízení **[Katalogu](DevicesCatalogs)** je seznam souborů v určitém adresáři.

* Zařízení **[Úložiště](DevicesStorage)** je fyzický disk, na kterém jsou uloženy soubory. Obvykle je „připojen“ nebo „připojen“ k počítači a má fyzický úložný prostor.

* **Virtuální** zařízení je jakákoli nefyzická položka používaná k seskupování dalších zařízení. Samo o sobě nemá žádné vlastnosti a může agregovat čísla ze souvisejících dílčích zařízení.

* **Skupina** je virtuální zařízení na vrcholu hierarchie.

 * **Fyzická skupina** je jedinečná a vyhrazená skupina pro hierarchii fyzických zařízení (počítač, telefon, disk atd.).

 * Jakákoli jiná je **Viruální skupina**, ke které lze „přiřadit“ existující katalogy pro usnadnění vyhledávání a statistiky.

### Hierarchie

![](/img/devices_model.png)


## Funkce

Ty jsou vždy k dispozici v horní části obrazovky.

### Výběr mezi 3 pohledy

Zařízení lze vypsat a spravovat 3 způsoby:

**[Strom zařízení](DevicesTree)**: Toto zobrazení zobrazuje úplný a nefiltrovaný seznam zařízení v hierarchii / stromové struktuře.

**[Seznam úložiště](DevicesStorage)**: Toto zobrazení zobrazuje pouze úložná zařízení a je filtrováno na základě panelu [Výběr](Selection).

**[Seznam katalogů](DevicesCatalogs)**: Toto zobrazení zobrazuje pouze katalogová zařízení a je filtrováno na základě panelu [Výběr](Selection).

### Zobrazit celou tabulku
Klepnutím na tuto možnost zobrazíte všechna data dostupná v zobrazení.

Pokud není zaškrtnuto, obvykle se skryjí data, která nemusí být každý den potřeba (např. interní ID).

To může pomoci zachovat jednodušší a čitelnější pohled.

### Zaznamenejte snímek dat
Toto tlačítko spustí záznam všech hodnot zařízení (velikost, soubory, místo atd.) nezávisle na aktuálním výběru.

Tyto záznamy podporují vytváření [Statistiky](Statistics), a to zejména pro sledování sbírky globálně a nezávisle na jednotlivých aktualizacích zařízení.
