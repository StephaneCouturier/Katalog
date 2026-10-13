---
version: "3.0"
---
# Explorer
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Résumé
Cette page décrit la page **Explorer**, qui montre le contenu d'**un catalogue** sous forme d'arborescence de répertoires et de liste de fichiers, **même lorsque son périphérique n'est pas connecté** :
* l'[arborescence des répertoires](#directory-tree) du catalogue, à gauche,
* la [liste des fichiers](#file-list) du dossier sélectionné, à droite, avec la taille de chaque sous-dossier,
* le [menu contextuel](#file-context-menu) d'un fichier ou d'un dossier,
* l'[étiquetage d'un répertoire](#tag-folder) de l'arborescence.

![Page Explorer : arborescence des répertoires à gauche, liste des fichiers du dossier sélectionné à droite](/img/explore_01_overview.png)

## Ouvrir un catalogue {#opening}
Un catalogue s'ouvre dans Explorer avec *Explorer*, dans le menu contextuel d'un catalogue :
* sur la page [Sélection](Selection#context-menu) : le catalogue est également sélectionné,
* sur la page [Périphériques](Devices),
* depuis un résultat de recherche, avec *Explorer le dossier* (voir [Résultats de recherche](SearchResults#file-context-menu)) : Explorer s'ouvre directement sur le dossier du fichier.

Tant que la page Explorer est ouverte, **cliquer sur un catalogue** dans la page [Sélection](Selection) l'ouvre aussi dans Explorer.

En mode *Mémoire*, les fichiers d'un catalogue sont d'abord chargés en mémoire. Pour un grand catalogue, cela peut prendre un moment : la progression s'affiche dans le panneau d'activité en bas de la fenêtre, et *Stop* annule le chargement (Explorer continue alors d'afficher le catalogue précédent). Ouvrir un autre catalogue pendant un chargement le remplace. Un catalogue déjà chargé s'ouvre immédiatement.

*Explorer* dans le panneau latéral affiche à nouveau la page avec le dernier catalogue exploré depuis le démarrage de Katalog.<br/>
*Fermer* (ou `Échap`) ferme la page. Lorsqu'elle a été ouverte depuis un résultat de recherche, elle revient aux mêmes résultats.

## Arborescence des répertoires {#directory-tree}
Le panneau de gauche indique le nombre de répertoires du catalogue et sa **taille totale**, puis l'arborescence de ses répertoires.<br/>
Faites glisser la ligne entre l'arborescence et la liste des fichiers pour élargir ou réduire l'arborescence ; la largeur est conservée pour la fois suivante, et partagée avec Katalog 2.

À l'ouverture d'un catalogue, l'arborescence montre le dossier du catalogue et les deux premiers niveaux de répertoires en dessous ; les niveaux plus profonds sont d'abord repliés.<br/>
Un répertoire qui contient des sous-répertoires a une flèche : cliquez dessus pour replier ou déplier cette branche.<br/>
Cliquez sur un répertoire pour afficher son contenu dans la liste des fichiers.

Quatre boutons au-dessus de l'arborescence changent la part de l'arborescence affichée à la fois :

| Bouton | Effet |
|--------|-------|
| *Réduire d'un niveau* | Replie le niveau le plus profond actuellement affiché. |
| *Développer d'un niveau* | Déplie un niveau supplémentaire. |
| *Tout réduire* | Replie tout jusqu'au dossier du catalogue. |
| *Tout développer* | Déplie tous les répertoires du catalogue. |

Un bouton est grisé lorsqu'il n'y a plus rien à replier ou à déplier.<br/>
Ouvrir un sous-dossier depuis la liste des fichiers déplie sa branche : le répertoire sélectionné est ainsi toujours visible dans l'arborescence.<br/>
Les branches repliées et dépliées ne sont pas conservées : l'arborescence repart des deux premiers niveaux à chaque ouverture d'un catalogue.

### Étiqueter ce dossier {#tag-folder}
**Faites un clic droit** sur un répertoire de l'arborescence (ou un appui long) pour ouvrir son menu, qui contient une entrée : *Étiqueter ce dossier*.<br/>
Elle ouvre la page [Étiquettes](Tags) par-dessus toute la page Explorer, avec le chemin de ce répertoire déjà rempli ; choisissez l'étiquette et confirmez-la là. Le répertoire sélectionné dans l'arborescence et la liste des fichiers ne changent pas.<br/>
*Fermer* (ou `Échap`) sur Étiquettes revient ensuite à la même page Explorer et au même répertoire.

## Liste des fichiers {#file-list}
Le haut de la liste des fichiers indique le chemin du dossier sélectionné, ainsi que les options d'affichage :

| Option | Effet |
|--------|-------|
| *Afficher les dossiers* | Liste les sous-dossiers avec les fichiers. |
| *et tous les sous-dossiers* | Liste le contenu de tous les sous-dossiers, à n'importe quelle profondeur. Disponible avec *Afficher les dossiers*. |
| *Trier d'abord les dossiers* | Trie la liste avec les dossiers en premier, puis les fichiers. Disponible avec *Afficher les dossiers*. |

En dessous, *Nombre de Fichiers* et *Taille Totale* portent sur les fichiers situés **directement** dans le dossier sélectionné, sans ses sous-dossiers.

### Taille des dossiers
Chaque dossier listé indique, dans la colonne *Taille*, la taille totale de **tout ce qu'il contient**, à n'importe quelle profondeur. Un dossier sans fichier affiche une taille de 0.<br/>
Les plus gros dossiers peuvent ainsi être repérés sans connecter le périphérique.

Un même dossier peut afficher deux chiffres différents : sa taille dans la liste de son dossier parent (tout ce qu'il contient), et sa *Taille Totale* une fois sélectionné (uniquement les fichiers qu'il contient directement).<br/>
Avec *et tous les sous-dossiers*, un dossier est aussi compté dans la taille de chaque dossier au-dessus de lui : les tailles de la liste ne s'additionnent pas pour donner le total.

### Colonnes
| Colonne | Contenu |
|---------|---------|
| *Nom* | Nom du fichier ou du dossier, avec une icône pour son type. |
| *Taille* | Taille du fichier, ou taille totale d'un dossier. |
| *Date* | Date de dernière modification. |
| *Type de Fichier*, *Type MIME* | Type du fichier. |
| *Largeur*, *Hauteur*, *Durée* | Dimensions des images et des vidéos, durée des fichiers audio et vidéo. |
| *Artiste*, *Album*, *Titre* | Pour les fichiers audio. |
| *Somme de contrôle (SHA256)*, *Date de somme de contrôle* | Somme de contrôle du fichier, et date de son calcul. |

Les colonnes *Largeur* à *Titre* ne sont affichées que pour les catalogues qui incluent les métadonnées, et les colonnes de somme de contrôle ne sont remplies que pour les catalogues créés avec les sommes de contrôle (voir [Créer](Create)).

### Utiliser la liste
* **Cliquez** sur un fichier pour l'ouvrir avec son application par défaut. **Cliquez** sur un dossier pour l'ouvrir dans Explorer.
* **Faites un clic droit** sur une ligne (ou un appui long) pour ouvrir son [menu contextuel](#file-context-menu).
* **Cliquez sur le titre d'une colonne** pour trier selon cette colonne ; cliquez à nouveau pour inverser l'ordre. Le tri est conservé pour les visites suivantes.
* **Faites glisser** la ligne entre deux titres de colonnes pour changer la largeur d'une colonne ; **double-cliquez** dessus pour ajuster la colonne à son contenu. La largeur de *Nom* est conservée.
* Le clavier fonctionne comme dans les [résultats de recherche](SearchResults#keyboard) : flèches, `Page préc.` `Page suiv.`, `Début` `Fin`, `Entrée` pour ouvrir, `Menu` ou `Maj+F10` pour le menu contextuel, saisie de lettres pour aller à un nom.

Lorsque le périphérique du catalogue n'est pas disponible (déconnecté, ou son chemin a changé), le catalogue peut toujours être parcouru, mais l'ouverture d'un fichier, ou d'un dossier dans le gestionnaire de fichiers, est refusée et un court message apparaît en bas de la fenêtre : *Le périphérique n'est pas actif. Il est peut-être déconnecté, ou son chemin a peut-être changé.*

Lorsque le périphérique est disponible mais que le fichier ou le dossier n'existe plus (par exemple si le catalogue doit être mis à jour), l'ouverture est également refusée et le message est *Fichier introuvable ou illisible* ou *Dossier introuvable.*

## Menu contextuel {#file-context-menu}

![Menu contextuel d'un fichier dans Explorer](/img/explore_02_context_menu.png)

Un clic droit sur une ligne ouvre un menu adapté au type de la ligne.

### Pour un fichier
| Élément | Action |
|---------|--------|
| *Ouvrir le fichier* | Ouvre le fichier avec son application par défaut. |
| *Ouvrir le dossier* | Ouvre le dossier du fichier dans le gestionnaire de fichiers. |
| *Afficher les métadonnées étendues (JSON)* | Affiche toutes les métadonnées enregistrées pour le fichier, sous forme de tableau, avec *Copier JSON*. Affiché lorsque le catalogue inclut les métadonnées étendues. |
| *Copier le nom de fichier sans extension* | Copie `report_2025` depuis `/home/username/Documents/report_2025.pdf`. |
| *Copier le nom de fichier avec extension* | Copie `report_2025.pdf`. |
| *Copier le chemin du répertoire* | Copie `/home/username/Documents`. |
| *Copier le chemin absolu du fichier* | Copie `/home/username/Documents/report_2025.pdf`. |
| *Copier la somme de contrôle* | Copie la somme de contrôle enregistrée. Affiché lorsque le fichier en a une. |
| *Calculer la somme de contrôle (SHA-256)* | Calcule la somme de contrôle du fichier et l'enregistre. Affiché lorsque le fichier n'a pas encore de somme de contrôle. |
| *Vérifier la somme de contrôle (SHA-256)* | Calcule à nouveau la somme de contrôle et la compare à celle enregistrée. Si elles diffèrent, les deux valeurs sont affichées, avec *Mettre à jour la somme de contrôle* pour enregistrer la nouvelle. |
| *Déplacer le fichier vers la corbeille* | Déplace le fichier dans la corbeille du système. |
| *Supprimer le fichier* | Supprime le fichier **définitivement**, après confirmation. Disponible uniquement lorsque *Autoriser les actions qui suppriment des fichiers (sans récupération)* est activé dans les [Paramètres](Settings#behavior). |

### Pour un dossier
| Élément | Action |
|---------|--------|
| *Ouvrir le dossier* | Ouvre le dossier dans le gestionnaire de fichiers. |
| *Copier le nom du dossier* | Copie `Documents` depuis `/home/username/Documents`. |
| *Copier le chemin du répertoire* | Copie `/home/username/Documents`. |
| *Déplacer le dossier vers la corbeille* | Déplace le dossier et son contenu dans la corbeille du système. |

## Développement
* Voir le backlog du [développement d'Explorer](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=explore).
