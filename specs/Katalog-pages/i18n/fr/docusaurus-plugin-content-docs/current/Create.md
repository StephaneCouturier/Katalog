---
version: "2.10"
---
# Créer
![2.10](https://img.shields.io/badge/Version-2.10-blue)

## Résumé
Cette page décrit toutes les fonctionnalités de l'écran **Créer** et comment les utiliser.<br/>
Depuis cet écran, l'utilisateur peut **créer un catalogue de fichiers**.<br/>
Cela se fait en 3 étapes principales :
1. Sélectionner le chemin source : le périphérique ou le répertoire contenant les fichiers à inclure dans le nouveau catalogue.
1. Sélectionner les options pour inclure ou exclure certains fichiers.
1. Sélectionner le [Stockage](DevicesStorage), définir le nom du catalogue et créer le catalogue.

![Aperçu de l'écran Créer](/img/screen_create_01.png)

## Sélectionner le chemin source
Il existe 3 façons de sélectionner le chemin source du répertoire contenant les fichiers à inclure dans le nouveau [Catalogue](DevicesCatalogs) :
1. en tapant le chemin dans la zone de texte
1. en utilisant l'arborescence du système de fichiers, développez et cliquez sur le bon périphérique ou répertoire
1. ou en cliquant sur le bouton *Sélectionner* qui ouvrira une fenêtre de dialogue pour aider à choisir le dossier.

Le chemin sélectionné apparaîtra toujours dans la zone de texte et l'application utilisera ce chemin pour parcourir et cataloguer son contenu.

## Sélectionner les options pour inclure/exclure des fichiers
Choisissez les types de fichiers à inclure dans votre catalogue.

### Filtrage des types de fichiers
Katalog utilise une détection intelligente des types de fichiers basée sur l'extension, qui prend en charge des centaines de formats automatiquement.

**Fonctionnement de la détection des types de fichiers :**
- **Détection initiale** : les types de fichiers sont déterminés en analysant les extensions à l'aide d'un cache d'extension intelligent construit à partir de la base de données MIME du système
- **Support étendu** : prend en charge des centaines de formats de fichiers
- **Évolutif** : les nouveaux formats sont automatiquement reconnus lors de la mise à jour du système

**Remarque sur la précision** :
- Les types de fichiers sont déterminés à partir de l'analyse des extensions lors de la création du catalogue, afin de maximiser les performances d'indexation.
- Mais des erreurs d'extension ou des extensions manquantes sont possibles
- Pour maximiser la précision, il est possible d'exécuter une vérification MIME sur les catalogues existants depuis l'écran Périphériques, et de corriger les fichiers avec des extensions trompeuses.

### Catégories de types de fichiers
Le contenu du catalogue peut être limité à un type particulier de fichiers.
Cette option sera applicable pour le catalogue à venir. Elle peut être modifiée ultérieurement en éditant le [Catalogue](DevicesCatalogs).

| Type  | Description | Définition | Exemples d'extensions |
| ------| ------------|------------|----------------------|
| Tous  | Tous les types de fichiers sans filtrage | | |
| Audio | Musique, podcasts, enregistrements audio et fichiers sonores | (selon les types MIME) | MP3, FLAC, AAC, M4A, OGG, WAV, AIFF, Opus, WMA, MIDI, AMR (50+ formats) |
| Image | Photos, graphiques, diagrammes, icônes et contenu visuel | (selon les types MIME) | JPG, PNG, HEIC, WebP, TIFF, RAW, SVG, XCF, GIF, BMP (100+ formats) |
| Texte | Documents, fichiers de code, balisage, fichiers de données et contenu lisible | (définition spécifique Katalog)<br/>Inclut tous les fichiers MIME commençant par « text/ » ainsi que des fichiers applicatifs comme PDF, Word, etc. | PDF, DOCX, ODT, Markdown, HTML, JSON, code source, ebooks (100+ formats) |
| Vidéo | Films, clips, animations et contenu vidéo | (selon les types MIME) | MP4, MKV, AVI, WebM, MOV, FLV, 3GP, OGV, M2TS (40+ formats) |
| Autre | Tous les autres types non couverts par les catégories ci-dessus | (définition spécifique Katalog) | ZIP, RAR, EXE, DLL, ISO, fichiers applicatifs non classifiés en Texte |
| Aucun | Fichiers dont le type n'a pas pu être déterminé à partir de l'extension | Fichiers sans extension ou avec des extensions inconnues | |

### Extraction de métadonnées
Choisissez la quantité de métadonnées à extraire de vos fichiers lors de la création du catalogue.
<br/>Cela affecte la vitesse de catalogage ou la taille de la collection, mais fournit des informations plus riches pour les recherches et les statistiques.

**Options disponibles :**
- **None** : aucune extraction de métadonnées (catalogage le plus rapide)
- **Media Basic** : extraction des métadonnées essentielles des images, vidéos et fichiers audio
- **Media Extended** : extraction de métadonnées complètes incluant les détails techniques
- **Full Extended** : extraction maximale pour tous les types de fichiers pris en charge

**Ce qu'extrait Media Basic :**
- **Audio** : artiste, album, détails de la piste, durée, débit
- **Images** : dimensions, orientation
- **Vidéos** : dimensions, durée, codec et fréquence d'images

**Ce qu'extrait Extended :**
Ce mécanisme repose sur la bibliothèque KFileMetaData, qui détermine les types de fichiers pris en charge et les métadonnées.

**Impact sur les performances :**
- **None** : option la plus rapide, adaptée aux grands répertoires ou quand les métadonnées ne sont pas nécessaires
- **Media Basic/Extended** : impact modéré, traite uniquement les fichiers multimédia
- **Full Extended** : plus lent mais le plus complet, extrait depuis tous les formats pris en charge

**Types de fichiers pris en charge pour les métadonnées :**
- **Images** : jpg, png, gif, bmp, tiff, webp, svg, heic, raw, xcf
- **Vidéos** : mp4, mkv, avi, mov, wmv, flv, webm, m4v, mpg, 3gp, ogv, vob
- **Audio** : mp3, wav, flac, ogg, m4a, aac, wma, opus, aiff, mid, amr

Ce paramètre s'applique uniquement à ce catalogue et peut être modifié ultérieurement en éditant le [Catalogue](DevicesCatalogs).

**Remarque :** l'extraction de métadonnées nécessite des fichiers lisibles. Les fichiers corrompus ou avec des restrictions d'accès seront ignorés sans affecter le processus de catalogage.

### Somme de contrôle des fichiers
La somme de contrôle SHA256 peut être calculée lors de l'indexation pour la recherche de doublons ou de différences.
⚠️ C'est un processus bien plus long que les autres options d'indexation car il lit TOUTES les données pour calculer les sommes de contrôle.
Comme pour les métadonnées, l'option peut être sélectionnée à la création du catalogue ou modifiée ultérieurement ; en cas d'interruption du processus, les sommes de contrôle déjà calculées sont sauvegardées et la prochaine mise à jour reprendra pour les fichiers restants.
Les sommes de contrôle peuvent être utilisées comme option de recherche de doublons ou de recherche de différences.

### Inclure les fichiers cachés {#other-options}
Les fichiers cachés ne sont pas inclus par défaut, mais cette option permet de les inclure.<br/>
Cette option sera applicable pour le catalogue à venir.<br/>
Elle peut être modifiée ultérieurement en éditant le [Catalogue](DevicesCatalogs).

### Panneau Paramètres globaux

Le panneau *Paramètres globaux* regroupe les paramètres qui s'appliquent à tous les catalogues. Il peut être réduit ou développé à l'aide du bouton bascule en haut du panneau.

### Exclure des répertoires (global) {#exclude-directories}

:::note
Ces exclusions sont **globales** : elles s'appliquent à **tous** les catalogues, aussi bien lors de la création de nouveaux catalogues que lors de la mise à jour des catalogues existants.
:::

Il est possible d'exclure des répertoires du catalogage, lors de la création ou lors des mises à jour.<br/>
Saisissez un chemin ou un motif textuel, puis cliquez sur *Ajouter*.<br/>
L'entrée est alors visible dans la liste ci-dessous.<br/>
Toute entrée peut être supprimée par un clic droit et en sélectionnant *Supprimer*.<br/>

**Fonctionnement de l'exclusion :**

L'exclusion utilise une **correspondance textuelle** : tout fichier ou dossier dont le chemin complet **contient** le texte d'exclusion sera ignoré. Cela signifie :

- **Chemin complet** : saisir `/home/user/Downloads/temp` exclura ce répertoire spécifique et tout son contenu.
- **Nom de dossier** : saisir `node_modules` exclura **chaque** répertoire `node_modules` dans tous les catalogues.
- **Chemin partiel** : saisir `.cache` exclura des répertoires comme `/home/user/.cache/` mais aussi `/home/user/.cachedata/` car la correspondance est basée sur le contenu du texte.

La correspondance est **sensible à la casse**.

![Liste des répertoires exclus globalement avec des exemples d'entrées](/img/screen_create_04_exclude.png)

### Exclure des dossiers (par catalogue)

En plus des exclusions globales, il est possible de définir des dossiers exclus s'appliquant uniquement au catalogue en cours de création.

- Saisissez un chemin de dossier manuellement, ou accédez-y à l'aide du bouton *Choisir*.
- Cliquez sur *Ajouter* pour l'ajouter à la liste en attente.
- Supprimez toute entrée par un clic droit et en sélectionnant *Supprimer*.

Les exclusions par catalogue sont sauvegardées avec le catalogue une fois sa création terminée. Elles s'appliquent en complément des exclusions globales — un dossier ignoré par l'une ou l'autre règle ne sera pas indexé.

Cette option peut être modifiée ultérieurement en éditant le [Catalogue](DevicesCatalogs).

## Définir et créer le catalogue
#### Sélectionner le périphérique de stockage
Un catalogue doit être associé à un périphérique physique [Stockage](DevicesStorage), pour faciliter la recherche ultérieure ou activer les statistiques.<br/>
Par défaut, Katalog pré-crée un périphérique de stockage par défaut, le disque local.<br/>
Cela peut être mis à jour ultérieurement dans l'écran de l'[Arborescence](DevicesTree).<br/>
Si vous avez besoin d'un stockage différent pour ce catalogue, cliquez sur *Ajouter un stockage* et ajoutez-en un via les écrans [Arborescence](DevicesTree) ou [Stockage](DevicesStorage).

Ce choix sera applicable pour le catalogue à venir.<br/>
Il peut être modifié ultérieurement en éditant le [Catalogue](DevicesCatalogs).

#### Saisir un nom
Saisissez un nom pour votre catalogue.<br/>
Les noms en double ne sont actuellement pas autorisés.

Le bouton *Générer* permet de créer un nom basé sur le chemin du dossier, en remplaçant les barres obliques <code>/</code> par le trait de soulignement <code>_</code>.

#### Créer le catalogue
Lorsque vous êtes prêt, cliquez sur le bouton *Créer un catalogue* pour enregistrer le catalogue et démarrer le processus de catalogage récursif du contenu du chemin (tous les sous-répertoires seront inclus).

Une fois le processus terminé,
- Un message confirme la création et indique le nombre de fichiers et la taille totale des fichiers du dossier sélectionné.
- votre disque local (un périphérique de stockage ajouté automatiquement) a également été mis à jour, et le message indique l'espace libre, utilisé et total :

![Message de confirmation de création avec le nombre de fichiers et le résumé de l'espace de stockage](/img/screen_create_02.png)

l'écran [Périphériques](DevicesTree) s'affichera pour montrer le catalogue dans l'arborescence des périphériques.

Le nouveau catalogue est automatiquement sélectionné dans le panneau [Sélection](Selection), prêt à être utilisé pour [Rechercher](Search) du contenu.

## Guide de performances

### Qu'est-ce qui affecte la vitesse d'analyse ?

#### 1. Extraction de métadonnées (impact le plus fort : ~10× de ralentissement)
- Métadonnées d'images : ~2–3 ms/fichier (lecture de l'en-tête)
- Métadonnées vidéo : ~5–15 ms/fichier (recherche et analyse du conteneur)
- Solution : utiliser « Media Basic » uniquement, pas « Full Extended »

#### 2. Mode de base de données
- Mode mémoire : plus rapide, utilise la RAM
- Mode fichier SQLite : plus lent, limité par les entrées/sorties

#### 3. Type de stockage
- SSD : ~100 000 fichiers/min
- HDD : ~20 000–30 000 fichiers/min (la fragmentation a un impact)
- Stockage réseau : très variable

#### 4. Dossiers exclus
- Plus il y a d'exclusions, plus l'analyse est rapide
- Exemple : exclure .cache, node_modules, etc.

#### 5. Charge du système
- D'autres processus intensifs peuvent interférer

### Benchmarks de performances

| Fichiers | Stockage | Métadonnées | Durée |
|----------|----------|-------------|-------|
| 5 000 | SSD | None | 10 s |
| 5 000 | SSD | Basic | 50 s |
| 95 000 | HDD | Basic | 47 s (1re fois) / 10 s (en cache) |

## Développement
Quelques idées d'évolutions pour cet écran :
* Personnaliser les types de fichiers et/ou utiliser les types MIME
* Pour en savoir plus, consultez la liste de [développements Créer](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=create).
