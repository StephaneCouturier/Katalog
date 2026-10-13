---
version: "3.0"
---
# Résultats de recherche
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Résumé
Cette page décrit la page **Résultats**, qui s'ouvre à droite de la page [Chercher](Search) lorsqu'une recherche est lancée :
* le [résumé](#summary-line) de ce qui a été trouvé,
* la [liste des résultats](#results-list), triée et parcourue avec la souris ou le clavier,
* les [filtres](#filters) qui réduisent la liste sans relancer la recherche,
* le [menu contextuel](#file-context-menu) d'un fichier,
* les [actions sur tous les résultats](#batch-process) : export, sommes de contrôle, métadonnées, corbeille et suppression.

![Page Résultats : ligne de résumé, liste des fichiers trouvés, filtres en bas](/img/searchresults_01_overview.png)

## Ligne de résumé {#summary-line}
Le haut de la page indique :
* le périphérique parcouru (par exemple *Groupe physique / Photo Drive*),
* le nombre de résultats : fichiers, dossiers, doublons ou différences trouvés,
* la taille totale des fichiers trouvés, leur plus petite et leur plus grande taille, et l'intervalle de leurs dates.

Lorsque le périphérique d'un fichier n'est pas disponible (déconnecté, ou son chemin a changé), l'ouverture du fichier ou de son dossier est refusée et un court message apparaît en bas de la fenêtre : *Le périphérique n'est pas actif. Il est peut-être déconnecté, ou son chemin a peut-être changé.*

Lorsque le périphérique est disponible mais que le fichier ou le dossier n'existe plus (par exemple si le catalogue doit être mis à jour), l'ouverture est également refusée et le message est *Fichier introuvable ou illisible* ou *Dossier introuvable.*

## Liste des résultats {#results-list}

### Colonnes
| Colonne | Contenu |
|---------|---------|
| *Nom* | Nom du fichier, avec une icône pour son type. |
| *Taille* | Taille du fichier. |
| *Date* | Date de dernière modification. |
| *Répertoire* | Dossier du fichier. |
| *Nom du catalogue* | Catalogue auquel appartient le fichier. |
| *Type de Fichier*, *Type MIME* | Type du fichier. |
| *Largeur*, *Hauteur* | Dimensions, pour les images et les vidéos. |
| *Durée* | Durée, pour les fichiers audio et vidéo. |
| *Artiste*, *Album*, *Titre* | Pour les fichiers audio. |
| *Somme de contrôle (SHA256)*, *Date de somme de contrôle* | Somme de contrôle du fichier, et date de son calcul. |

Les colonnes de métadonnées et de somme de contrôle ne sont remplies que pour les catalogues créés avec ces options (voir [Créer](Create)).<br/>
Lorsque la recherche liste **uniquement des dossiers** (voir [Critères de dossier](Search#folder-criteria)), les colonnes *Nom*, *Taille* et *Date* ne sont pas affichées, et chaque ligne est un dossier.

### Utiliser la liste
* **Cliquez** sur une ligne pour ouvrir le fichier avec son application par défaut.
* **Faites un clic droit** sur une ligne (ou un appui long) pour ouvrir son [menu contextuel](#file-context-menu).
* **Cliquez sur le titre d'une colonne** pour trier selon cette colonne ; cliquez à nouveau pour inverser l'ordre. Le tri est conservé pour les recherches suivantes.
* **Faites glisser** la ligne entre deux titres de colonnes pour changer la largeur d'une colonne ; **double-cliquez** dessus pour ajuster la colonne à son contenu. Les largeurs de *Nom*, *Répertoire* et *Nom du catalogue* sont conservées.

### Clavier {#keyboard}
Après avoir cliqué sur une ligne, la liste peut être utilisée au clavier :

| Touche | Action |
|--------|--------|
| `↑` `↓` | Ligne précédente ou suivante. |
| `Page préc.` `Page suiv.` | Une page vers le haut ou vers le bas. |
| `Début` `Fin` | Première ou dernière ligne. |
| `Entrée` | Ouvre le fichier, comme un clic. |
| `Menu` ou `Maj+F10` | Ouvre le menu contextuel de la ligne. |
| `←` `→` | Fait défiler la liste horizontalement. |
| Saisie de lettres | Va à la ligne suivante dont le nom commence par le texte saisi. |

## Filtres {#filters}

![Filtres en bas de la page Résultats : filtre rapide et liste des catalogues](/img/searchresults_02_filters.png)

La barre en bas de la page réduit la liste **sans relancer la recherche** :
* *Rechercher…* : n'affiche que les fichiers dont le **nom** contient le texte saisi, au fur et à mesure de la saisie, sans tenir compte de la casse.
* *Catalogue avec résultats* : liste les catalogues où des résultats ont été trouvés. Cochez un ou plusieurs catalogues pour n'afficher que leurs résultats. *Tous* coche tous les catalogues, *Aucun* efface le choix (tous les résultats sont affichés). Le bouton indique le catalogue choisi, ou le nombre de catalogues choisis.

Les deux filtres s'appliquent ensemble, et tous deux sont effacés à la fin d'une nouvelle recherche.

## Menu contextuel {#file-context-menu}

![Menu contextuel d'un fichier dans les résultats](/img/searchresults_03_context_menu.png)

Un clic droit sur une ligne ouvre un menu qui commence par le nom du fichier :

| Élément | Action |
|---------|--------|
| *Ouvrir le fichier* | Ouvre le fichier avec son application par défaut. |
| *Ouvrir le dossier* | Ouvre le dossier du fichier dans le gestionnaire de fichiers. |
| *Explorer le dossier* | Ouvre le dossier dans la page [Explorer](Explore), à l'intérieur de son catalogue. *Fermer* (ou `Échap`) sur Explorer revient aux mêmes résultats. Non disponible pour une recherche dans un lecteur connecté. |
| *Afficher les métadonnées étendues (JSON)* | Affiche toutes les métadonnées enregistrées pour le fichier, sous forme de tableau, avec *Copier JSON*. |
| *Copier le nom de fichier sans extension* | Copie `report_2025` depuis `/home/username/Documents/report_2025.pdf`. |
| *Copier le nom de fichier avec extension* | Copie `report_2025.pdf`. |
| *Copier le chemin du répertoire* | Copie `/home/username/Documents`. |
| *Copier le chemin absolu du fichier* | Copie `/home/username/Documents/report_2025.pdf`. |
| *Calculer la somme de contrôle (SHA-256)* | Calcule la somme de contrôle du fichier et l'enregistre. Affiché lorsque le fichier n'a pas encore de somme de contrôle. |
| *Copier la somme de contrôle* | Copie la somme de contrôle enregistrée. Affiché lorsque le fichier en a une. |
| *Vérifier la somme de contrôle (SHA-256)* | Calcule à nouveau la somme de contrôle et la compare à celle enregistrée. Si elles diffèrent, les deux valeurs sont affichées, avec *Mettre à jour la somme de contrôle* pour enregistrer la nouvelle. |
| *Déplacer dans la corbeille* | Déplace le fichier dans la corbeille du système. |
| *Supprimer le fichier* | Supprime le fichier **définitivement**, après confirmation. Disponible uniquement lorsque *Autoriser les actions qui suppriment des fichiers (sans récupération)* est activé dans les [Paramètres](Settings#behavior). |

Lorsque les résultats sont des dossiers, les éléments portant sur un nom de fichier sont affichés mais désactivés.

## Actions sur tous les résultats {#batch-process}
La liste à côté de *Exporter* permet de choisir une action, et le bouton voisin (*Exécuter le processus sur tous les résultats*) l'applique à **tous les résultats** :

| Action | Description |
|--------|-------------|
| *Exporter en CSV* | Enregistre les résultats dans un fichier CSV dans le dossier *Téléchargements* ; son chemin est affiché à la fin. |
| *Exporter vers le catalogue* | Enregistre les résultats comme nouveau catalogue, sous un périphérique virtuel *Résultats de recherche* (voir [Périphériques](Devices)). |
| *Vérifier les sommes de contrôle* | Calcule les sommes de contrôle de tous les résultats et les compare à celles enregistrées, après confirmation. Un rapport donne le nombre de fichiers concordants, non concordants, nouvellement calculés et en erreur. |
| *Extraire les métadonnées* | Lit les métadonnées de tous les résultats et les enregistre, après confirmation. Un rapport donne le nombre de fichiers mis à jour, ignorés et en erreur. |
| *Déplacer dans la corbeille* | Déplace tous les résultats dans la corbeille, après confirmation. |
| *Supprimer* | Supprime tous les résultats **définitivement**, après confirmation. Disponible uniquement lorsque *Autoriser les actions qui suppriment des fichiers (sans récupération)* est activé dans les [Paramètres](Settings#behavior). |

## Développement
* Voir le backlog du [développement de la Recherche](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=search).
