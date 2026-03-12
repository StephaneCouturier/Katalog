---
version: "2.10"
---
# Périphériques
![2.10](https://img.shields.io/badge/Version-2.10-blue)

## Résumé
Cette page décrit le concept de **Périphérique** dans Katalog et la partie supérieure de l'écran **Périphériques**.

![Exemple montrant des catalogues organisés sous des disques physiques et un groupe Photos virtuel](/img/devices_example1_cut.png)

Dans cet exemple, plusieurs catalogues ont été créés à partir de 2 disques physiques.<br/>
Les catalogues de photos/images ont été attribués à un périphérique *Photos* virtuel.<br/>
Cela permet de rechercher uniquement dans ces éléments et fournit le total du nombre et de la taille des fichiers photos.

## Modèle

### Définitions

* Un périphérique **[Catalogue](DevicesCatalogs)** est un index de fichiers issus d'un répertoire particulier.

* Un périphérique **[Stockage](DevicesStorage)** est un lecteur physique sur lequel les fichiers sont stockés. En général, il est monté ou connecté à l'ordinateur et dispose d'un espace de stockage physique.

* Un périphérique **Virtuel** est tout élément non physique utilisé pour regrouper d'autres périphériques. Il n'a pas de propriétés propres et peut agréger les totaux des sous-périphériques associés.

* Un **Groupe** est un périphérique virtuel au sommet de la hiérarchie.

    * Le **Groupe Physique** est un groupe unique et réservé à la hiérarchie des périphériques physiques (ordinateur, téléphone, disque, etc.).

    * Tout autre groupe est un **Groupe Virtuel** auquel des catalogues existants peuvent être attribués pour faciliter la recherche et les statistiques.

### Hiérarchie

![Diagramme montrant la hiérarchie des périphériques avec groupes, stockages et catalogues](/img/devices_model.png)

## Fonctionnalités

Toujours disponibles en haut de l'écran :

### Choisir entre 3 vues

Les périphériques peuvent être listés et gérés de 3 façons :

**[Arborescence des périphériques](DevicesTree)** : affiche la liste complète et non filtrée de tous les périphériques dans une structure hiérarchique.

**[Liste de stockage](DevicesStorage)** : affiche uniquement les périphériques de stockage, filtrés selon le panneau [Sélection](Selection).

**[Liste des catalogues](DevicesCatalogs)** : affiche uniquement les périphériques de type catalogue, filtrés selon le panneau [Sélection](Selection).

### Afficher le tableau complet
Lorsqu'elle est activée, toutes les colonnes disponibles sont affichées dans la vue courante.

Lorsqu'elle est décochée, les colonnes non nécessaires au quotidien (comme les identifiants internes) sont masquées, ce qui rend la vue plus simple et plus lisible.

### Mettre à jour le périphérique actif
Met à jour le périphérique actuellement sélectionné — re-scanne ses fichiers depuis le chemin source.

Ce bouton est actif lorsqu'un périphérique dont le chemin source est accessible (affiché avec une icône colorée) est sélectionné dans l'une des trois vues.

### Enregistrer un instantané
Enregistre les valeurs actuelles de tous les périphériques (nombre de fichiers, taille des fichiers, espace libre, espace total) indépendamment de la sélection ou des filtres en cours.

Après l'enregistrement, un résumé est affiché avec les nouveaux totaux et l'écart depuis le précédent instantané (delta).

Ces enregistrements alimentent les [Statistiques](Statistics) et permettent de suivre la collection globalement au fil du temps.
