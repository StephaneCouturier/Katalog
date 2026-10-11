---
version: "3.0"
---
# Sélection
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Résumé

Cette page décrit la page **Sélection** : la liste de tous les périphériques de la collection, présentée sous forme d'arborescence de cartes. C'est la première page de la fenêtre, et les autres écrans s'ouvrent à côté d'elle (voir [Interface](Interface#pages)).

Sélectionner un périphérique définit la **portée** des autres écrans :
* [Recherche](Search) : la recherche porte sur les fichiers du périphérique sélectionné,
* [Périphériques](Devices) : avec *Filtrer depuis la sélection*, seuls le périphérique sélectionné et ses sous-périphériques sont listés,
* [Créer](Create) : le stockage sélectionné est proposé pour le nouveau catalogue,
* [Sauvegarde](BackUp) : les liens de sauvegarde sont filtrés sur le périphérique sélectionné,
* [Statistiques](Statistics) : les graphiques montrent le périphérique sélectionné.

## Périphérique sélectionné {#selected-device}
Le haut de la page montre toujours le **périphérique sélectionné**, avec son icône et son nom. Lorsqu'aucun périphérique n'est sélectionné, il affiche *Tous* : la collection entière.

Cliquer dessus fait défiler la liste jusqu'à la carte sélectionnée, ce qui est pratique dans une longue liste. Cela ne change pas la sélection.

## Cartes des périphériques {#device-cards}
<div className="screenshot-side">
![Page Sélection : périphérique sélectionné en haut, champ de filtre, arborescence de cartes avec un groupe physique sélectionné et les infos des périphériques affichées](/img/selection_01_overview.png)
</div>

Chaque périphérique de la collection est présenté sous forme de carte, en retrait sous son périphérique parent.
* **Cliquez** sur une carte pour sélectionner le périphérique. La carte sélectionnée est mise en évidence.
* **Faites un clic droit** sur une carte (ou un appui long sur un écran tactile) pour ouvrir son [menu contextuel](#context-menu).
* La flèche à droite d'une carte masque ou affiche ses sous-périphériques.

Le type de périphérique se reconnaît à son icône et au style de son nom :

| Périphérique | Icône | Nom |
|--------------|-------|-----|
| Périphérique ou groupe **Virtuel** | plusieurs disques | gras italique, atténué |
| Périphérique de **Stockage** | disque dur | gras, légèrement atténué |
| **Catalogue** | disque optique (une icône lorsque le catalogue est disponible, une autre lorsqu'il ne l'est pas) | normal |

Voir [Périphériques](Devices) pour savoir ce que sont les périphériques Virtuel, Stockage et Catalogue.

Lorsque les infos des périphériques sont affichées, une seconde ligne sous le nom indique :
* pour un **Catalogue** : son nombre de fichiers et leur taille totale,
* pour un périphérique de **Stockage** ou **Virtuel** : le nombre de fichiers et la taille totale de ses catalogues, puis l'espace utilisé.

<div className="screenshot-side-end" />

## Filtre et boutons de l'arborescence {#filter}

<div className="screenshot-side">

![Filtre des périphériques avec « back » saisi : seuls les périphériques correspondants sont listés](/img/selection_02_filter.png)

</div>

Au-dessus des cartes :

| Contrôle | Utilisation |
|----------|-------------|
| Champ *Nom...* | Filtre les cartes **par nom de périphérique** : seuls les périphériques dont le nom contient le texte saisi sont listés, sans tenir compte de la casse. Il filtre les périphériques, pas les fichiers. Le bouton en forme de croix l'efface. `Ctrl+F` place le curseur dans ce champ. |
| *Afficher les infos du périphérique* | Affiche ou masque la seconde ligne des cartes. |
| *Réduire d'un niveau* | Masque le niveau de périphériques le plus profond actuellement affiché. |
| *Développer d'un niveau* | Affiche un niveau de périphériques supplémentaire. |

<div className="screenshot-side-end" />

## Menu contextuel {#context-menu}

<div className="screenshot-side">

![Menu contextuel d'une carte Catalogue](/img/selection_03_context_menu.png)

</div>

Le menu contextuel commence par le nom du périphérique, suivi de :

| Élément | Affiché pour | Action |
|---------|--------------|--------|
| *Chercher* | tous les périphériques | Sélectionne le périphérique et ouvre la page [Recherche](Search). |
| *Actualiser* | les périphériques Virtuels et de Stockage, et les Catalogues disponibles | Met à jour le périphérique : analyse à nouveau les fichiers de ses catalogues. Demandé pendant qu'une autre opération est en cours, il est ajouté à la [file d'attente](Interface#queue). |
| *Explorer* | les Catalogues | Sélectionne le catalogue et l'ouvre dans la page [Explorer](Explore). |
| *Ouvrir le dossier* | tous les périphériques | Ouvre le dossier du périphérique dans le gestionnaire de fichiers. |
| *Modifier* | tous les périphériques | Ouvre la page de modification du périphérique (voir [Périphériques](Devices)). |

*Chercher* et *Explorer* sélectionnent d'abord le périphérique, de sorte que la carte mise en évidence correspond toujours à ce qui est recherché ou exploré. *Actualiser*, *Ouvrir le dossier* et *Modifier* agissent sur le périphérique cliqué **sans changer la sélection**.

<div className="screenshot-side-end" />

## Rechercher dans un lecteur connecté
La recherche directe dans un dossier d'un lecteur connecté, sans catalogue, se choisit sur la page [Recherche](Search) elle-même (*Lecteur connecté*), pas dans la Sélection.

## Développement
* Voir le backlog du [développement de la Sélection](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=selection).
