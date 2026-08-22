import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import QtQuick.Controls   // required for ScrollBar attached property (namespace alias prevents it)
import org.kde.kirigami as Kirigami

ColumnLayout {
    id: pageSearchResults_column
    spacing: 0

    signal closeRequested()

    // Sort state
    property int  sortColumn:    -1
    property bool sortAscending: true

    Component.onCompleted: {
        var col   = appManager1.getSearchSortColumn()
        var order = appManager1.getSearchSortOrder()
        if (col >= 0) {
            pageSearchResults_column.sortColumn    = col
            pageSearchResults_column.sortAscending = (order === 0)
        }
    }

    // ── Helpers ──────────────────────────────────────────────────────────
    function formatFileSize(size) {
        let n = Number(size)
        if (!n || n <= 0)              return ""
        if (n < 1024)                  return n + " B"
        if (n < 1024 * 1024)           return (n / 1024).toFixed(1) + " KiB"
        if (n < 1024 * 1024 * 1024)    return (n / (1024 * 1024)).toFixed(1) + " MiB"
        if (n < 1024 ** 4)             return (n / (1024 ** 3)).toFixed(2) + " GiB"
        return                                (n / (1024 ** 4)).toFixed(2) + " TiB"
    }
    function formatDate(dateStr) {
        let s = String(dateStr ?? "")
        return s.length >= 10 ? s.substring(0, 10) : s
    }
    function formatDuration(secs) {
        let s = Number(secs)
        if (!s || s <= 0) return ""
        let h   = Math.floor(s / 3600)
        let m   = Math.floor((s % 3600) / 60)
        let sec = Math.floor(s % 60)
        return String(h).padStart(2, '0') + ":" + String(m).padStart(2, '0') + ":" + String(sec).padStart(2, '0')
    }

    // ── Device path ──────────────────────────────────────────────────────
    Rectangle {
        Layout.fillWidth: true
        implicitHeight: devicePathLabel.implicitHeight + Kirigami.Units.largeSpacing * 2
        color: Kirigami.Theme.backgroundColor

        RowLayout {
            anchors {
                left: parent.left; right: parent.right
                verticalCenter: parent.verticalCenter
                leftMargin:  Kirigami.Units.largeSpacing
                rightMargin: Kirigami.Units.largeSpacing
                topMargin: Kirigami.Units.largeSpacing
                bottomMargin: Kirigami.Units.largeSpacing
            }
            spacing: Kirigami.Units.smallSpacing
            Kirigami.Icon {
                source: "drive-harddisk"
                implicitWidth:  Kirigami.Units.iconSizes.smallMedium
                implicitHeight: Kirigami.Units.iconSizes.smallMedium
                opacity: 0.7
            }
            Controls.Label {
                id: devicePathLabel
                text: newSearch1.properties.devicePath ?? ""
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.9
                elide: Text.ElideLeft
                Layout.fillWidth: true
                opacity: 0.8
            }
        }
    }

    // ── Summary bar ──────────────────────────────────────────────────────
    Rectangle {
        Layout.fillWidth: true
        implicitHeight: summaryRow.implicitHeight + Kirigami.Units.largeSpacing * 2
        color: Kirigami.Theme.backgroundColor

        RowLayout {
            id: summaryRow
            anchors {
                left: parent.left;  right: parent.right
                verticalCenter: parent.verticalCenter
                leftMargin:  Kirigami.Units.largeSpacing
                rightMargin: Kirigami.Units.largeSpacing
            }
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Icon {
                source: "edit-find"
                implicitWidth:  Kirigami.Units.iconSizes.small
                implicitHeight: Kirigami.Units.iconSizes.small
            }

            Controls.Label {
                font.bold: true
                text: {
                    let n = Number(newSearch1.properties.filesFoundNumber ?? 0).toLocaleString(Qt.locale(), "f", 0)
                    if (newSearch1.properties.searchOnDuplicates)
                        return qsTr("%1 duplicate(s) found").arg(n)
                    if (newSearch1.properties.searchOnDifferences)
                        return qsTr("%1 difference(s) found").arg(n)
                    if (newSearch1.properties.showFoldersOnly)
                        return qsTr("%1 folder(s) found").arg(n)
                    return qsTr("%1 file(s) found").arg(n)
                }
            }

            Controls.Label { text: "·"; opacity: 0.4; visible: (newSearch1.properties.filesFoundTotalSize ?? 0) > 0 }
            Controls.Label {
                visible: (newSearch1.properties.filesFoundTotalSize ?? 0) > 0
                text: qsTr("Total: %1").arg(formatFileSize(newSearch1.properties.filesFoundTotalSize))
            }

            Controls.Label { text: "·"; opacity: 0.4; visible: (newSearch1.properties.filesFoundMinSize ?? 0) > 0 }
            Controls.Label {
                visible: (newSearch1.properties.filesFoundMinSize ?? 0) > 0
                text: qsTr("Min: %1   Max: %2")
                      .arg(formatFileSize(newSearch1.properties.filesFoundMinSize))
                      .arg(formatFileSize(newSearch1.properties.filesFoundMaxSize))
                opacity: 0.8
            }
            Controls.Label {
                text: {
                    let d1 = formatDate(newSearch1.properties.filesFoundMinDate)
                    let d2 = formatDate(newSearch1.properties.filesFoundMaxDate)
                    if (d1 && d1 === d2) return "  " + d1
                    if (d1 && d2)        return "  " + d1 + " – " + d2
                    return ""
                }
                opacity: 0.7
                font.pointSize: Kirigami.Theme.smallFont.pointSize
            }

            Item { Layout.fillWidth: true }


            FontMetrics {
                id: batchComboFontMetrics
                font: batchActionCombo.font
            }

            Controls.ComboBox {
                id: batchActionCombo
                implicitWidth: {
                    var maxW = 0
                    for (var i = 0; i < model.length; i++) {
                        var w = batchComboFontMetrics.advanceWidth(model[i].text || "")
                        if (w > maxW) maxW = w
                    }
                    return maxW
                         + Kirigami.Units.iconSizes.small
                         + Kirigami.Units.smallSpacing * 3
                         + leftPadding + rightPadding
                         + (indicator ? indicator.width : 0)
                }
                textRole: "text"
                valueRole: "value"
                displayText: ""
                model: [
                    { text: qsTr("Export to CSV"),     value: "csv",              iconName: "document-save"    },
                    { text: qsTr("Export to Catalog"), value: "catalog",          iconName: "media-optical"    },
                    { text: qsTr("Verify Checksums"),  value: "verify-checksums", iconName: "dialog-ok-apply"  },
                    { text: qsTr("Extract Metadata"),  value: "get-metadata",     iconName: "video-mp4"        },
                    { text: qsTr("Move to Trash"),     value: "trash",            iconName: "user-trash"       },
                    { text: qsTr("Delete"),            value: "delete",           iconName: "edit-delete"      }
                ]
                delegate: Controls.ItemDelegate {
                    width:  batchActionCombo.width
                    height: modelData.value === "---" ? 9 : implicitHeight
                    text:      modelData.value === "---" ? "" : modelData.text
                    icon.name: modelData.value === "---" ? "" : modelData.iconName
                    highlighted: batchActionCombo.highlightedIndex === index
                    enabled: modelData.value !== "---"
                    background: Rectangle {
                        color: modelData.value === "---"
                               ? Kirigami.Theme.backgroundColor
                               : (highlighted ? Kirigami.Theme.highlightColor : "transparent")
                    }
                    Kirigami.Separator {
                        visible: modelData.value === "---"
                        anchors.centerIn: parent
                        width: parent.width - Kirigami.Units.largeSpacing * 2
                    }
                }
                contentItem: RowLayout {
                    function positionToRectangle(pos) { return Qt.rect(0, 0, 0, 0) }
                    property int selectionStart: 0
                    spacing: Kirigami.Units.smallSpacing
                    Item { width: Kirigami.Units.smallSpacing }
                    Kirigami.Icon {
                        source: {
                            let m = batchActionCombo.currentIndex >= 0
                                    ? batchActionCombo.model[batchActionCombo.currentIndex] : null
                            return (m && m.value !== "---") ? m.iconName : ""
                        }
                        implicitWidth:  Kirigami.Units.iconSizes.small
                        implicitHeight: Kirigami.Units.iconSizes.small
                    }
                    Controls.Label {
                        //text: batchActionCombo.displayText
                        text: batchActionCombo.currentIndex >= 0
                              ? batchActionCombo.model[batchActionCombo.currentIndex].text : ""
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }

            Controls.Button {
                //text: qsTr("Run")
                icon.name: "document-export"
                enabled: (newSearch1.properties.filesFoundNumber ?? 0) > 0
                onClicked: {
                    let action = batchActionCombo.currentValue
                    if (action === "---") return
                    if (action === "csv") {
                        let path = appManager1.exportSearchResultsToCSV()
                        if (path)
                            showPassiveNotification(qsTr("Exported to: %1").arg(path))
                        else
                            showPassiveNotification(qsTr("Export failed — no results or write error"))
                    } else if (action === "catalog") {
                        let name = appManager1.exportSearchResultsAsCatalog()
                        if (name)
                            showPassiveNotification(qsTr("Results exported to catalog: %1").arg(name))
                        else
                            showPassiveNotification(qsTr("Export failed — no results or database error"))
                    } else {
                        batchConfirmDialog.batchAction = action
                        batchConfirmDialog.open()
                    }
                }
            }
        }
    }

    Kirigami.Separator {
        Layout.fillWidth: true
        //color: Kirigami.Theme.separatorColor
        color: Kirigami.Theme.alternateBackgroundColor
    }

    // ── Column headers + Table ───────────────────────────────────────────
    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true

        Controls.HorizontalHeaderView {
            id: headerView
            anchors { top: parent.top; left: parent.left; right: parent.right }
            syncView: tableView
            clip: true
            implicitHeight: 34
            resizableColumns: true

                delegate: Rectangle {
                    required property int    column
                    required property string display
                    color: Kirigami.Theme.backgroundColor
                    implicitHeight: headerView.implicitHeight

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
                        text: pageSearchResults_column.sortColumn === column
                              ? (pageSearchResults_column.sortAscending ? "▲" : "▼") : ""
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.7
                        color: Kirigami.Theme.textColor
                    }

                    Rectangle {//Vertical separator
                        anchors { top: parent.top; bottom: parent.bottom; right: parent.right }
                        width: 1
                        color: Kirigami.Theme.separatorColor ?? "transparent"
                    }

                    TapHandler {
                        onTapped: {
                            tableView.selectedRow = -1
                            if (pageSearchResults_column.sortColumn === column) {
                                pageSearchResults_column.sortAscending = !pageSearchResults_column.sortAscending
                            } else {
                                pageSearchResults_column.sortColumn    = column
                                pageSearchResults_column.sortAscending = true
                            }
                            appManager1.sortSearch(pageSearchResults_column.sortColumn,
                                                   pageSearchResults_column.sortAscending ? 0 : 1)
                        }
                    }
                }
            }

        Kirigami.Separator {
            id: headerSep
            anchors { top: headerView.bottom; left: parent.left; right: parent.right }
        }

        TableView {
            id: tableView
            anchors { top: headerSep.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }
                columnSpacing: 0
                rowSpacing: 0
                model: appManager1.searchSortModel

                property int selectedRow: -1

                ScrollBar.vertical:   ScrollBar { policy: ScrollBar.AsNeeded }
                ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AsNeeded }

                rowHeightProvider: function(row) { return 30 }

                columnWidthProvider: function(column) {
                    let w = tableView.explicitColumnWidth(column)
                    if (w >= 0) return w
                    switch (column) {
                        case  0: return 250  // Name
                        case  1: return 90   // Size
                        case  2: return 140  // Date
                        case  3: return 320  // Directory
                        case  4: return 140  // Catalog Name
                        case  5: return 80   // Catalog ID
                        case  6: return 0    // orderValue (hidden)
                        case  7: return 0    // Path (hidden)
                        case  8: return 80   // File Type
                        case  9: return 140  // MIME Type
                        case 10: return 60   // Width (merged image + video)
                        case 11: return 60   // Height (merged image + video)
                        case 12: return 80   // Duration (merged video + audio)
                        case 13: return 0    // Video Width  (merged into Width, hidden — matches K2)
                        case 14: return 0    // Video Height (merged into Height, hidden — matches K2)
                        case 15: return 0    // Audio Duration (merged into Duration, hidden — matches K2)
                        case 16: return 140  // Artist
                        case 17: return 140  // Album
                        case 18: return 140  // Title
                        case 19: return 260  // Checksum (SHA256)
                        case 20: return 130  // Checksum Date
                    }
                    return 100
                }

                delegate: Rectangle {
                    required property int     row
                    required property int     column
                    required property var     display
                    required property string  fileType
                    implicitHeight: 30

                    function iconForType(ft) {
                        switch (ft) {
                            case "folder": return "folder"
                            case "audio":  return "audio-x-mpeg"
                            case "image":  return "image-jpeg"
                            case "video":  return "video-mp4"
                            case "text":   return "view-list-text"
                            case "other":  return "document-open"
                            default:       return "application-x-zerosize"
                        }
                    }

                    readonly property bool darkTheme: Kirigami.Theme.backgroundColor.hslLightness < 0.5
                    color: tableView.selectedRow === row
                           ? Kirigami.Theme.highlightColor
                           : (row % 2 === 0
                              ? (darkTheme ? Kirigami.Theme.backgroundColor : "#ffffff")
                              : (darkTheme ? "#161b1d" : "#e9f7fc"))

                    // Column separator
                    Rectangle {
                        visible: column > 0
                        anchors { top: parent.top; bottom: parent.bottom; left: parent.left }
                        width: 1
                        color: Kirigami.Theme.separatorColor ?? Kirigami.Theme.textColor
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
                        source: iconForType(fileType)
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
                            // Merged metadata columns combine two source columns per row
                            // (image∥video for size, video∥audio for duration), matching K2's
                            // FilesView. The primary value arrives as `display`; the secondary
                            // is read from its sibling column. Do this in QML because `display`
                            // exposes the raw per-column value, not FilesView's merged output.
                            let m = appManager1.searchSortModel
                            if (column === 10) {   // Width  — image (10) ∥ video (13)
                                let w = Number(display) || Number(m.data(m.index(row, 13), Qt.DisplayRole))
                                return w > 0 ? String(w) : ""
                            }
                            if (column === 11) {   // Height — image (11) ∥ video (14)
                                let h = Number(display) || Number(m.data(m.index(row, 14), Qt.DisplayRole))
                                return h > 0 ? String(h) : ""
                            }
                            if (column === 12) {   // Duration — video (12) ∥ audio (15)
                                let d = Number(display) || Number(m.data(m.index(row, 15), Qt.DisplayRole))
                                return formatDuration(d)
                            }
                            if (display === undefined || display === null) return ""
                            if (column === 1)  return formatFileSize(Number(display))
                            return String(display)
                        }
                        horizontalAlignment: (column === 1 || column === 12 || column === 15)
                                             ? Text.AlignRight : Text.AlignLeft
                        color: tableView.selectedRow === row
                               ? Kirigami.Theme.highlightedTextColor
                               : Kirigami.Theme.textColor
                        elide: (column === 3 || column === 7) ? Text.ElideLeft : Text.ElideRight
                        clip: true
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.9
                    }

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        onClicked: function(mouse) {
                            tableView.selectedRow = row
                            if (mouse.button === Qt.LeftButton) {
                                var lm = appManager1.searchSortModel
                                let lFileName = String(lm.data(lm.index(row, 0), Qt.DisplayRole) ?? "")
                                let lFolder   = String(lm.data(lm.index(row, 3), Qt.DisplayRole) ?? "")
                                appManager1.openFile(lFolder + "/" + lFileName)
                            }
                            if (mouse.button === Qt.RightButton)
                                openResultContextMenu()
                        }
                        onPressAndHold: {
                            tableView.selectedRow = row
                            openResultContextMenu()
                        }

                        function openResultContextMenu() {
                            var m = appManager1.searchSortModel
                            let fileName    = String(m.data(m.index(row, 0),  Qt.DisplayRole) ?? "")
                            let folder      = String(m.data(m.index(row, 3),  Qt.DisplayRole) ?? "")
                            let catalogName = String(m.data(m.index(row, 4),  Qt.DisplayRole) ?? "")
                            let catalogId   = Number(m.data(m.index(row, 5),  Qt.DisplayRole) ?? -1)
                            let checksum    = String(m.data(m.index(row, 19), Qt.DisplayRole) ?? "")
                            resultContextMenu.openForRow(row, fileName, folder, checksum, catalogId, catalogName)
                        }
                    }
                }
        }
    }

    // ── Context menu ─────────────────────────────────────────────────────
    Controls.Menu {
        id: resultContextMenu
        property string fileName:    ""
        property string folder:      ""
        property string fullPath:    ""
        property string checksum:    ""
        property int    catalogId:   -1
        property string catalogName: ""

        function openForRow(r, name, dir, cksum, catId, catName) {
            fileName    = name
            folder      = dir
            fullPath    = dir + "/" + name
            checksum    = cksum
            catalogId   = catId
            catalogName = catName
            popup()
        }

        Controls.MenuItem {
            text: resultContextMenu.fileName || qsTr("(no selection)")
            enabled: false
            font.bold: true
        }
        Controls.MenuSeparator {}
        Controls.MenuItem {
            text: qsTr("Open file")
            icon.name: "document-open"
            enabled: resultContextMenu.fileName !== ""
            onTriggered: appManager1.openFile(resultContextMenu.fullPath)
        }
        Controls.MenuItem {
            text: qsTr("Open folder")
            icon.name: "document-open"
            enabled: resultContextMenu.folder !== ""
            onTriggered: appManager1.openFolder(resultContextMenu.folder)
        }
        Controls.MenuItem {
            text: qsTr("Explore folder")
            icon.name: "view-list-tree"
            enabled: false
        }
        Controls.MenuSeparator {}
        Controls.MenuItem {
            text: qsTr("Show extended metadata (JSON)")
            icon.name: "configure"
            readonly property bool hasMetadata:
                resultContextMenu.catalogName !== "" &&
                resultContextMenu.catalogName !== "Connected" &&
                appManager1.catalogIncludesExtendedMetadata(resultContextMenu.catalogId)
            visible: hasMetadata
            height:  hasMetadata ? implicitHeight : 0
            onTriggered: {
                let fields = appManager1.getFileMetadataParsedFields(
                    resultContextMenu.catalogId,
                    resultContextMenu.fileName,
                    resultContextMenu.folder)
                if (fields.length > 0) {
                    metadataDialog.fields    = fields
                    metadataDialog.jsonText  = appManager1.getFileMetadataJson(
                        resultContextMenu.catalogId,
                        resultContextMenu.fileName,
                        resultContextMenu.folder)
                    metadataDialog.fileLabel = resultContextMenu.fileName
                    metadataDialog.open()
                } else {
                    showPassiveNotification(qsTr("No extended metadata available"))
                }
            }
        }
        Controls.MenuSeparator {
            readonly property bool hasMetadata:
                resultContextMenu.catalogName !== "" &&
                resultContextMenu.catalogName !== "Connected" &&
                appManager1.catalogIncludesExtendedMetadata(resultContextMenu.catalogId)
            visible: hasMetadata
            height:  hasMetadata ? implicitHeight : 0
        }
        Controls.MenuItem {
            text: qsTr("Copy folder path")
            icon.name: "edit-copy"
            onTriggered: {
                appManager1.copyToClipboard(resultContextMenu.folder)
                showPassiveNotification(qsTr("Folder path copied to clipboard"))
            }
        }
        Controls.MenuItem {
            text: qsTr("Copy file absolute path")
            icon.name: "edit-copy"
            onTriggered: {
                appManager1.copyToClipboard(resultContextMenu.fullPath)
                showPassiveNotification(qsTr("Full path copied to clipboard"))
            }
        }
        Controls.MenuItem {
            text: qsTr("Copy file name with extension")
            icon.name: "edit-copy"
            onTriggered: {
                appManager1.copyToClipboard(resultContextMenu.fileName)
                showPassiveNotification(qsTr("File name copied to clipboard"))
            }
        }
        Controls.MenuItem {
            text: qsTr("Copy file name without extension")
            icon.name: "edit-copy"
            onTriggered: {
                let n = resultContextMenu.fileName
                let dot = n.lastIndexOf(".")
                appManager1.copyToClipboard(dot > 0 ? n.substring(0, dot) : n)
                showPassiveNotification(qsTr("File name copied to clipboard"))
            }
        }
        Controls.MenuSeparator {}
        Controls.MenuItem {
            text: qsTr("Calculate Checksum (SHA-256)")
            icon.name: "configure"
            visible: resultContextMenu.checksum === ""
            height:  resultContextMenu.checksum === "" ? implicitHeight : 0
            onTriggered: {
                let cksum = appManager1.calculateAndSaveChecksum(
                    resultContextMenu.fullPath,
                    resultContextMenu.fileName,
                    resultContextMenu.folder,
                    resultContextMenu.catalogId)
                if (cksum.length > 0) {
                    resultContextMenu.checksum = cksum
                    checksumResultDialog.checksumText = cksum
                    checksumResultDialog.wasSaved = resultContextMenu.catalogId > 0
                    checksumResultDialog.open()
                } else {
                    showPassiveNotification(qsTr("File not found or could not be read"))
                }
            }
        }
        Controls.MenuItem {
            text: qsTr("Copy Checksum")
            icon.name: "edit-copy"
            visible: resultContextMenu.checksum !== ""
            height:  resultContextMenu.checksum !== "" ? implicitHeight : 0
            onTriggered: {
                appManager1.copyToClipboard(resultContextMenu.checksum)
                showPassiveNotification(qsTr("Checksum copied to clipboard"))
            }
        }
        Controls.MenuItem {
            text: qsTr("Verify Checksum (SHA-256)")
            icon.name: "configure"
            visible: resultContextMenu.checksum !== ""
            height:  resultContextMenu.checksum !== "" ? implicitHeight : 0
            onTriggered: {
                let result = appManager1.verifyFileChecksum(
                    resultContextMenu.fullPath,
                    resultContextMenu.checksum)
                if (result === "match") {
                    checksumResultDialog.checksumText = resultContextMenu.checksum
                    checksumResultDialog.wasSaved = false
                    checksumResultDialog.open()
                } else if (result.startsWith("mismatch:")) {
                    checksumMismatchDialog.expectedChecksum = resultContextMenu.checksum
                    checksumMismatchDialog.actualChecksum   = result.substring(9)
                    checksumMismatchDialog.open()
                } else {
                    showPassiveNotification(qsTr("Error: ") + result.substring(6))
                }
            }
        }
        Controls.MenuSeparator {}
        Controls.MenuItem {
            text: qsTr("Move to Trash")
            icon.name: "user-trash"
            enabled: resultContextMenu.fileName !== ""
            onTriggered: {
                if (appManager1.moveFileToTrash(resultContextMenu.fullPath))
                    showPassiveNotification(qsTr("File moved to trash"))
                else
                    showPassiveNotification(qsTr("Could not move file to trash"))
            }
        }
        Controls.MenuItem {
            text: qsTr("Delete file")
            icon.name: "edit-delete"
            enabled: resultContextMenu.fileName !== ""
            onTriggered: {
                deleteFileDialog.open()
            }
        }
    }

    // ── Metadata dialog ───────────────────────────────────────────────────
    MetadataDialog { id: metadataDialog }

    // ── Checksum result dialog ────────────────────────────────────────────
    Kirigami.Dialog {
        id: checksumResultDialog
        property string checksumText: ""
        property bool   wasSaved: false
        title: qsTr("Checksum (SHA-256)")
        standardButtons: Kirigami.Dialog.Close
        preferredWidth: Kirigami.Units.gridUnit * 36
        padding: Kirigami.Units.largeSpacing

        contentItem: ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            Controls.Label {
                text: "SHA-256: " + checksumResultDialog.checksumText
                wrapMode: Text.WrapAnywhere
                Layout.fillWidth: true
                font.family: "monospace"
            }
            Controls.Label {
                text: checksumResultDialog.wasSaved
                      ? qsTr("Checksum saved to database.")
                      : qsTr("Checksums match.")
                opacity: 0.7
            }
            Controls.Button {
                text: qsTr("Copy to Clipboard")
                icon.name: "edit-copy"
                onClicked: {
                    appManager1.copyToClipboard(checksumResultDialog.checksumText)
                    showPassiveNotification(qsTr("Checksum copied to clipboard"))
                }
            }
        }
    }

    // ── Checksum mismatch dialog ──────────────────────────────────────────
    Kirigami.Dialog {
        id: checksumMismatchDialog
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
                        resultContextMenu.fullPath,
                        resultContextMenu.fileName,
                        resultContextMenu.folder,
                        resultContextMenu.catalogId)
                    resultContextMenu.checksum = checksumMismatchDialog.actualChecksum
                    showPassiveNotification(qsTr("Checksum saved to database"))
                    checksumMismatchDialog.close()
                }
            }
        ]

        contentItem: ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            Controls.Label { text: qsTr("Checksums do not match."); font.bold: true }
            Controls.Label { text: qsTr("Expected:"); opacity: 0.7 }
            Controls.Label {
                text: checksumMismatchDialog.expectedChecksum
                wrapMode: Text.WrapAnywhere
                Layout.fillWidth: true
                font.family: "monospace"
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.85
            }
            Controls.Label { text: qsTr("Actual:"); opacity: 0.7 }
            Controls.Label {
                text: checksumMismatchDialog.actualChecksum
                wrapMode: Text.WrapAnywhere
                Layout.fillWidth: true
                font.family: "monospace"
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.85
            }
        }
    }

    // ── Delete confirmation dialog ────────────────────────────────────────
    Kirigami.Dialog {
        id: deleteFileDialog
        title: qsTr("Delete File")
        standardButtons: Kirigami.Dialog.Cancel
        preferredWidth: Kirigami.Units.gridUnit * 28
        padding: Kirigami.Units.largeSpacing

        customFooterActions: [
            Kirigami.Action {
                text: qsTr("Delete")
                icon.name: "edit-delete"
                onTriggered: {
                    if (appManager1.deleteSingleFile(resultContextMenu.fullPath))
                        showPassiveNotification(qsTr("File deleted"))
                    else
                        showPassiveNotification(qsTr("Could not delete file"))
                    deleteFileDialog.close()
                }
            }
        ]

        contentItem: Controls.Label {
            text: qsTr("Permanently delete this file? This cannot be undone.\n\n%1")
                  .arg(resultContextMenu.fullPath)
            wrapMode: Text.WordWrap
        }
    }

    // ── Batch action bar ─────────────────────────────────────────────────
    Kirigami.Separator { Layout.fillWidth: true }

    RowLayout {
        Layout.fillWidth: true
        Layout.leftMargin:  Kirigami.Units.largeSpacing
        Layout.rightMargin: Kirigami.Units.largeSpacing
        Layout.topMargin:    Kirigami.Units.smallSpacing
        Layout.bottomMargin: Kirigami.Units.smallSpacing
        spacing: Kirigami.Units.smallSpacing

        Controls.Label { text: "" }

        Item { Layout.fillWidth: true }
    }

    Kirigami.Dialog {
        id: batchConfirmDialog
        property string batchAction: ""
        title: {
            if (batchAction === "trash")            return qsTr("Move to Trash")
            if (batchAction === "delete")           return qsTr("Delete Files")
            if (batchAction === "verify-checksums") return qsTr("Verify Checksums")
            if (batchAction === "get-metadata")     return qsTr("Include Metadata")
            return ""
        }
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        padding: Kirigami.Units.largeSpacing
        preferredWidth: Kirigami.Units.gridUnit * 28

        contentItem: Controls.Label {
            text: {
                let n = newSearch1.properties.filesFoundNumber ?? 0
                if (batchConfirmDialog.batchAction === "trash")
                    return qsTr("Move all %1 result(s) to trash?").arg(n)
                if (batchConfirmDialog.batchAction === "delete")
                    return qsTr("Permanently delete all %1 result(s)? This cannot be undone.").arg(n)
                if (batchConfirmDialog.batchAction === "verify-checksums")
                    return qsTr("Calculate and verify checksums for all %1 result(s)?").arg(n)
                if (batchConfirmDialog.batchAction === "get-metadata")
                    return qsTr("Include metadata for all %1 result(s)?").arg(n)
                return ""
            }
            wrapMode: Text.WordWrap
        }

        onAccepted: {
            if (batchAction === "trash") {
                let n = appManager1.batchMoveSearchResultsToTrash()
                showPassiveNotification(qsTr("%1 file(s) moved to trash").arg(n))
            } else if (batchAction === "delete") {
                let n = appManager1.batchDeleteSearchResults()
                showPassiveNotification(qsTr("%1 file(s) deleted").arg(n))
            } else if (batchAction === "verify-checksums") {
                let r = appManager1.batchVerifyChecksums()
                batchResultDialog.batchAction = "verify-checksums"
                batchResultDialog.resultText =
                    qsTr("Matched: %1\nMismatched: %2\nNew checksums calculated: %3\nErrors: %4")
                    .arg(r.matched ?? 0).arg(r.mismatched ?? 0).arg(r.calculated ?? 0).arg(r.errors ?? 0)
                batchResultDialog.open()
            } else if (batchAction === "get-metadata") {
                let r = appManager1.batchGetMetadata()
                batchResultDialog.batchAction = "get-metadata"
                batchResultDialog.resultText =
                    qsTr("Updated: %1\nSkipped: %2\nErrors: %3")
                    .arg(r.updated ?? 0).arg(r.skipped ?? 0).arg(r.errors ?? 0)
                batchResultDialog.open()
            }
        }
    }

    Kirigami.Dialog {
        id: batchResultDialog
        property string batchAction: ""
        property string resultText: ""
        title: batchAction === "verify-checksums" ? qsTr("Verify Checksums") : qsTr("Include Metadata")
        standardButtons: Kirigami.Dialog.Ok
        padding: Kirigami.Units.largeSpacing
        preferredWidth: Kirigami.Units.gridUnit * 24
        contentItem: Controls.Label {
            text: batchResultDialog.resultText
            wrapMode: Text.WordWrap
        }
    }

    // ── Refresh when a new search runs ───────────────────────────────────
    ListModel { id: emptyModel }

    Connections {
        target: root
        function onSearchTriggered() {
            tableView.selectedRow = -1
            tableView.model = emptyModel
            tableView.model = appManager1.searchSortModel
            tableView.forceLayout()
        }
    }
}
