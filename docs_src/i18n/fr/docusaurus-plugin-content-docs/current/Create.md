---
version: "3.0"
---
# Créer
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Résumé
Cette page décrit la page **Créer**, où un nouveau **catalogue** est créé : un index des fichiers d'un dossier, conservé dans la collection pour pouvoir être recherché et exploré même lorsque le périphérique est déconnecté.

Cela se fait en trois étapes :
1. [Définir le catalogue](#definition) : le dossier à indexer, son périphérique de stockage et son nom.
1. Choisir les [options de contenu](#content-options) : types de fichiers, sous-dossiers, fichiers cachés, métadonnées, sommes de contrôle et exclusions.
1. Cliquer sur *Créer*.

![Page Créer : définition du catalogue, options de contenu et paramètres globaux](/img/create_01_overview.png)

## Définition du catalogue {#definition}

| Champ | Description |
|-------|-------------|
| *Chemin source* | Le dossier à indexer. Il est rempli avec le chemin du périphérique sélectionné sur la page [Sélection](Selection), et peut être saisi ou choisi avec *Sélectionner le chemin*. |
| *Stockage* | Le périphérique de stockage où se trouve le dossier : le disque ou l'ordinateur (voir [Périphériques](Devices#model)). Seuls les périphériques de stockage sont listés. Une nouvelle collection en a déjà un, *Disque local*. Pour en ajouter un autre, utilisez *Ajouter un Stockage* sur la page [Périphériques](Devices#page-actions). |
| *Nom du catalogue* | Le nom du nouveau catalogue. Chaque catalogue doit avoir son propre nom. *Générer le nom à partir du chemin* (le bouton à côté du champ) en crée un à partir du chemin source, avec `_` à la place de `/` et `:`. |

Lorsqu'un champ manque ou est incorrect, un message en haut de la page indique ce qui est nécessaire, par exemple *Indiquez un nom pour ce nouveau catalogue.* Il disparaît lorsque le champ est corrigé. Il en va de même lorsque le dossier source n'existe pas, ou lorsqu'un autre catalogue porte déjà ce nom.

Lorsque le dossier source ne contient aucun fichier, une question est d'abord posée : le dossier est peut-être vide, ou le périphérique n'est peut-être pas monté à cet endroit. *Oui* crée tout de même le catalogue, vide.

## Options de contenu {#content-options}
Les options déterminent ce qui est indexé. Elles peuvent être modifiées plus tard en modifiant le catalogue (voir [Modifier un périphérique](DevicesEdit#content-options)) ; le catalogue doit alors être mis à jour pour en tenir compte.

### Type de fichier {#file-type}
Le catalogue peut être limité à un type de fichiers.

| Type | Description | Définition | Exemples d'extensions |
|------|-------------|------------|-----------------------|
| Tous | Tous les types de fichiers, sans filtrage. | | |
| Audio | Musique, podcasts, enregistrements audio et fichiers sonores. | Selon la définition des types MIME. | MP3, FLAC, AAC, M4A, OGG, WAV, AIFF, Opus, WMA, MIDI, AMR (plus de 50 formats) |
| Image | Photos, graphiques, diagrammes, icônes et contenus visuels. | Selon la définition des types MIME. | JPG, PNG, HEIC, WebP, TIFF, RAW, SVG, XCF, GIF, BMP (plus de 100 formats) |
| Texte | Documents, fichiers de code, balisage, fichiers de données et contenus lisibles. | Définition propre à Katalog : tous les fichiers dont le type MIME commence par `text/`, plus des fichiers d'application comme les documents PDF et Word. | PDF, DOCX, ODT, Markdown, HTML, JSON, fichiers de code source, livres numériques (plus de 100 formats) |
| Vidéo | Films, clips, animations et contenus vidéo. | Selon la définition des types MIME. | MP4, MKV, AVI, WebM, MOV, FLV, 3GP, OGV, M2TS (plus de 40 formats) |
| Autre | Tous les autres types de fichiers. | Définition propre à Katalog. | ZIP, RAR, EXE, DLL, ISO : exécutables, archives, fichiers système et fichiers d'application non classés comme Texte. |
| Aucun | Fichiers dont le type ne peut pas être déterminé à partir de l'extension. | Fichiers sans extension ou avec une extension inconnue. | |

Le type d'un fichier est déterminé à partir de son **extension**, à l'aide de la base de données MIME du système : des centaines de formats sont reconnus, et de nouveaux au fil des mises à jour du système. Cela permet une indexation rapide. Un fichier dont l'extension est erronée ou absente peut donc recevoir un mauvais type.

### Inclure les sous-dossiers {#include-subdirectories}
Coché (par défaut), les fichiers de tous les sous-dossiers sont indexés. Décoché, uniquement les fichiers situés directement dans le chemin source.

### Inclure les fichiers cachés {#include-hidden-files}
*Aucun* (par défaut) ignore les fichiers et dossiers cachés ; *Tous* les inclut.<br/>
Sur un dossier personnel ou un disque système, les dossiers cachés (`.cache`, `.local`…) peuvent contenir beaucoup de fichiers : exclure ceux qui ne sont pas utiles permet de garder un catalogue léger.

### Inclure les métadonnées {#include-metadata}
La quantité d'informations lue dans le contenu des fichiers. Cela allonge l'indexation, mais donne des résultats plus riches dans la [Recherche](Search) (dimensions, durée, artiste…) et dans les [Statistiques](Statistics).

| Niveau | Ce qui est lu |
|--------|---------------|
| *Aucun* | Aucune métadonnée : le plus rapide. |
| *Média basique* | L'essentiel des images, vidéos et fichiers audio : dimensions et orientation des images ; dimensions, durée, codec et fréquence d'images des vidéos ; artiste, album, piste, durée et débit des fichiers audio. |
| *Média étendu* | Toutes les métadonnées des images, vidéos et fichiers audio, y compris les détails techniques. |
| *Complet étendu* | Toutes les métadonnées de chaque type de fichier pris en charge. |

Les niveaux étendus utilisent la bibliothèque KFileMetaData, qui détermine les types et champs pris en charge ; leurs métadonnées peuvent être consultées avec *Afficher les métadonnées étendues (JSON)* dans les résultats de recherche et dans Explorer.<br/>
Les niveaux Média ne lisent que les fichiers multimédias : les autres fichiers sont ignorés. Les fichiers endommagés ou illisibles sont ignorés sans interrompre le catalogue.

Fichiers multimédias pris en charge :
* **Images** : jpg, png, gif, bmp, tiff, webp, svg, heic, raw, xcf
* **Vidéos** : mp4, mkv, avi, mov, wmv, flv, webm, m4v, mpg, 3gp, ogv, vob
* **Audio** : mp3, wav, flac, ogg, m4a, aac, wma, opus, aiff, mid, amr

### Inclure la somme de contrôle {#include-checksum}
*SHA-256* calcule une somme de contrôle pour chaque fichier : une empreinte de son contenu. Les sommes de contrôle rendent exactes les [recherches de doublons et de différences](Search), et permettent de vérifier plus tard que les fichiers n'ont pas changé.<br/>
⚠️ C'est beaucoup plus long que les autres options, car **toutes** les données de chaque fichier sont lues (voir le [Guide de performance](#performance)).

### Exclure les dossiers ou fichiers {#exclude-folders-or-files}
Chemins ou noms qui ne sont pas indexés **dans ce catalogue**.<br/>
Saisissez un chemin ou un nom dans *Chemin ou nom à exclure*, ou choisissez un dossier avec *Sélectionner le chemin*, puis cliquez sur *Ajouter*. Le bouton *Supprimer* à côté d'une entrée la retire de la liste.<br/>
La liste est enregistrée avec le catalogue.

## Paramètres globaux {#global-parameters}
La section *Paramètres globaux* contient les paramètres partagés par **tous les catalogues** de la collection. Le bouton sur sa ligne de titre la replie ou la déplie.

### Exclusions globales de dossiers {#exclude-directories}
Chemins exclus de **chaque catalogue**, lors de sa création et à chacune de ses mises à jour.<br/>
Saisissez un chemin dans *Chemin à exclure globalement* (ou utilisez *Sélectionner le chemin*), puis cliquez sur *Ajouter*. *Supprimer* à côté d'une entrée la retire.

### Fonctionnement des exclusions
Les exclusions globales et celles du catalogue s'additionnent : un fichier ou un dossier écarté par l'une ou l'autre liste n'est pas indexé.<br/>
Une exclusion est une **correspondance de texte** : tout fichier ou dossier dont le chemin complet **contient** le texte est ignoré. La correspondance est sensible à la casse.
* **Chemin complet** : `/home/username/Downloads/temp` ignore ce dossier et tout son contenu.
* **Nom de dossier** : `node_modules` ignore **tous** les dossiers `node_modules`, où qu'ils soient.
* **Partie d'un nom** : `.cache` ignore `/home/username/.cache/`, mais aussi `/home/username/.cachedata/`, puisque les deux chemins contiennent le texte.

## Création du catalogue {#creating}
*Créer* enregistre le catalogue et lance l'indexation. La progression est affichée dans le panneau d'activité en bas de la fenêtre (voir [Interface](Interface)). Si une autre opération est en cours, la création attend dans la file d'attente.

L'indexation se déroule en deux phases :
1. la **liste des fichiers** est lue et enregistrée : le catalogue est utilisable dès la fin de cette phase,
2. ensuite, si elles ont été choisies, les **métadonnées** et les **sommes de contrôle** sont lues.

*Stop* met fin à la création :
* pendant la première phase, le catalogue n'est pas créé,
* pendant la seconde phase, le catalogue est **conservé** avec sa liste complète de fichiers ; les métadonnées ou sommes de contrôle pas encore lues sont complétées à la prochaine mise à jour du catalogue.

À la fin de la création, le nouveau catalogue est sélectionné sur la page [Sélection](Selection) et la page [Périphériques](Devices) s'ouvre avec un rapport : le nom et le chemin du catalogue, son nombre de fichiers et sa taille totale, et, lorsque son stockage a été mis à jour, l'espace utilisé, libre et total du stockage avec ce qui a été ajouté. Si une analyse des métadonnées ou des sommes de contrôle n'est pas terminée, un message l'explique à la place.<br/>
*Fermer* (ou `Échap`) ferme la page.

## Guide de performance {#performance}

### Qu'est-ce qui influence la vitesse d'analyse ?


| # | Facteur | Niveau d'impact | Ce qui le détermine | Remarques |
|---|---------|:---:|---|---|
| 1 | **Somme de contrôle des fichiers** | ⚫⚫⚫⚫⚫ Critique | **Volume total de données** (lit 100 % des octets, tampon de 8 Mo), puis vitesse du disque, puis nombre de fichiers | SHA-256 via Qt `QCryptographicHash`. Un disque dur de 2 To à ~100 Mo/s = ~5,5 heures rien que pour les E/S. SSD à ~500 Mo/s ≈ 1 heure. Reprise possible en cas d'interruption. |
| 2 | **Matériel de stockage** | ⚫⚫⚫⚫ Très élevé | E/S séquentielles ou aléatoires, débit, latence | SSD ~100 000 fichiers/min, disque dur ~20–30 000 (la fragmentation aggrave la situation). Réseau : très variable. Influence TOUS les autres facteurs. |
| 3 | **Métadonnées : Complet étendu** | ⚫⚫⚫ Élevé | **Nombre de fichiers** (surtout les vidéos : 5–15 ms/fichier pour analyser les conteneurs), répartition des types de fichiers | Utilise KFileMetaData. La vidéo et l'audio nécessitent de se déplacer dans la structure du fichier. Ralentissement d'environ ×10 par rapport à Aucun constaté. |
| 4 | **Métadonnées : Média basique/étendu** | ⚫⚫⚫ Élevé | Nombre de fichiers multimédias uniquement | Lit uniquement les en-têtes (images : ~2–3 ms). Les fichiers non multimédias sont entièrement ignorés : la répartition des types de fichiers compte donc beaucoup. |
| 6 | **Inclure les fichiers cachés**  | ⚫⚫  Moyen | Nombre de fichiers supplémentaires (peut être énorme sur les disques système/personnels : `.cache`, `.local`, `node_modules`…) | Effet multiplicateur : plus de fichiers = plus de tout ce qui précède. Les dossiers exclus sont la parade. |
| 7 | **Mode de base de données** | ⚫ Faible à moyen | Débit d'écriture pendant l'indexation | Mémoire > Fichier > Hébergé. Le mode Mémoire évite les E/S d'écriture SQLite pendant l'analyse ; le mode Fichier est limité par les écritures. Les transactions par lots (1000 fichiers/lot) atténuent déjà cet effet. |
| 8 | **Filtre de type de fichier** | ⚫ Faible à moyen | Réduit le nombre de fichiers traités pour les métadonnées | Filtrer sur Audio/Vidéo/Image ignore tous les Texte/Autre pour les métadonnées : important pour un disque au contenu mélangé. |
| 9 | **Nombre de fichiers (total)** | ⚫ Faible | Détermine le temps de parcours du système de fichiers et le nombre d'insertions en base | Même sans aucune option, énumérer 500 000 fichiers prend du temps. Indépendant des tailles. |
| 10 | **Dossiers exclus** | ⚫ Faible | Réduit le nombre effectif de fichiers | `node_modules`, `.cache` et les dossiers temporaires peuvent représenter 20–30 % des fichiers d'un dossier personnel. |
| 11 | **Charge du système / processeur** | ⚫ Faible | L'extraction parallèle des métadonnées utilise 4–8 cœurs | La somme de contrôle est limitée par les E/S, le processeur y compte donc moins. Les métadonnées sont limitées par le processeur. |

Autres traitements longs, non utilisés par Créer ou Actualiser :

| # | Facteur | Niveau d'impact | Ce qui le détermine | Remarques |
|---|---------|:---:|---|---|
| 5 | **Vérification des types MIME** | ⚫⚫⚫⚫ Très élevé | Nombre de fichiers (lit une partie du contenu de chaque fichier) | Opération de mise à jour uniquement. La « déduction » à partir de l'extension ne coûte rien ; la vérification lit les octets des fichiers. L'impact dépend du nombre de fichiers, pas de leur taille. |
| 12 | **Export de la collection** | ⚫⚫⚫⚫ Très élevé | Export des tables vers SQLite ou csv || De 20 minutes à plus d'1 heure pour 1 million de fichiers


Échantillon de test / jeu de données : 60 Go, 40 000 fichiers, dont un fichier de 20 Go, et un mélange de textes, d'images et de vidéos.

| Test | Échantillon  | SSD 2 To (extrapolé) | Disque dur 2 To (estimé) | Base de l'extrapolation |
|---------|:-:|:-:|:-:|---|
| Indexation de base | 00:00:04 | **~00:02:37** | ~00:05–08:00 | Nombre de fichiers |
| Média basique | 00:01:23 | **~00:54:00** | ~02:00–03:00 | Nombre de fichiers (multimédias uniquement) |
| Média étendu | 00:01:52 | **~01:13:00** | ~03:00–04:00 | Nombre de fichiers (plus lourd par vidéo) |
| Complet étendu | 00:01:23 | **~00:54:00** | ~02:00–03:00 | Nombre de fichiers  |
| Somme de contrôle seule | 00:09:20 | **~06:06:00** | ~20:00–30:00 | Octets totaux / débit |
| Somme de contrôle + Média basique | 00:11:06 | **~07:15:00** | ~22:00–33:00 | Octets (prépondérant) + nombre |


## Développement
Quelques idées de développement pour cet écran :
* Cataloguer une instance cloud (par exemple Nextcloud).
* Personnaliser les types de fichiers.
* Pour en savoir plus, voir le backlog du [développement de Créer](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=create).
