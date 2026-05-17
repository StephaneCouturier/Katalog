import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

// Right panel of the Explore page: file and folder list for the selected folder.
Item {
    id: root

    property int    catalogId:         0
    property string currentFolderPath: ""
    property string catalogPath:       ""
    property string catalogName:       ""
    property bool   showFolders:       true
    property bool   showSubFolders:    false

    signal folderNavigated(string folderPath)

    function loadEntries() {
        if (currentFolderPath === "") return
        fileListModel.clear()
        var entries = appManager1.getExploreEntries(currentFolderPath, showFolders, showSubFolders)
        for (var i = 0; i < entries.length; i++) {
            fileListModel.append(entries[i])
        }
    }

    function iconForType(fileType, entryType) {
        if (entryType === "folder") return "folder"
        switch (fileType) {
            case "audio":  return "audio-x-mpeg"
            case "image":  return "image-jpeg"
            case "video":  return "video-mp4"
            case "text":   return "view-list-text"
            case "other":  return "document-open"
            default:       return "application-x-zerosize"
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Header: current folder path + options
        RowLayout {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Icon {
                source: "folder"
                implicitWidth:  Kirigami.Units.iconSizes.small
                implicitHeight: Kirigami.Units.iconSizes.small
            }
            Controls.Label {
                text: root.currentFolderPath !== "" ? root.currentFolderPath : qsTr("No folder selected")
                elide: Text.ElideLeft
                Layout.fillWidth: true
                font.italic: true
                color: Kirigami.Theme.disabledTextColor
            }

            Controls.CheckBox {
                id: showFoldersCheck
                text: qsTr("Folders")
                checked: root.showFolders
                onCheckedChanged: {
                    root.showFolders = checked
                    if (!checked) {
                        root.showSubFolders = false
                        showSubFoldersCheck.checked = false
                    }
                    root.loadEntries()
                }
            }

            Controls.CheckBox {
                id: showSubFoldersCheck
                text: qsTr("Sub-folders")
                checked: root.showSubFolders
                enabled: root.showFolders
                onCheckedChanged: {
                    root.showSubFolders = checked
                    root.loadEntries()
                }
            }
        }

        Kirigami.Separator { Layout.fillWidth: true }

        // File list header row
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin:  Kirigami.Units.smallSpacing
            Layout.rightMargin: Kirigami.Units.smallSpacing
            spacing: 0

            Controls.Label {
                text: qsTr("Name")
                font.bold: true
                Layout.preferredWidth: 260
                Layout.minimumWidth:   120
            }
            Controls.Label {
                text: qsTr("Size")
                font.bold: true
                Layout.preferredWidth: 90
                horizontalAlignment: Text.AlignRight
            }
            Controls.Label {
                text: qsTr("Date")
                font.bold: true
                Layout.preferredWidth: 150
                Layout.leftMargin: Kirigami.Units.smallSpacing
            }
            Controls.Label {
                text: qsTr("Directory")
                font.bold: true
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.smallSpacing
                elide: Text.ElideRight
            }
        }

        Kirigami.Separator { Layout.fillWidth: true }

        // File/folder list
        Controls.ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: fileListView
                model: ListModel { id: fileListModel }

                delegate: Item {
                    id: fileDelegate
                    width:  fileListView.width
                    height: Kirigami.Units.gridUnit * 1.6

                    readonly property string dName:       model.name        ?? ""
                    readonly property var    dSize:       model.size        ?? 0
                    readonly property string dDate:       model.dateUpdated ?? ""
                    readonly property string dFolderPath: model.folderPath  ?? ""
                    readonly property string dFullPath:   model.fullPath    ?? ""
                    readonly property string dEntryType:  model.entryType   ?? ""
                    readonly property string dFileType:   model.fileType    ?? ""
                    readonly property string dChecksum:   model.checksumSha256 ?? ""
                    readonly property int    dIndex:      index

                    Rectangle {
                        anchors.fill: parent
                        color: fileListView.currentIndex === index
                               ? Kirigami.Theme.highlightColor
                               : (index % 2 === 0 ? "transparent" : Qt.alpha(Kirigami.Theme.alternateBackgroundColor, 0.4))
                    }

                    RowLayout {
                        anchors { fill: parent; leftMargin: Kirigami.Units.smallSpacing; rightMargin: Kirigami.Units.smallSpacing }
                        spacing: Kirigami.Units.smallSpacing

                        Kirigami.Icon {
                            source: root.iconForType(fileDelegate.dFileType, fileDelegate.dEntryType)
                            implicitWidth:  Kirigami.Units.iconSizes.small
                            implicitHeight: Kirigami.Units.iconSizes.small
                        }

                        Controls.Label {
                            text: fileDelegate.dName
                            elide: Text.ElideRight
                            Layout.preferredWidth: 250
                            Layout.minimumWidth:   80
                            color: fileListView.currentIndex === index
                                   ? Kirigami.Theme.highlightedTextColor
                                   : Kirigami.Theme.textColor
                        }

                        Controls.Label {
                            text: fileDelegate.dEntryType === "file"
                                  ? appManager1.formatDataSize(fileDelegate.dSize) : ""
                            Layout.preferredWidth: 90
                            horizontalAlignment: Text.AlignRight
                            color: fileListView.currentIndex === index
                                   ? Kirigami.Theme.highlightedTextColor
                                   : Kirigami.Theme.disabledTextColor
                        }

                        Controls.Label {
                            text: fileDelegate.dDate
                            Layout.preferredWidth: 150
                            elide: Text.ElideRight
                            color: fileListView.currentIndex === index
                                   ? Kirigami.Theme.highlightedTextColor
                                   : Kirigami.Theme.disabledTextColor
                        }

                        Controls.Label {
                            text: fileDelegate.dEntryType === "file" ? fileDelegate.dFolderPath : ""
                            elide: Text.ElideLeft
                            Layout.fillWidth: true
                            color: fileListView.currentIndex === index
                                   ? Kirigami.Theme.highlightedTextColor
                                   : Kirigami.Theme.disabledTextColor
                        }
                    }

                    // Single click to select
                    TapHandler {
                        onTapped: {
                            fileListView.currentIndex = fileDelegate.dIndex
                            if (fileDelegate.dEntryType === "folder") {
                                root.folderNavigated(fileDelegate.dFullPath)
                            }
                        }
                    }

                    // Double-click to open file
                    TapHandler {
                        onDoubleTapped: {
                            if (fileDelegate.dEntryType === "file") {
                                appManager1.openFile(fileDelegate.dFullPath)
                            }
                        }
                    }

                    // Right-click context menu
                    TapHandler {
                        acceptedButtons: Qt.RightButton
                        onTapped: {
                            fileListView.currentIndex = fileDelegate.dIndex
                            fileContextMenu.popup()
                        }
                    }

                    Controls.Menu {
                        id: fileContextMenu

                        Controls.MenuItem {
                            text: fileDelegate.dEntryType === "folder" ? qsTr("Open folder") : qsTr("Open file")
                            icon.name: "document-open"
                            onTriggered: {
                                if (fileDelegate.dEntryType === "folder")
                                    appManager1.openFolder(fileDelegate.dFullPath)
                                else
                                    appManager1.openFile(fileDelegate.dFullPath)
                            }
                        }

                        Controls.MenuItem {
                            text: qsTr("Open folder")
                            icon.name: "document-open-data"
                            visible: fileDelegate.dEntryType === "file"
                            height: visible ? implicitHeight : 0
                            onTriggered: appManager1.openFolder(fileDelegate.dFolderPath)
                        }

                        Controls.MenuSeparator {}

                        Controls.MenuItem {
                            text: qsTr("Copy folder path")
                            icon.name: "edit-copy"
                            onTriggered: appManager1.copyToClipboard(fileDelegate.dFolderPath)
                        }

                        Controls.MenuItem {
                            text: qsTr("Copy absolute path")
                            icon.name: "edit-copy"
                            visible: fileDelegate.dEntryType === "file"
                            height: visible ? implicitHeight : 0
                            onTriggered: appManager1.copyToClipboard(fileDelegate.dFullPath)
                        }

                        Controls.MenuItem {
                            text: qsTr("Copy file name")
                            icon.name: "edit-copy"
                            visible: fileDelegate.dEntryType === "file"
                            height: visible ? implicitHeight : 0
                            onTriggered: appManager1.copyToClipboard(fileDelegate.dName)
                        }

                        Controls.MenuItem {
                            text: qsTr("Copy file name without extension")
                            icon.name: "edit-copy"
                            visible: fileDelegate.dEntryType === "file"
                            height: visible ? implicitHeight : 0
                            onTriggered: {
                                var n = fileDelegate.dName
                                var dot = n.lastIndexOf(".")
                                appManager1.copyToClipboard(dot > 0 ? n.substring(0, dot) : n)
                            }
                        }

                        Controls.MenuItem {
                            text: qsTr("Copy file checksum")
                            icon.name: "edit-copy"
                            visible: fileDelegate.dEntryType === "file" && fileDelegate.dChecksum !== ""
                            height: visible ? implicitHeight : 0
                            onTriggered: appManager1.copyToClipboard(fileDelegate.dChecksum)
                        }

                        Controls.MenuSeparator {
                            visible: fileDelegate.dEntryType === "file"
                            height: visible ? implicitHeight : 0
                        }

                        Controls.MenuItem {
                            text: qsTr("Move file to Trash")
                            icon.name: "user-trash"
                            visible: fileDelegate.dEntryType === "file"
                            height: visible ? implicitHeight : 0
                            onTriggered: {
                                if (appManager1.moveFileToTrash(fileDelegate.dFullPath))
                                    fileListModel.remove(fileDelegate.dIndex)
                            }
                        }

                        Controls.MenuItem {
                            text: qsTr("Move folder to Trash")
                            icon.name: "user-trash"
                            visible: fileDelegate.dEntryType === "folder"
                            height: visible ? implicitHeight : 0
                            onTriggered: {
                                if (appManager1.moveFileToTrash(fileDelegate.dFullPath))
                                    fileListModel.remove(fileDelegate.dIndex)
                            }
                        }

                        Controls.MenuItem {
                            text: qsTr("Delete file")
                            icon.name: "edit-delete"
                            visible: fileDelegate.dEntryType === "file"
                            height: visible ? implicitHeight : 0
                            onTriggered: {
                                if (appManager1.deleteSingleFile(fileDelegate.dFullPath))
                                    fileListModel.remove(fileDelegate.dIndex)
                            }
                        }
                    }
                }
            }
        }
    }

    onCurrentFolderPathChanged: loadEntries()
}
