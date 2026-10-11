---
version: "3.0"
---
# Lignes de Commande
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Résumé
Cette page décrit les fonctionnalités et options disponibles en ligne de commande qui peuvent être exécutées depuis la console.
Sans avoir besoin de l'interface graphique, certaines tâches peuvent donc être automatisées.

Ces commandes fonctionnent actuellement uniquement sous **Linux**.

![Sortie de l'aide en ligne de commande affichant les actions et options disponibles](/img/k2/commandlines_01_help.png)

## Synopsis

```bash
Katalog [ACTION] [OPTIONS] [ARGUMENTS]
```

### Lancer la commande `Katalog`
Les exemples de cette page utilisent `Katalog` comme nom de commande. La façon d'appeler Katalog depuis un terminal dépend de son mode d'installation :

- **Flatpak** (Flathub) : placer les actions et options après l'identifiant de l'application.
  ```bash
  flatpak run io.github.stephanecouturier.Katalog list_catalogs
  ```
  Flatpak installe aussi un lanceur nommé `io.github.stephanecouturier.Katalog`, qui peut être appelé directement lorsque son dossier est dans le `PATH`.
- **AppImage** : placer les actions et options après le fichier AppImage.
  ```bash
  ./Katalog_linux_v3.0-qt6.AppImage list_catalogs
  ```

Pour taper `Katalog` comme dans les exemples, définir un alias, par exemple dans `~/.bashrc` :
```bash
alias Katalog='flatpak run io.github.stephanecouturier.Katalog'
```

La ligne de commande utilise le même fichier de paramètres que la fenêtre : sans `--collection`, elle travaille sur la dernière collection ouverte dans Katalog.

## Actions

Les actions suivantes sont disponibles comme arguments positionnels :

### `list_catalogs`
Liste tous les catalogues avec leur ID, état actif et nom.

**Utilisation :**
```bash
Katalog list_catalogs [OPTIONS]
```

**Exemple :**
```bash
Katalog list_catalogs --verbose
```

![Sortie en ligne de commande listant tous les catalogues avec leur ID, état et nom](/img/k2/commandlines_02_list.png)

### `update_catalog`
Met à jour un catalogue spécifique par ID de périphérique.

**Utilisation :**
```bash
Katalog update_catalog <deviceID> [OPTIONS]
```

**Arguments :**
- `deviceID` - L'ID du périphérique du catalogue à mettre à jour

**Exemple :**
```bash
Katalog update_catalog 5
```
![Sortie en ligne de commande après la mise à jour d'un catalogue spécifique](/img/k2/commandlines_03_catalog.png)

### `update_all_active`
Met à jour tous les catalogues actifs dans la collection.

**Utilisation :**
```bash
Katalog update_all_active [OPTIONS]
```

**Exemple :**
```bash
Katalog update_all_active --verbose
```
![Sortie en ligne de commande après la mise à jour de tous les catalogues actifs](/img/k2/commandlines_04_all.png)

### `search`
Exécute une recherche en utilisant les derniers critères de recherche de l'historique, avec des remplacements optionnels.

**Utilisation :**
```bash
Katalog search [OPTIONS]
```

**Exemple :**
```bash
Katalog search --text "vacances" --type image --limit 100
```

## Options Générales

### `-h, --help`
Affiche les informations d'aide et quitte.

### `-v, --version`
Affiche les informations de version et quitte.

### `-c, --collection <chemin>`
Spécifie le chemin vers le dossier de collection.

**Exemple :**
```bash
Katalog search --collection "/chemin/vers/ma/collection"
```

### `--verbose`
Active la sortie verbeuse pour le débogage et les informations détaillées.

**Exemple :**
```bash
Katalog list_catalogs --verbose
```

## Options de Recherche

Ces options sont disponibles lors de l'utilisation de l'action `search` pour remplacer les critères de recherche :

### `--limit <nombre>`
Limite le nombre de fichiers à afficher dans les résultats de recherche.

**Exemple :**
```bash
Katalog search --limit 50
```

### `--selectedDeviceID <deviceID>`
Spécifie dans quel ID de périphérique rechercher.
- Par défaut : utilise la valeur du fichier de paramètres
- Lorsqu'utilisé avec `--collection` : par défaut 0 (Tous les périphériques)

**Exemple :**
```bash
Katalog search --selectedDeviceID 2
```

### `--text <terme-recherche>`
Spécifie le texte ou la phrase de recherche à chercher.

**Exemple :**
```bash
Katalog search --text "photos de famille"
```

### `--type <type-fichier>`
Filtre les résultats par type de fichier.

**Valeurs valides :**
- `all` (par défaut)
- `audio`
- `image`
- `text`
- `video`

**Exemple :**
```bash
Katalog search --type audio
```

### `--size-min <taille>`
Définit le filtre de taille minimale du fichier.

**Format :** Nombre suivi de l'unité (ex. 1MB, 5GB)

**Exemple :**
```bash
Katalog search --size-min 1MB
```

### `--size-max <taille>`
Définit le filtre de taille maximale du fichier.

**Format :** Nombre suivi de l'unité (ex. 100MB, 2GB)

**Exemple :**
```bash
Katalog search --size-max 100MB
```

### `--date-after <date>`
Filtre les fichiers modifiés après la date spécifiée.

**Format :** YYYY-MM-DD

**Exemple :**
```bash
Katalog search --date-after 2023-01-01
```

### `--date-before <date>`
Filtre les fichiers modifiés avant la date spécifiée.

**Format :** YYYY-MM-DD

**Exemple :**
```bash
Katalog search --date-before 2023-12-31
```

### `--case-sensitive`
Active la recherche de texte sensible à la casse.

**Exemple :**
```bash
Katalog search --text "MonFichier" --case-sensitive
```

### `--search-in <portée>`
Définit la portée de recherche pour la correspondance de texte.

**Valeurs valides :**
- `filenames` (par défaut)
- `files-and-folders`
- `folder-paths`

**Exemple :**
```bash
Katalog search --text "documents" --search-in folder-paths
```

### `--text-criteria <critères>`
Spécifie comment le texte de recherche doit être recherché.

**Valeurs valides :**
- `all-words` (par défaut)
- `exact-phrase`
- `begins-with`
- `any-word`

**Exemple :**
```bash
Katalog search --text "photo vacances" --text-criteria exact-phrase
```

### `--exclude <termes-exclusion>`
Exclut les fichiers contenant les termes spécifiés.

**Exemple :**
```bash
Katalog search --text "photo" --exclude "sauvegarde temp"
```

### `--no-history`
Commence avec les critères de recherche par défaut au lieu de charger depuis l'historique de recherche.

**Exemple :**
```bash
Katalog search --no-history --text "nouveaufichier"
```

## Codes de Sortie

- **0** : Succès
- **1** : Erreur ou échec
- **-1** : Code interne pour continuer en mode GUI (non retourné à l'utilisateur)

## Exemples

### Gestion Basique des Catalogues

Lister tous les catalogues :
```bash
Katalog list_catalogs
```

Mettre à jour un catalogue spécifique avec sortie détaillée :
```bash
Katalog update_catalog 3 --verbose
```

Mettre à jour tous les catalogues actifs :
```bash
Katalog update_all_active
```

### Exemples de Recherche

Recherche de texte simple :
```bash
Katalog search --text "vacances"
```

Rechercher des images de plus de 5MB :
```bash
Katalog search --type image --size-min 5MB
```

Rechercher des fichiers modifiés en 2023 :
```bash
Katalog search --date-after 2023-01-01 --date-before 2023-12-31
```

Recherche complexe avec critères multiples :
```bash
Katalog search --text "projet" --type text --search-in files-and-folders --case-sensitive --limit 200
```

Rechercher dans un périphérique spécifique :
```bash
Katalog search --selectedDeviceID 2 --text "documents" --verbose
```

### Utilisation d'un Chemin de Collection Personnalisé

Rechercher dans une collection différente :
```bash
Katalog --collection "/chemin/vers/collection" search --text "photos"
```

Mettre à jour les catalogues dans une collection spécifique :
```bash
Katalog --collection "/chemin/vers/collection" update_all_active
```

## Notes

- Quand aucune action n'est spécifiée, Katalog se lance en mode GUI
- Les critères de recherche sont chargés depuis l'historique de recherche par défaut, sauf si `--no-history` est utilisé
- Les options de ligne de commande remplacent les valeurs de l'historique de recherche
- Toutes les opérations de recherche respectent l'état actif des catalogues
- Unités de taille de fichier supportées : KB, MB, GB, TB (insensible à la casse)
- Les formats de date doivent être au format YYYY-MM-DD

## Dépannage

**ID de périphérique invalide :**
Assurez-vous que l'ID du périphérique existe en exécutant d'abord `Katalog list_catalogs`.

**Problèmes de connexion à la base de données :**
Vérifiez que le chemin de collection est correct et accessible.

**La recherche ne retourne aucun résultat :**
Essayez d'utiliser `--no-history` pour commencer avec les critères par défaut, ou vérifiez si le périphérique sélectionné contient des fichiers indexés.

**Erreurs de permissions :**
Assurez-vous que Katalog a un accès en lecture/écriture au dossier de collection et aux fichiers de base de données.
