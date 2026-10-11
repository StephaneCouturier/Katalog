---
version: "3.0"
---
# Statistiques
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Résumé
Cette page décrit la page **Statistiques** : un graphique de l'évolution d'un périphérique dans le temps, pour le périphérique sélectionné sur la page [Sélection](Selection).

![Page Statistiques : graphique d'un périphérique de stockage, avec la taille totale des fichiers, l'espace utilisé et l'espace total](/img/statistics_01_overview.png)

## Origine des données {#data}
Chaque fois qu'un périphérique est mis à jour, et chaque fois qu'un **instantané** est enregistré (*Instantané* sur la page [Périphériques](Devices#page-actions)), Katalog conserve un relevé de ses chiffres : nombre de fichiers, taille totale des fichiers, et pour les périphériques de stockage, espace libre et total.<br/>
Le graphique trace ces relevés. Un périphérique qui n'a jamais été mis à jour, ni jamais fait partie d'un instantané, n'a pas de données : le graphique affiche *Aucune donnée*.

## Ce qui est affiché {#chart}
Le titre du graphique indique le nom du périphérique, la donnée affichée et son unité.

| Périphérique sélectionné | *Taille Totale des Fichiers* | *Nombre de Fichiers* |
|--------------------------|------------------------------|----------------------|
| Catalogue | Taille totale de ses fichiers. | Nombre de ses fichiers. |
| Stockage, Virtuel | Taille totale des fichiers de ses catalogues, plus l'espace **utilisé** et l'**espace total** du stockage. | Nombre de fichiers de ses catalogues. |

Les tailles sont affichées dans l'unité adaptée aux valeurs (KiB, MiB, GiB ou TiB).

## Options {#options}

| Option | Effet |
|--------|-------|
| *Source* | *tous les enregistrements*, *mises à jour uniquement* ou *instantanés uniquement*. Les instantanés donnent des points réguliers pour toute la collection ; les mises à jour ajoutent un point à chaque mise à jour du périphérique. |
| *Données* | *Taille Totale des Fichiers* ou *Nombre de Fichiers*. |
| *À partir du* | N'affiche que les relevés à partir de cette date. *Sélectionner une date* ouvre une boîte de dialogue avec des choix rapides (*Il y a 1 jour*, *Il y a 1 semaine*, *Il y a 1 mois*, *Il y a 1 an*) ; *Effacer* affiche à nouveau tous les relevés. |
| *Afficher chaque valeur* | Marque chaque relevé d'un point sur les courbes. |

Les options sont conservées pour la collection.

## Zoom {#zoom}
* **Cliquez et faites glisser** sur le graphique pour zoomer sur cette zone.
* **Faites un clic droit** ou **double-cliquez** pour revenir au graphique complet.

## Développement
* Voir le backlog du [développement des Statistiques](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=statistics).
