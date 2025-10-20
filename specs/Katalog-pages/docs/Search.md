# Search
## Summary
This page describes all the features of the **Search** screen and how to use them.<br/>
The main features of this screen are:

* Search for files from the Catalogs or from connected drives
* Use multiple **criteria** to refine and reduce the number of results:
  * the Search will filter results based on the text you entered in the line _Text_ by default.
  * and can be refined with more [search text options](Search#search-text-criteria),
  * providing [file attributes](Search#file-criteria) such as size or type,
  * highlighting [duplicated files](Search#duplicates-on),
  * or highlighting [differences](Search#differences-on) between 2 catalogs.
* Use the **Results** with a [context menu](Search#file-context-menu) (right click) or [Export Results](Search#batch-process) them,
* Re-use previous criteria from the Search History.
* All search history and criteria are saved and available the next time Katalog is opened, except for the results themselves.
![](/img/screen_search_01.png)

## Search from the Catalogs or from connected drives
While Katalog is specifically designed to manage catalogs of files to search offline, it is still possible to search directly on connected (online) drives.
In the left panel, the tab "Filters" is split into 2 parts, only one can be activated:

### Search in file catalogs
Use 3 optional filters to limit the list of catalogs in which to search for files:
- _Location_: this is listing distinct locations as entered in the screen "Storage". Selecting one location reduces the list of Storage devices and Catalogs to the matching ones.
- _Storage_:  this is listing distinct storage devices as entered in the screen "Storage". Selecting one storage device reduces the list to the matching Catalogs.
- _Catalogs_: this narrows the search down to 1 specific catalog.

### Search in connected drives
Search directly on the computer and connected drives.
This can save time to search in a specific folder rather than an entire drove or catalog, or it can help to get a quick view after recent changes without having to update a catalog before.

## Search speed
Searching can be a very long process.
It depends first on the number of files in the collection, and already when it is over a milion files (which is what a freshly installed OS easily contains).
Then it can be a matter of computer and disk speed.

### Tips
Always try to uy


### Stopping a search

To enable stopping a search where a mistake was made or the selection is too big, is it possible to click on the same button "Search" which is displaying "Stop" during the search process.

This is only available when using the "File" database mode (the collection of catalogs is store in an SQLite database file).


## Search criteria
This panel groups all criteria that can refine and reduce the number of results. It can be hidden.<br/>
They apply to file path and names, file information, and tags, and can identify duplicates, or help see differences between 2 catalogs<br/>
Results can be a list of files, or a list of folders containing matching files.<br/>
![](/img/screen_search_02.png)
### Search Text criteria {#search-text-criteria}
#### File name
This area is the place to type the text to be used for the search on file name, which may include the file path.

Definition: Here a "word" is a group of characters separated from another group by a space character.<br/>
This can be used to find folders, files, or files in certain folders.<br/>
By default this is the only criteria selected<br/>
All criteria can be blanked/reset with the Reset button.

To facilitate the search of key words from a copied text:
- This text can be paste from the clipboard,
- and it can be cleaned from special characters: removed by this function:  .  ,  _  - (  )  [  ]  {  } 

When available, clicking the check box placed before a criteria will enable/disable the criteria, without losing the selected values.<br/>

#### With
Specify how the words typed in the "Text" should be used:
* _All Words_: a file or folder is returned only if all the words in the Text are found.
* _Begin With_: only for the "File names" option, the file name must start with the Text (including spaces between words).
* _Any word_: a file or folder is returned if at least one of the words in Text is found.

#### In
Specify in what part of the absolute file path the Text should be used.
* _File names only_:        the Text words will be used only to look into filenames.
* _Folder path only_:       the Text words will be used only to look into the folder paths.
* _Folder and File names_:  the Text words will be used to look both into the folder paths and the file names.

#### case sensitive
Force the search to match exact characters (small caps or capital letters).

#### exclude
As opposed to the "Text", this will exclude results if _any_ of the words provided is found in the file path or name.

### File criteria {#file-criteria}

#### Size
Set the range of file size by entering a number and unit for the minimum and maximum file size.

#### File Types

KFileMetadata Intelligence
File types are now dynamically detected using:

MIME Database Integration: Leverages system MIME type database
KFileMetadata Framework: Uses KDE's metadata extraction system
Intelligent Caching: Builds optimized type detection cache on startup
Extensible Detection: Automatically supports new formats as system updates

        | Type  | Extensions                                        |
        | ------| --------------------------------------------------|
        | Audio | aif, mp3, ogg, wav                                |
        | Image | png, jpg, gif, xcf, tif, bmp, raw                 |
        | Text  | txt, pdf, odt, idx, html, rtf, doc, docx, epub    |
        | Video | wmv, avi, mp4, mkv, flv, webm, m4v, vob, ogv, mov |

#### Dates
Set the range of file modified date.

#### Duplicates on {#duplicates-on}
Option to refine the results and only display the files that could be duplicates.<br/>
The notion of Duplicates can be defined as one or a combination of several parts:
the Name, the Size, the Date Modified.
At least one of these options is necessary to consider what could be duplicates.

#### Differences on {#differences-on}
Find differences between any 2 devices (Virtual, Storage, Catalogs) (useful to compare with a backup for instance).<br/>
As for Duplicates, it combines with other Search criteria and the notion of difference can be applied to Name, Size, or Date Modified.
![](/img/screen_search_03_diff.png)

### Folder criteria

#### Show folders only
Option to display folders only as results instead of the files.

#### Tags
Option to refine results on a selected _Tag_.
Tags are defined in the screen _Tags_. Currently only folders can be tagged.

## Results
The results of the search are displayed in this area.<br/>

### Catalogs with results
The left panel shows the list of Catalogs in which results where found.<br/>
A click on one of these Catalogs will trigger the same search but refined on the selected catalog.<br/>
This panel can be hidden to save space and display more file results information.<br/>

### Files (or Duplicates) found
The right panel shows the list of files or folder that where found mathcing the Search criteria.<br/>
The top part indicates the number of files found or the number of duplicates (number of unique results) and the total size of the files found. <br/>
The icon next opens a windows to see more statistics on the results:<br/>
Total size, min, max, average size, min, max date.<br/><br/>

### Batch process {#batch-process}
| Case             | Menu entry         |
| -----------------| -------------------|
| Export Results   |Export results to a new Catalog (with which a more precise search can be done) or export to a csv file, named with the date, and located in the [Collection folder](Settings#collection). |
| Rename (KRename) | Open all the files listed in the results with [KRename](https://apps.kde.org/krename/) |
| &#9888; Move to Trash | move all the files listed in the results to the trash can. |
| &#9888; Delete  | delete all the files listed in the results (no recovery).|

### File context menu {#file-context-menu}
Right clicking on a result line will open a context menu with the following options:
* _Open File_:  the default app of the computer will be used to open the file, if it is available/online.
* _Open Folder_: the default file manager of the computer will be used to open the folder and display its contents, if it is available/online.

Copy to the clipboard some file information, with the example of the full path of a file: _/home/user/documents/filename.txt_
* _Copy folder path_:                  _/home/user/documents_
* _Copy file absolute path_:           _/home/user/documents/filename.txt_
* _Copy file name with extension_:     _filename.txt_
* _Copy file name without extension_:  _filename_

File operations:
* _Move to Trash_
* _Delete file_

## Search History
Every time a Search is triggered, the Search criteria and selected Catalogs are saved to a history file located in the Collection folder.<br/>
This file is then loaded and displayed in this table.<br/>
A click on a line of the table will restore all criteria values to the Search criteria and allow to re-run the exact same search as before or facilitate some adaptation.<br/>
This panel can be hidden to save space and display more file results information.<br/>
![](/img/screen_search_04_search_history.png)
