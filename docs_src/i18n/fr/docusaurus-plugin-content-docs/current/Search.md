---
version: "2.13"
---
# Rechercher
![2.13](https://img.shields.io/badge/Version-2.13-blue)

## Résumé
Cette page décrit toutes les fonctionnalités de l'écran **Rechercher** et comment les utiliser.

Les principales fonctionnalités de cet écran sont :

* Rechercher des fichiers dans les catalogues ou sur des lecteurs connectés
* Utiliser plusieurs **critères** pour affiner et réduire le nombre de résultats :
  * filtrer par [texte dans le nom de fichier ou le chemin](Search#search-text-criteria),
  * filtrer par [attributs de fichier](Search#file-criteria) tels que la taille, le type ou la date,
  * filtrer par [métadonnées de fichier](Search#file-metadata-criteria) telles que les dimensions d'image ou la durée audio/vidéo,
  * mettre en évidence les [fichiers dupliqués](Search#duplicates-on),
  * ou mettre en évidence les [différences](Search#differences-on) entre 2 catalogues.
* Utiliser les **Résultats** avec un [menu contextuel](Search#file-context-menu) (clic droit) ou [traiter les résultats en lot](Search#batch-process),
* Réutiliser les critères précédents depuis l'historique de recherche.
* Tous les critères de recherche sont enregistrés et disponibles lors de la prochaine ouverture de Katalog, à l'exception des résultats eux-mêmes.

![Écran Rechercher affichant le panneau de critères à gauche et les résultats à droite](/img/screen_search_01.png)

## Source de recherche

### Rechercher dans les catalogues de fichiers
Le panneau **[Sélection](Selection)** sur la gauche fournit des filtres optionnels pour limiter les catalogues dans lesquels rechercher :
- *Emplacement* : limite les catalogues à ceux correspondant à l'emplacement sélectionné dans l'écran [Stockage](DevicesStorage).
- *Stockage* : limite les catalogues à ceux d'un périphérique de stockage spécifique.
- *Catalogue* : restreint la recherche à un seul catalogue.

### Rechercher dans les lecteurs connectés
Recherchez directement sur l'ordinateur et les lecteurs connectés sans avoir besoin d'un catalogue.
C'est utile pour chercher dans un dossier précis, ou pour obtenir un aperçu rapide après des modifications récentes sans mettre à jour un catalogue au préalable.

## Critères de recherche
Ce panneau regroupe tous les critères qui affinent et réduisent le nombre de résultats. Le panneau peut être masqué pour économiser de l'espace.

Ils s'appliquent aux chemins et noms de fichiers, aux attributs de fichiers, aux métadonnées, aux étiquettes, et peuvent identifier des doublons ou des différences entre 2 catalogues.
Les résultats peuvent être une liste de fichiers, ou une liste de dossiers contenant les fichiers correspondants.

![Panneau des critères de recherche affichant les sections texte, fichier, métadonnées, doublons et différences](/img/screen_search_02.png)

### Critères de texte {#search-text-criteria}

#### Nom de fichier
Saisir le texte à rechercher dans les noms de fichiers et/ou les chemins de dossiers.

Un « mot » est un groupe de caractères séparé d'un autre groupe par un espace. Cela peut être utilisé pour trouver des dossiers, des fichiers, ou des fichiers dans certains dossiers.

Le champ de texte accepte plusieurs lignes. Chaque ligne est traitée comme un terme de recherche indépendant et les résultats correspondant à **n'importe quelle** ligne sont retournés (logique OU). Cela fonctionne avec tous les modes *Avec*.

- Appuyer sur **Entrée** pour lancer la recherche.
- Appuyer sur **Maj+Entrée** pour insérer un nouveau ligne.

![Champ de texte de recherche affichant plusieurs lignes utilisées comme termes de recherche indépendants en logique OU](/img/screen_search_05_list_for_file_name.png)

Boutons à côté du champ de texte :
- *Coller depuis le presse-papiers* — colle le contenu du presse-papiers dans le champ de recherche.
- *Nettoyer le texte de recherche* — supprime les caractères spéciaux (`.  ,  _  -  (  )  [  ]  {  }  /  \  '  "`).

Lorsqu'une case à cocher est placée avant un critère, il peut être activé ou désactivé sans perdre la valeur saisie.

#### Avec
Spécifie comment les mots du champ *Texte* doivent être mis en correspondance :

| Option | Comportement |
|--------|-------------|
| *Tous les mots* | Retourne les résultats uniquement si tous les mots sont trouvés (par défaut) |
| *Expression exacte* | Retourne les résultats où l'expression exacte (y compris l'ordre des mots et les espaces) est trouvée |
| *Commence par* | Le nom de fichier doit commencer par le texte — disponible uniquement avec *Noms de fichiers uniquement* |
| *N'importe quel mot* | Retourne les résultats si au moins un des mots est trouvé |
| *Regex* | Le texte de recherche est utilisé comme expression régulière (syntaxe PCRE2) |

:::note
En mode *Regex*, un motif invalide ne retourne aucun résultat. L'option *Sensible à la casse* s'applique toujours.
:::

#### Dans
Spécifie quelle partie du chemin de fichier doit être recherchée :

| Option | Comportement |
|--------|-------------|
| *Noms de fichiers uniquement* | Recherche uniquement dans les noms de fichiers (par défaut) |
| *Noms de fichiers ou chemins de dossiers* | Recherche dans les noms de fichiers et les chemins de dossiers |
| *Chemin de dossier uniquement* | Recherche uniquement dans les chemins de dossiers (non disponible avec *Commence par*) |

#### Sensible à la casse
Force la correspondance exacte des caractères (majuscules et minuscules sont distinguées).

#### Exclure
Exclut les résultats si *l'un quelconque* des mots fournis est trouvé dans le chemin ou le nom du fichier.

### Critères de fichier {#file-criteria}

Toute la section des critères de fichier peut être activée ou désactivée avec sa case à cocher.

#### Taille
Définir une taille minimale et/ou maximale de fichier. Chaque limite accepte un nombre et une unité.

Unités disponibles : **Octets**, **Kio**, **Mio**, **Gio**, **Tio**

#### Type de fichier
Filtrer les résultats par type de fichier. Types disponibles :

| Type | Description |
|------|-------------|
| Tous | Pas de filtre de type (par défaut) |
| Audio | Fichiers audio (ex. mp3, ogg, wav, flac…) |
| Image | Fichiers image (ex. jpg, png, gif, raw…) |
| Texte | Documents et fichiers texte (ex. pdf, docx, odt, epub…) |
| Vidéo | Fichiers vidéo (ex. mp4, mkv, avi, mov…) |
| Autre | Fichiers ne correspondant à aucun des types ci-dessus |
| Aucun | Fichiers sans type reconnu |

Les types de fichiers sont détectés dynamiquement à partir de la base de données MIME du système (KFileMetadata), la liste exacte des extensions s'adapte donc au système.

#### Dates
Définir une date de modification minimale et/ou maximale.

### Critères de métadonnées de fichier {#file-metadata-criteria}

La section de métadonnées permet de rechercher dans le contenu embarqué dans les fichiers (nécessite que le catalogue ait été indexé avec les métadonnées étendues).

![Section des critères de métadonnées affichant les champs de texte, les dimensions d'image et la durée](/img/screen_search_metadata_criteria.png)

Toute la section de métadonnées peut être activée ou désactivée avec sa case à cocher.

#### Texte des métadonnées
Rechercher du texte dans les champs de métadonnées tels que le nom d'artiste, l'album, le titre, l'auteur ou tout autre contenu textuel embarqué.

#### Dimensions des métadonnées (images et vidéos)
Filtrer par dimensions en pixels :
- *Hauteur min / max* — plage de hauteur en pixels
- *Largeur min / max* — plage de largeur en pixels

#### Durée des métadonnées (audio et vidéo)
Filtrer par durée de lecture en utilisant une plage horaire.

### Critères de dossier

#### Afficher les dossiers uniquement
Affiche les chemins de dossiers comme résultats au lieu des fichiers individuels.

:::note
*Afficher les dossiers uniquement* ne peut pas être combiné avec *Trouver les différences*.
:::

#### Étiquettes {#tags-criteria}
Filtrer les résultats aux fichiers ou dossiers auxquels une étiquette spécifique est attribuée.
Les étiquettes sont définies dans l'écran [Étiquettes](Tags).

### Doublons {#duplicates-on}

Trouver les fichiers potentiellement dupliqués, en fonction d'un ou d'une combinaison de :

| Champ | Description |
|-------|-------------|
| *Nom de fichier* | Même nom de fichier |
| *Taille de fichier* | Même taille de fichier |
| *Date de modification* | Même date de modification |
| *Somme de contrôle (SHA-256)* | Même valeur de somme de contrôle ou valeur différente |

L'**opérateur de somme de contrôle** (= ou ≠) permet de trouver les fichiers avec la *même* somme de contrôle (vrais doublons) ou les fichiers avec des sommes de contrôle *différentes* malgré d'autres attributs identiques.

**Options de portée :**
- *Dans le périphérique/catalogue sélectionné* — trouve les doublons à l'intérieur du périphérique ou catalogue actuellement sélectionné.
- *Comparer deux périphériques* — compare les fichiers entre deux périphériques ou catalogues sélectionnés pour trouver des doublons entre eux.

:::note
Au moins un champ doit être sélectionné pour lancer une recherche de doublons.
:::

### Différences {#differences-on}

Trouver les fichiers présents dans un périphérique mais pas dans l'autre, ou avec des valeurs d'attributs différentes — utile pour comparer une source avec une sauvegarde.

![Section des critères de différences affichant les sélecteurs de périphériques et les cases à cocher des attributs](/img/screen_search_03_diff.png)

Sélectionner deux périphériques (Virtuel, Stockage ou Catalogue) à comparer, et choisir les attributs qui définissent une différence :

| Champ | Description |
|-------|-------------|
| *Nom de fichier* | Fichiers avec des noms différents |
| *Taille de fichier* | Fichiers avec des tailles différentes |
| *Date de modification* | Fichiers avec des dates de modification différentes |
| *Somme de contrôle (SHA-256)* | Fichiers avec la même somme de contrôle ou une différente |

L'**opérateur de somme de contrôle** (= ou ≠) fonctionne de la même manière que pour les Doublons.

:::note
Au moins un champ doit être sélectionné. *Trouver les différences* ne peut pas être combiné avec *Afficher les dossiers uniquement* ni avec *Trouver les doublons*.
:::

## Lancer une recherche

### Bouton Rechercher
Cliquer sur *Rechercher* pour démarrer. Le bouton change d'état selon l'opération :

Une recherche ne peut pas être lancée pendant la création d'un catalogue ou la mise à jour d'un périphérique, et celles-ci ne peuvent pas être lancées pendant une recherche : le bouton *Rechercher* est alors désactivé. La progression est affichée dans le panneau d'activité, avec toutes les autres opérations.

| État | Libellé du bouton | Bouton Arrêter |
|------|------------------|----------------|
| Inactif | *Rechercher* (vert) | Désactivé |
| En cours | *Pause* | Activé |
| En pause | *Reprendre* | Activé |

![Barre de statut affichant l'état de pause pendant une opération de recherche](/img/screen_search_statusbar_paused.png)

- **Pause / Reprendre** — suspendre et continuer la recherche sans perdre la progression. Non disponible en mode base de données Mémoire.
- **Arrêter** — annuler la recherche en cours. Les résultats partiels déjà trouvés sont affichés.

### Réinitialiser
Le bouton *Réinitialiser tout* remet tous les critères à leurs valeurs par défaut.

## Résultats

### Catalogues avec résultats
Le panneau gauche liste les catalogues dans lesquels des fichiers correspondants ont été trouvés.
Cliquer sur un catalogue dans cette liste relance la recherche restreinte à ce seul catalogue.
Ce panneau peut être masqué pour économiser de l'espace.

### Fichiers trouvés
Le panneau droit liste les fichiers ou dossiers correspondant aux critères de recherche.

L'en-tête indique le nombre de fichiers ou de doublons trouvés et la taille totale.
Cliquer sur l'icône statistiques ouvre la **boîte de dialogue des statistiques détaillées** :

| Statistique | Description |
|-------------|-------------|
| Fichiers trouvés | Nombre total de résultats correspondants |
| Fichiers traités | Nombre total de fichiers examinés |
| Complétion | Pourcentage traité (si la recherche a été arrêtée avant la fin, les résultats sont signalés comme incomplets) |
| Taille totale | Somme des tailles de fichiers |
| Taille min / max / moyenne | Distribution des tailles |
| Date min / max | Plage de dates des résultats |
| Catalogues traités | Nombre de catalogues recherchés sur le total |

### Menu contextuel {#file-context-menu}

Un clic droit sur une ligne de résultat ouvre un menu contextuel :

**Navigation :**
| Action | Description |
|--------|-------------|
| *Ouvrir le fichier* | Ouvre le fichier avec l'application par défaut du système |
| *Ouvrir le dossier* | Ouvre le dossier parent du fichier dans le gestionnaire de fichiers |
| *Explorer le dossier* | Navigue vers le dossier du fichier dans l'écran [Explorer](Explore) de Katalog |

**Métadonnées :**
| Action | Description |
|--------|-------------|
| *Afficher les métadonnées étendues (JSON)* | Affiche les métadonnées embarquées du fichier (disponible si le catalogue a été indexé avec les métadonnées étendues, ou si le type de fichier les prend en charge) |

**Copier dans le presse-papiers :** *(exemple : `/home/user/documents/fichier.txt`)*
| Action | Valeur copiée |
|--------|--------------|
| *Copier le chemin du dossier* | `/home/user/documents` |
| *Copier le chemin absolu du fichier* | `/home/user/documents/fichier.txt` |
| *Copier le nom du fichier avec extension* | `fichier.txt` |
| *Copier le nom du fichier sans extension* | `fichier` |

**Somme de contrôle :**
| Action | Condition | Description |
|--------|-----------|-------------|
| *Calculer la somme de contrôle (SHA-256)* | Aucune somme de contrôle stockée | Calcule et enregistre le hachage SHA-256 |
| *Copier la somme de contrôle* | Somme de contrôle stockée | Copie le hachage dans le presse-papiers |
| *Vérifier la somme de contrôle (SHA-256)* | Somme de contrôle stockée | Recalcule et compare avec la valeur stockée |

**Opérations sur les fichiers :**
| Action | Description |
|--------|-------------|
| *Déplacer vers la corbeille* | Déplace le fichier vers la corbeille système (avec confirmation) |
| *Supprimer le fichier* | Supprime définitivement le fichier (avec confirmation) |

### Traitement par lot {#batch-process}

Le bouton *Traiter les résultats* ouvre un menu d'opérations appliquées à tous les fichiers des résultats :

| Action | Description |
|--------|-------------|
| *Exporter les résultats* | Exporter vers un nouveau catalogue (pour des recherches plus précises) **ou** vers un fichier CSV nommé avec la date et enregistré dans le [dossier Collection](Settings#database-memory-mode) |
| *Renommer (KRename)* | Ouvre tous les fichiers des résultats dans [KRename](https://apps.kde.org/krename/) pour le renommage par lot |
| *Vérifier les sommes de contrôle* | Pour chaque fichier des résultats : si aucune somme de contrôle n'est encore enregistrée, calcule et sauvegarde le hachage SHA-256 ; si une somme est déjà enregistrée, compare la valeur réelle avec la valeur stockée (correspondance / divergence). La progression est affichée dans la barre d'état. |
| *Inclure les métadonnées* | Extrait les métadonnées étendues (dimensions des images, durée audio/vidéo, etc.) pour chaque fichier des résultats et les enregistre dans le catalogue. Utile pour les fichiers initialement indexés sans métadonnées. La progression est affichée dans la barre d'état. |
| ⚠ *Déplacer vers la corbeille* | Déplace tous les fichiers des résultats vers la corbeille système (avec confirmation affichant le nombre et la taille totale) |
| ⚠ *Supprimer* | Supprime définitivement tous les fichiers des résultats (avec confirmation — aucune récupération possible) |

## Historique des recherches
Chaque fois qu'une recherche est lancée, les critères et la sélection de périphérique sont enregistrés dans un fichier d'historique dans le dossier Collection.

Cet historique est affiché dans un tableau en bas de l'écran. Cliquer sur une ligne restaure tous les critères — la recherche doit ensuite être déclenchée manuellement.

Deux boutons permettent de gérer la liste de l'historique :
- *Garder les 10 dernières* — supprime toutes les entrées sauf les 10 plus récentes (avec confirmation).
- *Réinitialiser* — supprime toutes les entrées de l'historique (avec confirmation).

Ce panneau peut être masqué pour économiser de l'espace.

![Panneau d'historique des recherches affichant les recherches précédentes avec horodatages et critères](/img/screen_search_04_search_history.png)

## Développement
Quelques idées d'évolutions pour cet écran :
* Voir la liste de [développement Rechercher](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=search).
