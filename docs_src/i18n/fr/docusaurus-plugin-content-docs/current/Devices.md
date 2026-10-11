---
version: "3.0"
---
# Périphériques
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Résumé
Cette page décrit la page **Périphériques**, où les périphériques de la collection sont listés et gérés :
* le [modèle des périphériques](#model) : catalogues, stockages, périphériques virtuels et groupes,
* les [vues](#views) : tous les périphériques, stockages ou catalogues, en cartes ou en tableau,
* les [actions de la page](#page-actions) : mettre à jour tous les catalogues actifs, instantané, nouveau groupe ou stockage,
* le [menu contextuel](#context-menu) d'un périphérique : actualiser, explorer, éditer, sommes de contrôle, diviser, attribuer, supprimer.

La modification d'un périphérique (nom, chemin, options du catalogue, détails du stockage) est décrite sur la page [Modifier un périphérique](DevicesEdit).

![Page Périphériques : tous les périphériques sous forme d'arborescence de cartes](/img/devices_01_overview.png)

## Modèle des périphériques {#model}

### Définitions
* Un **Catalogue** est un index des fichiers d'un répertoire : son **chemin source**. Les catalogues se créent sur la page [Créer](Create).
* Un périphérique de **Stockage** est un disque ou un autre support physique où des fichiers sont stockés. Il a un chemin où il est monté, et un espace physique.
* Un périphérique **Virtuel** regroupe d'autres périphériques. Il n'a pas de données propres, et additionne les chiffres des périphériques situés en dessous de lui.
* Un **Groupe** est un périphérique virtuel situé en haut de l'arborescence :
    * le **Groupe physique** est unique et réservé aux périphériques physiques (ordinateur, disque, téléphone…) et à leurs catalogues,
    * tout autre groupe est un **Groupe virtuel**, auquel des catalogues existants sont attribués, par exemple tous les catalogues de photos dans un groupe *Photos*.

Tout périphérique peut avoir un **commentaire** : une note libre, affichée à côté de son nom.

### Hiérarchie

![Hiérarchie des périphériques : groupes, périphériques virtuels, périphériques de stockage et catalogues](/img/devices_model.png)

Un catalogue appartient à un stockage, dans le Groupe physique. Il peut aussi être attribué à des périphériques virtuels des Groupes virtuels : cela permet de chercher uniquement dans ces catalogues, et de voir leurs totaux.

### Périphériques disponibles {#available}
Un périphérique est **disponible** lorsque son chemin est accessible : le disque est connecté et monté. Son icône est affichée en couleur ; un périphérique non disponible a une icône grise.<br/>
Les catalogues des périphériques non disponibles peuvent toujours être recherchés et explorés ; seules les actions qui lisent le disque (actualiser, ouvrir, sommes de contrôle…) ont besoin du périphérique.

## Vues {#views}
La barre en haut de la page choisit ce qui est listé et comment.

| Bouton | Effet |
|--------|-------|
| *Tous les périphériques* | Toute l'arborescence : le Groupe physique avec ses stockages et catalogues, et les Groupes virtuels avec leurs périphériques attribués. |
| *Stockage* | Uniquement les périphériques de stockage. |
| *Catalogues* | Uniquement les catalogues. |
| *Cartes* | Chaque périphérique est une carte, avec son nom, son commentaire, ses chiffres et un menu d'actions. |
| *Tableau* | Chaque périphérique est une ligne, avec une colonne par valeur. |
| *Tableau complet* | Dans le tableau, affiche toutes les colonnes, y compris les identifiants internes et les détails moins utilisés. |
| *Filtrer depuis la sélection* | Liste uniquement le périphérique sélectionné sur la page [Sélection](Selection) et les périphériques situés en dessous de lui. |

Les choix sont conservés pour chaque collection.

![Page Périphériques : liste des stockages en tableau, avec la barre des totaux](/img/devices_02_table.png)

### Totaux
Avec *Stockage* ou *Catalogues*, une barre sous les boutons donne les totaux des périphériques listés :
* *Catalogues* : nombre de catalogues, *Taille Totale des Fichiers*, *Nombre Total de Fichiers*,
* *Stockage* : nombre de périphériques, *Espace Total*, espace *Utilisé* et *Libre*.

### Cartes
Une carte montre l'icône et le nom du périphérique, son commentaire, et une ligne avec son type, son nombre de fichiers et sa taille, l'espace utilisé, libre et total pour un stockage, et la date de sa dernière mise à jour.<br/>
Dans *Tous les périphériques*, les cartes sont en retrait pour montrer l'arborescence, et la flèche à droite d'une carte replie ou déplie les périphériques situés en dessous.<br/>
Le bouton de menu à droite d'une carte, un clic droit ou un appui long ouvre le [menu contextuel](#context-menu).

### Tableau
* **Cliquez sur le titre d'une colonne** pour trier selon cette colonne ; cliquez à nouveau pour inverser l'ordre. Les nombres, tailles et dates sont triés selon leur valeur.
* Dans *Tous les périphériques*, la flèche devant un nom replie ou déplie les périphériques situés en dessous.
* **Faites un clic droit** sur une ligne (ou un appui long) pour ouvrir le même [menu contextuel](#context-menu) que sur une carte.
* Le clavier fonctionne comme dans les [résultats de recherche](SearchResults#keyboard). Dans *Tous les périphériques*, `←` et `→` replient et déplient aussi la ligne sélectionnée.

Colonnes principales :

| Vue | Colonnes |
|-----|----------|
| *Tous les périphériques* | *Nom*, *Nombre de fichiers*, *Taille Totale*, *Espace utilisé*, *Espace libre*, *Espace total*, *Date de mise à jour*, *Chemin*. |
| *Stockage* | *Nom*, *ID de stockage*, *Nombre de fichiers*, *Taille Totale*, *Espace utilisé*, *Espace libre*, *Espace total*, *Date de mise à jour*, *Chemin*, *Étiquette*, *Système de fichiers*. |
| *Catalogues* | *Nom*, *Nombre de fichiers*, *Taille Totale*, *Date de mise à jour*, *Chemin*, *Type de Fichier*, *Caché*, *Métadonnées*, *Somme de contrôle*, *Stockage parent*. |

*Tableau complet* ajoute, selon la vue : *Type de périphérique*, *Actif*, les identifiants, *Type*, *Marque*, *Modèle*, *Numéro de Série*, *Date de Fabrication*, *Commentaire 1* à *Commentaire 3*, *Version de l'application*, et en mode Mémoire *Date de chargement* et *Chemin du fichier*.

## Actions de la page {#page-actions}

| Action | Description |
|--------|-------------|
| *Tous les actifs* | Met à jour tous les catalogues **disponibles** situés sous le périphérique sélectionné sur la page [Sélection](Selection) (toute la collection lorsque *Tous* est sélectionné). Une question propose d'abord d'afficher un résumé pour chaque catalogue (*Oui*), uniquement le résultat global (*Non*), ou d'annuler. |
| *Stop* | Arrête la mise à jour en cours. |
| *Instantané* | Enregistre les chiffres actuels de toute la collection (fichiers et taille des catalogues, espace libre et total des stockages), pour les [Statistiques](Statistics). Un résumé montre les nouveaux totaux et ce qui a changé depuis l'instantané précédent. |
| *Insérer un groupe virtuel* | Crée un nouveau Groupe virtuel et l'ouvre dans [Modifier un périphérique](DevicesEdit). |
| *Ajouter un Stockage* | Crée un nouveau périphérique de stockage dans le Groupe physique et l'ouvre dans [Modifier un périphérique](DevicesEdit). |
| *Fermer* | Ferme la page (également `Échap`). |

Les mises à jour demandées pendant qu'une autre est en cours ne sont pas perdues : elles attendent dans une file d'attente, affichée dans le panneau d'activité (voir [Interface](Interface)).

## Menu contextuel {#context-menu}

![Menu contextuel d'une carte de catalogue](/img/devices_03_context_menu.png)

Le menu commence par le nom du périphérique. Ses éléments dépendent du type de périphérique :

| Élément | Pour | Action |
|---------|------|--------|
| *Actualiser* | Stockage, Virtuel, Catalogue disponible | Relit les fichiers depuis le périphérique et met à jour le catalogue. Sur un périphérique de stockage ou virtuel, met à jour tous les catalogues situés en dessous. |
| *Explorer* | Catalogue | Ouvre le catalogue dans la page [Explorer](Explore). |
| *Modifier* | Tous | Ouvre le périphérique dans [Modifier un périphérique](DevicesEdit). |
| *Ouvrir le dossier* | Périphériques ayant un chemin | Ouvre le chemin dans le gestionnaire de fichiers. |
| *Filelight* | Stockage et Catalogue disponibles | Ouvre [Filelight](https://apps.kde.org/filelight/) sur le chemin, pour voir ce qui occupe l'espace. Nécessite que Filelight soit installé. |
| *Vérifier les sommes de contrôle* | Catalogue | Calcule à nouveau les sommes de contrôle de tous les fichiers et les compare à celles enregistrées, après confirmation. Un rapport donne le nombre total de fichiers, et ceux vérifiés, non concordants et manquants. |
| *Vérifier les types MIME* | Catalogue disponible | Lit le contenu de chaque fichier pas encore vérifié, pour trouver son type réel, et corrige le type déduit de l'extension lors de l'indexation. Lorsqu'un type diffère de l'extension, un message donne le nombre de différences et le chemin d'un rapport enregistré dans le dossier de la collection (le dossier personnel en mode *Hébergé*), avec *Ouvrir le rapport*. Non disponible pendant une autre opération sur un périphérique ou pendant une recherche. |
| *Diviser le catalogue par sous-répertoire* | Catalogue | Voir [Diviser un catalogue](#split). |
| *Diviser le catalogue par type de fichier* | Catalogue | Voir [Diviser un catalogue](#split). |
| *Ajouter un périphérique virtuel* | Virtuel | Crée un périphérique virtuel sous celui-ci et l'ouvre dans [Modifier un périphérique](DevicesEdit). |
| *Ajouter un périphérique de stockage* | Virtuel, dans le Groupe physique | Crée un périphérique de stockage sous celui-ci et l'ouvre dans [Modifier un périphérique](DevicesEdit). |
| *Attribuer le catalogue sélectionné* | Virtuel, dans un Groupe virtuel | Attribue à ce périphérique virtuel le catalogue sélectionné sur la page [Sélection](Selection). Disponible lorsqu'un catalogue est sélectionné. |
| *Désattribuer ce catalogue*, *Désattribuer ce stockage* | Catalogue ou stockage dans un Groupe virtuel | Retire le périphérique de ce périphérique virtuel, après confirmation. Le périphérique lui-même est conservé. |
| *Supprimer ce catalogue*, *Supprimer ce stockage*, *Supprimer* | Tous les autres | Supprime le périphérique de la collection, après confirmation. Un périphérique qui a encore des périphériques en dessous de lui ne peut pas être supprimé. Les fichiers sur le disque ne sont pas touchés. |

### Diviser un catalogue {#split}
Un catalogue peut être remplacé par plusieurs catalogues plus petits. Le catalogue d'origine est supprimé : **cette opération ne peut pas être annulée.**

* *Diviser le catalogue par sous-répertoire* : crée un catalogue pour chaque sous-répertoire situé directement sous le chemin source, nommé `<nom du catalogue>_<sous-répertoire>`, et un catalogue nommé `<nom du catalogue>_(root)` pour les fichiers situés directement dans le chemin source. Une confirmation est d'abord demandée.
* *Diviser le catalogue par type de fichier* : crée un catalogue par type de fichier trouvé (Audio, Image, Texte, Vidéo, Autre), sur le même chemin source. La boîte de dialogue propose :
    * *Vérifier puis diviser* : lance d'abord *Vérifier les types MIME*, pour un résultat exact, puis divise. Disponible lorsque le périphérique est disponible.
    * *Diviser sans vérifier* : utilise les types de fichiers déjà enregistrés dans le catalogue.

## Développement
* Voir le backlog du [développement des Périphériques](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=devices).
