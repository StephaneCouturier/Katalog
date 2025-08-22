# Structure du code

## Résumé
Cette page fournit des informations sur la manière dont le code source est organisé et sur toute pratique courante utilisée pour faciliter sa compréhension, sa maintenance et son évolution.

## Modèle et structure des fichiers
* Pour le traitement des données, de nombreux *objets* supportent les liens avec la base de données : collection, périphérique, stockage, catalogue, recherche, tag
* Chaque onglet/écran de Katalog est géré dans un fichier cpp différent, appartenant au code de la fenêtre principale.

## Pratique du code
* Commentaires, commentaires, commentaires.
* variables : premier mot en majuscule, tous les autres commençant par une majuscule : ceciEstUneVariable.
* champs de base de données : pour faciliter la compatibilité entre SQLite et Postgres, les champs sont nommés en minuscules, les mots séparés par un trait de soulignement : ceci_est_un_nom_de_champs

![](/img/code_structure.png)
