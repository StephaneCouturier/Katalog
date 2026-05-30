---
version: "2.12"
---
# Statistics
![2.12](https://img.shields.io/badge/Version-2.12-blue)

## Summary
This page describes all the features of the **Statistics** screen and how to use them.

All data come from records of various updates or *snapshots*.

This screen provides views of the contents and the evolution of a collection: 
1. For catalog devices: the number of files, or the total file size.
1. For Storage devices: used and total space, and the related catalogs' total file size, or the number of files.
1. For Virtual devices: related devices number of files, or total space and the related catalogs' total file size.

![Statistics screen showing chart of collection evolution](/img/screen_statistics_01.png)

## Features
### Data option
* The data is based on the selected device from the [Selection](Selection) panel.
* *Source*: choose if all data should be used, or updates and snapshots idenpendantly.
* *Type of data*: choose to display total file size (which may include device used and total space), or number of files only.
* *Display each value*: choose to display a small diamond for each data point

### Edit File
* available in [Memory mode](Settings#database-memory-mode) only.
* *Edit Statistics* button: It can be usefull to edit the Statistics file to correct some numbers, the button  will open the file in the application associated with csv files. Be careful to keep it a **tab separated file**
* *Reload* button: the data is reloaded and the graph refreshed.

### Graph
* left click and hold to zoom on a part of the graph.
* right click to zoom out.
* click the button *Reload* to come back to the original zoom.

## Development
* See the backlog of [Statistics development](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=statistics).
