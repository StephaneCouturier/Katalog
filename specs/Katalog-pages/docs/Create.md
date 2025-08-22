# Create
## Summary
This page describes all the features of the **Create** screen and how to use them.<br/>
From this screen, the user can **create a catalog of files**.<br/>
It is done in 3 main steps:
1. Select the Source path: the device or directory with the files to be included in the new catalog.
1. Select options to include or exclude some particular files.
1. Select the [Storage](DevicesStorage) and define the catalog name, and create the catalog.

![](/img/screen_create_01.png)

## Select the Source path
There are 3 ways to  select the source path of the directory with the files to be included in the new [Catalog](DevicesCatalogs):
1. by typing the path in the text edit zone
1. by using the treeview of the file system, just expand and click on the right device or directory
1. or by clicking the button *Select* which will open a dialog window to help selecting the folder.

The selected path will always appear in the text edit zone, and the application will use this path to browse and catalog its contents.

## Select options to include/exclude files
### include File Type
The contents can be limited to a particular type of files, 4 are available and will include files with the extensions as listed here:
        | Type  | Extensions                                        |
        | ------| --------------------------------------------------|
        | Audio | aif, mp3, ogg, wav                                |
        | Image | png, jpg, gif, xcf, tif, bmp, raw                 |
        | Text  | txt, pdf, odt, idx, html, rtf, doc, docx, epub    |
        | Video | wmv, avi, mp4, mkv, flv, webm, m4v, vob, ogv, mov |

This option will be applicable for the catalog moving forward.<br/>
It can be changed later by editing the [Catalog](DevicesCatalogs).

### Other options:
#### Include Hidden files
Hidden files are not included by default, but this options enables to include them.<br/>
This option will be applicable for the catalog moving forward.<br/>
It can be changed later by editing the [Catalog](DevicesCatalogs).

#### Exclude directories
It is possible to exclude entire directories from being cataloged.<br/>
Enter the path of the directory and by clicking on the button *Add directory to exclude from catalogs*.<br/>
The directory is then visible in the list below.<br/>
Any directory can be removed by a right click and  then visible in the list below.<br/>
Note: these exclusions are **global**, which means that these folders would be excluded for all catalogs.<br/>

![](/img/screen_create_04_exclude.png)

## Define & Create the catalog
#### Select the Storage device
A Catalog shall be associated with a [Storage](DevicesStorage) physical device, to facilitate later search or enable statistics.<br/>
By default, Katalog pre-creates a default Storage device, the local disk.<br/>
This can be updated later in the [Devices](DevicesTree) virtual tree screen.<br/>
If you need a different and new Storage for this catalog, click *Add Storage*, and add one using the [Devices](DevicesTree) or the [Storage](DevicesStorage) screens.

This choice will be applicable for the catalog moving forward.<br/>
It can be changed later by editing the [Catalog](DevicesCatalogs).

#### Enter a Name
Enter a name for your catalog.<br/>
Duplicate names are currently not allowed.

The button *Generate* can create a name based on the folder path, replacing slashes <code>/</code> by underscore <code>_</code>.

#### Create the catalog
When ready, click the button *Create Catalog* to save the catalog itself and start the process of cataloging the contents of the path recursively (all sub-directories will be included).

Once the process is completed,
- A message confirms the creation and provides the number of files and total file size of the selected folder for this catalog.
- your local drive, (a Storage device which was added automatically) was also updated, and the message provides a view of free, used, and total space:

![](/img/screen_create_02.png)

the [Devices](DevicesTree) screen will be display to show the Catalog in the device tree.

The new catalog is automatically selected in the [Selection](Selection) panel, ready to be used to [Search](Search) for the contents.

## Development
Some ideas of developments for this screen:
* to customize file types and/or use mimetypes
* exclude folders by catalog (not only globally)
* exclude folders by the name (not need to put the full path)
* For more, see the backlog of [Create development](https://github.com/users/StephaneCouturier/projects/7/views/1?filterQuery=create).
