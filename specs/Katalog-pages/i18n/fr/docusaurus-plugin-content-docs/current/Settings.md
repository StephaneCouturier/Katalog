# Paramètres
## Résumé
Cette page décrit toutes les fonctionnalités de l'écran **Paramètres** et comment les utiliser.
* Gestion de données
* Langue et thème
* À propos
![](/img/screen_settings_01.png)
## Gestion de données
### Collection

Une collection est un groupe unique d'appareils et toutes les informations associées telles que les statistiques.<br/>
Le *dossier de Collection* est le dossier sur votre ordinateur dans lequel toutes les données d'une collection sont stockées.<br/>
Il est possible d'avoir plusieurs Collections.

### Modes de données
Katalog propose 2 "*Modes de données*" ou différentes manières de stocker et de gérer les données.

Remarque 1 : il n'existe pas encore de fonctionnalité permettant de convertir une collection d'un mode vers l'autre mode.

Note 2 : changer de mode nécessite de cliquer sur *Appliquer et redémarrer*
 | Mode | Type de base de données | Stockage de données | Fichiers | Vitesse de recherche | Vitesse de catalogage |
 | -------| -------------------|---|---|---|---|
 | **Mémoire** (par défaut)| mémoire informatique | dans les fichiers .csv séparés par des tabulations (pour les appareils, les statistiques, etc.) et dans les fichiers .idx (pour les listes de fichiers de catalogues)|Meilleur pour une synchronisation régulière des fichiers vers un cloud|Vitesse de recherche la plus rapide une fois les catalogues en mémoire (temps de recherche plus long) la première fois qu'un catalogue est utilisé) | Légèrement plus rapide |
 | **Fichier** | Fichier SQLite, faible utilisation de la mémoire | le tout dans le fichier SQLite | toutes les données regroupées dans un seul fichier pouvant atteindre plusieurs centaines de Mo | Plus rapide pour la première recherche, plus lente pour les recherches répétitives dans une grande collection | Légèrement plus lent |

![](/img/settings_database-model.png)

### Mode mémoire de base de données {#database-memory-mode}
![](/img/screen_settings_02_memory.png)

Actions du dossier de collecte :
* Tapez le chemin du dossier Collection et appuyez sur Entrée pour charger la collection
* Sélectionnez le chemin du dossier Collection et chargez la collection
* Ouvrez le dossier de collection dans le gestionnaire de fichiers par défaut du système.

Paramètres du mode mémoire
* Sauvegarde : Activer ou désactiver (par défaut) la conservation d'une copie d'un catalogue avant de le mettre à jour (la copie aura une extension .bak)
* Démarrage : activez ou désactivez (par défaut) le préchargement des derniers catalogues utilisés (dernière sélection) pour obtenir une recherche plus rapide.
* Démarrage : Activer ou désactiver (par défaut) le chargement du dernier catalogue ouvert dans l'écran [Explorer] (Explorer).


### Mode fichier de base de données
![](/img/screen_settings_03_file.png)
Actions du dossier de collecte :
* Tapez le chemin du fichier et appuyez sur Entrée pour charger la collection
* *Sélectionner et ouvrir le fichier de base de données* fournit un moyen de sélectionner et de charger la collection
* *Modifier* : Ouvrez la base de données SQLite dans un éditeur de base de données (ex : [SQLite Browser](http://sqlitebrowser.org)).
* *Nouveau* : Créez un nouveau fichier de collection et chargez-le.

## Langue et thème
* Choisissez la langue de l'application.
* Choisissez le thème de l'application et redémarrez pour l'appliquer.
* Possibilité d'utiliser une taille d'icône plus grande.
* Bouton pour *Ouvrir le fichier de paramètres* (fichier local où les options de Katalop et les dernières sélections sont stockées).

### Thèmes
|Thème du catalogue|Contexte d'utilisation|
|---|---|
|Thème par défaut|s'adapte automatiquement à tous les systèmes d'exploitation et thèmes clairs/sombres|
|Katalog Colors (light)|uniquement pour les thèmes de bureau Light (ne convient pas aux thèmes de bureau sombres)|
|Katalog Colors (foncé)|uniquement pour les thèmes de bureau sombres (ne convient pas aux thèmes de bureau clairs)|

Exemples:
- Bureau léger / Katalog Colors (thème clair)
![](/img/settings-themes-light-desktop-katalog-colors-light.png)

- Bureau sombre / Katalog Colors (thème sombre)
![](/img/settings-themes-dark-desktop-katalog-colors-dark.png)

- Bureau sombre / Thème du bureau
![](/img/settings-themes-dark-desktop-default-theme.png)

## À propos
* Version et date de l'application.
* Option pour rechercher une nouvelle version au démarrage.
* Bouton pour ouvrir ce site de documentation.
* Bouton pour ouvrir les notes de version.
