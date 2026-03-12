---
version: "2.10"
---
# Paramètres
![2.10](https://img.shields.io/badge/Version-2.10-blue)

## Résumé
Cette page décrit toutes les fonctionnalités de l'écran **Paramètres** et comment les utiliser.
* Gestion de données
* Langue et thème
* À propos

![Aperçu général de l'écran Paramètres](/img/screen_settings_01.png)

## Gestion de données

### Collection

Une collection est un groupe unique d'appareils et toutes les informations associées telles que les statistiques.<br/>
Le *dossier de Collection* est le dossier sur votre ordinateur dans lequel toutes les données d'une collection sont stockées.<br/>
Il est possible d'avoir plusieurs Collections.

### Modes de données
Katalog propose 3 "*Modes de données*" ou différentes manières de stocker et de gérer les données.

Remarque 1 : il n'existe pas encore de fonctionnalité permettant de convertir une collection d'un mode vers un autre.

Remarque 2 : changer de mode nécessite de cliquer sur *Appliquer et redémarrer*

| Mode | Type de base de données | Stockage de données | Fichiers | Vitesse de recherche | Vitesse de catalogage |
| -------| -------------------|---|---|---|---|
| **Mémoire** (par défaut) | mémoire informatique | dans les fichiers .csv séparés par des tabulations (pour les appareils, les statistiques, etc.) et dans les fichiers .idx (pour les listes de fichiers de catalogues) | Meilleur pour une synchronisation régulière des fichiers vers un cloud | Vitesse de recherche la plus rapide une fois les catalogues en mémoire (temps plus long lors de la première utilisation d'un catalogue) | Légèrement plus rapide |
| **Fichier** | Fichier SQLite, faible utilisation de la mémoire | le tout dans le fichier SQLite | toutes les données regroupées dans un seul fichier pouvant atteindre plusieurs centaines de Mo | Plus rapide pour la première recherche, plus lente pour les recherches répétitives dans une grande collection | Légèrement plus lent |
| **Hébergé** | Serveur MySQL/MariaDB | toutes les données stockées dans le serveur de base de données hébergé | Données centralisées sur un serveur, accessibles depuis plusieurs machines sur le réseau | Performances de requête côté serveur, adapté aux grandes collections | Légèrement plus lent (surcharge réseau) |

![Diagramme des modes de données](/img/settings_database-model.png)

### Mode mémoire de base de données {#database-memory-mode}
![Paramètres du mode mémoire affichant le chemin du dossier de collection et les options associées](/img/screen_settings_02_memory.png)

Actions du dossier de collection :
* Saisir le chemin du dossier Collection et appuyer sur Entrée pour charger la collection
* Sélectionner le chemin du dossier Collection et charger la collection
* Ouvrir le dossier de collection dans le gestionnaire de fichiers par défaut du système
* Exporter la collection vers un fichier de base de données SQLite, pour basculer vers le mode *Fichier*

Paramètres du mode mémoire :
* *Sauvegarde* : activer ou désactiver (par défaut) la conservation d'une copie d'un catalogue avant de le mettre à jour (la copie aura une extension `.bak`)
* *Démarrage* : activer ou désactiver (par défaut) le préchargement des derniers catalogues utilisés (dernière sélection) pour obtenir une recherche plus rapide
* *Démarrage* : activer ou désactiver (par défaut) le chargement du dernier catalogue ouvert dans l'écran [Explorer](Explore)

### Mode fichier de base de données
![Paramètres du mode fichier affichant le chemin du fichier de base de données et les options associées](/img/screen_settings_03_file.png)

Actions du fichier de collection :
* Saisir le chemin du fichier et appuyer sur Entrée pour charger la collection
* *Sélectionner et ouvrir le fichier de base de données* : sélectionner et charger un fichier de collection existant
* *Modifier* : ouvrir la base de données SQLite dans un éditeur de base de données (ex : [SQLite Browser](http://sqlitebrowser.org))
* *Nouveau* : créer un nouveau fichier de collection et le charger
* *Exporter vers le mode mémoire* : exporter la collection vers des fichiers CSV et d'index, pour basculer vers le mode *Mémoire*

### Mode hébergé de base de données
![Paramètres du mode hébergé affichant les champs de connexion au serveur](/img/screen_settings_05_hosted.png)

Les données de la collection sont enregistrées dans une base de données hébergée sur un serveur local ou réseau (MySQL/MariaDB).

Paramètres de connexion :
* **Nom d'hôte** — le nom d'hôte ou l'adresse IP du serveur de base de données (par défaut : `localhost`)
* **Nom de la base de données** — le nom de la base de données sur le serveur
* **Port** — le numéro de port (par défaut : `3306` pour MySQL/MariaDB)
* **Nom d'utilisateur** — le nom d'utilisateur de la base de données
* **Mot de passe** — le mot de passe de la base de données

Remplir tous les champs et cliquer sur *Appliquer et redémarrer* pour se connecter.

#### Sécurité
* Seuls les noms d'hôtes **locaux** (localhost, 127.x.x.x) et de **réseau privé** (192.168.x.x, 10.x.x.x, 172.16-31.x.x) sont acceptés. Les adresses IP publiques et les noms de domaine sont rejetés.
* Lors de la connexion à une adresse réseau privée, une boîte de dialogue de confirmation s'affiche avant de procéder.

#### Prérequis
* Un serveur MySQL/MariaDB doit être en cours d'exécution et accessible.
* La base de données doit déjà exister sur le serveur (Katalog créera automatiquement les tables requises).
* Le pilote SQL Qt correspondant doit être installé (`QMYSQL` pour MySQL/MariaDB).

## Langue et thème
* Choisir la langue de l'application.
* Choisir le thème de l'application et redémarrer pour l'appliquer.
* Option pour utiliser une taille d'icône plus grande.
* Option pour activer le tri des fichiers sensible à la casse.
* Bouton pour *Ouvrir le fichier de paramètres* (fichier local où les options de Katalog et les dernières sélections sont stockées).

### Thèmes
| Thème Katalog | Contexte d'utilisation |
|---|---|
| Thème par défaut | s'adapte automatiquement à tous les systèmes d'exploitation et thèmes clairs/sombres |
| Katalog Color (clair) | uniquement pour les thèmes de bureau clairs (ne convient pas aux thèmes sombres) |
| Katalog Color (sombre) | uniquement pour les thèmes de bureau sombres (ne convient pas aux thèmes clairs) |

Exemples :

- Bureau clair / Katalog Color (clair)
![Bureau clair avec le thème Katalog Color clair](/img/settings-themes-light-desktop-katalog-colors-light.png)

- Bureau sombre / Katalog Color (sombre)
![Bureau sombre avec le thème Katalog Color sombre](/img/settings-themes-dark-desktop-katalog-colors-dark.png)

- Bureau sombre / Thème par défaut
![Bureau sombre avec le thème par défaut](/img/settings-themes-dark-desktop-default-theme.png)

## À propos
* Version et date de l'application.
* Option pour rechercher une nouvelle version au démarrage.
* Bouton pour ouvrir ce site de documentation.
* Bouton pour ouvrir les notes de version.
