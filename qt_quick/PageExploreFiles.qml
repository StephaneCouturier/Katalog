import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import QtQuick.Controls   // required for ScrollBar attached property (namespace alias prevents it)
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

    // Sort state
    property int  sortColumn:    -1
    property bool sortAscending: true

    // Active entry state for context menu and checksum dialogs
    property string _activeFilePath:   ""
    property string _activeFileName:   ""
    property string _activeFolderPath: ""
    property string _activeChecksum:   ""
    property string _activeEntryType:  ""

    signal folderNavigated(string folderPath)

    function loadEntries() {
        if (currentFolderPath === "") return
        exploreTableView.selectedRow = -1
        appManager1.loadExploreEntries(currentFolderPath, showFolders, showSubFolders)
        var stats = appManager1.getExploreFolderStats(currentFolderPath)
        root.folderFileCount = stats.fileCount
        root.folderTotalSize = stats.totalSize
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
            Layout.leftMargin:   Kirigami.Units.smallSpacing
            Layout.rightMargin:  Kirigami.Units.smallSpacing
            Layout.topMargin:    Kirigami.Units.smallSpacing
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

        // ── Column headers + Table ────────────────────────────────────────
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            Controls.HorizontalHeaderView {
                id: exploreHeaderView
                anchors { top: parent.top; left: parent.left; right: parent.right }
                syncView: exploreTableView
                clip: true
                implicitHeight: 34
                resizableColumns: true

                delegate: Rectangle {
                    required property int    column
                    required property string display
                    color: Kirigami.Theme.backgroundColor
                    implicitHeight: exploreHeaderView.implicitHeight

                    RowLayout {
                        anchors {
                            left: parent.left; right: sortIndicator.left
                            verticalCenter: parent.verticalCenter
                            leftMargin: 6; rightMargin: 2
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            text: display
                            elide: Text.ElideRight
                            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.9
                            color: Kirigami.Theme.textColor
                        }
                    }

                    Controls.Label {
                        id: sortIndicator
                        anchors { right: parent.right; verticalCenter: parent.verticalCenter; rightMargin: 4 }
                        text: root.sortColumn === column ? (root.sortAscending ? "▲" : "▼") : ""
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.7
                        color: Kirigami.Theme.textColor
                    }

                    Rectangle { // Vertical separator
                        anchors { top: parent.top; bottom: parent.bottom; right: parent.right }
                        width: 1
                        color: Kirigami.Theme.separatorColor ?? "transparent"
                    }

                    TapHandler {
                        onTapped: {
                            exploreTableView.selectedRow = -1
                            if (root.sortColumn === column) {
                                root.sortAscending = !root.sortAscending
                            } else {
                                root.sortColumn    = column
                                root.sortAscending = true
                            }
                            appManager1.sortExplore(root.sortColumn, root.sortAscending ? 0 : 1)
                        }
                    }
                }
            }

            Kirigami.Separator {
                id: exploreHeaderSep
                anchors { top: exploreHeaderView.bottom; left: parent.left; right: parent.right }
            }

            TableView {
                id: exploreTableView
                anchors { top: exploreHeaderSep.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }
                columnSpacing: 0
                rowSpacing: 0
                model: appManager1.exploreSortModel

                property int selectedRow: -1

                ScrollBar.vertical:   ScrollBar { policy: ScrollBar.AsNeeded }
                ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AsNeeded }

                rowHeightProvider: function(row) { return 30 }

                columnWidthProvider: function(column) {
                    let w = exploreTableView.explicitColumnWidth(column)
                    if (w >= 0) return w
                    switch (column) {
                        case 0: return 260  // Name
                        case 1: return 90   // Size
                        case 2: return 150  // Date
                        case 3: return 320  // Directory
                    }
                    return 100
                }

                delegate: Rectangle {
                    required property int    row
                    required property int    column
                    required property var    display
                    required property string entryType
                    required property string fileType
                    required property string fullPath
                    required property string folderPath
                    required property string name
                    required property string checksumSha256
                    implicitHeight: 30

                    readonly property string checksum: checksumSha256

                    readonly property bool darkTheme: Kirigami.Theme.backgroundColor.hslLightness < 0.5
                    color: exploreTableView.selectedRow === row
                           ? Kirigami.Theme.highlightColor
                           : (row % 2 === 0
                              ? (darkTheme ? Kirigami.Theme.backgroundColor : "#ffffff")
                              : (darkTheme ? "#161b1d" : "#e9f7fc"))

                    // Column separator
                    Rectangle {
                        visible: column > 0
                        anchors { top: parent.top; bottom: parent.bottom; left: parent.left }
                        width: 1
                        color: Kirigami.Theme.separatorColor
                        opacity: 0.4
                    }

                    // File type icon (column 0 only)
                    Kirigami.Icon {
                        id: fileTypeIcon
                        visible: column === 0
                        anchors {
                            left: parent.left
                            verticalCenter: parent.verticalCenter
                            leftMargin: 6
                        }
                        source: {
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
                        implicitWidth:  Kirigami.Units.iconSizes.small
                        implicitHeight: Kirigami.Units.iconSizes.small
                    }

                    Controls.Label {
                        anchors {
                            left: column === 0 ? fileTypeIcon.right : parent.left
                            right: parent.right
                            verticalCenter: parent.verticalCenter
                            leftMargin:  column === 0 ? 4 : 6
                            rightMargin: 4
                        }
                        text: {
                            if (display === undefined || display === null) return ""
                            if (column === 1) return (Number(display) > 0 && entryType === "file")
                                                     ? appManager1.formatDataSize(Number(display)) : ""
                            if (column === 3) return entryType === "file" ? String(display) : ""
                            return String(display)
                        }
                        horizontalAlignment: column === 1 ? Text.AlignRight : Text.AlignLeft
                        color: exploreTableView.selectedRow === row
                               ? Kirigami.Theme.highlightedTextColor
                               : (column === 0 ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor)
                        elide: column === 3 ? Text.ElideLeft : Text.ElideRight
                        clip: true
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.9
                    }

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        onClicked: function(mouse) {
                            exploreTableView.selectedRow = row
                            if (mouse.button === Qt.LeftButton && entryType === "folder") {
                                root.folderNavigated(fullPath)
                            }
                            if (mouse.button === Qt.RightButton) {
                                root._activeFilePath   = fullPath
                                root._activeFileName   = name
                                root._activeFolderPath = folderPath
                                root._activeChecksum   = checksum
                                root._activeEntryType  = entryType
                                exploreContextMenu.popup()
                            }
                        }
                        onDoubleClicked: {
                            if (entryType === "file")
                                appManager1.openFile(fullPath)
                        }
                    }
                }
            }
        }
    }

    // ── Context menu ─────────────────────────────────────────────────────
    Controls.Menu {
        id: exploreContextMenu

        Controls.MenuItem {
            text: root._activeEntryType === "folder" ? qsTr("Open folder") : qsTr("Open file")
            icon.name: "document-open"
            onTriggered: {
                if (root._activeEntryType === "folder")
                    appManager1.openFolder(root._activeFilePath)
                else
                    appManager1.openFile(root._activeFilePath)
            }
        }

        Controls.MenuItem {
            text: qsTr("Open folder")
            icon.name: "document-open-data"
            visible: root._activeEntryType === "file"
            height: visible ? implicitHeight : 0
            onTriggered: appManager1.openFolder(root._activeFolderPath)
        }

        Controls.MenuSeparator {}

        Controls.MenuItem {
            text: qsTr("Copy folder path")
            icon.name: "edit-copy"
            onTriggered: appManager1.copyToClipboard(root._activeFolderPath)
        }

        Controls.MenuItem {
            text: qsTr("Copy folder name")
            icon.name: "edit-copy"
            visible: root._activeEntryType === "folder"
            height: visible ? implicitHeight : 0
            onTriggered: appManager1.copyToClipboard(root._activeFileName)
        }

        Controls.MenuItem {
            text: qsTr("Copy absolute path")
            icon.name: "edit-copy"
            visible: root._activeEntryType === "file"
            height: visible ? implicitHeight : 0
            onTriggered: appManager1.copyToClipboard(root._activeFilePath)
        }

        Controls.MenuItem {
            text: qsTr("Copy file name")
            icon.name: "edit-copy"
            visible: root._activeEntryType === "file"
            height: visible ? implicitHeight : 0
            onTriggered: appManager1.copyToClipboard(root._activeFileName)
        }

        Controls.MenuItem {
            text: qsTr("Copy file name without extension")
            icon.name: "edit-copy"
            visible: root._activeEntryType === "file"
            height: visible ? implicitHeight : 0
            onTriggered: {
                var n = root._activeFileName
                var dot = n.lastIndexOf(".")
                appManager1.copyToClipboard(dot > 0 ? n.substring(0, dot) : n)
            }
        }

        Controls.MenuItem {
            text: qsTr("Copy Checksum")
            icon.name: "edit-copy"
            visible: root._activeEntryType === "file" && root._activeChecksum !== ""
            height: visible ? implicitHeight : 0
            onTriggered: appManager1.copyToClipboard(root._activeChecksum)
        }

        Controls.MenuSeparator {
            visible: root._activeEntryType === "file"
            height: visible ? implicitHeight : 0
        }

        Controls.MenuItem {
            text: qsTr("Calculate Checksum (SHA-256)")
            icon.name: "document-properties"
            visible: root._activeEntryType === "file" && root._activeChecksum === ""
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
            visible: root._activeEntryType === "file" && root._activeChecksum !== ""
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
            visible: root._activeEntryType === "file"
            height: visible ? implicitHeight : 0
            onTriggered: {
                if (appManager1.moveFileToTrash(root._activeFilePath))
                    appManager1.exploreRemoveRow(exploreTableView.selectedRow)
            }
        }

        Controls.MenuItem {
            text: qsTr("Move folder to Trash")
            icon.name: "user-trash"
            visible: root._activeEntryType === "folder"
            height: visible ? implicitHeight : 0
            onTriggered: {
                if (appManager1.moveFileToTrash(root._activeFilePath))
                    appManager1.exploreRemoveRow(exploreTableView.selectedRow)
            }
        }

        Controls.MenuItem {
            text: qsTr("Delete file")
            icon.name: "edit-delete"
            visible: root._activeEntryType === "file"
            height: visible ? implicitHeight : 0
            onTriggered: {
                if (appManager1.deleteSingleFile(root._activeFilePath))
                    appManager1.exploreRemoveRow(exploreTableView.selectedRow)
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
