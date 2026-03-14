---
version: "2.10"
---
# Přehled
![2.10](https://img.shields.io/badge/Version-2.10-blue)

## Indexování souborů a správce úložných zařízení
Katalog je výkonný <b>správce katalogů souborů</b>, který vám pomáhá organizovat a vyhledávat soubory.<br/>
Poskytuje úplný přehled souborů napříč <b>více úložnými zařízeními bez nutnosti jejich připojení</b>.<br/>
Katalog vytváří komplexní <b>indexy, abyste mohli prohledávat celou svou kolekci souborů z jednoho místa</b>, ať už spravujete interní a externí disky, USB flash disky, síťová úložiště nebo optické disky (Blu-ray, DVD, CD).

- **Katalogizujte vše**: Vytvářejte podrobné indexy souborů z jakéhokoli úložného zařízení
- **Vyhledávejte offline**: Najděte soubory okamžitě bez připojení nebo připojení původního zařízení
- **Organizujte svou kolekci**: Spravujte více úložných zařízení a jejich katalogy v jednotné hierarchii


![Výsledky vyhledávání napříč více katalogy](/img/screen_search_01.png)
<b>8 hlavních záložek pro 8 hlavních funkcí</b>
![8 hlavních záložek Katalog](/img/global_tabwidget.png)
1. [Hledat](Search) soubory napříč více úložnými zařízeními **bez nutnosti jejich připojení**
1. Organizovat úložná [Zařízení](Devices) a jejich katalogy v jednotné hierarchii s **Virtuálními** zařízeními
1. [Procházet](Explore) hierarchii katalogů a soubory
1. [Vytvořit](Create) katalogy **souborů**
1. Získat [Statistiky](Statistics) o vašich kolekcích souborů a využití úložiště
1. Přizpůsobit [Štítky](Tags) a přiřadit je adresářům pro další možnosti vyhledávání
1. Porovnat katalogy [Záloha](BackUp) pro potvrzení pokrytí zálohovaných souborů a složek mezi zdrojovými a cílovými zařízeními
1. Přizpůsobit si zážitek pomocí [Nastavení](Settings), jako je jazyk a téma
<br/>

<div className="row">
  <div className="col col--6">
  <br/><br/>a panel pro [Výběr](Selection)
  <br/>
  * Vyberte vyhledávání v <b>Katalozích</b> nebo přímo v <b>Připojených discích</b><br/>
  * Nastavte zařízení v <b>hierarchii, která se má použít</b> pro *Hledat*, *Vytvořit* (nadřazené zařízení), *Statistiky* nebo správu *Zálohy*<br/>
  </div>
  <div className="col col--6" style={{maxWidth: '200px'}}>
    ![Panel výběru](/img/global_selection_panel.png)
  </div>
</div>

---
## Klíčové funkce

### Bohaté informace o souborech
- **[Inteligence typu souboru](Create#enhanced-file-type-filtering)**: Standardní detekce z přípon a ověření typu MIME
- **[Možnosti katalogu](Create#select-options-to-includeexclude-files)**: Zahrňte pouze [typ souborů](Create#enhanced-file-type-filtering) a [zahrňte/vyloučte adresáře nebo skryté soubory](Create#other-options)
- **[Extrakce metadat](Create#metadata-extraction)**: Automaticky extrahujte metadata z obrázků (rozměry, informace o fotoaparátu), videí (trvání, rozlišení) a zvukových souborů (interpret, album, trvání) nebo jakéhokoli jiného typu souboru.
- **[Kontrolní součet](Create#file-checksum)**: Výpočet SHA256 pro detekci duplicit a rozdílů
- **[Systém štítků složek](Tags)**: Organizujte a kategorizujte složky pomocí vlastních štítků

### Výkonné vyhledávání a objevování
- **[Pokročilé parametry vyhledávání](Search#search-text-criteria)**: Najděte soubory podle názvu, cesty, velikosti, data, typu souboru a metadat
- **[Inteligentní filtrování](Search#file-criteria)**: Použijte více kritérií současně pro rychlé zúžení výsledků
- **[Najít duplikáty](Search#duplicates-on)**: Identifikujte duplicitní soubory napříč různými úložnými zařízeními
- **[Najít rozdíly](Search#differences-on)**: Zobrazte rozdíly mezi dvěma umístěními úložiště nebo verzemi záloh


### Správa zařízení a katalogů
- **[Organizace zařízení](Devices)**: Organizujte úložná zařízení v hierarchické struktuře (Virtuální > Úložiště > Katalogy)
- **[Aktualizace](DevicesCatalogs)**: Udržujte katalogy aktuální pomocí ručních aktualizací nebo automatických aktualizací
- **[Podpora importu](DevicesCatalogs#import)**: Importujte katalogy z jiných nástrojů, jako je VVV


### Analýza a správa
- **[Průzkumník souborů](Explore)**: Procházejte obsah katalogu, jako by bylo zařízení připojeno
- **[Statistiky](Statistics)**: Sledujte své kolekce souborů a využití úložiště
- **[Správa záloh a archivů](BackUp)**: Kopírujte nebo přesouvejte soubory mezi katalogy, porovnávejte pokrytí záloh a spravujte zálohovací vazby
- **[Dávkové operace](Search#batch-process)**: Exportujte výsledky a provádějte hromadné akce na souborech

### Pokročilé funkce
- **[Rozhraní příkazového řádku](CommandLines)**: Automatizujte aktualizace katalogů a vyhledávání pomocí příkazového řádku (Linux)
- **[Flexibilita databáze](Settings#collection)**: Vyberte si mezi úložištěm souborů CSV nebo databází SQLite

---
## Podpora více platforem
| Hlavní operační systém | Distribuce / Verze    | Balíčky    |
|-------------------|-------------|-------------|
| <div style={{display: 'flex', alignItems: 'center', gap: '10px'}}><img src={require('/img/linux.png').default} width="40" /> GNU/Linux</div>         | Jakýkoli 64bitový, glibc 2.38+ <br/>Jakýkoli 32bitový, glibc 2.35     | AppImage <br/>Přenosný |
| <div style={{display: 'flex', alignItems: 'center', gap: '10px'}}><img src={require('/img/windows.png').default} width="40" /> Microsoft Windows</div> | 64bitový: Windows 10 a Windows 11    | Instalátor <br/> Přenosný       |
| <div style={{display: 'flex', alignItems: 'center', gap: '10px'}}><img src={require('/img/macos.png').default} width="40" /> Apple macOS</div>       | 14+      | Instalátor<br/> Přenosný       |

---
## Podpora více jazyků

### Jazyky aplikace
Katalog je k dispozici v těchto jazycích:

| Místní nastavení | Jazyk          |
|----------|----------------|
| bg_BG    | Bulharština    |
| cz_CZ    | Čeština        |
| da_DK    | Dánština       |
| de_DE    | Němčina        |
| en_US    | Angličtina     |
| es_ES    | Španělština    |
| et_EE    | Estonština     |
| fi_FI    | Finština       |
| fr_FR    | Francouzština  |
| el_GR    | Řečtina        |
| hi_IN    | Hindština      |
| hr_HR    | Chorvatština   |
| hu_HU    | Maďarština     |
| id_ID    | Indonéština    |
| it_IT    | Italština      |
| ja_JP    | Japonština     |
| lt_LT    | Litevština     |
| lv_LV    | Lotyština      |
| nb_NO    | Norština       |
| nl_NL    | Nizozemština   |
| pl_PL    | Polština       |
| pt_PT    | Portugalština  |
| ro_RO    | Rumunština     |
| si_SI    | Slovinština    |
| sk_SK    | Slovenština    |
| sr_RS    | Srbština       |
| sv_SE    | Švédština      |
| uk_UA    | Ukrajinština   |
| zh_CN    | Čínština       |

### Jazyky dokumentace
Dokumentace je k dispozici v těchto jazycích:
| Místní nastavení | Jazyk      |
|----------|------------|
| en_US    | Angličtina |
| cz_CZ    | Čeština    |
| fr_FR    | Francouzština |

---

**Jste připraveni organizovat svou kolekci souborů?** Začněte s **[Tutoriálem](tutorial)** a vytvořte svůj první katalog za méně než 5 minut.