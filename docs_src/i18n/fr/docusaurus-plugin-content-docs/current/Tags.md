---
version: "2.12"
---
# Étiquettes
![2.12](https://img.shields.io/badge/Version-2.12-blue)

## Résumé
Cette page décrit toutes les fonctionnalités de l'écran **Etiquettes** et comment les utiliser.<br/>
À partir de cet écran, l'utilisateur peut **attribuer plusieurs étiquettes à n'importe quel dossier**.<br/>
Cela peut ensuite être utilisé dans l'écran [Recherche](Search) pour affiner les résultats ;<br/>

Le balisage d'un dossier se fait en 3 étapes principales :
1. Sélectionnez le chemin source du dossier,
1. Sélectionnez ou créez un nouveau Tag,
1. Cliquez pour étiquetter le dossier.

![Écran Étiquettes montrant l'interface d'attribution des étiquettes](/img/screen_tags_01.png)

## Attribuer des étiquettes aux répertoires
Remarque : Les étiquettes sont attribuées à un chemin de dossier absolu, elles sont donc indépendantes de la hiérarchie des périphériques et des catalogues.

### Sélectionnez le chemin source
Il existe 3 manières de sélectionner le chemin source du dossier contenant les fichiers à inclure dans le nouveau catalogue :
1. en tapant le chemin dans la zone d'édition de texte
1. en utilisant l'arborescence du système de fichiers, développez simplement et cliquez sur le bon périphérique ou dossier
1. ou en cliquant sur le bouton *Sélectionner* qui ouvrira une fenêtre de dialogue d'aide à la sélection du dossier.

Le chemin sélectionné apparaîtra toujours dans la zone d'édition de texte et l'application utilisera ce chemin pour la étiquette.

### Sélectionnez un tag existant ou créez-en un
* tapez un nouveau nom de tag dans le zome d'édition sous "Sélectionner un tag"
* ou cliquez sur n'importe quel élément de la liste des étiquettes existantes pour réutiliser une étiquette existante

### Attribue le tag au dossier sélectionné
Cliquez sur le bouton "Marquer le dossier".<br/>
Cela enregistrera l'association entre le dossier et le nom de la étiquette.<br/>
La liste "Dossiers et étiquettes actuels" est actualisée avec la nouvelle entrée.

## Modification des étiquettes
### Supprimer les associations existantes
Dans la liste "Dossiers et tags actuels", faites un clic droit sur l'association à supprimer et sélectionnez *Supprimer cette étiquette*.
### Modifier le fichier Tags
En mode *Mémoire*, le bouton *Ouvrir fichier* permet d'éditer directement le fichier source (le format ne doit pas être modifié).


## Développement
Quelques idées d'évolutions pour cet écran :
* étiquetter les périphériques
* étiquetter les dossiers dans des catalogues spécifiques
* Pour en savoir plus, consultez la liste de [Développements de étiquettes](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=tag).
