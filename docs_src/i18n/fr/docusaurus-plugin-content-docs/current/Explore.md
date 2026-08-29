---
version: "2.13"
---
# Explorer
![2.13](https://img.shields.io/badge/Version-2.13-blue)

## Résumé
Cette page décrit toutes les fonctionnalités de l'écran **Explorer** et comment les utiliser.

L'objectif de cet écran est d'**explorer le contenu d'un catalogue de fichiers** même lorsque le périphérique physique n'est pas connecté.

Cette vue charge les données après un clic droit sur un catalogue depuis le panneau **[Sélection](Selection)** ou la vue de liste **[Catalogues](DevicesCatalogs)**, en choisissant l'entrée de menu *Explorer*.

![Écran Explorer affichant l'arborescence des répertoires à gauche et la liste des fichiers à droite](/img/explore_01.png)

## Fonctionnalités

L'écran est divisé en deux panneaux :

- **Panneau gauche** — arborescence des répertoires du catalogue. Cliquer sur un répertoire affiche ses fichiers dans le panneau de droite.
- **Panneau droit** — liste des fichiers du répertoire sélectionné.

Cliquer sur un fichier dans le panneau de droite tentera de l'ouvrir avec l'application par défaut du système, si le périphérique est connecté. Cliquer sur un dossier navigue à l'intérieur de ce dossier.

Lorsque les dossiers sont affichés, chacun indique sa taille totale — les fichiers qu'il contient plus tout ce que contiennent les dossiers en dessous — afin de repérer les dossiers les plus volumineux sans connecter le périphérique. La taille totale du catalogue est affichée en haut, à côté du chemin du dossier.

### Arborescence des répertoires

Le panneau gauche s'ouvre sur la racine du catalogue et ses deux premiers niveaux de répertoires ; les niveaux plus profonds sont repliés au départ.

Un répertoire qui contient des sous-répertoires porte une petite flèche : cliquer dessus replie ou déplie cette branche. Les répertoires sans sous-répertoire n'ont pas de flèche, et toutes les lignes restent alignées.

Quatre boutons au-dessus de l'arborescence changent la quantité de hiérarchie affichée d'un seul coup :

| Bouton | Effet |
|--------|-------|
| *Réduire d'un niveau* | Replie le niveau le plus profond actuellement affiché |
| *Développer d'un niveau* | Déplie un niveau supplémentaire de l'arborescence |
| *Tout réduire* | Replie tout jusqu'à la racine du catalogue |
| *Tout développer* | Déplie tous les répertoires du catalogue |

Un bouton est grisé lorsqu'il n'a plus rien à replier ou à déplier.

Naviguer dans un dossier depuis le panneau de droite déplie la branche à laquelle il appartient, afin que le répertoire sélectionné soit toujours visible dans l'arborescence.

L'arborescence se rouvre toujours sur la racine du catalogue et ses deux premiers niveaux : l'état replié ou déplié n'est pas conservé d'une visite à l'autre.

### Options d'affichage

Trois options contrôlent ce qui apparaît dans la liste des fichiers :

- **Afficher les dossiers** — lorsqu'activé, les entrées de dossiers sont affichées avec les fichiers. L'activation de cette option active également les deux options ci-dessous.
- **Afficher les sous-dossiers** — lorsqu'activé, les fichiers de tous les sous-dossiers sont listés ensemble dans la liste de fichiers.
- **Trier les dossiers en premier** — bouton qui retrie la liste pour afficher d'abord les dossiers (alphabétiquement), puis les fichiers (alphabétiquement).

## Menu contextuel des répertoires (clic droit) {#directory-context-menu}

Un clic droit sur un répertoire dans le panneau gauche affiche :

![Menu contextuel du répertoire avec l'option d'étiqueter le dossier](/img/explore_02_context.png)

- *Étiqueter ce dossier* — ouvre l'écran [Étiquettes](Tags) avec ce dossier pré-rempli, pour lui attribuer une étiquette.

## Menu contextuel fichiers et dossiers (clic droit) {#file-context-menu}

Un clic droit sur une entrée dans le panneau droit affiche un menu contextuel qui s'adapte au type d'entrée sélectionnée.

![Menu contextuel des fichiers affichant les opérations disponibles : ouvrir, copier, somme de contrôle et supprimer](/img/explore_03_context.png)

### Pour les fichiers

| Action | Description |
|--------|-------------|
| *Ouvrir le fichier* | Ouvre le fichier avec l'application par défaut du système |
| *Ouvrir le dossier* | Ouvre le dossier parent du fichier dans le gestionnaire de fichiers |
| *Afficher les métadonnées étendues (JSON)* | Affiche les métadonnées détaillées (disponible uniquement si le catalogue a été indexé avec les métadonnées étendues) |
| *Copier le chemin du dossier* | Copie le chemin du dossier parent dans le presse-papiers |
| *Copier le chemin absolu du fichier* | Copie le chemin complet du fichier dans le presse-papiers |
| *Copier le nom du fichier avec extension* | Copie le nom du fichier (avec extension) dans le presse-papiers |
| *Copier le nom du fichier sans extension* | Copie le nom du fichier (sans extension) dans le presse-papiers |
| *Copier la somme de contrôle* | Copie la somme de contrôle stockée dans le presse-papiers (affiché uniquement si une somme de contrôle est stockée) |
| *Calculer la somme de contrôle (SHA-256)* | Calcule et enregistre la somme de contrôle SHA-256 du fichier (affiché uniquement si aucune somme de contrôle n'est stockée) |
| *Vérifier la somme de contrôle (SHA-256)* | Recalcule la somme de contrôle et la compare avec la valeur stockée (affiché uniquement si une somme de contrôle est stockée) |
| *Déplacer le fichier vers la corbeille* | Déplace le fichier vers la corbeille du système |
| *Supprimer le fichier* | Supprime définitivement le fichier |

### Pour les dossiers

| Action | Description |
|--------|-------------|
| *Ouvrir le dossier* | Ouvre le dossier dans le gestionnaire de fichiers du système |
| *Copier le chemin du dossier* | Copie le chemin du dossier dans le presse-papiers |
| *Copier le nom du dossier* | Copie le nom du dossier dans le presse-papiers |
| *Déplacer le dossier vers la corbeille* | Déplace le dossier vers la corbeille du système |

## Développement
Quelques idées d'évolutions pour cet écran :
* Voir la liste de [développement Explorer](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=explore).
