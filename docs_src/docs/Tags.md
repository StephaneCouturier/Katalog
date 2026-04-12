---
version: "2.11"
---
# Tags
![2.11](https://img.shields.io/badge/Version-2.11-blue)

## Summary
This page describes all the features of the **Tags** screen and how to use them.<br/>
From this screen, the user can **assign several tags to any folder**.<br/>
This can then be used in the [Search](Search) screen to refine the results;<br/>

Tagging a folder is done in 3 main steps:
1. Select the Source path of the folder,
1. Select or create a new Tag,
1. Click to Tag the Folder.

![Tags screen showing tag assignment interface](/img/screen_tags_01.png)

## Assign Tags to directories
Note: Tag are assigned to an absolute folder path, therefore it is independent of devices hierarchy and Catalogs.

### Select the Source path
There are 3 ways to  select the source path of the folder with the files to be included in the new catalog:
1. by typing the path in the text edit zone
1. by using the treeview of the file system, just expand and click on the right device or folder
1. or by clicking the button *Select* which will open a dialog window to help selecting the folder.

The selected path will always appear in the text edit zone, and the application will use this path for the tag.

### Select an existing tag or create one
* type a new Tag name in the edit zome under "Select a tag"
* or click on any item of the Existing tags list to re-use an existing one

### Assign the tag to the selected folder
Click on the button "Tag the Folder".<br/>
This will record the association between the folder and the tag name.<br/>
The list "Current folders and tags" is refresh with the new entry.

## Modifying Tags
### Delete existing associations
From the list "Current folders and tags", right click on the association to be deleted and selected *Remove this tag*.
### Edit the Tags file
In *Memory* mode, the button *Open file* can be used to edit the source file directly (the format should not be changed).


## Development
Some ideas of developments for this screen:
* Tag devices
* Tag folders in specific catalogs
* For more, see the backlog of [Tags developments](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=tag).

