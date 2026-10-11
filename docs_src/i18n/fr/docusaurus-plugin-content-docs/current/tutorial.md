---
sidebar_position: 1
version: "3.0"
---

# Tutoriel
![3.0](https://img.shields.io/badge/Version-3.0-blue)

Découvrons **Katalog en moins de 5 minutes** : créer une collection, cataloguer un dossier et y faire une recherche.

## Télécharger et lancer
Téléchargez Katalog pour Linux, Windows ou macOS : voir [Support multi-plateformes](Overview#multi-platform-support) pour les paquets disponibles et la configuration requise.

## Premier démarrage {#first-start}
Au premier démarrage, un message de bienvenue indique la langue et le thème appliqués (ils peuvent être changés plus tard dans les [Paramètres](Settings#application)), et propose :
* *Créer nouveau...* : crée une nouvelle **collection**, l'endroit où Katalog conserve toutes ses données. Une boîte de dialogue propose où l'enregistrer, sous forme de fichier de base de données (`.db`). Un dossier de votre répertoire personnel est un bon emplacement.
* *Ouvrir existant...* : ouvre une base de données de collection (`.db`) créée auparavant, par exemple avec Katalog 2.

Il est possible de fermer les boîtes de dialogue sans choisir : Katalog démarre alors sans collection, et une collection est ouverte ou créée plus tard depuis le panneau latéral (voir [Interface](Interface)).

![Premier démarrage : message de bienvenue](/img/tutorial_01_welcome.png)

Après *Créer nouveau...*, un message liste les trois étapes pour créer un premier catalogue. *OK* ouvre la page [Créer](Create).

## Créer un catalogue {#create}
Un **catalogue** est l'index des fichiers d'un dossier.
1. *Chemin source* : saisissez le dossier à cataloguer, ou choisissez-le avec *Sélectionner le chemin*. Par exemple, votre dossier *Documents*.
1. *Stockage* : gardez *Disque local*, le périphérique de stockage que Katalog a créé pour votre ordinateur.
1. *Nom du catalogue* : saisissez un nom, ou utilisez *Générer le nom à partir du chemin*.
1. Cliquez sur *Créer* en haut de la page.

![Page Créer avec un dossier sélectionné](/img/tutorial_02_create.png)

La progression est affichée en bas de la fenêtre. À la fin, un message confirme que le catalogue a été créé, et il apparaît dans l'arborescence des périphériques de la page [Sélection](Selection), sous *Disque local*.

## Rechercher des fichiers {#search}
1. Sur la page [Sélection](Selection), faites un clic droit sur le nouveau catalogue et choisissez *Chercher* : le catalogue est sélectionné et la page [Chercher](Search) s'ouvre à côté.
1. Saisissez un texte dans le champ du nom de fichier, par exemple `report`.
1. Cliquez sur *Chercher* (ou appuyez sur `Entrée`).

La page [Résultats](SearchResults) s'ouvre à droite, avec les fichiers trouvés. Cliquez sur un fichier pour l'ouvrir, ou faites un clic droit dessus pour plus d'actions.

![Recherche et résultats](/img/interface_01_overview.png)

Le catalogue reste consultable lorsque le dossier ou le disque n'est pas connecté : c'est la raison d'être de Katalog. Cataloguez vos disques externes de la même façon, chacun sous son propre périphérique de stockage (voir [Périphériques](Devices)), et recherchez dans tous à la fois en sélectionnant le *Groupe physique* sur la page Sélection.

## Étapes suivantes
* Utilisez d'autres [critères de recherche](Search) : taille, dates, types de fichiers, doublons, différences.
* Parcourez un catalogue dans [Explorer](Explore).
* Organisez vos disques dans [Périphériques](Devices).
* Sauvegardez un dossier avec [Sauvegarde](BackUp).
* Toutes les parties de la fenêtre sont décrites dans [Interface](Interface).
