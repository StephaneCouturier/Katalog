---
version: "3.0"
---
# Přehled
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Indexace souborů a správa úložných zařízení
Katalog je <b>správce katalogů souborů</b>, který pomáhá organizovat a hledat soubory.<br/>
Poskytuje úplný přehled o souborech na <b>více úložných zařízeních, aniž by musela být připojena</b>.<br/>
Katalog vytváří <b>rejstříky, takže můžete prohledávat celou svou sbírku souborů z jednoho místa</b>, ať spravujete interní a externí disky, USB flash disky, síťová úložiště nebo optické disky (Blu-ray, DVD, CD).

- **Katalogizujte vše**: vytvářejte podrobné rejstříky souborů libovolného úložného zařízení.
- **Vyhledávejte offline**: najděte soubory bez připojení původního zařízení.
- **Organizujte kolekci**: spravujte úložná zařízení a jejich katalogy v jedné hierarchii.
- **Zálohujte**: kopírujte nebo přesouvejte soubory, které v záloze chybí, pomocí katalogů.

![Okno Katalogu: stránky Výběr, Vyhledávání a Výsledky vedle sebe](/img/interface_01_overview.png)

## Stránky
Okno zobrazuje vlevo stránku **[Výběr](Selection)**: strom všech zařízení kolekce. Výběr zařízení určuje rozsah ostatních stránek. Každá funkce se otevírá jako stránka vedle ní, z postranního panelu (viz [Rozhraní](Interface)):

1. [Vyhledávejte](Search) soubory na všech úložných zařízeních, **aniž by musela být připojena**, a pracujte s [výsledky](SearchResults).
1. Organizujte [Zařízení](Devices) a jejich katalogy v jedné hierarchii, s **virtuálními** zařízeními pro jejich seskupení, a [upravujte](DevicesEdit) je.
1. [Prozkoumejte](Explore) složky a soubory katalogu.
1. [Vytvářejte](Create) katalogy souborů.
1. Kopírujte nebo přesouvejte soubory pomocí odkazů [Zálohy](BackUp) a kontrolujte pokrytí záloh.
1. Sledujte růst kolekce ve [Statistice](Statistics).
1. Přiřazujte složkám [Štítky](Tags) pro další možnosti vyhledávání.
1. Nastavte kolekci a aplikaci v [Nastavení](Settings): datový režim, import, téma, jazyk.

---
## Hlavní funkce

### Bohaté informace o souborech
- **[Rozpoznání typu souboru](Create#file-type)**: typ každého souboru se určuje podle přípony, pro stovky formátů.
- **[Možnosti katalogu](Create#content-options)**: zahrnout pouze jeden [typ souborů](Create#file-type), podsložky nebo ne, [skryté soubory](Create#include-hidden-files) a [vyloučit složky nebo soubory](Create#global-parameters).
- **[Extrakce metadat](Create#include-metadata)**: rozměry obrázků a videí, délka videí a zvukových souborů, umělec a album hudby nebo jakákoli metadata jiných typů souborů.
- **[Kontrolní součet souborů](Create#include-checksum)**: kontrolní součty SHA-256 pro přesné hledání duplikátů a rozdílů a pro ověřování souborů.
- **[Štítky složek](Tags)**: organizujte a třiďte složky vlastními štítky.

### Vyhledávání a objevování
- **[Kritéria vyhledávání](Search)**: hledejte soubory podle názvu (více textů, vyloučení, regulární výrazy), složky, velikosti, data, typu souboru a metadat.
- **[Hledání duplikátů](Search#duplicates-on)**: najděte duplicitní soubory napříč zařízeními.
- **[Hledání rozdílů](Search#differences-on)**: zobrazte rozdíly mezi dvěma zařízeními, například mezi složkou a její zálohou.
- **[Akce s výsledky](SearchResults#batch-process)**: filtrujte výsledky, exportujte je, ověřujte kontrolní součty, extrahujte metadata, přesouvejte do koše.

### Zařízení a katalogy
- **[Hierarchie zařízení](Devices#model)**: fyzická skupina, úložná zařízení a katalogy a virtuální skupiny.
- **[Aktualizace](Devices#context-menu)**: udržujte katalogy aktuální, po jednom nebo všechny dostupné najednou.
- **[Změny cesty](DevicesEdit#path-change)**: když je disk připojen jinde, opravte všechny jeho katalogy během několika sekund, bez čtení disku.
- **[Import a aktualizace kolekce](Settings#import-update)**: importujte zařízení z jiné kolekce Katalogu nebo z VVV a později je obnovte.

### Analýza a správa
- **[Průzkumník souborů](Explore)**: procházejte obsah katalogu, jako by bylo zařízení připojeno, s velikostí každé složky.
- **[Statistika](Statistics)**: sledujte růst kolekce a úložného místa.
- **[Záloha a archivace](BackUp)**: kopírujte nebo přesouvejte soubory mezi katalogy, zobrazte náhled, spusťte více odkazů najednou a kontrolujte pokrytí.
- **[Datové režimy](Settings#database-memory-mode)**: uchovávejte kolekci ve složce souborů, v databázi SQLite nebo v hostované databázi.

### Katalog 2
Katalog 2 zůstává k dispozici, s vlastní [dokumentací](Katalog2), včetně [příkazového řádku](k2/CommandLines) (Linux).

---
## Podpora více platforem {#multi-platform-support}

| Hlavní operační systém | Distribuce / Minimální verze    | Balíčky    |
|-------------------|-------------|-------------|
| <div style={{display: 'flex', alignItems: 'center', gap: '10px'}}><img src={require('/img/linux.png').default} width="40" /> GNU/Linux</div>         | 64bitový, glibc 2.38+ <br/>32bitový, glibc 2.35     | [FlatHub/Flatpak](https://flathub.org/en/apps/io.github.stephanecouturier.Katalog)<br/>[AppImage](https://sourceforge.net/projects/katalogg/files/) <br/>[Přenosný](https://sourceforge.net/projects/katalogg/files/) |
| <div style={{display: 'flex', alignItems: 'center', gap: '10px'}}><img src={require('/img/windows.png').default} width="40" /> Microsoft Windows</div> | 64bitový: Windows 10 a Windows 11    | [Instalátor](https://sourceforge.net/projects/katalogg/files/) <br/> [Přenosný](https://sourceforge.net/projects/katalogg/files/)       |
| <div style={{display: 'flex', alignItems: 'center', gap: '10px'}}><img src={require('/img/macos.png').default} width="40" /> Apple macOS</div>       | 14 (Sonoma)  | [Instalátor](https://sourceforge.net/projects/katalogg/files/)<br/> [Přenosný](https://sourceforge.net/projects/katalogg/files/)       |


### Kompatibilita s macOS

- **Minimální verze macOS**: macOS 14 (Sonoma) nebo novější
- **Testováno na**: macOS 15 (Sequoia), Apple Silicon
- **Architektura**: pouze Apple Silicon (M1/M2/M3). Intelové Macy **nejsou** podporovány — Rosetta 2 překládá software pro Intel tak, aby běžel na Apple Silicon, nikoli naopak, takže tuto verzi nelze na Intelovém Macu spustit.
- **Sestavovací prostředí**: sestaveno na macOS 14 (Sonoma) s Qt 6 a KDE Frameworks 6 (KF6) přes Homebrew

**Důležité: aplikace není podepsaná ani notarizovaná.**

macOS aplikaci ve výchozím nastavení zablokuje. Pro spuštění Katalogu vyzkoušejte jeden z těchto postupů:

1. Klikněte na aplikaci pravým tlačítkem → zvolte „Otevřít“ (nikoli dvojklik) → potvrďte v dialogu
2. Nebo přejděte do Nastavení systému → Soukromí a zabezpečení → sjeďte dolů a klikněte na „Přesto otevřít“
3. Nebo spusťte v Terminálu: `xattr -cr /cesta/ke/Katalog.app` a poté otevřete normálně. Pokud je přístup odepřen, přidejte na začátek `sudo`.

**Známé omezení**:

na Macích spravovaných organizací (MDM/firemní profil) mohou být tato řešení blokována bezpečnostními zásadami. Pro spravované Macy momentálně neexistuje řešení — vyžadovalo by to podepsání aplikace certifikátem Apple Developer.

K dispozici je také starší sestavení (v2.5, čisté Qt6, bez závislostí na KF6) se stejnými požadavky na macOS.

---
## Podpora více jazyků

### Jazyky aplikace
Katalog je k dispozici v těchto jazycích:

| Místní nastavení | Jazyk | Místní nastavení | Jazyk | Místní nastavení | Jazyk |
|---|---|---|---|---|---|
| bg_BG | Bulharština | hi_IN | Hindština | pl_PL | Polština |
| cs_CZ | Čeština | hr_HR | Chorvatština | pt_PT | Portugalština |
| da_DK | Dánština | hu_HU | Maďarština | ro_RO | Rumunština |
| de_DE | Němčina | id_ID | Indonéština | si_SI | Slovinština |
| en_US | Angličtina | it_IT | Italština | sk_SK | Slovenština |
| es_ES | Španělština | ja_JP | Japonština | sr_RS | Srbština |
| et_EE | Estonština | lt_LT | Litevština | sv_SE | Švédština |
| fi_FI | Finština | lv_LV | Lotyština | uk_UA | Ukrajinština |
| fr_FR | Francouzština | nb_NO | Norština | zh_CN | Čínština |
| el_GR | Řečtina | nl_NL | Nizozemština |  |  |

### Jazyky dokumentace
Dokumentace je k dispozici v těchto jazycích:
| Místní nastavení | Jazyk      |
|----------|------------|
| en_US    | Angličtina |
| cs_CZ    | Čeština    |
| fr_FR    | Francouzština |

---

**Jste připraveni organizovat svou kolekci souborů?** Začněte s **[Tutoriálem](tutorial)** a vytvořte svůj první katalog za méně než 5 minut.
