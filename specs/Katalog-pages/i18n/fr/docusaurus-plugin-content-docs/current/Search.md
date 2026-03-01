# Recherche
## Résumé
Cette page décrit toutes les fonctionnalités de l'écran **Recherche** et comment les utiliser.<br/>
Les principales caractéristiques de cet écran sont :

* Rechercher des fichiers dans les catalogues ou à partir de lecteurs connectés
* Utilisez plusieurs **critères** pour affiner et réduire le nombre de résultats :
 * La recherche filtrera les résultats en fonction du texte que vous avez saisi dans la ligne _Texte_ par défaut.
 * et peut être affiné avec plus d'[options de recherche de texte](Search#search-text-criteria),
 * fournir des [attributs de fichier](Search#file-criteria) tels que la taille ou le type,
 * mise en évidence des [fichiers dupliqués](Search#duplicates-on),
 * ou mettre en évidence les [différences](Search#differences-on) entre 2 catalogues.
* Utilisez les **Résultats** avec un [menu contextuel](Search#file-context-menu) (clic droit) ou [Exporter les résultats](Search#batch-process) eux,
* Réutilisez les critères précédents de l'historique de recherche.
* Tous les historiques et critères de recherche sont enregistrés et disponibles lors de la prochaine ouverture de Katalog, à l'exception des résultats eux-mêmes.
![](/img/screen_search_01.png)
## Recherche depuis les catalogues ou depuis les lecteurs connectés
Bien que Katalog soit spécifiquement conçu pour gérer des catalogues de fichiers à rechercher hors ligne, il est toujours possible d'effectuer une recherche directement sur des lecteurs connectés (en ligne).
Dans le panneau de gauche, l'onglet "Filtres" est divisé en 2 parties, une seule peut être activée :

### Rechercher dans les catalogues de fichiers
Utilisez 3 filtres optionnels pour limiter la liste des catalogues dans lesquels rechercher des fichiers :
- _Location_ : il s'agit d'une liste d'emplacements distincts tels que saisis dans l'écran "Stockage". La sélection d'un emplacement réduit la liste des périphériques de stockage et des catalogues à ceux correspondants.
- _Storage_ : il s'agit d'une liste de périphériques de stockage distincts tels que saisis dans l'écran "Stockage". La sélection d'un périphérique de stockage réduit la liste aux catalogues correspondants.
- _Catalogues_ : cela restreint la recherche à 1 catalogue spécifique.

### Rechercher dans les lecteurs connectés
Recherchez directement sur l'ordinateur et les lecteurs connectés.
Cela peut permettre de gagner du temps lors de la recherche dans un dossier spécifique plutôt que dans un lecteur ou un catalogue entier, ou cela peut aider à obtenir un aperçu rapide des modifications récentes sans avoir à mettre à jour un catalogue auparavant.

## Critères de recherche
Ce panel regroupe tous les critères permettant d'affiner et de réduire le nombre de résultats. Il peut être caché.<br/>
Ils s'appliquent au chemin et aux noms des fichiers, aux informations sur les fichiers et aux balises, et peuvent identifier les doublons ou aider à voir les différences entre 2 catalogues<br/>
Les résultats peuvent être une liste de fichiers ou une liste de dossiers contenant les fichiers correspondants.<br/>
![](/img/screen_search_02.png)
### Critères de recherche texte {#search-text-criteria}
#### Nom de fichier
Cette zone est l'endroit où saisir le texte à utiliser pour la recherche sur le nom du fichier, qui peut inclure le chemin du fichier.

Définition : Ici un « mot » est un groupe de caractères séparé d'un autre groupe par un caractère espace.<br/>
Cela peut être utilisé pour rechercher des dossiers, des fichiers ou des fichiers dans certains dossiers.<br/>
Par défaut, c'est le seul critère sélectionné<br/>
Tous les critères peuvent être effacés/réinitialisés avec le bouton Réinitialiser.

Pour faciliter la recherche de mots clés à partir d'un texte copié :
- Ce texte peut être collé depuis le presse-papier,
- et il peut être nettoyé des caractères spéciaux : supprimé par cette fonction : . , _ - ( ) [ ] { }

Lorsqu'elle est disponible, cliquer sur la case à cocher placée avant un critère activera/désactivera le critère, sans perdre les valeurs sélectionnées.<br/>

#### Avec
Précisez comment les mots saisis dans le « Texte » doivent être utilisés :
* _Tous les mots_ : un fichier ou un dossier n'est renvoyé que si tous les mots du texte sont trouvés.
* _Begin With_ : uniquement pour l'option "Noms de fichiers", le nom du fichier doit commencer par le Texte (y compris les espaces entre les mots).
* _N'importe quel mot_ : un fichier ou un dossier est renvoyé si au moins un des mots du texte est trouvé.

#### Dans
Spécifiez dans quelle partie du chemin absolu du fichier le texte doit être utilisé.
* _Noms de fichiers uniquement_ : les mots de texte seront utilisés uniquement pour examiner les noms de fichiers.
* _Chemin du dossier uniquement_ : les mots de texte seront utilisés uniquement pour examiner les chemins des dossiers.
* _Noms de dossiers et de fichiers_ : les mots de texte seront utilisés pour examiner à la fois les chemins de dossiers et les noms de fichiers.

#### sensible aux majuscules et minuscules
Forcez la recherche à faire correspondre les caractères exacts (petites majuscules ou majuscules).

#### exclure
Contrairement au « Texte », cela exclura les résultats si _l'un_ des mots fournis est trouvé dans le chemin ou le nom du fichier.

### Critères de fichier {#file-criteria}

#### Taille
Définissez la plage de taille de fichier en entrant un nombre et une unité pour la taille minimale et maximale du fichier.

#### Types de fichier
 | Tapez | Rallonges |
 | ------| -------------------------------------------------- |
 | Audio | aif, mp3, ogg, wav |
 | Images | png, jpg, gif, xcf, tif, bmp, brut |
 | Texte | txt, pdf, odt, idx, html, rtf, doc, docx, epub |
 | Vidéo | wmv, avi, mp4, mkv, flv, webm, m4v, vob, ogv, mov |

#### Dates
Définissez la plage de dates de modification du fichier.

#### Doublons sur {#duplicates-on}
Possibilité d'affiner les résultats et d'afficher uniquement les fichiers qui pourraient être des doublons.<br/>
La notion de Doublons peut être définie comme une ou une combinaison de plusieurs parties :
le Nom, la Taille, la Date de Modification.
Au moins une de ces options est nécessaire pour considérer ce qui pourrait être des doublons.

#### Différences sur {#differences-on}
Trouver les différences entre 2 périphériques de n'importe quelle nature (Virtuel, Stockage, Catalogues) (utile pour comparer avec une sauvegarde par exemple).<br/>
Quant aux Doublons, il se combine avec d'autres critères de Recherche et la notion de différence peut s'appliquer au Nom, à la Taille ou à la Date de modification.
![](/img/screen_search_03_diff.png)

### Critères de dossier

#### Afficher uniquement les dossiers
Option pour afficher les dossiers uniquement sous forme de résultats au lieu de fichiers.

#### Mots clés
Option pour affiner les résultats sur un _Tag_ sélectionné.
Les tags sont définis dans l'écran _Tags_. Actuellement, seuls les dossiers peuvent être balisés.

## Résultats
Les résultats de la recherche sont affichés dans cette zone.<br/>

### Catalogues avec résultats
Le panneau de gauche affiche la liste des catalogues dans lesquels les résultats ont été trouvés.<br/>
Un clic sur l'un de ces Catalogues déclenchera la même recherche mais affinée sur le catalogue sélectionné.<br/>
Ce panneau peut être masqué pour économiser de l'espace et afficher davantage d'informations sur les résultats du fichier.<br/>

### Fichiers (ou doublons) trouvés
Le panneau de droite affiche la liste des fichiers ou dossiers trouvés selon les critères de recherche.<br/>
La partie supérieure indique le nombre de fichiers trouvés ou le nombre de doublons (nombre de résultats uniques) et la taille totale des fichiers trouvés. <br/>
L'icône ouvre ensuite une fenêtre pour voir plus de statistiques sur les résultats :<br/>
Taille totale, min, max, taille moyenne, date min, max.<br/><br/>

### Traitement par lots {#batch-process}
| Cas | Entrée de menu |
| -----------------| -------------------|
| Exporter les résultats | Exporter les résultats vers un nouveau catalogue (avec lequel une recherche plus précise peut être effectuée) ou exporter vers un fichier csv, nommé avec la date et situé dans le [dossier Collection] (Paramètres # collection). |
| Renommer (KRename) | Ouvrez tous les fichiers répertoriés dans les résultats avec [KRename](https://apps.kde.org/krename/) |
| &#9888; Déplacer vers la corbeille | déplacez tous les fichiers répertoriés dans les résultats vers la corbeille. |
| &#9888; Supprimer | supprimez tous les fichiers répertoriés dans les résultats (pas de récupération).|

### Menu contextuel du fichier {#file-context-menu}
Un clic droit sur une ligne de résultat ouvrira un menu contextuel avec les options suivantes :
* _Ouvrir le fichier_ : l'application par défaut de l'ordinateur sera utilisée pour ouvrir le fichier, s'il est disponible/en ligne.
* _Open Folder_ : le gestionnaire de fichiers par défaut de l'ordinateur sera utilisé pour ouvrir le dossier et afficher son contenu, s'il est disponible/en ligne.

Copiez dans le presse-papiers quelques informations sur le fichier, avec l'exemple du chemin complet d'un fichier : _/home/user/documents/filename.txt_
* _Copier le chemin du dossier_ : _/home/user/documents_
* _Copier le chemin absolu du fichier_ : _/home/user/documents/filename.txt_
* _Copier le nom du fichier avec l'extension_ : _filename.txt_
* _Copier le nom du fichier sans extension_ : _filename_

Opérations sur les fichiers :
* _Mettre à la corbeille_
* _Supprimer le fichier_

## Historique des recherches
Chaque fois qu'une recherche est déclenchée, les critères de recherche et les catalogues sélectionnés sont enregistrés dans un fichier historique situé dans le dossier Collection.<br/>
Ce fichier est ensuite chargé et affiché dans ce tableau.<br/>
Un clic sur une ligne du tableau rétablira toutes les valeurs des critères aux critères de recherche et permettra de relancer exactement la même recherche que précédemment ou de faciliter une certaine adaptation.<br/>
Ce panneau peut être masqué pour économiser de l'espace et afficher davantage d'informations sur les résultats du fichier.<br/>

![](/img/screen_search_04_search_history.png)
