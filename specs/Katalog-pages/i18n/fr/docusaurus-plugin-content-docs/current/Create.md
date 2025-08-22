# Créer
## Résumé
Cette page décrit toutes les fonctionnalités de l'écran **Créer** et comment les utiliser.<br/>
Depuis cet écran, l'utilisateur peut **créer un catalogue de fichiers**.<br/>
Cela se fait en 3 étapes principales :
1. Sélectionnez le chemin source : le périphérique ou le répertoire contenant les fichiers à inclure dans le nouveau [Catalogue](DevicesCatalogs).
1. Sélectionnez les options pour inclure ou exclure certains fichiers.
1. Sélectionnez le [Stockage](DevicesStorage) et définissez le nom du catalogue, et créez le catalogue.

![](/img/screen_create_01.png)

## Sélectionnez le chemin source
Il existe 3 manières de sélectionner le chemin source du répertoire contenant les fichiers à inclure dans le nouveau catalogue :
1. en tapant le chemin dans la zone d'édition de texte
1. en utilisant l'arborescence du système de fichiers, développez simplement et cliquez sur le bon périphérique ou répertoire
1. ou en cliquant sur le bouton *Sélectionner* qui ouvrira une fenêtre de dialogue d'aide à la sélection du dossier.

Le chemin sélectionné apparaîtra toujours dans la zone d'édition de texte et l'application utilisera ce chemin pour parcourir et cataloguer son contenu.

## Sélectionnez les options pour inclure/exclure des fichiers
### inclure le type de fichier
Le contenu peut être limité à un type particulier de fichiers, 4 sont disponibles et incluront des fichiers avec les extensions répertoriées ici :
        | Type  | Extensions                                        |
        | ------| --------------------------------------------------|
        | Audio | aif, mp3, ogg, wav                                |
        | Image | png, jpg, gif, xcf, tif, bmp, raw                 |
        | Texte | txt, pdf, odt, idx, html, rtf, doc, docx, epub    |
        | Vidéo | wmv, avi, mp4, mkv, flv, webm, m4v, vob, ogv, mov |

Cette option sera applicable pour le catalogue à venir.<br/>
Il peut être modifié ultérieurement en éditant le [Catalogue](DevicesCatalogs).

### Autres options:
#### Inclure les fichiers cachés
Les fichiers cachés ne sont pas inclus par défaut, mais cette option permet de les inclure.<br/>
Cette option sera applicable pour le catalogue à venir.<br/>
Il peut être modifié ultérieurement en éditant le [Catalogue](DevicesCatalogs).

#### Exclure les répertoires
Il est possible d'exclure des répertoires entiers du catalogage.<br/>
Saisissez le chemin du répertoire et en cliquant sur le bouton *Ajouter un répertoire à exclure des catalogues*.<br/>
Le répertoire est alors visible dans la liste ci-dessous.<br/>
N'importe quel répertoire peut être supprimé par un clic droit puis visible dans la liste ci-dessous.<br/>
Remarque : ces exclusions sont **globales**, ce qui signifie que ces dossiers seraient exclus de tous les catalogues.<br/>

![](/img/screen_create_04_exclude.png)

## Définir et créer le catalogue
#### Sélectionnez le périphérique de stockage
Un catalogue doit être associé à un périphérique physique [Stockage](DevicesStorage), pour faciliter la recherche ultérieure ou activer les statistiques.<br/>
Par défaut, Katalog pré-crée un périphérique de stockage par défaut, le disque local.<br/>
Cela peut être mis à jour ultérieurement dans l'écran de l'[Arborescence](DevicesTree) des périphériques.<br/>
Si vous avez besoin d'un stockage nouveau et différent pour ce catalogue, cliquez sur *Ajouter un stockage* et ajoutez-en un à l'aide des écrans [Arborescence](DevicesTree) ou [Stockage](DevicesStorage).

Ce choix sera applicable pour le catalogue à venir.<br/>
Il peut être modifié ultérieurement en éditant le [Catalogue] (DevicesCatalogs).

#### Entrez un nom
Saisissez un nom pour votre catalogue.<br/>
Les noms en double ne sont actuellement pas autorisés.

Le bouton *Générer* permet de créer un nom basé sur le chemin du dossier, en remplaçant les barres obliques <code>/</code> par le trait de soulignement <code>_</code>.

#### Créer le catalogue
Lorsque vous êtes prêt, cliquez sur le bouton *Créer un catalogue* pour enregistrer le catalogue lui-même et démarrer le processus de catalogage récursif du contenu du chemin (tous les sous-répertoires seront inclus).

Une fois le processus terminé,
- Un message confirme la création et fournit le nombre de fichiers et la taille totale des fichiers du dossier sélectionné pour ce catalogue.
- votre disque local (un périphérique de stockage qui a été ajouté automatiquement) a également été mis à jour et le message donne une vue de l'espace libre, utilisé et total :

![](/img/screen_create_02.png)

l'écran [Périphériques/Arborescence] (DevicesTree) s'affichera pour afficher le catalogue dans l'arborescence des périphériques.

Le nouveau catalogue est automatiquement sélectionné dans le panneau [Sélection](Selection), prêt à être utilisé pour [Rechercher](Search) le contenu.

## Développement
Quelques idées d'évolutions pour cet écran :
* pour personnaliser les types de fichiers et/ou utiliser des types MIME
* exclure les dossiers par catalogue (pas seulement globalement)
* exclure les dossiers par leur nom (pas besoin de mettre le chemin complet)
* Pour en savoir plus, consultez la liste de [développements](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=create).
