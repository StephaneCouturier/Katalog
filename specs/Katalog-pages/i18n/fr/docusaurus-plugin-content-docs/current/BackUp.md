# Sauvegarde
## Résumé
Cette page décrit toutes les fonctionnalités de l'écran **Sauvegarde** et comment les utiliser.<br/>
A partir de cet écran, l'utilisateur peut **gérer les sauvegardes de catalogue**.<br/>

<b>Fonction de mappage</b> : pour l'instant, l'écran permet d'associer un catalogue, considéré comme la source, à un autre catalogue, considéré comme la cible de la sauvegarde.
Cela permet de vérifier la couverture de la sauvegarde pour les périphériques, et de comparer la taille de la source et de la cible de la sauvegarde, le nombre de fichiers et la date de mise à jour.

<b>Katalog ne dispose pas encore de fonctions permettant de copier automatiquement des fichiers.</b><br/>
De nombreuses applications simples ou avancées sont disponibles, telles que [KBackUp](https://apps.kde.org/fr/kbackup) ou [LuckyBackup](https://luckybackup.sourceforge.net)

![](/img/screen_create_01.png)

## Mappage et comparaison de catalogues

Pour aider à lister et comparer les répertoires sources et leur sauvegarde, Katalog peut aider à mapper les catalogues.

Cela suppose que l'utilisateur crée manuellement

### Créer un mappage

#### Exemple et catalogues
![](/img/screen_backup_1_devices.png)

#### Sélectionner la source
![](/img/screen_backup_2_select_source.png)

#### Sélectionner la cible
![](/img/screen_backup_3_select_target.png)

#### Nommer et créer
![](/img/screen_backup_4_create_mapping.png)

#### Comparer la source et la cible de sauvegarde
![](/img/screen_backup_5_comparison.png)

### Supprimer un mappage
Sélectionnez la ligne entière en cliquant sur le numéro de ligne (à gauche du tableau),<br/>
Et cliquez sur le bouton "Supprimer la sélection".

![](/img/screen_backup_6_delete.png)

## Développement
Quelques idées de développements pour cet écran :
* Opérations de catalogage depuis cet écran (mise à jour, recherche des différences, etc.)
* Copie de fichiers de base
* Pour en savoir plus, consultez le backlog de [développement de BackUp](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=BackUp).
