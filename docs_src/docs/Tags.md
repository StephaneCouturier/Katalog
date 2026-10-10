---
version: "3.0"
---
# Tags
![3.0](https://img.shields.io/badge/Version-3.0-blue)

## Summary
This page describes the **Tags** page, where **tags are assigned to folders**: for example *Holidays* on the folders of holiday photos, wherever they are.<br/>
A tag can then be used in [Search](Search#tags-criteria) to find only the files of the folders with that tag.

![Tags page: add a tag, and the list of current folders and tags](/img/tags_01_overview.png)

A tag is assigned to a **folder path**, not to a catalog or a device: it applies to every catalog that contains this path.<br/>
A folder can have several tags, and a tag can be on several folders, but a folder has each tag only once.

## Add a tag {#add}
1. *Folder*: type the path of the folder, or choose it with *Select a folder*. It is filled in already when Tags was opened with *Tag this folder* in [Explore](Explore#tag-folder).
1. *Tag*: choose an existing tag in the list, or type a new name.
1. Click *Tag the folder*.

When the folder or the tag name is missing, or when the folder already has this tag, a message at the top of the page says so.<br/>
The new association appears in the list below.

## Current folders and tags {#list}
The list shows each folder with its tag.
* The list above the *Tag* column filters on one tag; *All* shows them all.
* The button at the end of a line (*Remove this tag*) removes that association. The folder and its files are not touched.

## Development
Some ideas of developments for this screen:
* Tag devices.
* Tag folders in specific catalogs.
* For more, see the backlog of [Tags developments](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=tag).
