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
    property int    folderFileCount:   0
    property var    folderTotalSize:   0

    // Active file state for checksum dialogs
    property string _activeFilePath:   ""
    property string _activeFileName:   ""
    property string _activeFolderPath: ""
    property string _activeChecksum:   ""

    signal folderNavigated(string folderPath)

    function loadEntries() {
        if (currentFolderPath === "") return
        fileListModel.clear()
        var entries = appManager1.getExploreEntries(currentFolderPath, showFolders, showSubFolders)
        for (var i = 0; i < entries.length; i++) {
            fileListModel.append(entries[i])
        }
        var stats = appManager1.getExploreFolderStats(currentFolderPath)
        root.folderFileCount = stats.fileCount
        root.folderTotalSize = stats.totalSize
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

        // Files count and total size row
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin:  Kirigami.Units.smallSpacing
            Layout.rightMargin: Kirigami.Units.smallSpacing
            Layout.topMargin:   Kirigami.Units.smallSpacing
            Layout.bottomMargin: Kirigami.Units.smallSpacing
            spacing: Kirigami.Units.smallSpacing

            Controls.Label { text: qsTr("Number of Files") }
            Controls.Label {
                text: root.folderFileCount.toLocaleString(Qt.locale(), "f", 0)
                font.bold: true
            }
            Item { Layout.preferredWidth: Kirigami.Units.gridUnit }
            Controls.Label { text: qsTr("Total Size") }
            Controls.Label {
                text: appManager1.formatDataSize(root.folderTotalSize) || "0"
                font.bold: true
            }
            Item { Layout.fillWidth: true }
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
            Controls.ScrollBar.vertical.policy:   Controls.ScrollBar.AsNeeded
            Controls.ScrollBar.horizontal.policy: Controls.ScrollBar.AsNeeded

            ListView {
                id: fileListView
                model: ListModel { id: fileListModel }
                contentWidth: Math.max(width, Kirigami.Units.gridUnit * 44)

                delegate: Item {
                    id: fileDelegate
                    width:  Math.max(fileListView.width, Kirigami.Units.gridUnit * 44)
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
                            root._activeFilePath   = fileDelegate.dFullPath
                            root._activeFileName   = fileDelegate.dName
                            root._activeFolderPath = fileDelegate.dFolderPath
                            root._activeChecksum   = fileDelegate.dChecksum
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
                            text: qsTr("Copy folder name")
                            icon.name: "edit-copy"
                            visible: fileDelegate.dEntryType === "folder"
                            height: visible ? implicitHeight : 0
                            onTriggered: appManager1.copyToClipboard(fileDelegate.dName)
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
                            text: qsTr("Copy Checksum")
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
                            text: qsTr("Calculate Checksum (SHA-256)")
                            icon.name: "document-properties"
                            visible: fileDelegate.dEntryType === "file" && fileDelegate.dChecksum === ""
                            height: visible ? implicitHeight : 0
                            onTriggered: {
                                var cksum = appManager1.calculateAndSaveChecksum(
                                    root._activeFilePath,
                                    root._activeFileName,
                                    root._activeFolderPath,
                                    root.catalogId)
                                if (cksum.length > 0) {
                                    root._activeChecksum = cksum
                                    exploreChecksumResultDialog.checksumText = cksum
                                    exploreChecksumResultDialog.wasSaved = root.catalogId > 0
                                    exploreChecksumResultDialog.open()
                                } else {
                                    showPassiveNotification(qsTr("File not found or could not be read"))
                                }
                            }
                        }

                        Controls.MenuItem {
                            text: qsTr("Verify Checksum (SHA-256)")
                            icon.name: "document-properties"
                            visible: fileDelegate.dEntryType === "file" && fileDelegate.dChecksum !== ""
                            height: visible ? implicitHeight : 0
                            onTriggered: {
                                var result = appManager1.verifyFileChecksum(
                                    root._activeFilePath,
                                    root._activeChecksum)
                                if (result === "match") {
                                    exploreChecksumResultDialog.checksumText = root._activeChecksum
                                    exploreChecksumResultDialog.wasSaved = false
                                    exploreChecksumResultDialog.open()
                                } else if (result.startsWith("mismatch:")) {
                                    exploreChecksumMismatchDialog.expectedChecksum = root._activeChecksum
                                    exploreChecksumMismatchDialog.actualChecksum   = result.substring(9)
                                    exploreChecksumMismatchDialog.open()
                                } else {
                                    showPassiveNotification(qsTr("Error: ") + result.substring(6))
                                }
                            }
                        }

                        Controls.MenuSeparator {}

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

    // ── Checksum result dialog ────────────────────────────────────────────
    Kirigami.Dialog {
        id: exploreChecksumResultDialog
        property string checksumText: ""
        property bool   wasSaved: false
        title: qsTr("Checksum (SHA-256)")
        standardButtons: Kirigami.Dialog.Close
        preferredWidth: Kirigami.Units.gridUnit * 36
        padding: Kirigami.Units.largeSpacing

        contentItem: ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            Controls.Label {
                text: "SHA-256: " + exploreChecksumResultDialog.checksumText
                wrapMode: Text.WrapAnywhere
                Layout.fillWidth: true
                font.family: "monospace"
            }
            Controls.Label {
                text: exploreChecksumResultDialog.wasSaved
                      ? qsTr("Checksum saved to database.")
                      : qsTr("Checksums match.")
                opacity: 0.7
            }
            Controls.Button {
                text: qsTr("Copy to Clipboard")
                icon.name: "edit-copy"
                onClicked: {
                    appManager1.copyToClipboard(exploreChecksumResultDialog.checksumText)
                    showPassiveNotification(qsTr("Checksum copied to clipboard"))
                }
            }
        }
    }

    // ── Checksum mismatch dialog ──────────────────────────────────────────
    Kirigami.Dialog {
        id: exploreChecksumMismatchDialog
        property string expectedChecksum: ""
        property string actualChecksum:   ""
        title: qsTr("Checksum Mismatch")
        standardButtons: Kirigami.Dialog.Cancel
        preferredWidth: Kirigami.Units.gridUnit * 36
        padding: Kirigami.Units.largeSpacing

        customFooterActions: [
            Kirigami.Action {
                text: qsTr("Update Checksum")
                icon.name: "document-save"
                onTriggered: {
                    appManager1.calculateAndSaveChecksum(
                        root._activeFilePath,
                        root._activeFileName,
                        root._activeFolderPath,
                        root.catalogId)
                    root._activeChecksum = exploreChecksumMismatchDialog.actualChecksum
                    showPassiveNotification(qsTr("Checksum saved to database"))
                    exploreChecksumMismatchDialog.close()
                }
            }
        ]

        contentItem: ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            Controls.Label { text: qsTr("Checksums do not match."); font.bold: true }
            Controls.Label { text: qsTr("Expected:"); opacity: 0.7 }
            Controls.Label {
                text: exploreChecksumMismatchDialog.expectedChecksum
                wrapMode: Text.WrapAnywhere
                Layout.fillWidth: true
                font.family: "monospace"
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.85
            }
            Controls.Label { text: qsTr("Actual:"); opacity: 0.7 }
            Controls.Label {
                text: exploreChecksumMismatchDialog.actualChecksum
                wrapMode: Text.WrapAnywhere
                Layout.fillWidth: true
                font.family: "monospace"
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.85
            }
        }
    }
}
