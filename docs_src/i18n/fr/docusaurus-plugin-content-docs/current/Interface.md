---
version: "3.0"
---

# Interface
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Résumé
Cette page décrit les éléments de la fenêtre de Katalog communs à tous les écrans : le panneau latéral, l'ouverture des pages côte à côte, les raccourcis clavier, le panneau d'activité, les options d'apparence et la page *À propos*.

![Fenêtre principale : panneau épinglé à gauche, pages Recherche et Résultats côte à côte](/img/interface_01_overview.png)

La fenêtre comporte trois zones :
* le **panneau latéral** à gauche, pour ouvrir une collection et aller à n'importe quel écran,
* les **pages**, affichées côte à côte, en commençant par la [Sélection](Selection),
* le **panneau d'activité** en bas, pendant la création d'un catalogue, la mise à jour d'un périphérique ou une recherche.

## Pages {#pages}
La page [Sélection](Selection) est le point de départ : elle liste les périphériques de la collection et occupe toujours la première colonne.

Ouvrir un écran depuis le panneau latéral (par exemple *Chercher*, *Périphériques* ou *Explorer*) affiche sa page à côté de la Sélection. Un seul écran est ouvert à la fois : en ouvrir un autre le remplace.<br/>
Lancer une recherche ouvre la page *Résultats* à droite de la page *Chercher*. Depuis un résultat, *Explorer le dossier* ouvre la page [Explorer](Explore) sur ce dossier.

Chaque page a un bouton *Fermer* dans sa barre d'outils, qui revient en arrière d'une étape. *Paramètres* et *À propos* s'ouvrent par-dessus les autres pages et se ferment aussi avec leur bouton *Fermer*.

### Afficher ou masquer la Sélection {#show-hide-selection}
Quand un écran est ouvert à côté de la Sélection, la Sélection peut être masquée pour laisser plus de place à cet écran.<br/>
Utilisez le bouton en bas du panneau latéral : il indique *Sélection affichée* ou *Sélection masquée*. Il n'est disponible que lorsqu'un écran est ouvert à côté de la Sélection. Le choix est conservé pour le prochain démarrage.

### Taille et position de la fenêtre
La taille et la position de la fenêtre, ainsi que son état agrandi, sont restaurés au démarrage suivant.

## Panneau latéral {#drawer}

<div className="screenshot-side">

![Panneau épinglé : la collection ouverte en haut, puis les menus et les écrans](/img/interface_02_drawer.png)

</div>

Le haut du panneau latéral montre la **collection ouverte** : une icône pour son mode (Mémoire, Fichier ou Hébergé) et son nom :
* en mode *Mémoire*, le nom du dossier de la collection,
* en mode *Fichier*, le nom du fichier de base de données,
* en mode *Hébergé*, le nom du serveur, un `/`, puis le nom de la base de données.

En dessous :

| Élément | Utilisation |
|---------|-------------|
| *Ouvrir...* | Ouvre une collection, ou rouvre une collection récente : voir [Ouvrir une collection](#open). |
| *Nouveau...* | Crée une nouvelle collection : *Base de données SQLite...* crée un nouveau fichier de base de données et l'ouvre. |
| *Sélection* … *Étiquettes* | Ouvrent l'écran : [Sélection](Selection), [Chercher](Search), [Périphériques](Devices), [Explorer](Explore), [Créer](Create), [Sauvegarde](BackUp), [Statistiques](Statistics), [Étiquettes](Tags). |
| *Paramètres* | Ouvre les [Paramètres](Settings). |
| *Documentation* | Ouvre cette documentation dans le navigateur web. |
| *À propos* | Ouvre la page [À propos](#about). |
| *Quitter* | Ferme Katalog. |

<div className="screenshot-side-end" />

### Ouvrir une collection {#open}

<div className="screenshot-side">

![Panneau latéral avec le menu Ouvrir... déplié : les trois façons d'ouvrir une collection, puis les collections récentes, et les deux boutons en bas](/img/interface_02_drawer_open.png)

</div>

*Ouvrir...* déplie un menu avec :
* *Base de données SQLite...* : ouvre un fichier de base de données (`.db`), en mode *Fichier*,
* *Dossier de collection...* : ouvre un dossier de collection, en mode *Mémoire*,
* *Base de données hébergée...* : ouvre une base de données sur un serveur, en mode *Hébergé*.

<div className="screenshot-side-gap" style={{height: '6rem'}} />

Sous *Collections récentes*, le menu liste ensuite les **10 collections ouvertes le plus récemment**, la plus récente en premier : cliquez sur l'une d'elles pour la rouvrir directement.

Passer à une autre collection ne redémarre pas Katalog : la collection est ouverte directement et tous les écrans sont actualisés.

<div className="screenshot-side-gap" style={{height: '9rem'}} />

### Panneau épinglé ou flottant
Le premier bouton en bas du panneau latéral indique *Panneau épinglé* ou *Panneau flottant* :
* **épinglé** : le panneau reste ouvert à gauche des pages,
* **flottant** : le panneau s'ouvre par-dessus les pages et se ferme après un choix.

Le choix est conservé pour le prochain démarrage.

Le second bouton indique *Sélection affichée* ou *Sélection masquée* : voir [Afficher ou masquer la Sélection](#show-hide-selection).

<div className="screenshot-side-end" />

## Raccourcis clavier {#keyboard-shortcuts}

| Touche | Action |
|--------|--------|
| `Échap` | Revient en arrière d'une étape : comme le bouton *Fermer* de la dernière page ouverte (ou *Annuler* sur la page de modification d'un périphérique). |
| `Ctrl+F` | Place le curseur dans le filtre des périphériques de la page [Sélection](Selection). |
| `Ctrl+Q` | Quitte Katalog sous Linux (`Cmd+Q` sous macOS). |

Fonctionnement de `Échap` :
* Une boîte de dialogue, un menu ou une liste déroulante ouverts se ferment d'abord ; la page en dessous reste ouverte.
* `Échap` ferme la page même lorsque le curseur est dans un champ de texte.
* Sur la page de modification d'un périphérique et sur la page [Créer](Create), `Échap` abandonne ce qui a été saisi, comme *Annuler* et *Fermer*. Il n'y a pas de confirmation.

La navigation au clavier dans les résultats de recherche est décrite sur la page [Résultats de recherche](SearchResults#keyboard).

## Panneau d'activité {#activity-panel}

![Panneau d'activité en bas : une mise à jour de périphérique en cours, avec la file d'attente montrant une mise à jour en attente](/img/interface_03_activity_panel.png)

Le panneau d'activité apparaît en bas des pages pendant la **création d'un catalogue**, la **mise à jour d'un périphérique** ou une **recherche**. C'est l'endroit unique pour suivre ces opérations, quelle que soit la page ouverte :
* le message d'état indique ce qui est en cours de traitement et où en est l'opération,
* *Stop* arrête l'opération en cours.

Quand une opération se termine (achevée, arrêtée ou en échec), le panneau reste visible quelques secondes avec son état final, puis se masque.

### File d'attente {#queue}
Katalog exécute une seule création de catalogue ou mise à jour de périphérique à la fois. Une création ou une mise à jour demandée pendant qu'une autre est en cours est **ajoutée à la file d'attente** au lieu d'être refusée :
* le panneau affiche *File d'attente* et le nombre d'opérations en attente,
* chaque opération en attente est listée (*Créer* ou *Actualiser*, avec le nom du périphérique) et peut être retirée avec *Supprimer*,
* *Effacer* vide la file d'attente,
* les opérations en attente s'exécutent l'une après l'autre, dans l'ordre où elles ont été demandées.

*Stop* termine seulement l'opération en cours : la suivante dans la file démarre alors. Un périphérique déjà en cours ou en attente n'est pas ajouté une seconde fois.

Une recherche ne peut pas démarrer pendant une création ou une mise à jour (le bouton *Chercher* est désactivé), et une création ou une mise à jour ne peut pas démarrer pendant une recherche.

## Apparence {#appearance}
L'aspect de Katalog se règle dans la section *Application* des [Paramètres](Settings#application) :
* **Thème** : couleurs du bureau ou couleurs propres à Katalog, suivant toujours le mode clair ou sombre du système.
* **Taille des icônes et du texte** : icônes plus grandes, et texte plus petit ou plus grand.
* **Langue** : la langue de l'interface.

Les boutons qui n'affichent qu'une icône montrent leur nom lorsque le pointeur de la souris reste dessus.

## À propos {#about}

![Page À propos avec l'action « Copier les informations de version et de système »](/img/interface_04_about.png)

La page *À propos* indique la version de Katalog, sa licence et son auteur.

*Copier les informations de version et de système* copie un court texte dans le presse-papiers, prêt à être collé dans un rapport de bug. Il contient :
* la version de Katalog et sa date de publication,
* les versions de Qt et de KDE Frameworks,
* le système d'exploitation, le noyau et l'architecture du processeur,
* la langue de l'interface,
* le mode et la version de la base de données,
* le serveur d'affichage et l'environnement de bureau.

Il ne contient **aucune information personnelle** : pas de nom d'utilisateur ni d'ordinateur, aucun chemin vers des fichiers ou des collections, aucun nom de collection, ni adresse de serveur ni mot de passe.

## Développement
* Voir le backlog du [développement de l'Interface](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=Interface).
