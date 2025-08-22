# Vytvořit
## Souhrn
Tato stránka popisuje všechny funkce obrazovky **Vytvořit** a jak je používat.<br/>
Na této obrazovce může uživatel **vytvořit katalog souborů**.<br/>
Provádí se ve 3 hlavních krocích:
1. Vyberte cestu ke zdroji: zařízení nebo adresář se soubory, které mají být zahrnuty do nového katalogu.
1. Vyberte možnosti pro zahrnutí nebo vyloučení některých konkrétních souborů.
1. Vyberte [Úložiště](DevicesStorage) a definujte název katalogu a vytvořte katalog.

![](/img/screen_create_01.png)
## Vyberte cestu ke zdroji
Existují 3 způsoby, jak vybrat zdrojovou cestu k adresáři se soubory, které mají být zahrnuty do nového [Katalogu](DevicesCatalogs):
1. zadáním cesty v zóně pro úpravy textu
1. pomocí stromového zobrazení systému souborů stačí rozbalit a kliknout na správné zařízení nebo adresář
1. nebo kliknutím na tlačítko *Vybrat*, které otevře dialogové okno pro pomoc s výběrem složky.

Vybraná cesta se vždy objeví v zóně pro úpravy textu a aplikace tuto cestu použije k procházení a katalogizaci jejího obsahu.

## Vyberte možnosti pro zahrnutí/vyloučení souborů
### zahrnout Typ souboru
Obsah může být omezen na určitý typ souborů, 4 jsou k dispozici a budou zahrnovat soubory s příponami, jak je uvedeno zde:
 | Typ | Rozšíření |
 | ------| -------------------------------------------------- |
 | Audio | aif, mp3, ogg, wav |
 | Obrázek | png, jpg, gif, xcf, tif, bmp, raw |
 | Text | txt, pdf, odt, idx, html, rtf, doc, docx, epub |
 | Video | wmv, avi, mp4, mkv, flv, webm, m4v, vob, ogv, mov |

Tato možnost bude použitelná pro katalog, který bude pokračovat.<br/>
Lze jej později změnit úpravou [Katalogu](DevicesCatalogs).

### Jiné možnosti:
#### Zahrnout skryté soubory
Skryté soubory nejsou ve výchozím nastavení zahrnuty, ale tato volba je umožňuje zahrnout.<br/>
Tato možnost bude použitelná pro katalog, který bude pokračovat.<br/>
Lze jej později změnit úpravou [Katalogu](DevicesCatalogs).

#### Vyloučit adresáře
Z katalogizace je možné vyloučit celé adresáře.<br/>
Zadejte cestu k adresáři a kliknutím na tlačítko *Přidat adresář k vyloučení z katalogů*.<br/>
Adresář je pak viditelný v seznamu níže.<br/>
Libovolný adresář lze odstranit kliknutím pravým tlačítkem a poté jej zobrazit v seznamu níže.<br/>
Poznámka: Tato vyloučení jsou **globální**, což znamená, že tyto složky budou vyloučeny pro všechny katalogy.<br/>

![](/img/screen_create_04_exclude.png)

## Definujte a vytvořte katalog
#### Vyberte úložné zařízení
Katalog musí být spojen s fyzickým zařízením [Úložiště](DevicesStorage), aby se usnadnilo pozdější vyhledávání nebo statistiky.<br/>
Ve výchozím nastavení Katalog předvytváří výchozí úložné zařízení, místní disk.<br/>
Toto lze později aktualizovat na obrazovce virtuálního stromu [Zařízení](DevicesTree).<br/>
Pokud pro tento katalog potřebujete jiné a nové úložiště, klikněte na *Přidat úložiště* a přidejte jej pomocí obrazovek [Zařízení](DevicesTree) nebo [Úložiště](DevicesStorage).

Tato volba se bude vztahovat na další katalog.<br/>
Lze jej později změnit úpravou [Katalogu](DevicesCatalogs).

#### Zadejte jméno
Zadejte název katalogu.<br/>
Duplicitní názvy v současné době nejsou povoleny.

Tlačítko *Generovat* může vytvořit název na základě cesty ke složce a nahradit lomítka <code>/</code> podtržítkem <code>_</code>.

#### Vytvořte katalog
Až budete připraveni, klikněte na tlačítko *Vytvořit katalog* pro uložení samotného katalogu a zahájení procesu katalogizace obsahu cesty rekurzivně (budou zahrnuty všechny podadresáře).

Jakmile je proces dokončen,
- Zpráva potvrdí vytvoření a poskytne počet souborů a celkovou velikost souboru vybrané složky pro tento katalog.
- váš místní disk (úložné zařízení, které bylo přidáno automaticky) byl také aktualizován a zpráva poskytuje pohled na volné, použité a celkové místo:

![](/img/screen_create_02.png)

zobrazí se obrazovka [Zařízení](DevicesTree) pro zobrazení katalogu ve stromu zařízení.

Nový katalog se automaticky vybere na panelu [Výběr](Selection) a je připraven k použití k [Vyhledávání](Search) obsahu.

## Vývoj
Některé nápady na vývoj této obrazovky:
* pro přizpůsobení typů souborů a/nebo použití mimetypes
* vyloučit složky podle katalogu (nejen globálně)
* vyloučit složky podle názvu (není třeba uvádět celou cestu)
* Další informace najdete v nevyřízeném záznamu [Vytvořit vývoj](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=create).
