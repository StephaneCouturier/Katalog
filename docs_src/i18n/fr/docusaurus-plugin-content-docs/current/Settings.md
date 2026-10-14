---
version: "3.0"
---
# Paramètres
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Résumé
Cette page décrit toutes les fonctionnalités de l'écran **Paramètres** et comment les utiliser. L'écran a deux onglets :
* *Application* :
  * [Interface](#interface) : langue, thème, taille du texte, taille des icônes, taille de la fenêtre et options de comportement,
  * [Informations sur l'application](#app-information) : version et fichier de configuration,
* *Collection* :
  * [Collection et base de données](#collection-database) : la collection ouverte, son mode de données, l'[export vers un autre mode de données](#export), la version de la base de données et le contrôle qualité, et le dossier d'images,
  * [Importation et synchronisation de la collection](#import-update) : importer des périphériques depuis une autre collection, ou mettre à jour ceux importés précédemment.

*Paramètres* s'ouvre depuis le panneau latéral, par-dessus les autres pages, sur l'onglet *Application*. *Ouvrir... > Base de données hébergée...* dans le panneau latéral l'ouvre sur l'onglet *Collection*, avec le formulaire de connexion hébergée. *Fermer* (ou `Échap`) revient aux autres pages (voir [Interface](Interface)).

## Application {#application}

![Onglet Application : version, langue, thème, taille du texte, réinitialisation de la taille de la fenêtre et options de comportement](/img/settings_05_application.png)

### Interface {#interface}

#### Langue {#language}
La langue de l'interface. Au premier démarrage, Katalog utilise la langue du système lorsqu'elle est disponible, et l'anglais (US) sinon.

#### Thème {#theme}
| Thème | Description |
|-------|-------------|
| *Thème du bureau* | Suit les couleurs du bureau. |
| *Thème du bureau (gris)* | Suit les couleurs du bureau, avec des surfaces et des surbrillances grises. |
| *Couleurs de Katalog* | Les couleurs bleues propres à Katalog. |

Avec chaque thème, Katalog suit le **mode clair ou sombre** du système, sous Linux (KDE Plasma et autres bureaux), Windows et macOS, et change avec lui sans redémarrage. En dehors de KDE Plasma, Katalog utilise les couleurs claires et sombres de Breeze plutôt que le jeu de couleurs propre au bureau.

:::note
Sur certains bureaux Linux (par exemple Linux Mint avec Cinnamon), le mode sombre n'est pas détecté au démarrage de Katalog : la fenêtre s'ouvre en clair, puis suit le prochain changement de mode clair ou sombre.
:::

#### Taille du texte {#text-size}
Rend tout le texte de Katalog plus petit ou plus grand, de 80 % à 120 %, avec le curseur ou ses deux boutons. La taille part de la taille de police définie dans le système, et suit donc aussi un changement de la police du système. Le nom de la collection en haut du panneau latéral et les titres des pages gardent la taille du système.

#### Taille des icônes {#icon-size}
*Utiliser une taille d'icône plus grande* : affiche des icônes plus grandes dans les listes.

#### Taille de la fenêtre {#window-size}
*Réinitialiser la taille de la fenêtre* : remet la fenêtre à sa taille par défaut, **1280×800** (ou la taille de l'écran, s'il est plus petit). Une fenêtre agrandie est d'abord restaurée. La position de la fenêtre n'est pas modifiée. C'est aussi la taille de la fenêtre au tout premier démarrage de Katalog.

#### Comportement {#behavior}
| Option | Description |
|--------|-------------|
| *Le tri des fichiers est sensible à la casse* | Lorsque cette option est activée, le tri respecte la casse des lettres, ce qui donne cet ordre : AA, AB, AC, Aa, Ab, Ac. |
| *Actualiser le statut des périphériques au retour dans l’application* | Lorsque cette option est activée, le statut des périphériques (disponible ou non) est vérifié à nouveau chaque fois que la fenêtre de Katalog devient active : un disque branché ou retiré entre-temps est ainsi affiché correctement. Désactivée par défaut. |
| *Autoriser les actions qui suppriment des fichiers (sans récupération)* | Lorsque cette option est activée, des fichiers peuvent être **supprimés définitivement** depuis les résultats de [recherche](Search) et l'écran [Explorer](Explore). Désactivée par défaut : ces actions sont alors grisées. *Déplacer dans la corbeille*, qui peut être annulé, est toujours disponible. |

### Informations sur l'application {#app-information}

#### Version
* La version et la date de publication de Katalog, et *Notes de Version* pour lire ce qui a changé.
* *Vérifier les nouvelles versions au démarrage* : lorsque cette option est activée, Katalog vérifie au démarrage si une version plus récente est disponible. La version Flatpak ne vérifie pas : Flathub et votre logithèque annoncent ses mises à jour.

#### Fichier de configuration
Le chemin du fichier dans lequel Katalog conserve ses paramètres et ses derniers choix. *Ouvrir* l'ouvre dans l'application associée.

## Collection et base de données {#collection-database}

![Page Paramètres, onglet Collection : section Collection et base de données, collection de démonstration en mode Fichier](/img/settings_01_overview.png)

### Collection {#collection}
Une **collection** est un groupe unique de périphériques et de toutes les informations associées : catalogues, statistiques, étiquettes, historique des recherches et liens de sauvegarde.<br/>
Il est possible d'avoir plusieurs collections et de passer de l'une à l'autre.

Les collections s'ouvrent et se créent depuis le panneau latéral, avec *Ouvrir...* et *Nouveau...* (voir [Interface](Interface#drawer)). Passer à une autre collection ne redémarre pas Katalog.

La section affiche :
* *Mode de base de données* : le mode de données de la collection ouverte (voir ci-dessous),
* *Collection* : l'emplacement de la collection (dossier, fichier de base de données, ou serveur et nom de la base de données), avec un bouton *Modifier* :
  * en mode *Fichier*, il ouvre le fichier de base de données dans l'application qui lui est associée (par exemple [DB Browser for SQLite](https://sqlitebrowser.org)),
  * en mode *Mémoire*, il ouvre le dossier de la collection dans le gestionnaire de fichiers,
  * en mode *Hébergé*, il ouvre la base de données dans phpMyAdmin, à l'*URL phpMyAdmin* définie pour la connexion.

### Modes de données {#database-memory-mode}
Katalog propose 3 *modes de données*, c'est-à-dire 3 façons de stocker et de traiter les données d'une collection.

| Mode | Type de base de données | Stockage des données | Fichiers | Vitesse de recherche | Vitesse de catalogage |
|------|-------------------------|----------------------|----------|----------------------|-----------------------|
| **Mémoire** | mémoire de l'ordinateur | dans des fichiers `.csv` séparés par des tabulations (pour les périphériques, les statistiques, etc.) et dans des fichiers `.idx` (pour les listes de fichiers des catalogues) | mieux adapté à une synchronisation régulière des fichiers vers un cloud | recherche la plus rapide une fois les catalogues en mémoire (plus long la première fois qu'un catalogue est utilisé) | légèrement plus rapide |
| **Fichier** (par défaut) | fichier SQLite, faible utilisation de la mémoire | tout dans le fichier SQLite | toutes les données dans 1 fichier, qui peut atteindre plusieurs centaines de Mo | plus rapide pour la première recherche, plus lent pour des recherches répétées dans une grande collection | légèrement plus lent |
| **Hébergé** | serveur MySQL/MariaDB | tout sur le serveur de base de données | données centralisées sur un serveur, accessibles depuis plusieurs ordinateurs du réseau | performances des requêtes côté serveur, adapté aux grandes collections | légèrement plus lent (réseau) |

### Mode hébergé {#hosted-mode}

![Page Paramètres avec le formulaire de connexion hébergée](/img/settings_02_hosted.png)

En mode *Hébergé*, la collection est stockée dans une base de données MySQL/MariaDB sur un serveur local ou du réseau.<br/>
Le formulaire de connexion est affiché lorsque la collection ouverte est en mode *Hébergé*, ou en choisissant *Ouvrir...* > *Base de données hébergée...* dans le panneau latéral.

| Champ | Description |
|-------|-------------|
| *Nom d'hôte* | `localhost` ou l'adresse IP du serveur de base de données, sur cet ordinateur ou sur le réseau local (voir [Sécurité](#hosted-security)). Laissé vide, il devient `localhost`. |
| *Nom de la base de données* | Nom de la base de données sur le serveur. |
| *Port* | Port du serveur (`3306` pour MySQL/MariaDB). |
| *Nom d'utilisateur* | Nom d'utilisateur de la base de données. |
| *Mot de passe* | Mot de passe de la base de données. |
| *URL phpMyAdmin* | Adresse de phpMyAdmin, utilisée par le bouton *Modifier*. |

*Se connecter* ouvre la collection sur le serveur.

Prérequis :
* Un serveur MySQL/MariaDB doit être en fonctionnement et joignable.
* La base de données doit déjà exister sur le serveur ; Katalog crée les tables dont il a besoin.

#### Sécurité {#hosted-security}
Pour que les données d'une collection restent sur des ordinateurs que l'utilisateur contrôle, Katalog se connecte uniquement à un serveur de base de données situé sur cet ordinateur ou sur le réseau local. L'hôte est vérifié lors de *Se connecter*, et lorsqu'une collection *Hébergé* est ouverte depuis les *Collections récentes* du panneau latéral :

| Hôte | Ce qui se passe |
|------|-----------------|
| Cet ordinateur : `localhost`, `127.x.x.x` | Katalog se connecte. |
| Réseau local : `192.168.x.x`, `10.x.x.x`, `172.16.x.x` à `172.31.x.x` | Katalog demande d'abord une confirmation (*Connexion réseau privé*, *Vos données seront envoyées vers cette base de données réseau.*). *Non* est le choix par défaut ; rien n'est modifié s'il est choisi. |
| Toute autre adresse : une adresse publique, ou un nom de serveur (par exemple `mydb.example.com` ou `nas`) | Refusé : *Nom d'hôte invalide.* s'affiche en haut du formulaire avec les adresses acceptées. Rien n'est connecté ni enregistré. Une collection refusée ouverte depuis les *Collections récentes* affiche le formulaire, rempli, avec le même message. |

Un serveur du réseau local est indiqué par son adresse IP, pas par son nom.

### Export vers un autre mode de données {#export}
La collection ouverte peut être copiée vers un autre mode de données, par exemple pour déplacer une collection d'un serveur vers un fichier unique. La collection ouverte n'est pas modifiée et reste ouverte.

| Mode de données de la collection ouverte | *Exporter vers un fichier SQLite* | *Exporter vers le mode mémoire (csv)* |
|---|:---:|:---:|
| *Mémoire* | oui | — |
| *Fichier* | — | oui |
| *Hébergé* | oui | oui |

* *Exporter vers un fichier SQLite* demande le fichier à créer, `export.db` dans le dossier de la collection par défaut (dans le dossier personnel en mode *Hébergé*). Si le fichier existe déjà, il est d'abord déplacé dans la corbeille. En mode *Mémoire*, les fichiers et dossiers de tous les catalogues sont d'abord chargés, ce qui peut prendre du temps pour une grande collection.
* *Exporter vers le mode mémoire (csv)* demande un dossier, qui **doit être vide**. Katalog y écrit les fichiers `.csv` de la collection et les fichiers `.idx` de chaque catalogue.

La progression est affichée en bas de la page, avec *Annuler*. L'annulation supprime ce que l'export avait écrit. Un message indique le chemin de l'export lorsqu'il est terminé. Les boutons d'export ne sont pas disponibles pendant la création ou la mise à jour d'un catalogue, ni pendant une recherche.

La collection exportée peut ensuite être ouverte depuis le panneau latéral avec *Ouvrir...*.

:::note
En mode *Mémoire*, si une recherche a été lancée avant l'export, les dossiers qui ne contiennent aucun fichier peuvent manquer dans le fichier SQLite pour les catalogues qui ont été parcourus par la recherche. Pour les obtenir tous, exportez juste après l'ouverture de la collection.
:::

### Version de la base de données et contrôle qualité {#quality-check}

![Rapport du contrôle qualité de la collection de démonstration](/img/settings_03_quality_check.png)

*Version de la base de données* indique la version de la structure de la base de données de la collection ouverte.

*Contrôle qualité* lance une série de vérifications en lecture seule sur la collection ouverte et affiche un rapport. Il recherche les incohérences entre périphériques, stockages et catalogues :
* périphériques de stockage liés à un stockage manquant,
* stockages liés à aucun périphérique,
* ID de stockage utilisés par plus d'un stockage,
* périphériques catalogue liés à un catalogue manquant,
* catalogues liés à aucun périphérique,
* périphériques dont le périphérique parent est manquant.

Pour chaque vérification, le rapport liste ce qui a été trouvé, ou *Rien trouvé*. *Copier dans le presse-papiers* copie le rapport, par exemple pour le joindre à un rapport de bug.<br/>
Le contrôle qualité **ne répare rien** : il se contente de signaler.

### Dossier d'images {#images-folder}
Le *Dossier d'images* est le dossier où sont conservées les images des périphériques de stockage. C'est un paramètre propre à chaque collection.
* Saisissez un chemin dans le champ, ou cliquez sur *Sélectionner* pour choisir un dossier.
* S'il n'est pas défini, le dossier `images` situé dans le dossier de la collection est utilisé.

Les images placées dans ce dossier peuvent ensuite être choisies pour un périphérique de stockage lors de sa modification dans l'écran [Périphériques](Devices).

## Importation et synchronisation de la collection {#import-update}

![Section d'import avec une collection source sélectionnée, un périphérique choisi dans l'arborescence source, et le rapport d'import](/img/settings_04_import.png)

Katalog peut importer des périphériques d'une autre collection dans la collection ouverte, puis les mettre à jour plus tard quand la source change.
* La **source** est l'autre collection. Elle est seulement lue, jamais modifiée.
* La **cible** est la collection ouverte, où les périphériques et les catalogues sont ajoutés.

| Champ | Utilisation |
|-------|-------------|
| *Mode de données* | Type de la source : *Katalog / Fichier* (un fichier `.db`), *Katalog / Mémoire* (un dossier de collection), ou *VVV / Valeurs séparées par des tabulations* (un fichier exporté depuis VVV). |
| *Source* | La collection ou le fichier source, choisi avec *Sélectionner*. |
| *Périphérique* | Le périphérique à importer, choisi dans l'arborescence des périphériques de la source. *Importer* l'importe avec tout son contenu. |
| *Actualiser* | Les collections sources depuis lesquelles des périphériques ont été importés. *Actualiser* met à jour, depuis la source choisie, les catalogues de tous les périphériques importés depuis celle-ci. |

**Ce qui est importé**<br/>
Les périphériques, leurs catalogues et listes de fichiers, les statistiques, les liens de sauvegarde et les dossiers exclus. La hiérarchie de la source est conservée dans la cible en ajoutant les périphériques parents nécessaires. Lorsqu'un nom de périphérique ou de catalogue existe déjà dans la cible, celui importé est renommé (par exemple `My Drive (2)`).

**Rapport**<br/>
À la fin d'une mise à jour, un message résume ce qui a changé : le nombre de catalogues mis à jour, ainsi que le nombre et la taille totale des fichiers avant et après.

:::note
La collection source doit avoir la même version de base de données que la collection ouverte. Si elle a été créée avec une version plus ancienne de Katalog, ouvrez-la d'abord une fois avec cette version : sa base de données est alors mise à jour.
:::

**Import depuis VVV**<br/>
[VVV (Virtual Volumes View)](https://www.fsoft.it/VVV/) est une autre application de catalogage. Avec *VVV / Valeurs séparées par des tabulations*, sélectionnez un fichier exporté depuis VVV, puis cliquez sur *Importer* : ses catalogues sont ajoutés sous un nouveau groupe virtuel.

## Développement
* Voir le backlog du [développement des Paramètres](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=settings).
