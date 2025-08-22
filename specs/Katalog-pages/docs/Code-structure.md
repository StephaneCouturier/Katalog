# Code structure

## Summary
This page provides information about how the source code is organized and any common practice used to facilitate its understanding, maintenance, and evolution.

## Model and file structure
* For data handling, many *objects* support the links with the database:  collection, device, storage, catalog, search, tag
* Each tab / screen of Katalog is managed in a different cpp file, belonging to the mainwindow code.

## Code practice
* Comments, comments, comments.
* variables:  first word lower cap, all other starting with capital letter:    thisIsAVariableName.
* database fields: to help with compatibility between SQLite and Postgres, fields are named in lower case, words separated by underscore: this_is_a_fied_name

![](/img/code_structure.png)
