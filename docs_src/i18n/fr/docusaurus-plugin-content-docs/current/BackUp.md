---
version: "3.0"
---
# Sauvegarde
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Résumé
Cette page décrit la page **Sauvegarde**, où des fichiers sont copiés ou déplacés d'un catalogue vers un autre, en utilisant les données des catalogues pour savoir ce qui manque :
* les [concepts](#concepts) : liens, sauvegarde et archivage, conflits, mode source,
* la [liste des liens](#links), avec leur couverture et leur dernière exécution,
* la [création et la modification d'un lien](#link-form),
* l'[aperçu](#preview) et l'[exécution](#run) d'un lien, ou de [tous les liens listés](#run-listed),
* les [autres actions](#more-actions) sur un lien : dupliquer, répliquer les répertoires, inverser, exporter, supprimer, LuckyBackup.

![Page Sauvegarde : liste des liens avec leur source, leur cible, la différence et la dernière sauvegarde](/img/backup_01_overview.png)

## Concepts {#concepts}

### Liens
Un **lien** associe un catalogue **source** à un catalogue **cible**, où ses fichiers sont sauvegardés. Les deux sont des catalogues de la collection (voir [Périphériques](Devices#model)) : pour sauvegarder un dossier sur un disque externe, le dossier et le dossier de sauvegarde sur le disque doivent d'abord avoir chacun un catalogue (voir [Créer](Create)).

Katalog compare les deux catalogues pour trouver les fichiers de la source **absents de la cible**, puis les copie ou les déplace. Il n'a besoin d'aucun outil externe.

### Sauvegarde ou Archive {#type}

| Type | Opération | Objectif |
|------|-----------|----------|
| **Sauvegarde** | **Copie** les fichiers manquants vers la cible. | La source n'est jamais modifiée. L'objectif est la redondance et la récupération. |
| **Archive** | **Déplace** les fichiers vers la cible. | Chaque fichier est supprimé de la source une fois sa copie confirmée. L'objectif est de libérer la source, pour un stockage à long terme. Les dossiers vides laissés dans la source ne sont **pas** supprimés. |

Une sauvegarde :
* recrée la structure de dossiers de la source sur la cible, puis copie les fichiers manquants,
* n'écrase **pas** un fichier déjà présent dans la cible, même s'il est différent (voir [conflits](#conflicts)),
* ne supprime **pas** les fichiers de la cible absents de la source.

Un archivage sur le même système de fichiers déplace les fichiers instantanément ; entre systèmes de fichiers différents, chaque fichier est copié, puis supprimé de la source une fois la copie terminée. Si la copie échoue, le fichier source reste intact.

### Conflits {#conflicts}
Un **conflit** est un fichier présent au même endroit dans la source et dans la cible, mais avec une date, une taille ou une somme de contrôle différente. L'option *En cas de conflit* du lien décide de ce qui se passe :

| Situation | *Ignorer - laisser la cible inchangée* | *Renommer le plus ancien - renommer la cible, copier la source* |
|-----------|------|---------------|
| Source plus récente que la cible | conflit signalé | cible renommée, puis source copiée |
| Cible plus récente que la source | conflit signalé | ignoré, pour protéger la cible plus récente |
| Même date, taille différente | conflit signalé | ignoré (pas de gagnant évident) |
| Fichier source absent du disque | erreur | erreur |

Avec *Renommer l'ancien*, l'ancien fichier cible est renommé `nom_AAAAMMJJ-HHmmss.ext` (par exemple `report_20260225-102559.docx`) : la date est insérée avant l'extension, pour que le fichier puisse toujours être ouvert. Si la copie de la source échoue ensuite, le fichier renommé retrouve son nom : rien n'est perdu.<br/>
Ces fichiers renommés restent sur la cible et occupent de l'espace : ils se nettoient manuellement.

### Mode source {#source-mode}
* Par défaut, la liste des fichiers source provient du **catalogue** : cela fonctionne avec la source déconnectée pour la comparaison, et les dossiers exclus du catalogue ne sont pas sauvegardés.
* Avec *Analyser directement le disque source (nécessite une source connectée)*, le dossier source est lu sur le **disque** : chaque fichier est inclus, quels que soient le catalogue et ses exclusions. La source doit être connectée.

### Espace sur la cible {#space}

| Opération | Espace nécessaire sur la cible | Effet sur la source |
|-----------|-------------------------------|---------------------|
| **Sauvegarde** | Tous les fichiers à copier. | Aucun. |
| **Archive, même système de fichiers** | Aucun : les fichiers sont seulement déplacés. | Espace libéré. |
| **Archive, autre système de fichiers** | Tous les fichiers à déplacer. | Espace libéré après la suppression de chaque fichier. |
| **Conflits** ***Renommer l'ancien*** | Les fichiers renommés sont conservés : une copie de plus pour chacun. | Aucun. |

L'[aperçu](#preview) vérifie l'espace disponible sur la cible et avertit lorsqu'il est insuffisant ou faible (moins de 512 Mo restants après l'opération). Une exécution est vérifiée à nouveau avant de démarrer : si l'espace est insuffisant, elle est refusée, et le message indique l'espace requis, disponible et manquant. Lorsque l'espace de la cible ne peut pas être lu (disques réseau ou virtuels), il n'y a pas de vérification.

## Liste des liens {#links}
La page liste les liens sous forme de cartes.

### Filtres
| Filtre | Effet |
|--------|-------|
| *Type* | *Tous*, *Sauvegarde* ou *Archive*. |
| *Filtre* | *Tous* les liens, ou uniquement ceux dont le périphérique sélectionné sur la page [Sélection](Selection) est la *Source* ou la *Cible*. |

Une ligne au-dessus des cartes donne, pour le périphérique sélectionné, le nombre de liens, la taille de leurs sources et, pour les sauvegardes, le pourcentage **couvert** : la part des sources déjà présente dans les cibles.

### Cartes
Chaque carte indique :
* le nom du lien et son type,
* les catalogues **Source** et **Cible**, avec leur taille et leur nombre de fichiers,
* *Diff* pour une sauvegarde : ce qui manque à la cible, ou *À jour* ; *À déplacer* pour une archive : ce qui est encore dans la source, ou *Rien à déplacer*,
* *Dernière sauvegarde* ou *Dernier archivage* : date et taille de la dernière exécution, ou *jamais*,
* les boutons *Lancer la sauvegarde* / *Lancer l'archivage*, *Aperçu de la sauvegarde* / *Aperçu de l'archivage*, et *Plus d'actions*.

Lorsqu'il n'y a encore aucun lien, la page l'indique et *Ajouter* crée le premier.

## Créer ou modifier un lien {#link-form}
*Ajouter* dans la barre d'outils de la page ouvre le formulaire *Ajouter un lien* ; *Modifier* dans *Plus d'actions* ouvre le même formulaire, *Modifier le lien*, pour un lien existant.

![Formulaire Ajouter un lien](/img/backup_02_link_form.png)

| Champ | Description |
|-------|-------------|
| *Catalogue source* | Le catalogue dont les fichiers sont sauvegardés. |
| *Catalogue cible* | Le catalogue où ils sont écrits. |
| *Nom* | Nom du lien. Le bouton à côté (*Générer le nom automatiquement*) crée `<source> -> <cible>`. |
| *Type* | *Sauvegarde* (copie) ou *Archive* (déplacement), voir [ci-dessus](#type). |
| *Dossiers* | *Inclure les vides* (coché par défaut) : les dossiers qui ne contiennent rien, ni fichier ni sous-dossier, sont aussi recréés sur la cible. Décoché, seuls les dossiers qui contiennent quelque chose sont créés. Cela ne supprime jamais un dossier déjà présent sur la cible. |
| *Copie stricte* | *Reproduire exactement la structure des dossiers (par défaut)* : chaque fichier est copié à sa propre place, même si le même fichier se trouve déjà ailleurs dans la cible. Décoché, un fichier est ignoré lorsqu'un fichier de même nom et de même taille se trouve déjà n'importe où dans la cible : pas de doublons. Toujours désactivé pour une archive. |
| *En cas de conflit* | Voir [conflits](#conflicts). |
| *Mode source* | Voir [mode source](#source-mode). |

*Enregistrer* crée ou met à jour le lien ; *Annuler* ferme le formulaire. Un message en haut du formulaire indique ce qui manque.

## Aperçu {#preview}
*Aperçu de la sauvegarde* / *Aperçu de l'archivage* calcule ce que ferait une exécution, **sans rien copier**. Lorsque *Mettre à jour les catalogues* est coché, les deux catalogues sont d'abord mis à jour. Le résultat s'ouvre sur sa propre page :
* un résumé : fichiers à copier ou à déplacer, conflits, fichiers déjà présents dans la cible, et un avertissement lorsque la source ou la cible est hors ligne,
* la [vérification de l'espace](#space) de la cible,
* la liste des fichiers, avec leur *Statut* (*Copier*, *Déplacer* ou *Conflit*), *Nom du fichier*, *Chemin* et *Taille*,
* *Exporter en CSV* enregistre la liste dans le dossier de la collection (le dossier personnel en mode *Hébergé*), et affiche le chemin du fichier.

![Aperçu de la sauvegarde : résumé, vérification de l'espace et liste des fichiers](/img/backup_03_preview.png)

## Exécution {#run}
*Lancer la sauvegarde* / *Lancer l'archivage* démarre le lien. La source et la cible doivent toutes deux être disponibles.

* Avec *Mettre à jour les catalogues* coché (recommandé), les deux catalogues sont mis à jour **avant** la comparaison, pour que la sauvegarde travaille sur des données actuelles. Après une exécution qui n'a pas été annulée, la cible est de nouveau mise à jour (et la source aussi, pour une archive), pour que les catalogues montrent le résultat.
* Pendant l'exécution, la carte affiche la progression, le fichier en cours de copie, la vitesse et le temps restant (*Reste*). *Pause* la suspend après le fichier en cours, *Reprendre* continue, *Annuler* l'arrête : un fichier en cours de copie est supprimé de la cible, pour ne laisser aucun fichier partiel.
* À la fin, une ligne en haut de la page donne le résultat : fichiers copiés ou déplacés, archivés et copiés (*Renommer l'ancien*), conflits et erreurs. *Dernière sauvegarde* est mise à jour sur la carte.
* *Rapport*, sur cette ligne, ouvre la liste des fichiers de l'exécution, une ligne par fichier avec son statut (*Copié*, *Déplacé*, *Archivé & Copié*, *Conflit*, *Erreur*), son nom, son chemin et sa taille, ainsi que les totaux. C'est le rapport de la dernière exécution (du dernier lien, après *Lancer les liens listés*) ; il est conservé jusqu'à l'exécution suivante ou jusqu'à la fermeture de Katalog.

### Lancer les liens listés {#run-listed}
*Lancer les liens listés* exécute **les liens listés sur la page**, l'un après l'autre : un ensemble de sauvegardes peut ainsi être lancé en une fois et laissé se terminer.
* Les filtres déterminent ce qui est exécuté : pour exécuter un autre ensemble, changez d'abord les filtres.
* Seuls les liens dont la source **et** la cible sont disponibles sont exécutés ; les autres sont ignorés.
* Une confirmation est d'abord demandée : le nombre de liens qui seront exécutés, le volume des archives (une borne supérieure) et des sauvegardes (approximatif), et le nombre de liens ignorés. Ce sont des estimations à partir des totaux des catalogues ; pour un chiffre exact, utilisez l'*Aperçu* sur un lien.
* Chaque lien s'exécute comme s'il était lancé depuis sa propre carte, y compris *Mettre à jour les catalogues*.
* *Annuler* arrête **toute** l'exécution : les liens encore en attente ne sont pas démarrés. *Pause* et *Reprendre* agissent sur le lien en cours.

## Plus d'actions {#more-actions}

| Action | Description |
|--------|-------------|
| *Modifier* | Ouvre le lien dans le [formulaire](#link-form). |
| *Dupliquer* | Crée un nouveau lien avec les mêmes paramètres, pour l'adapter. |
| *Répliquer les répertoires* | Crée la structure de dossiers de la source sur la cible, sans les fichiers. Le résultat donne les dossiers créés, ceux qui existaient déjà, et les erreurs. |
| *Inverser (échanger source et cible)* | Échange la source et la cible, par exemple pour une restauration. Lorsque le nom contient une flèche, ses deux moitiés sont aussi échangées. |
| *Exporter en CSV* | Calcule l'aperçu de ce lien, puis enregistre sa liste de fichiers dans un fichier CSV dans le dossier de la collection (le dossier personnel en mode *Hébergé*), et affiche le chemin du fichier. |
| *Supprimer* | Supprime le lien, après confirmation. Aucun fichier n'est touché, ni sur la source ni sur la cible. |

### Profil LuckyBackup {#luckybackup}
*LuckyBackup* dans la barre d'outils de la page crée un profil [LuckyBackup](https://luckybackup.sourceforge.net) à partir des **liens listés** sur la page : chaque lien devient une tâche. Le profil est enregistré dans `~/.luckyBackup/profiles/`. Voir [Profil LuckyBackup](BackUp_luckybackup_profile).

## Développement
Quelques idées de développement pour cet écran :

### Fonctionnalités futures
- [ ] Gestion des instantanés
- [ ] Motifs d'exclusion/inclusion
- [ ] Planification (cron/systemd/Planificateur de tâches)
- [ ] Fonction de restauration
- [ ] Options de compression
- [ ] Sauvegardes distantes (ssh)

### Futurs modes de conflit

| Mode | Comportement |
|------|--------------|
| **Écraser** | La source l'emporte toujours : la cible est écrasée sans avertissement. Pour les utilisateurs qui veulent que la source fasse foi quelle que soit la date. |
| **Toujours renommer** | Renomme toujours la cible et copie la source, même lorsque la cible est plus récente : archivage explicite et systématique. |

### Options futures

- **Nettoyage de la source après archivage** : option facultative pour supprimer les dossiers vides laissés dans la source après une opération d'Archive (déplacement).
- **Mode suppression** : option facultative pour supprimer les fichiers de la cible absents de la source.
- **Comparaison par somme de contrôle** : détecter les changements de contenu même lorsque le nom, la taille et la date correspondent.
- **Sauvegarde planifiée/automatique** : exécution sur minuterie ou lors de la mise à jour d'un catalogue.
- **Historique des sauvegardes** : journal des exécutions passées avec dates et statistiques.

### Fonctionnalités futures pour l'espace disque
- **Espace libre minimal par lien** : seuil configurable par l'utilisateur (par exemple, toujours garder 5 Go libres).
- **Arrêt anticipé en cas d'épuisement de l'espace en cours d'opération** : détecter les erreurs ENOSPC pendant la copie et s'arrêter immédiatement au lieu de continuer à échouer fichier après fichier.
- **Vérification de la source après archivage** : confirmer que l'espace de la source a bien été libéré à la fin de l'Archive.
- **Affichage de l'évolution de l'espace** : montrer l'espace de la cible au fil du temps dans l'onglet Statistiques.
- **Outil de nettoyage des fichiers archivés** : lister et supprimer en masse les fichiers `nom_AAAAMMJJ-HHmmss.ext` produits par le mode Renommer l'ancien.

* Pour en savoir plus, voir le backlog du [développement de la Sauvegarde](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=BackUp).
