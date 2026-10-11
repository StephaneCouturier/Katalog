---
version: "3.0"
---
# Modifier un périphérique
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Résumé
Cette page décrit la page **Modifier le périphérique**, où la définition d'un périphérique est modifiée :
* les [champs communs à tous les périphériques](#device) : nom, périphérique parent, commentaire,
* le [chemin source](#location) d'un catalogue ou d'un stockage, et [ce qui se passe quand il change](#path-change),
* les [options de contenu](#content-options) d'un catalogue,
* les [détails du stockage](#storage-details) et l'[image](#picture) d'un périphérique de stockage.

Les types de périphériques sont décrits sur la page [Périphériques](Devices#model).

![Page Modifier le périphérique pour un catalogue](/img/devicesedit_01_catalog.png)

## Ouverture et fermeture {#opening}
La page s'ouvre :
* avec *Modifier* dans le menu contextuel d'un périphérique, sur la page [Sélection](Selection#context-menu) ou [Périphériques](Devices#context-menu),
* après la création d'un périphérique avec *Insérer un groupe virtuel*, *Ajouter un Stockage*, *Ajouter un périphérique virtuel* ou *Ajouter un périphérique de stockage* sur la page [Périphériques](Devices).

| Action | Description |
|--------|-------------|
| *Enregistrer* | Enregistre les modifications, après les confirmations décrites ci-dessous, et ferme la page. |
| *Supprimer* | Supprime le périphérique de la collection, après confirmation. Un périphérique qui a encore des périphériques en dessous de lui ne peut pas être supprimé. |
| *Annuler* | Ferme la page sans enregistrer (également `Échap`). |

Lorsque la page a été ouverte depuis Périphériques, elle revient à Périphériques.

## Périphérique {#device}
Pour tous les périphériques :

| Champ | Description |
|-------|-------------|
| *Type* | Catalogue, Stockage ou Virtuel. Affiché uniquement, il ne peut pas être modifié. |
| *Nom* | Nom du périphérique, nécessaire pour enregistrer. |
| *Périphérique parent* | Périphérique auquel il appartient, dans l'arborescence. Un catalogue appartient à un stockage ; un stockage à un périphérique virtuel du Groupe physique. Le Groupe physique lui-même n'a pas de parent. |
| *Commentaire* | Note libre, affichée à côté du nom sur les cartes des périphériques. |

## Emplacement {#location}
Pour les catalogues et les périphériques de stockage :
* *Chemin source* : le dossier indexé par un catalogue, ou le dossier où un périphérique de stockage est monté (par exemple `/run/media/username/Photos` sous Linux, `E:\` sous Windows). *Sélectionner le chemin* ouvre une boîte de dialogue de choix de dossier.

Pour un nouveau périphérique de stockage, définir son chemin pour la première fois et enregistrer le met à jour immédiatement, ce qui remplit ses chiffres d'espace.

### Quand le chemin change {#path-change}
Lorsque le chemin d'un catalogue ou d'un stockage est modifié et enregistré, une boîte de dialogue montre l'ancien et le nouveau chemin et demande comment les index des catalogues doivent être mis à jour :

| Bouton | Effet |
|--------|-------|
| *Remplacer la racine du chemin* | Remplace l'ancien chemin par le nouveau dans tous les fichiers et dossiers indexés : ceux du catalogue, ou de tous les catalogues situés sous le stockage. Rien n'est lu sur le disque : c'est donc instantané et cela fonctionne avec le disque déconnecté. Le panneau d'activité indique combien de catalogues, de fichiers et de dossiers ont été modifiés. |
| *Ré-indexation complète* | Relit les fichiers depuis le nouveau chemin. Nécessite que le périphérique soit disponible. |
| *Ignorer* | Enregistre le nouveau chemin sans modifier les index. |

*Remplacer la racine du chemin* est le bon choix lorsque les fichiers n'ont pas changé et que seul l'endroit où le disque est monté a changé, par exemple `/mnt/drive1` devenant `/mnt/drive01`, ou `F:\` devenant `G:\`.

## Options de contenu {#content-options}
Pour les catalogues, les mêmes options que lors de la création du catalogue (voir [Créer](Create)) :

| Champ | Description |
|-------|-------------|
| *Type de fichier* | Tous, Audio, Image, Texte, Vidéo, Autre ou Aucun. |
| *Inclure les sous-dossiers* | Indexe aussi les fichiers des sous-dossiers, ou uniquement ceux situés directement dans le chemin source. |
| *Inclure les fichiers cachés* | Aucun ou Tous. |
| *Inclure les métadonnées* | Aucun, Média basique, Média étendu ou Complet étendu. |
| *Inclure la somme de contrôle* | Aucun ou SHA-256. |
| *Exclure les dossiers ou fichiers* | Chemins ou noms qui ne sont pas indexés. Saisissez un chemin ou un nom (ou utilisez *Sélectionner le chemin*) et cliquez sur *Ajouter* ; *Supprimer* en retire un de la liste. |

Lorsqu'une option ou le chemin source change, l'enregistrement liste les modifications et demande une confirmation, car le catalogue ne correspond plus à ce qui a été indexé. Une seconde question propose ensuite de mettre à jour le contenu du catalogue avec les nouveaux critères maintenant (*Oui*), ou plus tard (*Non*).

## Détails du stockage {#storage-details}
Pour les périphériques de stockage :

| Champ | Description |
|-------|-------------|
| *ID de stockage* | Le numéro du stockage, par exemple le numéro écrit sur le disque physique. Chaque stockage devrait avoir le sien : lorsqu'un autre stockage utilise déjà ce numéro, la page affiche *Un autre stockage utilise déjà cet ID.* Le numéro est tout de même enregistré, et peut être corrigé. |
| *Type*, *Étiquette*, *Système de fichiers* | Nature du périphérique, nom du disque, et son système de fichiers (ext4, NTFS, exFAT…). |
| *Espace total (octets)*, *Espace libre (octets)* | Espace du disque, avec la valeur dans une unité lisible à côté. |
| *Actualiser depuis le disque* | Lit *Type*, *Étiquette*, *Système de fichiers* et les valeurs d'espace depuis le disque. Nécessite que le disque soit disponible, et que son chemin ait d'abord été enregistré. |
| *Marque*, *Modèle*, *Numéro de série*, *Date de compilation* | Détails du disque, pour référence. |
| *Commentaire 1*, *Commentaire 2*, *Commentaire 3* | Notes libres. |

### Image {#picture}

![Détails du stockage avec une image du disque](/img/devicesedit_02_storage.png)

Une image aide à reconnaître un disque. Pour en ajouter une :
1. Placez l'image dans le **dossier d'images** de la collection (voir [Paramètres](Settings#images-folder)).
2. Choisissez-la dans la liste *Image*. Le bouton d'actualisation à côté de la liste (*Recharger les images*) relit le dossier après l'ajout d'un fichier.
3. *Enregistrer*.

L'image choisie est affichée sous la liste.<br/>
Lorsqu'aucune image n'est choisie, Katalog affiche le fichier nommé d'après l'**ID de stockage**, s'il existe dans le dossier d'images : par exemple `12.jpg` pour l'ID de stockage 12. Nommer une image d'après le numéro écrit sur le disque suffit pour qu'elle soit affichée, sans la choisir dans la liste. Le même nom fonctionne dans Katalog 2.

## Développement
* Voir le backlog du [développement des Périphériques](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=devices).
