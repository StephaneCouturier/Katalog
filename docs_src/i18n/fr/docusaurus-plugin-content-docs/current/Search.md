---
version: "3.0"
---
# Rechercher
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Résumé
Cette page décrit la page **Chercher** : les critères de recherche, le lancement d'une recherche et l'historique des recherches.

* Rechercher des fichiers dans les **catalogues** du périphérique sélectionné, ou directement dans un **lecteur connecté**.
* Combiner des **critères** pour affiner les résultats :
  * le [nom de fichier](#search-text-criteria), avec un ou plusieurs textes de recherche,
  * les [attributs de fichier](#file-criteria) : type, taille et date,
  * les [métadonnées du fichier](#file-metadata-criteria) : texte, dimensions d'image ou de vidéo, durée,
  * les [critères de dossier](#folder-criteria) : lister uniquement les dossiers, ou seulement les dossiers étiquetés,
  * trouver des [doublons](#duplicates-on), ou des [différences](#differences-on) entre 2 périphériques.
* Réutiliser les critères d'une recherche précédente depuis l'[historique](#search-history).

Les résultats s'ouvrent dans la page **Résultats**, à droite de la page Chercher : voir [Résultats de recherche](SearchResults).

![Page Chercher avec « sunset » dans les critères de nom de fichier, à côté de la page Résultats](/img/search_01_overview.png)

## Où chercher {#search-source}

| Option | Description |
|--------|-------------|
| *Catalogues* | Cherche dans les catalogues du périphérique sélectionné dans la [Sélection](Selection) : un catalogue, tous les catalogues d'un périphérique de stockage, ou d'un périphérique virtuel. Sans périphérique sélectionné (*Tous*), cherche dans tous les catalogues. |
| *Lecteur connecté* | Cherche directement dans un dossier de l'ordinateur ou d'un lecteur connecté, sans catalogue. Saisissez le chemin, ou utilisez *Sélectionner le chemin*. Pratique pour chercher dans un dossier qui n'a pas de catalogue, ou pour voir des changements récents sans mettre d'abord un catalogue à jour. |

## Critères de recherche
Les critères sont regroupés en sections. La case à cocher devant le titre d'une section active ou désactive toute la section **sans perdre ce qui y a été saisi**. Il en va de même pour les cases à cocher devant un critère isolé (*Type*, *Taille*, *Date*...).

### Nom de fichier {#search-text-criteria}

![Critères de nom de fichier avec deux textes de recherche et un texte d'exclusion](/img/search_02_file_name.png)

#### texte
Le texte à rechercher. Il peut s'agir d'**un ou plusieurs textes de recherche**, un par ligne :
* *+* ajoute une ligne, *−* la retire (la dernière ligne ne peut pas être retirée),
* un fichier est trouvé lorsqu'il correspond à **l'une quelconque** des lignes,
* la croix d'une ligne la vide.

Boutons à côté des lignes :
* *Coller depuis le presse-papier* : remplace les lignes par le contenu du presse-papiers, une ligne par ligne de texte. Pratique pour chercher une liste copiée depuis un tableur, un éditeur de texte ou un gestionnaire de fichiers.
* *Nettoyer le Texte de recherche des caractères comme _ - . ,* : remplace ces caractères par des espaces dans chaque ligne.

Coller plusieurs lignes avec `Ctrl+V` dans une ligne les ajoute comme nouvelles lignes. `Entrée` dans n'importe quelle ligne lance la recherche.

#### avec
Comment les mots de chaque ligne sont recherchés :

| Option | Comportement |
|--------|--------------|
| *Tous les Mots* | Tous les mots de la ligne sont trouvés, dans n'importe quel ordre (par défaut). |
| *Phrase exacte* | La ligne est trouvée telle que saisie, les mots dans le même ordre. |
| *Commence Par* | Le nom de fichier commence par la ligne. Uniquement avec *Noms de fichiers uniquement*. |
| *Un des Mots* | Au moins un mot de la ligne est trouvé. |
| *Regex* | La ligne est une expression régulière (syntaxe PCRE2). Une expression invalide ne trouve rien. |

Un « mot » est un groupe de caractères séparé d'un autre par un espace.

#### dans
Où le texte est recherché :

| Option | Comportement |
|--------|--------------|
| *Noms de fichiers uniquement* | Dans les noms de fichiers (par défaut). |
| *Noms de fichiers ou chemins des dossiers* | Dans les noms de fichiers et dans les chemins des dossiers. |
| *Chemins des Dossiers uniquement* | Uniquement dans les chemins des dossiers. |

#### sensible à la casse
Les majuscules et les minuscules sont considérées comme différentes.

#### exclure
Les fichiers sont écartés des résultats lorsqu'ils correspondent à **l'une quelconque** des lignes d'exclusion. Les lignes d'exclusion fonctionnent comme les lignes de recherche : *+*, *−*, coller et nettoyer.

### Attributs de fichier {#file-criteria}

#### Type
Uniquement les fichiers d'un type. Lorsque *Type* n'est pas coché, tous les types sont recherchés.

| Type | Description |
|------|-------------|
| *Tous* | Aucun filtre de type. |
| *Audio* | Fichiers audio (mp3, ogg, wav, flac...). |
| *Image* | Fichiers image (jpg, png, gif, raw...). |
| *Texte* | Documents et fichiers texte (pdf, docx, odt, epub...). |
| *Vidéo* | Fichiers vidéo (mp4, mkv, avi, mov...). |
| *Autre* | Fichiers d'aucun des types ci-dessus. |
| *Aucun* | Fichiers sans type reconnu. |

Les types de fichiers sont reconnus à partir de la base des types de fichiers du système : la liste exacte des extensions suit donc le système.

#### Taille
Une taille minimale et une taille maximale, chacune avec son unité : Octets, KiB, MiB, GiB ou TiB.

#### Date
Une date de modification minimale et maximale. *Sélectionner une date* ouvre un calendrier, avec des raccourcis : *Maintenant*, *Il y a 1 jour*, *Il y a 1 semaine*, *Il y a 1 mois*, *Il y a 1 an*.

### Métadonnées du fichier {#file-metadata-criteria}
Critères portant sur des informations enregistrées à l'intérieur des fichiers. Ils nécessitent des catalogues créés ou mis à jour avec l'option *Inclure les métadonnées* (voir [Créer](Create)).

| Critère | Description |
|---------|-------------|
| *Texte* | Texte dans les métadonnées, comme un artiste, un album, un titre ou un auteur. |
| *Taille* | *Largeur* et *Hauteur* en pixels, chacune avec un minimum et un maximum (images et vidéos). |
| *Durée* | Durée minimale et maximale, en heures:minutes:secondes (audio et vidéo). |

### Critères de dossier {#folder-criteria}

#### lister uniquement les dossiers dans les résultats
Les résultats sont les dossiers qui contiennent des fichiers correspondants, au lieu des fichiers.

#### Étiquette {#tags-criteria}
Uniquement les fichiers des dossiers qui portent cette étiquette. Les étiquettes s'attribuent dans la page [Étiquettes](Tags).

### Doublons {#duplicates-on}
Trouve les fichiers qui sont probablement le même fichier à plusieurs endroits.

*Sur* : ce que deux fichiers doivent avoir en commun, un ou plusieurs parmi :

| Champ | Description |
|-------|-------------|
| *Nom* | Même nom de fichier. |
| *Taille* | Même taille. |
| *Date* | Même date de modification. |
| *Somme de contrôle* | Avec `=`, même somme de contrôle : le contenu est identique. Avec `≠`, somme de contrôle différente : même nom ou même taille, mais un contenu différent. |

Les sommes de contrôle ne sont disponibles que pour les catalogues créés ou mis à jour avec l'option *Inclure la somme de contrôle* (voir [Créer](Create)).

*Portée* :
* *Dans le périphérique sélectionné* : doublons à l'intérieur du périphérique sélectionné,
* *Comparer deux périphériques* : doublons entre *Périphérique 1* et *Périphérique 2*, par exemple un ordinateur et un ancien portable.

### Différences {#differences-on}
Compare deux périphériques, *Entre* et *Et*, par exemple un dossier et sa sauvegarde. Trouve les fichiers présents dans un seul des deux, ou présents dans les deux mais avec une valeur différente pour les champs choisis dans *Sur* : *Nom*, *Taille*, *Date*, *Somme de contrôle* (`=` ou `≠`, comme pour les Doublons).

Les Différences ne sont disponibles que pour une recherche dans les *Catalogues*.

:::note
*Doublons* et *Différences* ne peuvent pas être utilisés en même temps.
:::

## Lancer une recherche {#running}
Les boutons en haut de la page :

| Bouton | Action |
|--------|--------|
| *Chercher* | Lance la recherche. `Entrée` dans un champ de texte fait de même. |
| *Pause* / *Reprendre* | Met en pause la recherche en cours et la reprend. |
| *Stop* | Arrête la recherche en cours. Les résultats déjà trouvés sont conservés. |
| *Réinitialiser* | Remet tous les critères à leurs valeurs par défaut. |
| *Historique* | Ouvre l'[historique des recherches](#search-history). |
| *Fermer* | Ferme la page Chercher. |

La progression de la recherche est affichée dans le [panneau d'activité](Interface#activity-panel).<br/>
Une recherche ne peut pas démarrer pendant la création d'un catalogue ou la mise à jour d'un périphérique : *Chercher* est désactivé pendant ce temps.

## Historique des recherches {#search-history}

![Historique des recherches : recherches passées avec leur date, leur périphérique et leurs critères](/img/search_03_history.png)

Chaque recherche est enregistrée dans l'historique de la collection, avec ses critères et le périphérique sur lequel elle a été lancée.

*Historique* ouvre la liste des recherches passées. Chaque ligne indique la date, le périphérique parcouru (ou *Tous*) et un résumé des critères.<br/>
Cliquer sur une ligne **restaure tous ses critères et le périphérique sélectionné**. Cliquez sur *Chercher* pour la relancer.

| Bouton | Action |
|--------|--------|
| *Conserver les 10 derniers* | Supprime toutes les recherches sauf les 10 plus récentes, après confirmation. |
| *Effacer* | Supprime tout l'historique, après confirmation. |
| *Annuler* | Ferme l'historique. |

## Développement
* Voir le backlog du [développement de la Recherche](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=search).
