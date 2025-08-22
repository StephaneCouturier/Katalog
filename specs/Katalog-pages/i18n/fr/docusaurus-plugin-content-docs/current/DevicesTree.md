# Périphériques : Virtuels & arborescence

## Résumé
Cette page décrit toutes les fonctionnalités de la vue **Arborescence des périphériques** de l'écran [Périphériques](Devices) et comment les utiliser.
* Un périphérique de stockage **virtuel** est un regroupement, non-physique, et utilisé pour lier des catalogues à partir de différents périphériques physiques.
* Leur utilisation peut aider à effectuer une [Recherche](Search) ou à obtenir des [Statistiques](Statistics) indépendamment des appareils physiques.
![](/img/devices_tree_01.png)
## Options de la barre supérieure

### Option d'affichage
* bouton ![](/img/device_tree_button_apply.png): Appliquer les options à l'arborescence du périphérique de sélection. Cela permet de créer une arborescence de périphériques plus simple et limitée dans le panneau [Sélection] (Sélection).
* **Groupe Physique** : Affichez le *Groupe physique* et ses périphériques associés.
* **Groupes virtuels** : Affichez les *Groupes virtuels* et leurs périphériques associés.
* **Stockage** : Affichez les périphériques *Stockage* (si décoché, les catalogues seraient également masqués).
* **Catalogues** : Affichez les appareils *Catalogue*.

### Boutons d'actions
* **Insérer un groupe virtuel**: Créez et insérez un nouveau périphérique de groupe virtuel en haut de la hiérarchie, puis ouvrez le panneau Modifier.
* **Ajouter un Virtuel**: Créez et insérez un nouveau périphérique virtuel sous le périphérique sélectionné dans la hiérarchie, puis ouvrez le panneau Modifier.

## Menu contextuel (clic droit)
### Création de périphériques
| Cas | Entrée de menu |Résultat|
| ------------| -------------------------------------------------- |--------------------------------------------------|
| Groupe physique / périphérique virtuel | Ajouter un périphérique virtuel |Un périphérique virtuel est créé sous le périphérique sélectionné|
| Groupe physique / périphérique virtuel | Ajouter un périphérique de stockage |Un périphérique de stockage est créé sous le périphérique sélectionné |

### Affectation du catalogue dans les groupes virtuels
| Cas | Entrée de menu |Résultat |
| ------------| -------------------------------------------------- |--------------------------------------------------|
| Groupe virtuel / périphérique virtuel | Attribuer le catalogue sélectionné | Si un catalogue est sélectionné dans le panneau [Sélection](Selection), il est attribué au périphérique sélectionné |
| Groupe virtuel / périphérique de catalogue | Annuler l'attribution de ce catalogue | Le catalogue sélectionné est retiré du périphérique virtuel sélectionné (le catalog lui même n'est pas effacé) |
### Création d'arborescence de stockage virtuel et d'éléments
* Chaque élément peut être renommé à l'aide du bouton *Modifier*
* Chaque élément peut être supprimé, tant qu'il n'a pas de sous-éléments ni catalogue attribué.
