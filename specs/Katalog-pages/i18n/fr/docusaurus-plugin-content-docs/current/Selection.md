---
version: "2.10"
---
# Sélection
![2.10](https://img.shields.io/badge/Version-2.10-blue)

## Résumé
Cette page décrit toutes les fonctionnalités du panneau **Sélection**, la partie gauche de l'interface utilisateur.
* Ce panneau permet d'affiner une sélection pour les différents écrans et fonctionnalités.
* Avec le mode *Recherche dans les catalogues de fichiers*, la sélection filtrera les informations pour les écrans [Rechercher](Search), [Périphériques](Devices), [Créer](Create) et [Statistiques](Statistics).
* Avec le mode *Recherche dans les lecteurs connectés*, un répertoire directement depuis les appareils connectés peut être sélectionné. Ceci est uniquement utilisé pour l'écran [Rechercher](Search).

![Panneau Sélection affichant l'arborescence des appareils et les contrôles de sélection](/img/selection_01.png)

## Interface
Boutons du haut :
* *Afficher/Masquer* : le bouton en haut à gauche permet de masquer et d'afficher à nouveau le panneau.
* *Réinitialiser* : ce bouton avec l'icône en forme de balai réinitialise la sélection actuelle, de sorte que tous les catalogues/données soient sélectionnés.
* *Recharger* : recharge l'intégralité des données de la collection, utile pour mettre à jour l'application suite à des modifications effectuées en dehors de celle-ci.

## Recherche dans les catalogues
Informations de sélection :
Cette section affiche la sélection actuelle de périphérique virtuel, de stockage ou de catalogue.

Les deux boutons situés à côté du libellé de l'arborescence des appareils permettent de réduire ou d'étendre l'arborescence d'un niveau.

### Menu contextuel (clic droit)

Le menu contextuel varie selon le type de périphérique sélectionné.

**Pour les périphériques de stockage et virtuels :**
* *Rechercher* : sélectionne l'élément et passe à l'écran de recherche (affiché uniquement si l'écran de recherche n'est pas déjà actif)
* *Mettre à jour* : déclenche la mise à jour (analyse des fichiers) du périphérique sélectionné et de tous ses catalogues

**Pour les périphériques de catalogue :**
* *Rechercher* : sélectionne l'élément et passe à l'écran de recherche (affiché uniquement si l'écran de recherche n'est pas déjà actif)
* *Mettre à jour* : déclenche la mise à jour (analyse des fichiers) du catalogue sélectionné (disponible uniquement si le catalogue est actif)
* *Explorer* : ouvre le catalogue sélectionné dans l'écran [Explorer](Explore)
* *Ouvrir le dossier* : ouvre le dossier source du catalogue dans le gestionnaire de fichiers du système (affiché uniquement si un chemin source est défini)

## Rechercher dans les lecteurs connectés
Avec cette option, une recherche peut être effectuée directement dans n'importe quel répertoire d'un lecteur connecté/monté, sans avoir besoin d'un catalogue.

Utiliser le bouton *Choisir un chemin* ou l'arborescence des répertoires pour sélectionner le dossier dans lequel effectuer la recherche.

![Panneau Sélection en mode lecteurs connectés affichant l'arborescence des répertoires et la sélection du chemin](/img/selection_02.png)
