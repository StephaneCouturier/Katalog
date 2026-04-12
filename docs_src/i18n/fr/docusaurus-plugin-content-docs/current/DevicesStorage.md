---
version: "2.11"
---
# Périphériques : Stockage
![2.11](https://img.shields.io/badge/Version-2.11-blue)

## Résumé
Cette page décrit toutes les fonctionnalités de la vue **Liste de stockage** de l'écran [Périphériques](Devices).

Un périphérique de **Stockage** représente un lecteur physique, un disque ou tout autre support de stockage à partir duquel un ou plusieurs catalogues peuvent être créés.

Les données associées à un périphérique de stockage combinent trois types d'informations :
- *Physiques* : **espace libre**, **espace utilisé**, **espace total**, **étiquette**, **système de fichiers** — lus depuis le lecteur lorsqu'il est connecté.
- *Calculées* : **nombre total de fichiers** et **taille totale des fichiers** — dérivés automatiquement des catalogues associés à ce stockage.
- *Définies par l'utilisateur* : **chemin**, **type**, **marque**, **modèle**, **numéro de série**, **date de construction**, **commentaires**.

Les périphériques de stockage ne peuvent être placés qu'à l'intérieur du *Groupe Physique* et de ses sous-éléments.

![Liste de stockage affichant les périphériques avec l'utilisation d'espace et le nombre de catalogues associés](/img/devices_storage_01.png)

## Liste et sélection
La liste des périphériques de stockage peut être restreinte à l'aide du panneau **[Sélection](Selection)** à gauche.

Lorsque le chemin d'un périphérique de stockage pointe vers un emplacement connecté et monté, l'icône est affichée en couleur, indiquant que le stockage est **actif**.

## Boutons d'action

| Bouton | Activé lorsque | Description |
|--------|---------------|-------------|
| *Mettre à jour* | Un périphérique de stockage est sélectionné | Met à jour le périphérique de stockage sélectionné et tous ses catalogues associés |

:::note
Le bouton *Tous les actifs* n'est pas disponible dans la vue Liste de stockage — il n'est actif que dans la vue Liste des catalogues.
:::

## Menu contextuel {#storage-context-menu}

Un clic droit sur un périphérique de stockage ouvre un menu contextuel :

![Menu contextuel d'un périphérique de stockage affichant les actions disponibles](/img/devices_storage_02_context.png)

| Action | Condition | Description |
|--------|-----------|-------------|
| *Mettre à jour* | Toujours | Met à jour le périphérique de stockage sélectionné et tous les catalogues en dessous |
| *Modifier* | Toujours | Ouvre le [panneau de modification](#edit) pour changer les champs du périphérique de stockage |
| *Ouvrir le dossier* | Chemin défini | Ouvre le chemin du stockage dans le gestionnaire de fichiers |
| *Filelight* | Stockage actif uniquement | Ouvre [Filelight](https://apps.kde.org/filelight/) dans le chemin du stockage |
| *Désaffecter ce stockage* | Stockage dans un sous-groupe | Retire le stockage de son périphérique virtuel parent (le stockage lui-même n'est pas supprimé) |
| *Supprimer ce stockage* | Toujours | Supprime définitivement le périphérique de stockage (uniquement possible si aucun catalogue ne lui est associé) |

## Modifier {#edit}

Le panneau de modification permet de changer tous les champs du périphérique de stockage :

![Panneau de modification d'un périphérique de stockage affichant tous les champs configurables](/img/devices_storage_03_edit.png)

| Champ | Description |
|-------|-------------|
| *Nom du périphérique* | Le nom affiché du périphérique de stockage |
| *Type* | Le type de périphérique (ex. disque interne, disque externe, USB, NAS…) |
| *Étiquette* | L'étiquette du système de fichiers du lecteur |
| *Système de fichiers* | Le type de système de fichiers (ex. ext4, NTFS, exFAT…) |
| *Marque* | Le fabricant du lecteur |
| *Modèle* | Le nom du modèle du lecteur |
| *Numéro de série* | Le numéro de série du lecteur |
| *Date de construction* | La date de fabrication du lecteur |
| *Commentaire 1 / 2 / 3* | Champs de texte libre pour des notes |
| *Image* | Une image associée à ce stockage — voir [Image du périphérique](#device-picture) ci-dessous |
| *Périphérique parent* | Le périphérique virtuel ou groupe auquel ce stockage appartient |

## Image du périphérique

Il est possible d'associer une image à un périphérique de stockage pour l'identifier visuellement.

Pour l'utiliser :
1. Placer des fichiers image dans le **dossier d'images** (configurable dans [Paramètres](Settings#images-folder), par défaut `<dossier_collection>/images`).
2. Dans le panneau de modification du stockage, sélectionner l'image souhaitée dans la liste déroulante *Image* — elle liste tous les fichiers image trouvés dans le dossier d'images.
3. Enregistrer le stockage pour appliquer l'association.

Si aucune image n'est explicitement assignée, Katalog recherche un fichier nommé `<IDstockage>.jpg` dans le dossier d'images comme solution de repli.

![Périphérique de stockage avec une image associée affichée dans le panneau de modification](/img/devices_storage_04_picture.png)

## Développement
Quelques idées d'évolutions pour cet écran :
* Voir la liste de [développement Périphériques](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=devices).
