---
version: "2.11"
---
# Příkazová Řádka
![2.11](https://img.shields.io/badge/Version-2.11-blue)

## Shrnutí
Tato stránka popisuje dostupné funkce a možnosti příkazové řádky, které lze spouštět z konzole.
Bez potřeby grafického rozhraní lze tedy automatizovat některé úkoly.

Tyto příkazy aktuálně fungují pouze pod **Linuxem**.

![Výstup nápovědy příkazové řádky zobrazující dostupné akce a možnosti](/img/commandlines_01_help.png)

## Syntaxe

```bash
./Katalog.sh [AKCE] [MOŽNOSTI] [ARGUMENTY]
```

## Akce

Následující akce jsou dostupné jako poziční argumenty:

### `list_catalogs`
Vypíše všechny katalogy s jejich ID, aktivním stavem a názvem.

**Použití:**
```bash
./Katalog.sh list_catalogs [MOŽNOSTI]
```

**Příklad:**
```bash
./Katalog.sh list_catalogs --verbose
```

![Výstup příkazové řádky se seznamem všech katalogů s ID, stavem a názvem](/img/commandlines_02_list.png)

### `update_catalog`
Aktualizuje konkrétní katalog podle ID zařízení.

**Použití:**
```bash
./Katalog.sh update_catalog <deviceID> [MOŽNOSTI]
```

**Argumenty:**
- `deviceID` - ID zařízení katalogu k aktualizaci

**Příklad:**
```bash
./Katalog.sh update_catalog 5
```
![Výstup příkazové řádky po aktualizaci konkrétního katalogu](/img/commandlines_03_catalog.png)

### `update_all_active`
Aktualizuje všechny aktivní katalogy v kolekci.

**Použití:**
```bash
./Katalog.sh update_all_active [MOŽNOSTI]
```

**Příklad:**
```bash
./Katalog.sh update_all_active --verbose
```
![Výstup příkazové řádky po aktualizaci všech aktivních katalogů](/img/commandlines_04_all.png)

### `search`
Provede vyhledávání pomocí posledních kritérií vyhledávání z historie, s volitelnými přepsáními.

**Použití:**
```bash
./Katalog.sh search [MOŽNOSTI]
```

**Příklad:**
```bash
./Katalog.sh search --text "dovolená" --type image --limit 100
```

## Obecné Možnosti

### `-h, --help`
Zobrazí informace o nápovědě a ukončí program.

### `-v, --version`
Zobrazí informace o verzi a ukončí program.

### `-c, --collection <cesta>`
Určuje cestu ke složce kolekce.

**Příklad:**
```bash
./Katalog.sh search --collection "/cesta/k/mé/kolekci"
```

### `--verbose`
Povolí podrobný výstup pro ladění a detailní informace.

**Příklad:**
```bash
./Katalog.sh list_catalogs --verbose
```

## Možnosti Vyhledávání

Tyto možnosti jsou dostupné při použití akce `search` pro přepsání kritérií vyhledávání:

### `--limit <číslo>`
Omezí počet souborů k zobrazení ve výsledcích vyhledávání.

**Příklad:**
```bash
./Katalog.sh search --limit 50
```

### `--selectedDeviceID <deviceID>`
Určuje ID zařízení, ve kterém se má vyhledávat.
- Výchozí: používá hodnotu ze souboru nastavení
- Při použití s `--collection`: výchozí je 0 (Všechna zařízení)

**Příklad:**
```bash
./Katalog.sh search --selectedDeviceID 2
```

### `--text <hledaný-výraz>`
Určuje text nebo frázi k vyhledání.

**Příklad:**
```bash
./Katalog.sh search --text "rodinné fotky"
```

### `--type <typ-souboru>`
Filtruje výsledky podle typu souboru.

**Platné hodnoty:**
- `all` (výchozí)
- `audio`
- `image`
- `text`
- `video`

**Příklad:**
```bash
./Katalog.sh search --type audio
```

### `--size-min <velikost>`
Nastaví filtr minimální velikosti souboru.

**Formát:** Číslo následované jednotkou (např. 1MB, 5GB)

**Příklad:**
```bash
./Katalog.sh search --size-min 1MB
```

### `--size-max <velikost>`
Nastaví filtr maximální velikosti souboru.

**Formát:** Číslo následované jednotkou (např. 100MB, 2GB)

**Příklad:**
```bash
./Katalog.sh search --size-max 100MB
```

### `--date-after <datum>`
Filtruje soubory upravené po zadaném datu.

**Formát:** YYYY-MM-DD

**Příklad:**
```bash
./Katalog.sh search --date-after 2023-01-01
```

### `--date-before <datum>`
Filtruje soubory upravené před zadaným datem.

**Formát:** YYYY-MM-DD

**Příklad:**
```bash
./Katalog.sh search --date-before 2023-12-31
```

### `--case-sensitive`
Povolí vyhledávání textu citlivé na velikost písmen.

**Příklad:**
```bash
./Katalog.sh search --text "MůjSoubor" --case-sensitive
```

### `--search-in <rozsah>`
Definuje rozsah vyhledávání pro shodu textu.

**Platné hodnoty:**
- `filenames` (výchozí)
- `files-and-folders`
- `folder-paths`

**Příklad:**
```bash
./Katalog.sh search --text "dokumenty" --search-in folder-paths
```

### `--text-criteria <kritéria>`
Určuje, jak má být vyhledáván text.

**Platné hodnoty:**
- `all-words` (výchozí)
- `exact-phrase`
- `begins-with`
- `any-word`

**Příklad:**
```bash
./Katalog.sh search --text "dovolená fotka" --text-criteria exact-phrase
```

### `--exclude <výrazy-k-vyloučení>`
Vyloučí soubory obsahující zadané výrazy.

**Příklad:**
```bash
./Katalog.sh search --text "fotka" --exclude "záloha temp"
```

### `--no-history`
Začne s výchozími kritérii vyhledávání místo načítání z historie vyhledávání.

**Příklad:**
```bash
./Katalog.sh search --no-history --text "novýsoubor"
```

## Návratové Kódy

- **0**: Úspěch
- **1**: Chyba nebo selhání
- **-1**: Interní kód pro pokračování v GUI režimu (nevrací se uživateli)

## Příklady

### Základní Správa Katalogů

Vypsat všechny katalogy:
```bash
./Katalog.sh list_catalogs
```

Aktualizovat konkrétní katalog s podrobným výstupem:
```bash
./Katalog.sh update_catalog 3 --verbose
```

Aktualizovat všechny aktivní katalogy:
```bash
./Katalog.sh update_all_active
```

### Příklady Vyhledávání

Jednoduché textové vyhledávání:
```bash
./Katalog.sh search --text "dovolená"
```

Vyhledat obrázky větší než 5MB:
```bash
./Katalog.sh search --type image --size-min 5MB
```

Vyhledat soubory upravené v roce 2023:
```bash
./Katalog.sh search --date-after 2023-01-01 --date-before 2023-12-31
```

Složité vyhledávání s více kritérii:
```bash
./Katalog.sh search --text "projekt" --type text --search-in files-and-folders --case-sensitive --limit 200
```

Vyhledat v konkrétním zařízení:
```bash
./Katalog.sh search --selectedDeviceID 2 --text "dokumenty" --verbose
```

### Použití Vlastní Cesty Kolekce

Vyhledat v jiné kolekci:
```bash
./Katalog.sh --collection "/cesta/ke/kolekci" search --text "fotky"
```

Aktualizovat katalogy v konkrétní kolekci:
```bash
./Katalog.sh --collection "/cesta/ke/kolekci" update_all_active
```

## Poznámky

- Když není zadána žádná akce, Katalog se spustí v GUI režimu
- Kritéria vyhledávání jsou ve výchozím nastavení načtena z historie vyhledávání, pokud není použito `--no-history`
- Možnosti příkazové řádky přepisují hodnoty z historie vyhledávání
- Všechny operace vyhledávání respektují aktivní stav katalogů
- Podporované jednotky velikosti souborů: KB, MB, GB, TB (necitlivé na velikost písmen)
- Formáty data musí být ve formátu YYYY-MM-DD

## Řešení Problémů

**Neplatné ID zařízení:**
Ujistěte se, že ID zařízení existuje spuštěním `./Katalog.sh list_catalogs` nejprve.

**Problémy s připojením k databázi:**
Ověřte, že cesta kolekce je správná a přístupná.

**Vyhledávání nevrací žádné výsledky:**
Zkuste použít `--no-history` pro začátek s výchozími kritérii, nebo zkontrolujte, zda vybrané zařízení obsahuje indexované soubory.

**Chyby oprávnění:**
Ujistěte se, že Katalog má přístup pro čtení/zápis ke složce kolekce a souborům databáze.
