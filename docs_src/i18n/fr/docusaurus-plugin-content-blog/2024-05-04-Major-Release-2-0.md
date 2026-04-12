---
slug: release-2-0
title: Major Release 2.0
authors:
  - StephaneCouturier
tags: [release]
---
# 2024-05-04 Version Majeure 2.0
Après avoir publié 1 version de Katalog chaque mois et comme aucun bug majeur n'avait été signalé, les versions ont été suspendues depuis juillet dernier pour permettre de travailler sur cette prochaine grande version.

Voici enfin la release 2.0 : [Release Notes](https://github.com/StephaneCouturier/Katalog/releases/tag/v2.0)

<!-- truncate -->

Ce développement s'est concentré sur 2 principaux catalyseurs :
- **Périphériques** : les listes de périphériques virtuels, de stockage et de catalogue sont désormais toutes accessibles à partir d'un seul onglet fournissant l'équivalent sur les 3 onglets précédents. Il offre la flexibilité de créer une **hiérarchie globale d'appareils** pour améliorer les fonctionnalités de recherche ou de statistiques.

- Les **données** peuvent désormais être gérées dans un **fichier de base de données** (format SQLite), tout en conservant la possibilité de conserver les fichiers CSV traditionnels. Cela permettra également une future option pour héberger la base de données sur un serveur.

Ces changements impliquaient des mises à jour majeures du modèle de données et de la gestion des données.<br/>
Par conséquent, la version prend en charge une procédure de migration 1.x > 2.0.

&#9888; **Sauvegardez auparavant les données de votre collection Katalog créées avec les versions précédentes**. &#9888;

Pour en savoir plus sur ces principaux changements, explorez les pages *Périphériques* et *Paramètres* de la Documentation.

Tous les commentaires via Facebook, Github, e-mail sont toujours appréciés 😉
