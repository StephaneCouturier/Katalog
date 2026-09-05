---
version: "2.13"
---
# Sauvegarde
![2.13](https://img.shields.io/badge/Version-2.13-blue)

## Résumé
Cette page décrit toutes les fonctionnalités de l'écran **BackUp** et comment les utiliser.<br/>
Depuis cet écran, l'utilisateur peut **gérer les sauvegardes de catalogue (copie) ou l'archivage (déplacement)**.<br/>

![Écran principal BackUp](/img/screen_backup_01.png)

## Concepts principaux

### Liens de catalogue

L'écran permet d'<b>associer un catalogue</b>, considéré comme la <b><i>source</i></b>, à un autre catalogue, considéré comme la <b><i>cible</i></b> de la sauvegarde.<br/>C'est le concept central de toutes les fonctionnalités de cet écran.

### Comparaison des catalogues

L'écran affiche la liste des liens et la <b>couverture des sauvegardes</b> pour l'ensemble de la sélection et par lien, en comparant la taille, le nombre de fichiers et la date de mise à jour de la source et de la cible.

### Réplication de répertoires

Nécessaire lors du processus de sauvegarde, il est possible de déclencher la copie de l'arborescence de dossiers séparément, sans les fichiers.

## BackUp ou Archive

| Opération | Traitement des fichiers | Objectif |
|-----------|------------------------|----------|
| **BackUp** | Opération de **copie** d'un catalogue source vers un catalogue cible. | La source n'est jamais modifiée. L'objectif est la redondance et la récupération. <br/> Plusieurs stratégies existeront avec le temps (complète, incrémentale, synchronisation). |
| **Archive** | Opération de **déplacement** d'un catalogue source vers un catalogue cible. | Les fichiers sont transférés puis supprimés de la source une fois la copie vérifiée. L'objectif est le déchargement à long terme et l'organisation du stockage. Les répertoires vides laissés dans la source ne sont **pas** supprimés. |



### Sauvegarde incrémentale (copie de fichiers)

La sauvegarde Katalog copie les fichiers d'un **catalogue source** vers un **catalogue cible** en utilisant les données indexées de Katalog.
<br/>Pour que cela fonctionne complètement, les catalogues seront mis à jour avant la sauvegarde et après (optionnel mais fortement recommandé).
<br/>Il n'y a aucune dépendance à un outil externe.
<br/><br/>Séquence d'actions :
- Compare les catalogues source et cible pour trouver les **fichiers manquants dans la cible**.
- Copie les fichiers manquants vers la cible, en recréant l'arborescence de dossiers.
- Ne **remplace pas** les fichiers existants dans la cible (même s'ils sont différents).
- Ne **supprime pas** les fichiers de la cible qui sont absents de la source.

Options "Copie stricte"
- <i>Copie stricte</i> (par défaut) : Katalog copiera les fichiers même s'ils sont déjà présents dans la cible. Si décoché, les fichiers déjà présents dans la cible (selon le nom, la taille et la date) ne seront pas copiés à nouveau.

Options "Répertoires"
- <i>Inclure les vides</i> (par défaut : coché) : l'arborescence des répertoires de la source est recréée dans la cible avant toute copie de fichier. Si coché, un répertoire qui ne contient rien du tout — ni fichier ni sous-répertoire — est également recréé. Si décoché, un tel répertoire n'est pas créé, tandis que tout répertoire qui contient quelque chose l'est toujours. Décocher cette option ne supprime jamais un répertoire déjà présent dans la cible.

Options de gestion des conflits
- <i>En cas de conflit</i> (par défaut : <i>Renommer le plus ancien</i>)

Katalog peut gérer les conflits de différentes manières, lorsqu'un fichier existe dans la cible mais que la date, la taille ou la somme de contrôle sont différentes.

Un **conflit** survient lorsqu'un fichier existe à la fois dans la source et dans la cible au même chemin, mais que la date, la taille ou la somme de contrôle diffèrent. Le mode contrôle le comportement dans ce cas.

#### Modes disponibles {#available-modes}

| Mode | Comportement |
|------|-------------|
| **Ignorer** (par défaut) | Aucune opération — la source n'est pas copiée, la cible n'est pas modifiée. Le conflit est signalé pour examen. |
| **Renommer le plus ancien** | Si la source est plus récente : renomme le fichier cible (en ajoutant un suffixe horodaté), puis copie la source. Si la cible est plus récente ou de même date : ignore (protège la cible plus récente). |

#### Tableau complet des scénarios

| # | Situation | Ignorer | Renommer le plus ancien |
|---|-----------|---------|------------------------|
| A | Source plus récente que la cible | conflit signalé | renomme la cible → copie la source ✓ |
| B | Cible plus récente que la source | conflit signalé | ignore (protège la cible plus récente) |
| C | Même date, taille différente | conflit signalé | ignore (pas de gagnant évident) |
| D | Fichier source manquant sur le disque | erreur | erreur |

> **Renommer le plus ancien — format du nom archivé** : l'ancien fichier cible est renommé en `nomoriginal_AAAAMMJJ-HHmmss.ext` (ex. `rapport_20260225-102559.docx`). L'horodatage est inséré avant l'extension afin que le fichier reste ouvrable. Ces fichiers s'accumulent sur la cible et doivent être supprimés manuellement pour récupérer de l'espace.

> **Renommer le plus ancien — garantie de sécurité** : si la copie du fichier source échoue après que la cible a déjà été renommée, le fichier renommé est automatiquement restauré à son nom d'origine. Aucune donnée n'est perdue.


### Archive (déplacement de fichiers)

L'opération Archive **déplace** les fichiers de la source vers la cible au lieu de les copier. Les fichiers sources sont supprimés après un transfert réussi et confirmé — en cas d'échec du transfert, le fichier source est conservé intact.

- Sur le **même système de fichiers** : le déplacement est instantané — aucune donnée n'est physiquement copiée ; seul l'emplacement du fichier change.
- **Entre systèmes de fichiers différents** : le fichier est d'abord copié vers la cible, puis supprimé de la source une fois la copie confirmée.

### Mode source {#source-mode}

Chaque lien de sauvegarde possède un **Mode source** qui contrôle ce qui est utilisé comme source lors de la comparaison et de la copie de fichiers.

| Mode | Description |
|------|-------------|
| **Catalogue** (par défaut) | Utilise l'index du catalogue. Fonctionne hors connexion — le périphérique source n'a pas besoin d'être connecté. Les règles d'exclusion de dossiers du catalogue sont appliquées : les dossiers exclus ne sont pas sauvegardés. |
| **Disque** | Parcourt directement le système de fichiers source. Le périphérique source **doit être connecté et monté**. Tous les fichiers sous le chemin source sont inclus — l'index du catalogue et les règles d'exclusion de dossiers sont entièrement ignorés. |

**Interface** : la case à cocher *Analyser le disque source directement* dans le panneau Créer un lien contrôle ce paramètre. Décochée = Catalogue (par défaut), cochée = Disque.

### Profil LuckyBackup

Il est possible d'exporter les liens BackUp vers un profil [LuckyBackup](https://luckybackup.sourceforge.net).
Voir la page dédiée : [Profil LuckyBackup](BackUp_luckybackup_profile)


## Gestion des liens BackUp

Pour aider à lister et comparer les répertoires sources et leurs sauvegardes, Katalog permet de lier des catalogues.

### Créer un lien BackUp

Le panneau *Créer un lien* peut être replié ou déplié à l'aide du bouton bascule en haut du panneau.

#### Champs du lien

| Champ | Description |
|-------|-------------|
| Nom | Étiquette du lien. Peut être généré automatiquement sous la forme `"<nom source> -> <nom cible>"`. |
| Type | `BackUp` (copie) ou `Archive` (déplacement). Contrôle si les fichiers source sont supprimés après le transfert. |
| Périphérique source | Le catalogue source. Les fichiers sont lus depuis son chemin indexé. |
| Périphérique cible | Le catalogue de destination de la sauvegarde. Les fichiers sont écrits dans son chemin. |
| Date de dernière sauvegarde | Date de la dernière sauvegarde complétée. Mise à jour automatiquement. |
| Taille de la dernière sauvegarde | Total d'octets transférés lors de la dernière sauvegarde. Mise à jour automatiquement. |
| Copie stricte | Si activé (par défaut), copie les fichiers par chemin — même si le fichier existe déjà ailleurs dans la cible. Si désactivé, ignore les fichiers déjà présents dans la cible (mode dédoublonnage). Non applicable pour les liens *Archive* (désactivé automatiquement). |
| En cas de conflit | Comportement lorsqu'un fichier existe au même chemin dans la source et la cible mais diffère. Par défaut : `RenommerLePlusAncien`. Voir [Modes disponibles](#available-modes). |
| Mode source | `Catalogue` (par défaut) ou `Disque`. Contrôle si la source est lue depuis l'index du catalogue ou en parcourant directement le système de fichiers. Voir [Mode source](#source-mode). |
| Répertoires - Inclure les vides | Si activé (par défaut), les répertoires qui ne contiennent ni fichier ni sous-répertoire sont recréés dans la cible. Si désactivé, seuls les répertoires qui contiennent quelque chose sont créés. |

#### Exemple et catalogues
Objectif : créer un lien entre la source sur le disque local et la cible sur un disque externe.
![Exemple de périphériques source et cible](/img/screen_backup_1_devices.png)

#### Sélectionner la source et la cible
- Avec le panneau Sélection et les 2 boutons "Charger les catalogues" pour la source et la cible, obtenez la liste des catalogues disponibles.
- Le bouton "sans liens" permet de limiter le nombre de catalogues affichés, en n'affichant que les catalogues qui n'ont pas encore de lien en tant que source (ou cible).
- Sélectionnez une source et une cible.

![Sélection de la source et de la cible](/img/screen_backup_2_select_source_target.png)


#### Définir le nom, les options et créer
- Générer un nom : nom du catalogue source + " -> " + nom du catalogue cible
- Définir l'option "Copie stricte"
- Définir l'option "Répertoires"
- Définir le comportement en cas de détection de conflit
- Créer le lien

![Création du lien](/img/screen_backup_4_create_mapping.png)

#### Comparer la source et la cible de sauvegarde
- Le lien apparaît dans la liste et la couverture est calculée.
![Comparaison source et cible](/img/screen_backup_5_comparison.png)

### Filtres de la liste des liens

La liste des liens peut être filtrée pour n'afficher que les liens pertinents :
- Boutons radio **Source / Cible** — affiche uniquement les liens pour lesquels le périphérique sélectionné est la source ou la cible.
- Menu déroulant **Type** — filtre par *BackUp* ou *Archive*.
- Case à cocher **Afficher le tableau complet** — affiche ou masque les colonnes de détail supplémentaires.

### Menu contextuel d'un lien

Un clic droit sur un lien dans la liste ouvre un menu contextuel avec les actions suivantes :

| Action | Description |
|--------|-------------|
| *Lancer la sauvegarde* / *Lancer l'archivage* | Démarre l'opération de sauvegarde ou d'archivage pour ce lien. |
| *Aperçu de la sauvegarde* / *Aperçu de l'archivage* | Lance un aperçu (simulation) sans copier aucun fichier. |
| *Répliquer les répertoires* | Copie uniquement l'arborescence de dossiers, sans les fichiers. |
| *Inverser (échanger source et cible)* | Échange la source et la cible du lien en un clic — utile pour inverser le sens d'une sauvegarde. |
| *Supprimer* | Supprime le lien (sans affecter les fichiers sur le disque). |

**Création de profil LuckyBackUp**
Katalog peut générer un profil LuckyBackUp prêt à l'emploi à partir des liens BackUp :
- Enregistré dans le répertoire `~/.luckyBackup/profiles/`
- Chaque lien de sauvegarde devient une tâche dans le profil
- Par défaut, **tous** les liens sont inclus
- Cocher *Liens sélectionnés uniquement* pour n'inclure que les liens visibles dans la liste filtrée (filtrée par périphérique Source/Cible et/ou Type)

## Exécution de la sauvegarde ou de l'archivage

### Prérequis
- Un lien BackUp/Archive est sélectionné
- Les deux catalogues doivent appartenir à des périphériques avec des chemins valides et accessibles.
- L'espace disponible sur la cible doit être suffisant pour copier/déplacer les fichiers.
- Bien que facultatif, il est recommandé de garder "Mettre à jour les catalogues" sélectionné pour une mise à jour avant et après la fin du processus.

### Aperçu
- Un aperçu (simulation) peut être lancé pour tester l'effet de la sauvegarde ou de l'archivage et générer un rapport.
- Le résultat de l'aperçu peut être **exporté** via le bouton *Exporter*, enregistrant la liste des opérations prévues dans un fichier.

### Pause, Reprise et Annulation

Pendant l'exécution, le bouton **Lancer la sauvegarde** change d'étiquette et de fonction :
- En cours d'**exécution** → cliquer pour **Mettre en pause** (suspend après la fin du fichier en cours)
- En **pause** → cliquer pour **Reprendre**

Un bouton **Annuler** est toujours disponible pendant l'exécution ou la pause. L'annulation arrête proprement l'opération — tout fichier en cours de copie est supprimé de la cible (aucun fichier partiel n'est laissé).

### Mise à jour du catalogue après exécution

Après une sauvegarde réussie, le catalogue cible sera mis à jour automatiquement pour refléter les fichiers nouvellement copiés, sans nécessiter une réindexation complète. *(Prévu — pas encore disponible.)*

## Comportement principal : copie ou archivage incrémental

Critères de comparaison :
- Correspondance par **nom de fichier + chemin relatif** (même fichier au même emplacement relatif).
- Un fichier est "manquant" si aucune correspondance n'existe dans le catalogue cible.

**Gestion des conflits**
| Décision | Choix | Justification |
|----------|-------|---------------|
| Supprimer des fichiers dans la cible ? | Non (v1). | Commencer prudemment — incrémental uniquement. |
| Écraser les conflits ? | Non (v1). Les signaler. | Éviter la perte de données. |
| Créer les répertoires manquants ? | Oui, toujours. | Nécessaire pour toute copie de fichier. |

### Gestion de l'espace disque

L'espace disque est une contrainte critique pour les opérations de sauvegarde et d'archivage. Manquer d'espace en cours d'opération laisse la cible dans un état partiel.

Besoins en espace par opération :

| Opération | Ce qui consomme de l'espace cible | Effet net sur la source |
|-----------|----------------------------------|------------------------|
| **BackUp** | Tous les fichiers à copier | Aucun |
| **Archive (même FS)** | Zéro — déplacement de métadonnées uniquement | Espace libéré sur la source |
| **Archive (FS différents)** | Fichiers copiés avant suppression de la source | Espace libéré après suppression |
| **Conflit RenommerLePlusAncien** | Fichier cible renommé et conservé (+1 copie) | Aucun jusqu'au nettoyage manuel |

### Vérification de l'espace (implémentée)

Calculée avant l'exécution et affichée dans l'aperçu.

| Condition | Seuil | Action |
|-----------|-------|--------|
| **Insuffisant** | espace disponible < espace requis | Bloqué : avertissement, opération non démarrée |
| **Faible** | disponible − requis < 512 Mo | Demande confirmation (Oui/Non pour continuer) |
| **OK** | disponible − requis ≥ 512 Mo | Continue sans notification |

Dans l'aperçu, le statut de l'espace est ajouté au résumé :
- **Insuffisant** → avertissement rouge : `⚠ Espace cible : X disponible, Y nécessaire`
- **Faible** → avertissement orange : `⚠ Espace cible faible : Z restant après l'opération`
- **OK** → aucune annotation

> **Remarque** : le mode *Renommer le plus ancien* ajoute définitivement des copies renommées sur la cible. Ces fichiers (`nom_AAAAMMJJ-HHmmss.ext`) doivent être supprimés manuellement pour récupérer de l'espace.

## Rapport
Après l'exécution, le tableau d'aperçu est remplacé par un **rapport de sauvegarde** — un tableau à quatre colonnes : **Statut**, **Nom du fichier**, **Chemin** et **Taille**.

Chaque ligne correspond à un fichier, avec les valeurs de statut suivantes :

| Statut | Signification |
|--------|---------------|
| *Copié* | Fichier copié avec succès vers la cible. |
| *Déplacé* | Fichier déplacé avec succès vers la cible (opération Archive). |
| *Archivé & Copié* | Un fichier cible en conflit a été renommé (mode RenommerLePlusAncien), puis la source a été copiée. |
| *Conflit* | Le fichier existe au même chemin dans la source et la cible mais diffère — non écrasé, signalé pour examen. |
| *Erreur* | Le fichier n'a pas pu être copié ou déplacé (permissions refusées, disque plein, etc.). |

Une ligne de résumé au-dessus du tableau affiche les totaux : fichiers copiés, archivés & copiés, conflits et erreurs.

---

## Développement
Quelques idées de développements pour cet écran :

### Fonctionnalités futures
- [ ] Gestion des instantanés
- [ ] Filtres d'inclusion/exclusion
- [ ] Planification (cron/systemd/Planificateur de tâches)
- [ ] Fonctionnalité de restauration
- [ ] Options de compression
- [ ] Sauvegardes distantes (ssh)

### Modes de conflit futurs

| Mode | Comportement |
|------|-------------|
| **Écraser** | La source prime toujours — écrase la cible silencieusement. Pour les utilisateurs qui veulent que la source fasse autorité quelle que soit la date. |
| **Toujours renommer** | Renomme toujours la cible et copie la source, même si la cible est plus récente — archivage agressif et explicite. |

### Options futures

- **Nettoyage de la source après archivage** : option pour supprimer les répertoires vides laissés dans la source après une opération Archive.
- **Mode suppression** : option pour supprimer les fichiers cibles absents de la source.
- **Comparaison par somme de contrôle** : détecter les modifications de contenu même lorsque le nom, la taille et la date correspondent.
- **Sauvegarde planifiée/automatique** : exécution sur minuterie ou lors de la mise à jour d'un catalogue.
- **Historique des sauvegardes** : journal des sauvegardes passées avec dates et statistiques.

### Options futures — Espace disque
- **Espace minimum par lien** : plancher configurable par l'utilisateur (ex. : toujours conserver 5 Go libres).
- **Arrêt anticipé en cas de saturation** : détecter les erreurs d'espace disque pendant la copie et arrêter immédiatement.
- **Vérification post-archivage** : confirmer que l'espace source a bien été libéré après l'archivage.
- **Affichage de la tendance d'espace** : afficher l'évolution de l'espace cible dans l'onglet Statistiques.
- **Outil de nettoyage des fichiers archivés** : lister et supprimer en masse les fichiers `nom_AAAAMMJJ-HHmmss.ext` produits par le mode RenommerLePlusAncien.

* Pour en savoir plus, consultez le backlog de [développement BackUp](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=BackUp).
