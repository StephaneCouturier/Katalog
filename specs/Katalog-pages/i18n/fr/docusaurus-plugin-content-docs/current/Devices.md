# Périphériques
## Résumé
Cette page décrit le concept de **Périphérique** dans Katalog et la partie supérieure de l'écran **Périphériques**.

![](/img/devices_example1_cut.png)

Dans cet exemple, plusieurs catalogues ont été créés à partir de 2 disques physiques.<br/>
Les catalogues avec photos/images/images ont été attribués à un périphérique *Photos* virtuel.<br/>
Cela permet de rechercher uniquement dans ces éléments et fournit le nombre total et la taille des fichiers de photos.<br/>

## Modèle


### Définitions

* Un périphérique **[Catalogue](DevicesCatalogs)** est une liste de fichiers dans un répertoire particulier.

* Un périphérique **[Storage](DevicesStorage)** est un lecteur physique sur lequel les fichiers sont stockés. Généralement, il est « monté » ou « connecté » à l’ordinateur et dispose d’un espace de stockage physique.

* Un périphérique **Virtuel** est tout élément non physique utilisé pour regrouper d'autres périphériques. Il n'a aucune propriété en soi et peut regrouper les nombres des sous-appareils associés.

* Un **Groupe** est un périphérique virtuel situé au sommet de la hiérarchie.

 * Le **Groupe Physique** est un groupe unique et réservé à la hiérarchie des périphériques physiques (ordinateur, téléphone, disque, etc.).

 * Tout autre groupe est un **Groupe Virtuel** auquel les catalogues existants peuvent être « attribués » pour faciliter la recherche et les statistiques.

### Hiérarchie

![](/img/devices_model.png)


## Fonctionnalités

Celles-ci sont toujours disponibles en haut de l’écran.

### Choisir entre 3 vues

Les périphériques peuvent être répertoriés et gérés de 3 manières :

**[Arborescence des Périphériques](DevicesTree)** : Cette vue affiche la liste complète et non filtrée des périphériques dans une hiérarchie, une arborescence.

**[Liste de stockage](DevicesStorage)** : cette vue affiche uniquement les périphériques de stockage et est filtrée en fonction du panneau [Sélection](Selection).

**[Liste du catalogue](DevicesCatalogs)** : Cette vue affiche uniquement les périphériques du catalogue et est filtrée en fonction du panneau [Sélection](Selection).

### Afficher le tableau complet
Cliquez sur cette option pour afficher toutes les données disponibles dans la vue.

Si cette case n'est pas cochée, cela masque généralement des données qui ne sont peut-être pas nécessaires au quotidien (ex : identifiant interne).

Cela peut aider à conserver une vue plus simple et plus lisible.

### Enregistrez un instantané des données
Ce bouton déclenche un enregistrement de toutes les valeurs des périphériques (taille, fichiers, espace, etc.) indépendamment de la sélection actuelle.

Ces enregistrements prennent en charge la création de [Statistiques](Statistics), et en particulier pour suivre la collection à l'échelle mondiale et indépendamment des mises à jour de chaque appareil.
