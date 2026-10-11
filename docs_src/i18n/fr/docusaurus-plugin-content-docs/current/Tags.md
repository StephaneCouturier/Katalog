---
version: "3.0"
---
# Étiquettes
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Résumé
Cette page décrit la page **Étiquettes**, où des **étiquettes sont attribuées à des dossiers** : par exemple *Vacances* sur les dossiers de photos de vacances, où qu'ils soient.<br/>
Une étiquette peut ensuite être utilisée dans la [Recherche](Search#tags-criteria) pour ne trouver que les fichiers des dossiers portant cette étiquette.

![Page Étiquettes : ajouter une étiquette, et la liste des dossiers et étiquettes actuels](/img/tags_01_overview.png)

Une étiquette est attribuée à un **chemin de dossier**, pas à un catalogue ni à un périphérique : elle s'applique à chaque catalogue qui contient ce chemin.<br/>
Un dossier peut avoir plusieurs étiquettes, et une étiquette peut être sur plusieurs dossiers, mais un dossier n'a chaque étiquette qu'une seule fois.

## Ajouter une étiquette {#add}
1. *Dossier* : saisissez le chemin du dossier, ou choisissez-le avec *Sélectionner un dossier*. Il est déjà rempli lorsque Étiquettes a été ouvert avec *Étiqueter ce dossier* dans [Explorer](Explore#tag-folder).
1. *Étiquette* : choisissez une étiquette existante dans la liste, ou saisissez un nouveau nom.
1. Cliquez sur *Étiqueter le dossier*.

Lorsque le dossier ou le nom de l'étiquette manque, ou lorsque le dossier a déjà cette étiquette, un message en haut de la page l'indique.<br/>
La nouvelle association apparaît dans la liste en dessous.

## Dossiers et étiquettes actuels {#list}
La liste montre chaque dossier avec son étiquette.
* La liste au-dessus de la colonne *Étiquette* filtre sur une étiquette ; *Tous* les affiche toutes.
* Le bouton en fin de ligne (*Supprimer cette étiquette*) supprime cette association. Le dossier et ses fichiers ne sont pas touchés.

## Développement
Quelques idées de développement pour cet écran :
* Étiqueter des périphériques.
* Étiqueter des dossiers dans des catalogues précis.
* Pour en savoir plus, voir le backlog du [développement des Étiquettes](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=tag).
