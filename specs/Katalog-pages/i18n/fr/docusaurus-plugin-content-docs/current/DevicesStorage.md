# Périphériques : Stockage

## Résumé
Cette page décrit toutes les fonctionnalités de la vue **Liste de stockage** de l'écran [Périphériques](Devices) et comment les utiliser.
* Un périphérique de **stockage** est un véritable lecteur physique, un disque ou un autre type de mémoire stockant des données sous forme de fichiers et à partir duquel vous pouvez créer un ou plusieurs catalogues.
* Les données associées à ce type d'appareil sont une combinaison de 3 types d'informations :
 * *stockage physique* : **espace libre**, **espace utilisé**, **espace total**. **étiquette**, **système de fichiers**.
 * *calculé* : le **nombre total de fichiers** et la **taille totale des fichiers** seront automatiquement renseignés si le stockage dispose de catalogues.
 * *Personnalisé par l'utilisateur* : **chemin**, **type**, **marque**, **modèle**, **numéro de série**, **date de construction**, **commentaire 1** , **commentaire 2**, **commentaire 3**.
* Leur utilisation peut vous aider à effectuer une recherche sur plusieurs catalogues associés à cet appareil particulier.
* Ils vous aideront également à voir plus de [statistiques](Statistics) sur tous vos appareils et ce qu'ils stockent.
* Le stockage ne peut faire partie que du *Groupe physique* et peut être placé sous n'importe quel *périphérique virtuel* de ce groupe, ce qui peut être utile pour les fonctionnalités de recherche ou de statistiques.
![](/img/devices_storage_01.png)
## Liste et sélection
La liste des catalogues peut être limitée en utilisant le panneau de gauche [Sélection](Selection).

## Boutons d'actions
* **Mise à jour** : (activé lorsqu'un catalogue est sélectionné) Mettez à jour le catalogue sélectionné en répertoriant à nouveau tous les fichiers à partir de son chemin source, selon ses critères.
* **Tous actifs** : Mettez à jour tous les catalogues affichés qui sont actifs (le chemin est accessible).

## Menu contextuel (clic droit)
Un clic droit sur l'un des catalogues répertoriés ouvre un menu contextuel permettant d'agir sur ce catalogue actif.
![](/img/devices_storage_02_context.png)
 | Entrée de menu | Action connexe |
 | ------------| -------------------------------------------------- |
 | **Mise à jour** | Mettre à jour le stockage sélectionné : cela mettra à jour le stockage lui-même et la mise à jour de tous les catalogues ci-dessous. |
 | **[Editer](#édition)** | Ouvrez un panneau pour modifier le nom, le chemin, etc. |
 | **Filelight** | Ouvrez [Filelight](https://apps.kde.org/filelight/) dans le chemin du périphérique de stockage. |
 | **Supprimer** | Supprimez le périphérique de stockage. Cela n'est possible que si aucun catalogue ne lui est associé. Cela ne supprime pas les valeurs associées dans les statistiques. |

## Édition
Le panneau permet de modifier tous les champs du périphérique de stockage, à l'exception de l'ID du périphérique lui-même.
![](/img/devices_storage_03_edit.png)

## Image de l'appareil
Cette fonctionnalité n'est actuellement disponible que pour le [Mode date mémoire](Settings#database-memory-mode) car les images sont stockées avec le dossier de collection.

Il est possible d'associer une image à un périphérique de stockage.<br/>
Un dossier *images* doit être créé dans le dossier de collection et l'image nommée avec l'ID de stockage (pas l'ID de l'appareil, mais l'ID de stockage).<br/>
Exemple : /home/user/Documents/KatalogCollectionFolder/images/3.jpg


![](/img/devices_storage_04_picture.png)
