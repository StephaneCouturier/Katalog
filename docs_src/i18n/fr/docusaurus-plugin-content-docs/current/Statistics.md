---
version: "2.12"
---
# Statistiques
![2.12](https://img.shields.io/badge/Version-2.12-blue)

## Résumé
Cette page décrit toutes les fonctionnalités de l'écran **Statistiques** et comment les utiliser.

Toutes les données proviennent d'enregistrements de diverses mises à jour ou *instantanés*.

Cet écran permet de visualiser le contenu et l'évolution d'une collection :
1. Pour les périphériques de catalogue : le nombre de fichiers ou la taille totale du fichier.
1. Pour les périphériques de stockage : espace utilisé et total, ainsi que la taille totale des fichiers des catalogues associés, ou le nombre de fichiers.
1. Pour les périphériques virtuels : nombre de fichiers des périphériques associés, ou espace total et taille totale des fichiers des catalogues associés.

![Écran Statistiques montrant le graphique d'évolution de la collection](/img/screen_statistics_01.png)

## Caractéristiques
### Option de Données
* Les données sont basées sur le périphérique sélectionné dans le panneau [Sélection] (Selection).
* *Source* : choisissez si toutes les données doivent être utilisées, ou les mises à jour et les instantanés indépendamment.
* *Type de données* : choisissez d'afficher la taille totale du fichier (qui peut inclure l'appareil utilisé et l'espace total) ou le nombre de fichiers uniquement.
* *Afficher chaque valeur* : choisissez d'afficher un petit losange pour chaque point de données

### Modifier le fichier
* disponible en [Mode mémoire](Settings#database-memory-mode) uniquement.
* Bouton *Modifier les statistiques* : Il peut être utile d'éditer le fichier Statistiques pour corriger certains chiffres, le bouton ouvrira le fichier dans l'application associée aux fichiers csv. Veillez à le conserver dans un **fichier séparé par des tabulations**
* Bouton *Recharger* : les données sont rechargées et le graphique rafraîchi.

### Graphique
* clic gauche et maintien pour zoomer sur une partie du graphique.
* clic droit pour effectuer un zoom arrière.
* cliquez sur le bouton *Recharger* pour revenir au zoom d'origine.

## Développement
* Voir la liste de [développement de Statistiques](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=statistics).
