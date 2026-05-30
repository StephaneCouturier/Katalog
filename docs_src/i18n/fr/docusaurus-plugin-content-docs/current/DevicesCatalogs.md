---
version: "2.12"
---
# Périphériques : Catalogues
![2.12](https://img.shields.io/badge/Version-2.12-blue)

## Résumé
Cette page décrit toutes les fonctionnalités de la vue **Liste des catalogues** de l'écran [Périphériques](Devices).

Un *Catalogue* est un index de fichiers issus d'un répertoire donné, appelé **chemin** du catalogue.<br/>
La **Liste des catalogues** affiche tous les catalogues de la [Collection](Settings#collection), filtrés selon le panneau [Sélection](Selection).

![Liste des catalogues affichant les noms, chemins, nombre de fichiers et périphériques de stockage associés](/img/devices_catalogs_01.png)

## Liste et sélection
La liste des catalogues peut être restreinte à l'aide du panneau **[Sélection](Selection)** à gauche.

Lorsque le chemin source d'un catalogue pointe vers un emplacement connecté et monté, l'icône du catalogue est affichée en couleur (bleue), indiquant que le catalogue est **actif**.

## Boutons d'action

| Bouton | Activé lorsque | Description |
|--------|---------------|-------------|
| *Mettre à jour* | Un catalogue est sélectionné | Re-scanne le catalogue sélectionné depuis son chemin source, selon ses critères (type de fichier, fichiers cachés, etc.) |
| *Tous les actifs* | Toujours (vue Liste des catalogues uniquement) | Met à jour tous les catalogues affichés dont le chemin source est accessible |
| *Arrêter* | Une mise à jour est en cours | Annule l'opération de mise à jour en cours |
| *Vérifier les types MIME* | Un catalogue actif est sélectionné | Vérifie à nouveau les types de fichiers de tous les fichiers du catalogue à l'aide de la base MIME système |
| *Importer* | Toujours | Importe des catalogues depuis un fichier d'export VVV — voir [Importer](#import) ci-dessous |

:::note
Le bouton *Tous les actifs* n'est disponible que dans la vue **Liste des catalogues**. Il est désactivé lorsque la vue Arborescence ou la vue Liste de stockage est sélectionnée.
:::

## Menu contextuel {#catalog-context-menu}

Un clic droit sur un catalogue ouvre un menu contextuel :

![Menu contextuel d'un catalogue affichant les actions disponibles](/img/devices_catalogs_02_context2.png)

| Action | Condition | Description |
|--------|-----------|-------------|
| *Mettre à jour* | Catalogue actif uniquement | Re-scanne le catalogue depuis son chemin source |
| *Explorer* | Toujours | Ouvre le catalogue dans l'écran [Explorer](Explore) pour parcourir ses dossiers et fichiers |
| *Modifier* | Toujours | Ouvre le [panneau de modification](#edit) pour changer les paramètres du catalogue |
| *Ouvrir le dossier* | Chemin défini et non un export | Ouvre le dossier source du catalogue dans le gestionnaire de fichiers |
| *Vérifier les sommes de contrôle* | Toujours | Recalcule et compare les sommes de contrôle de tous les fichiers du catalogue |
| *Filelight* | Catalogue actif uniquement | Ouvre [Filelight](https://apps.kde.org/filelight/) dans le chemin source du catalogue |
| *Désaffecter ce catalogue* | Catalogue attribué à un groupe virtuel | Retire le catalogue de son groupe virtuel (le catalogue lui-même n'est pas supprimé) |
| *Supprimer ce catalogue* | Catalogues du groupe physique et exports | Supprime définitivement le catalogue de la collection |
| *Diviser le catalogue par sous-répertoire* | Périphérique catalogue | Découpe le catalogue en un catalogue par sous-répertoire immédiat, plus un pour les fichiers à la racine — voir [Diviser le catalogue](#split-catalog) |
| *Diviser le catalogue par type de fichier* | Périphérique catalogue | Découpe le catalogue en un catalogue par type de fichier — voir [Diviser le catalogue](#split-catalog) |

## Diviser le catalogue {#split-catalog}

Deux opérations de division sont disponibles depuis le menu contextuel (clic droit) sur tout périphérique de type Catalogue. Les deux opérations suppriment le catalogue d'origine et le remplacent par un ensemble de catalogues plus petits. **Cette opération est irréversible.**

### Diviser par sous-répertoire

Crée un nouveau catalogue par sous-répertoire immédiat du chemin source du catalogue. Les fichiers situés directement à la racine (et non dans un sous-répertoire) sont regroupés dans un catalogue dédié nommé `[NomCatalogue]_(root)`.

Les nouveaux catalogues suivent le modèle `[NomCatalogue]_NomSousRépertoire`.

Une boîte de dialogue de confirmation indique le nombre de catalogues qui seront créés avant de procéder.

:::note
Si le catalogue n'a pas de sous-répertoires immédiats, l'opération est annulée et le catalogue d'origine reste inchangé.
:::

### Diviser par type de fichier

Crée un nouveau catalogue par type de fichier trouvé dans le catalogue (Audio, Image, Texte, Vidéo, Autre), en utilisant le même chemin source que l'original. Cela équivaut à créer manuellement ces catalogues avec un filtre de type de fichier lors de la création.

Avant de diviser, une boîte de dialogue propose :
- *Vérifier puis diviser* — relit chaque fichier depuis le disque à l'aide de la base MIME système pour s'assurer d'une détection précise du type, puis divise. Nécessite que le périphérique soit connecté.
- *Diviser sans vérifier* — divise immédiatement en utilisant les types de fichiers déjà enregistrés dans l'index du catalogue.

## Modifier {#edit}

Le panneau de modification permet de changer les champs suivants :

![Panneau de modification d'un catalogue affichant tous les champs configurables](/img/devices_catalogs_03_edit.png)

| Champ | Description |
|-------|-------------|
| *Nom du périphérique* | Le nom affiché du catalogue |
| *Périphérique parent* | Le périphérique de stockage auquel ce catalogue appartient |
| *Chemin source* | Le chemin du dossier depuis lequel le catalogue est construit |
| *Type de fichier* | Restreint le catalogue à un type de fichier spécifique (Tous, Audio, Image, Texte, Vidéo) |
| *Inclure les fichiers cachés* | Indique si les fichiers et dossiers cachés sont inclus lors du scan |
| *Métadonnées* | Niveau d'indexation des métadonnées : *Aucune*, *Standard* ou *Étendue* |
| *Somme de contrôle* | Si les sommes de contrôle sont calculées : *Aucune* ou *SHA-256* |
| *Dossiers exclus* | Liste des sous-dossiers à exclure du scan du catalogue |

Il est généralement recommandé de définir les options correctes lors de la **création** d'un catalogue plutôt que de les modifier ultérieurement.

### Changement du chemin source {#catalog-path-change}

Lorsque le *Chemin source* est modifié et enregistré, Katalog détecte le changement et propose trois options :

| Option | Description |
|--------|-------------|
| *Remplacer la racine du chemin* | Met à jour instantanément tous les chemins de fichiers et de dossiers indexés en remplaçant l'ancien préfixe par le nouveau — aucun re-scan requis, fonctionne sans le périphérique connecté |
| *Ré-indexer complètement* | Re-scanne le catalogue depuis le nouveau chemin source |
| *Ignorer* | Enregistre le nouveau chemin sans modifier l'index du catalogue |

*Remplacer la racine du chemin* est l'option la plus rapide lorsque les fichiers n'ont pas bougé et que seul le point de montage ou la lettre de lecteur a changé.

## Importer {#import}

Des catalogues peuvent être importés depuis un fichier d'export **VVV** (Virtual Volumes View) avec la tabulation comme séparateur.

Chaque volume physique VVV devient un catalogue Katalog séparé.

Étapes :
1. Dans VVV, choisir *Fichier / Exporter…* et sélectionner TAB comme séparateur.
2. Dans Katalog, aller dans la liste des catalogues, cliquer sur *Importer* et sélectionner le fichier créé précédemment.

:::note
Le chemin source et les autres informations sur les volumes VVV ne sont pas disponibles dans l'export. Utiliser le panneau *Modifier* ensuite pour ajouter le chemin source si vous souhaitez pouvoir mettre à jour le catalogue ultérieurement.
:::

## Développement
Quelques idées d'évolutions pour cet écran :
* Voir la liste de [développement Périphériques](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=devices).
