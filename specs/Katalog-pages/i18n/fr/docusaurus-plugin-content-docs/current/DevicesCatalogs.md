# Périphériques : Catalogues

## Résumé
Cette page décrit toutes les fonctionnalités de la vue **Liste des catalogues** de l'écran [Périphériques](Devices) et comment les utiliser.
* Un *Catalogue* est un index de fichiers d'un répertoire donné appelé **chemin** de catalogue.
* La **Liste des catalogues** affiche tous les catalogues de la [Collection](Settings#collection), filtrés selon la [Sélection](Selection).

![](/img/devices_catalogs_01.png)

## Liste et sélection
La liste des catalogues peut être limitée en utilisant le panneau de gauche [Sélection](Selection).
Si le chemin source d'un catalogue pointe vers un chemin actif/connecté/monté, l'icône du catalogue est colorée en bleu.

## Boutons d'actions
* **Mise à jour** : (activé lorsqu'un catalogue est sélectionné) Mettez à jour le catalogue sélectionné en répertoriant à nouveau tous les fichiers à partir de son chemin source, selon ses critères.
* **Tous actifs** : Mettez à jour tous les catalogues affichés qui sont actifs (le chemin est accessible).
* **[Import](#importer)** : Importe un catalogue depuis un autre outil. Actuellement, l'import depuis un export VVV (csv, séparés par des tabulations) est pris en charge.

## Menu contextuel (clic droit)

Un clic droit sur l'un des catalogues répertoriés ouvre un menu contextuel permettant d'agir sur ce catalogue actif.

![](/img/devices_catalogs_02_context2.png)

 | Entrée de menu | Action connexe |
 | ------------| -------------------------------------------------- |
 | **Mise à jour** | Mettre à jour le catalogue sélectionné en listant à nouveau tous les fichiers depuis son chemin source, selon ses critères (type de fichier, fichiers cachés, etc.) |
 | **Explorer** | Ouvrez le fichier de catalogue sélectionné dans l'écran [Explorer](Explore) pour afficher les dossiers et fichiers du catalogue. |
 | **[Editer](#édition)** | Ouvrez un panneau pour modifier le nom, le chemin, le périphérique de stockage, etc. Consultez les détails ci-dessous avant de modifier un champ pour comprendre les conséquences. |
 | **Filelight** | Ouvrez [Filelight](https://apps.kde.org/filelight/) dans le chemin du catalogue. |
 | **Supprimer** | Supprimez le catalogue. (Cela ne supprime pas le fichier de sauvegarde, ni les valeurs associées dans les statistiques) |

## Édition
Il est généralement recommandé de créer un nouveau catalogue avec les bons paramètres initiaux :<br/>
Sinon, le panneau donne accès à la modification des champs suivants :
* **Nom de l'appareil**
* **Nom du parent (ID)** (périphérique de stockage différent).
* **Chemin source**
* **Type de fichier**
* **Inclure les fichiers cachés**
![](/img/devices_catalogs_03_edit.png)
## Importer
Il est désormais possible d'importer tous les volumes physiques VVV à partir d'un seul fichier d'export réalisé en utilisant la tabulation comme séparateur.<br/>
Chaque volume physique VVV deviendra 1 catalogue distinct. <br/>
Limite : le chemin source et d'autres informations sur le volume ne sont pas disponibles dans l'export depuis VVV.
- Dans VVV, choisissez Fichier / Exporter..., et sélectionnez TAB comme séparateur
- Depuis Katalog, écran Catalogues : cliquez sur le bouton Importer et sélectionnez le fichier créé précédemment.
- A l'aide du bouton Modifier, il est possible d'ajouter le chemin source du catalogue pour permettre les mises à jour ultérieurement.
