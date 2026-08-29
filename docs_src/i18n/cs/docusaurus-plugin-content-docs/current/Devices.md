---
version: "2.13"
---
# Zařízení
![2.13](https://img.shields.io/badge/Version-2.13-blue)

## Souhrn
Tato stránka popisuje koncept **Zařízení** v Katalogu a horní část obrazovky **Zařízení**.

![Příklad zobrazující katalogy uspořádané pod fyzickými disky a virtuální skupinou Photos](/img/devices_example1_cut.png)

V tomto příkladu bylo vytvořeno několik katalogů ze 2 fyzických disků.<br/>
Katalogy s fotkami/obrázky byly přiřazeny k virtuálnímu zařízení *Photos*.<br/>
To umožňuje vyhledávat pouze v nich a poskytuje celkový počet a velikost fotografických souborů.

## Model

### Definice

* Zařízení **[Katalogu](DevicesCatalogs)** je index souborů z určitého adresáře.

* Zařízení **[Úložiště](DevicesStorage)** je fyzický disk, na kterém jsou uloženy soubory. Obvykle je připojen k počítači a má fyzický úložný prostor.

* **Virtuální** zařízení je jakákoli nefyzická položka používaná ke seskupování dalších zařízení. Samo o sobě nemá žádné vlastnosti a může agregovat součty ze souvisejících podzařízení.

* **Skupina** je virtuální zařízení na vrcholu hierarchie.

    * **Fyzická skupina** je jedinečná a vyhrazená skupina pro hierarchii fyzických zařízení (počítač, telefon, disk atd.).

    * Jakákoli jiná je **Virtuální skupina**, ke které lze přiřadit existující katalogy pro usnadnění vyhledávání a statistiky.

* Každé zařízení může mít **Komentář**: volný textový popisek, který se zadává ve formuláři úprav zařízení a zobrazuje se vedle názvu zařízení v seznamu zařízení.

### Hierarchie

![Diagram zobrazující hierarchii zařízení se skupinami, úložišti a katalogy](/img/devices_model.png)

## Funkce

Vždy dostupné v horní části obrazovky:

### Výběr mezi 3 zobrazeními

Zařízení lze zobrazovat a spravovat třemi způsoby:

**[Strom zařízení](DevicesTree)**: Zobrazuje úplný a nefiltrovaný seznam všech zařízení v hierarchické / stromové struktuře.

**[Seznam úložiště](DevicesStorage)**: Zobrazuje pouze úložná zařízení, filtrovaná podle panelu [Výběr](Selection).

**[Seznam katalogů](DevicesCatalogs)**: Zobrazuje pouze katalogová zařízení, filtrovaná podle panelu [Výběr](Selection).

### Zobrazit celou tabulku
Pokud je povoleno, zobrazí se v aktuálním zobrazení všechny dostupné sloupce.

Pokud není zaškrtnuto, sloupce, které nejsou každodenně potřebné (jako interní ID), jsou skryty, čímž je zobrazení jednodušší a čitelnější.

### Aktualizovat aktivní zařízení
Aktualizuje aktuálně vybrané zařízení — znovu prohledá jeho soubory ze zdrojové cesty.

Toto tlačítko je povoleno, pokud je v některém ze tří zobrazení vybráno zařízení s dostupnou zdrojovou cestou (zobrazeno barevnou ikonou).

### Zaznamenat snímek
Zaznamená aktuální hodnoty všech zařízení (počet souborů, velikost souborů, volné místo, celkový prostor) nezávisle na aktuálním výběru nebo filtrech.

Po zaznamenání se zobrazí souhrn s novými součty a změnou oproti předchozímu snímku (delta).

Tyto záznamy podporují [Statistiku](Statistics) a umožňují sledovat sbírku globálně v průběhu času.
