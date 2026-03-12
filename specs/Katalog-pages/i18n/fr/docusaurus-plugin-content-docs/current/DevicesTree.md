---
version: "2.10"
---
# Arborescence des périphériques
![2.10](https://img.shields.io/badge/Version-2.10-blue)

## Résumé
Cette page décrit toutes les fonctionnalités de la vue **Arborescence des périphériques** de l'écran [Périphériques](Devices).

L'arborescence des périphériques affiche la hiérarchie complète de tous les périphériques — le Groupe Physique avec ses périphériques de stockage et ses catalogues, et tous les Groupes Virtuels avec leurs catalogues attribués.

![Arborescence des périphériques montrant la hiérarchie complète des périphériques physiques et virtuels](/img/devices_tree_01.png)

## Options d'affichage

La barre supérieure contrôle quelles parties de l'arborescence sont affichées :

| Option | Description |
|--------|-------------|
| *Groupe Physique* | Affiche ou masque le Groupe Physique et tous ses périphériques |
| *Groupes Virtuels* | Affiche ou masque tous les Groupes Virtuels et leurs périphériques attribués |
| *Stockage* | Affiche ou masque les périphériques de stockage (masquer le stockage masque également les catalogues en dessous) |
| *Catalogues* | Affiche ou masque les périphériques de type catalogue |

Le bouton *Appliquer à la Sélection* enregistre les options d'affichage actuelles et les applique à l'arborescence affichée dans le panneau [Sélection](Selection), afin que les deux vues restent cohérentes.

## Boutons d'action

| Bouton | Description |
|--------|-------------|
| *Insérer un groupe virtuel* | Crée un nouveau Groupe Virtuel au niveau supérieur et ouvre le panneau de modification |
| *Ajouter un Virtuel* | Crée un nouveau périphérique virtuel sous le périphérique sélectionné et ouvre le panneau de modification |
| *Développer l'arborescence* | Développe tous les nœuds de l'arborescence |
| *Réduire l'arborescence* | Réduit tous les nœuds de l'arborescence |

## Menu contextuel {#tree-context-menu}

Un clic droit sur un périphérique de l'arborescence ouvre un menu contextuel dont les entrées dépendent du type de périphérique sélectionné.

### Périphériques de type Catalogue

![Menu contextuel d'un catalogue dans le Groupe Physique](/img/devices_tree_02_context_phy_virt.png)

| Action | Condition | Description |
|--------|-----------|-------------|
| *Mettre à jour* | Catalogue actif uniquement | Re-scanne le catalogue depuis son chemin source |
| *Explorer* | Toujours | Ouvre le catalogue dans l'écran [Explorer](Explore) |
| *Modifier* | Toujours | Ouvre le panneau de modification pour changer les paramètres du catalogue |
| *Ouvrir le dossier* | Chemin défini et non un export | Ouvre le dossier source du catalogue dans le gestionnaire de fichiers |
| *Vérifier les sommes de contrôle* | Toujours | Recalcule et compare les sommes de contrôle de tous les fichiers du catalogue |
| *Filelight* | Catalogue actif uniquement | Ouvre [Filelight](https://apps.kde.org/filelight/) dans le chemin source |
| *Désaffecter ce catalogue* | Catalogue dans un Groupe Virtuel | Retire le catalogue du groupe virtuel (le catalogue lui-même n'est pas supprimé) |
| *Supprimer ce catalogue* | Catalogues du groupe physique et exports | Supprime définitivement le catalogue de la collection |

### Périphériques de stockage

![Menu contextuel d'un périphérique de stockage dans l'arborescence](/img/devices_tree_03_context_phy_storage.png)

| Action | Condition | Description |
|--------|-----------|-------------|
| *Mettre à jour* | Toujours | Met à jour le périphérique de stockage et tous ses catalogues |
| *Modifier* | Toujours | Ouvre le panneau de modification pour changer les champs du périphérique de stockage |
| *Ouvrir le dossier* | Chemin défini | Ouvre le chemin du stockage dans le gestionnaire de fichiers |
| *Filelight* | Stockage actif uniquement | Ouvre [Filelight](https://apps.kde.org/filelight/) dans le chemin du stockage |
| *Désaffecter ce stockage* | Stockage dans un sous-groupe | Retire le stockage de son périphérique virtuel parent |
| *Supprimer ce stockage* | Toujours | Supprime définitivement le périphérique de stockage |

### Périphériques et groupes virtuels

Les Groupes Virtuels et les périphériques virtuels partagent le même menu contextuel, avec quelques variations :

![Menu contextuel d'un périphérique virtuel dans le Groupe Physique](/img/devices_tree_03_context_vir_virtual.png)
![Menu contextuel d'un périphérique virtuel dans un Groupe Virtuel avec attribution de catalogue](/img/devices_tree_03_context_vir_catalog.png)

| Action | Condition | Description |
|--------|-----------|-------------|
| *Mettre à jour* | Toujours | Met à jour tous les catalogues et périphériques de stockage sous ce périphérique virtuel |
| *Modifier* | Toujours | Ouvre le panneau de modification pour renommer le périphérique virtuel |
| *Ouvrir le dossier* | Chemin défini | Ouvre le chemin dans le gestionnaire de fichiers |
| *Ajouter un périphérique virtuel* | Toujours | Crée un nouveau périphérique virtuel sous ce périphérique |
| *Ajouter un périphérique de stockage* | Éléments du Groupe Physique uniquement | Crée un nouveau périphérique de stockage sous ce périphérique |
| *Attribuer le catalogue sélectionné* | Groupes Virtuels uniquement (un catalogue doit être sélectionné dans [Sélection](Selection)) | Attribue le catalogue actuellement sélectionné à ce périphérique virtuel |
| *Supprimer* | Toujours (sauf le Groupe Physique racine) | Supprime le périphérique virtuel (uniquement possible s'il n'a pas de sous-éléments ni de catalogues attribués) |

Attribution et désaffectation de catalogues dans les groupes virtuels :

![Attribution d'un catalogue à un périphérique virtuel](/img/devices_tree_03_context_vir_assign.png)
![Désaffectation d'un catalogue d'un périphérique virtuel](/img/devices_tree_03_context_vir_unassign.png)

- *Attribuer le catalogue sélectionné* : attribue le catalogue actuellement sélectionné dans le panneau [Sélection](Selection) au périphérique virtuel choisi.
- *Désaffecter ce catalogue* : disponible dans le menu contextuel du catalogue lorsque celui-ci est dans un Groupe Virtuel — retire l'attribution sans supprimer le catalogue.

## Développement
Quelques idées d'évolutions pour cet écran :
* Voir la liste de [développement Périphériques](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=devices).
