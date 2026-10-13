---
version: "3.0"
---
# Aperçu général
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Indexation de fichiers & Gestionnaire de périphériques de stockage
Katalog est un <b>gestionnaire de catalogues de fichiers</b> qui vous aide à organiser et à retrouver vos fichiers.<br/>
Il donne une vue complète des fichiers répartis sur <b>plusieurs périphériques de stockage, sans qu'ils aient besoin d'être connectés</b>.<br/>
Katalog crée des <b>index pour que vous puissiez rechercher dans toute votre collection de fichiers depuis un seul endroit</b>, que vous gériez des disques internes et externes, des clés USB, du stockage réseau ou des disques optiques (Blu-ray, DVD, CD).

- **Tout cataloguer** : créer des index détaillés des fichiers de n'importe quel périphérique de stockage.
- **Rechercher hors ligne** : trouver des fichiers sans connecter ni monter le périphérique d'origine.
- **Organiser la collection** : gérer les périphériques de stockage et leurs catalogues dans une seule hiérarchie.
- **Sauvegarder** : copier ou déplacer les fichiers absents d'une sauvegarde, à l'aide des catalogues.

![Fenêtre de Katalog : pages Sélection, Chercher et Résultats côte à côte](/img/interface_01_overview.png)

## Pages
La fenêtre affiche la page **[Sélection](Selection)** à gauche : l'arborescence de tous les périphériques de la collection. Sélectionner un périphérique définit la portée des autres pages. Chaque fonctionnalité s'ouvre comme une page à côté d'elle, depuis le panneau latéral (voir [Interface](Interface)) :

1. [Rechercher](Search) des fichiers sur tous les périphériques de stockage, **sans qu'ils aient besoin d'être connectés**, et agir sur les [résultats](SearchResults).
1. Organiser les [Périphériques](Devices) et leurs catalogues dans une seule hiérarchie, avec des périphériques **virtuels** pour les regrouper, et les [modifier](DevicesEdit).
1. [Explorer](Explore) les dossiers et les fichiers d'un catalogue.
1. [Créer](Create) des catalogues de fichiers.
1. Copier ou déplacer des fichiers avec des liens de [Sauvegarde](BackUp), et vérifier la couverture des sauvegardes.
1. Suivre la croissance de la collection dans les [Statistiques](Statistics).
1. Attribuer des [Étiquettes](Tags) à des dossiers, pour des possibilités de recherche supplémentaires.
1. Configurer la collection et l'application dans les [Paramètres](Settings) : mode de données, import, thème, langue.

---
## Fonctionnalités clés

### Informations riches sur les fichiers
- **[Détection du type de fichier](Create#file-type)** : le type de chaque fichier est déterminé à partir de son extension, pour des centaines de formats.
- **[Options du catalogue](Create#content-options)** : inclure seulement un [type de fichiers](Create#file-type), les sous-dossiers ou non, les [fichiers cachés](Create#include-hidden-files), et [exclure des dossiers ou des fichiers](Create#global-parameters).
- **[Extraction des métadonnées](Create#include-metadata)** : dimensions des images et des vidéos, durée des vidéos et des fichiers audio, artiste et album de la musique, ou toute métadonnée des autres types de fichiers.
- **[Somme de contrôle des fichiers](Create#include-checksum)** : sommes de contrôle SHA-256 pour des recherches exactes de doublons et de différences, et pour vérifier les fichiers.
- **[Étiquettes de dossiers](Tags)** : organiser et classer les dossiers avec vos propres étiquettes.

### Recherche et découverte
- **[Critères de recherche](Search)** : trouver des fichiers par nom (plusieurs textes, exclusions, expressions régulières), dossier, taille, date, type de fichier et métadonnées.
- **[Trouver les doublons](Search#duplicates-on)** : repérer les fichiers en double entre périphériques.
- **[Trouver les différences](Search#differences-on)** : voir les différences entre deux périphériques, par exemple un dossier et sa sauvegarde.
- **[Actions sur les résultats](SearchResults#batch-process)** : filtrer les résultats, les exporter, vérifier les sommes de contrôle, extraire les métadonnées, déplacer dans la corbeille.

### Périphériques et catalogues
- **[Hiérarchie des périphériques](Devices#model)** : groupe physique, périphériques de stockage et catalogues, et groupes virtuels.
- **[Mise à jour](Devices#context-menu)** : garder les catalogues à jour, un par un ou tous les disponibles en une fois.
- **[Changements de chemin](DevicesEdit#path-change)** : lorsqu'un disque est monté ailleurs, corriger tous ses catalogues en quelques secondes, sans lire le disque.
- **[Import et mise à jour de collection](Settings#import-update)** : importer des périphériques depuis une autre collection Katalog ou depuis VVV, et les actualiser plus tard.

### Analyse et gestion
- **[Explorateur de fichiers](Explore)** : parcourir le contenu d'un catalogue comme si le périphérique était connecté, avec la taille de chaque dossier.
- **[Statistiques](Statistics)** : suivre la croissance de la collection et de l'espace de stockage.
- **[Sauvegarde et archivage](BackUp)** : copier ou déplacer des fichiers entre catalogues, prévisualiser, exécuter plusieurs liens en une fois, et vérifier la couverture.
- **[Modes de données](Settings#database-memory-mode)** : conserver la collection dans un dossier de fichiers, dans une base de données SQLite, ou dans une base de données hébergée.

### Katalog 2
Katalog 2 reste disponible, avec sa [documentation](Katalog2), y compris la [ligne de commande](k2/CommandLines) (Linux).

---
## Support multi-plateformes {#multi-platform-support}

| Système d'exploitation principal | Distributions / Versions minimales    | Packaging    |
|-------------------|-------------|-------------|
| <div style={{display: 'flex', alignItems: 'center', gap: '10px'}}><img src={require('/img/linux.png').default} width="40" /> GNU/Linux</div>         | 64bits, glibc 2.38+ <br/>32bits, glibc 2.35     | [FlatHub/Flatpak](https://flathub.org/en/apps/io.github.stephanecouturier.Katalog)<br/>[AppImage](https://sourceforge.net/projects/katalogg/files/) <br/>[Portable](https://sourceforge.net/projects/katalogg/files/) |
| <div style={{display: 'flex', alignItems: 'center', gap: '10px'}}><img src={require('/img/windows.png').default} width="40" /> Microsoft Windows</div> | 64bits : Windows 10 & Windows 11    | [Installateur](https://sourceforge.net/projects/katalogg/files/) <br/> [Portable](https://sourceforge.net/projects/katalogg/files/)       |
| <div style={{display: 'flex', alignItems: 'center', gap: '10px'}}><img src={require('/img/macos.png').default} width="40" /> Apple macOS</div>       | 15 (Sequoia) | [Installateur](https://sourceforge.net/projects/katalogg/files/)<br/> [Portable](https://sourceforge.net/projects/katalogg/files/)       |


### Compatibilité macOS

- **Version minimale de macOS** : macOS 15 (Sequoia) ou ultérieure
- **Testé sur** : macOS 15 (Sequoia), Apple Silicon
- **Architecture** : Apple Silicon (M1/M2/M3) uniquement. Les Mac Intel ne sont **pas** pris en charge — Rosetta 2 traduit les logiciels Intel pour les exécuter sur Apple Silicon, et non l'inverse : il n'existe donc aucun moyen d'exécuter cette version sur un Mac Intel.
- **Environnement de compilation** : compilé sur macOS 15 (Sequoia) avec Qt 6 et KDE Frameworks 6 (KF6), via Homebrew

**Important : l'application n'est ni signée ni notariée.**

macOS bloque l'application par défaut. Pour lancer Katalog, essayez l'une de ces méthodes :

1. Clic droit sur l'application → choisissez « Ouvrir » (et non un double-clic) → confirmez dans la boîte de dialogue
2. Ou allez dans Réglages Système → Confidentialité et sécurité → faites défiler puis cliquez sur « Ouvrir quand même »
3. Ou exécutez dans le Terminal : `xattr -cr /chemin/vers/Katalog.app` puis ouvrez normalement. Ajoutez `sudo` devant si l'autorisation est refusée.

**Limitation connue** :

sur les Mac gérés par une organisation (profil MDM/entreprise), ces contournements peuvent être bloqués par la politique de sécurité. Il n'existe actuellement aucune solution pour les Mac gérés — cela nécessiterait de signer l'application avec un certificat Apple Developer.

Une version héritée (v2.5, Qt6 pur, sans dépendances KF6) est également disponible, avec les mêmes prérequis macOS.

---
## Support multilingue

### Langues de l'application
Katalog est disponible en :

| Locale | Langue | Locale | Langue | Locale | Langue |
|--------|--------|--------|--------|--------|--------|
| bg_BG  | Bulgare    | hi_IN  | Hindi      | pl_PL  | Polonais   |
| cs_CZ  | Tchèque    | hr_HR  | Croate     | pt_PT  | Portugais  |
| da_DK  | Danois     | hu_HU  | Hongrois   | ro_RO  | Roumain    |
| de_DE  | Allemand   | id_ID  | Indonésien | si_SI  | Slovène    |
| en_US  | Anglais    | it_IT  | Italien    | sk_SK  | Slovaque   |
| es_ES  | Espagnol   | ja_JP  | Japonais   | sr_RS  | Serbe      |
| et_EE  | Estonien   | lt_LT  | Lituanien  | sv_SE  | Suédois    |
| fi_FI  | Finnois    | lv_LV  | Letton     | uk_UA  | Ukrainien  |
| fr_FR  | Français   | nb_NO  | Norvégien  | zh_CN  | Chinois    |
| el_GR  | Grec       | nl_NL  | Néerlandais |       |            |

### Langues de la documentation
La documentation est disponible en :
| Locale   | Langue    |
|----------|-----------|
| en_US    | Anglais   |
| cs_CZ    | Tchèque   |
| fr_FR    | Français  |

---

**Prêt à organiser votre collection de fichiers ?** Commencez avec le **[Tutoriel](tutorial)** pour créer votre premier catalogue en moins de 5 minutes.
