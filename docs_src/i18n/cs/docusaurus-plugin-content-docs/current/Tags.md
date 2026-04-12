---
version: "2.11"
---
# Štítky
![2.11](https://img.shields.io/badge/Version-2.11-blue)

## Souhrn
Tato stránka popisuje všechny funkce obrazovky **Štítky** a jak je používat.<br/>
Na této obrazovce může uživatel **přiřadit několik značek libovolné složce**.<br/>
To pak lze použít na obrazovce [Vyhledávání](Search) k upřesnění výsledků;<br/>

Označení složky se provádí ve 3 hlavních krocích:
1. Vyberte zdrojovou cestu složky,
1. Vyberte nebo vytvořte nový štítek,
1. Klepnutím označte složku.

![Obrazovka Štítky zobrazující rozhraní pro přiřazování štítků](/img/screen_tags_01.png)

## Přiřaďte štítky adresářům
Poznámka: Štítky jsou přiřazeny k absolutní cestě ke složce, proto jsou nezávislé na hierarchii zařízení a katalozích.

### Vyberte cestu ke zdroji
Existují 3 způsoby, jak vybrat zdrojovou cestu ke složce se soubory, které mají být zahrnuty do nového katalogu:
1. zadáním cesty v zóně pro úpravy textu
1. pomocí stromového zobrazení systému souborů stačí rozbalit a kliknout na správné zařízení nebo složku
1. nebo kliknutím na tlačítko *Vybrat*, které otevře dialogové okno pro pomoc s výběrem složky.

Vybraná cesta se vždy objeví v zóně pro úpravy textu a aplikace tuto cestu použije pro značku.

### Vyberte existující značku nebo ji vytvořte
* zadejte nový název značky v oblasti úprav pod "Vybrat značku"
* nebo kliknutím na libovolnou položku v seznamu existujících značek znovu použijte existující

### Přiřaďte značku vybrané složce
Klikněte na tlačítko "Označit složku".<br/>
Tím se zaznamená spojení mezi složkou a názvem značky.<br/>
Seznam "Aktuální složky a značky" se aktualizuje novým záznamem.

## Úprava značek
### Smazat existující přidružení
V seznamu "Aktuální složky a značky" klikněte pravým tlačítkem na asociaci, kterou chcete odstranit, a vyberte *Odstranit tuto značku*.
### Upravte soubor značek
V režimu *Paměť* lze pomocí tlačítka *Otevřít soubor* přímo upravit zdrojový soubor (formát by se neměl měnit).


## Vývoj
Některé nápady na vývoj této obrazovky:
* Označte zařízení
* Složky značek v konkrétních katalozích
* Další informace najdete v nevyřízeném záznamu [Vývoj značek](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=tag).
