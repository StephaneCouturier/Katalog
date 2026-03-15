import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import QtQuick.Controls   // required for ScrollBar attached property (namespace alias prevents it)
import org.kde.kirigami as Kirigami

ColumnLayout {
    id: pageSearchResults_column
    spacing: 0

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
    function formatDuration(secs) {
        let s = Number(secs)
        if (!s || s <= 0) return ""
        let h   = Math.floor(s / 3600)
        let m   = Math.floor((s % 3600) / 60)
        let sec = Math.floor(s % 60)
        return String(h).padStart(2, '0') + ":" + String(m).padStart(2, '0') + ":" + String(sec).padStart(2, '0')
    }
    function formatDate(dateStr) {
        let s = String(dateStr ?? "")
        return s.length >= 10 ? s.substring(0, 10) : s
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
                source: "search"
                implicitWidth:  Kirigami.Units.iconSizes.small
                implicitHeight: Kirigami.Units.iconSizes.small
            }

            Controls.Label {
                font.bold: true
                text: {
                    let n = newSearch1.properties.filesFoundNumber ?? 0
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


            Controls.ComboBox {
                id: batchActionCombo
                implicitWidth: 150
                textRole: "text"
                valueRole: "value"
                model: [
                    { text: qsTr("Export to CSV"),  value: "csv",    iconName: "document-save-as" },
                    { text: qsTr("Move to Trash"),  value: "trash",  iconName: "user-trash"       },
                    { text: qsTr("Delete"),         value: "delete", iconName: "edit-delete"      }
                ]
                delegate: Controls.ItemDelegate {
                    width: batchActionCombo.width
                    text: modelData.text
                    icon.name: modelData.iconName
                    highlighted: batchActionCombo.highlightedIndex === index
                }
                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing
                    Item { width: Kirigami.Units.smallSpacing }
                    Kirigami.Icon {
                        source: batchActionCombo.currentIndex >= 0
                                ? batchActionCombo.model[batchActionCombo.currentIndex].iconName : ""
                        implicitWidth:  Kirigami.Units.iconSizes.small
                        implicitHeight: Kirigami.Units.iconSizes.small
                    }
                    Controls.Label {
                        text: batchActionCombo.displayText
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }

            Controls.Button {
                //text: qsTr("Run")
                icon.name: "media-playback-start"
                enabled: (newSearch1.properties.filesFoundNumber ?? 0) > 0
                onClicked: {
                    let action = batchActionCombo.currentValue
                    if (action === "csv") {
                        let path = appManager1.exportSearchResultsToCSV()
                        if (path)
                            showPassiveNotification(qsTr("Exported to: %1").arg(path))
                        else
                            showPassiveNotification(qsTr("Export failed — no results or write error"))
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

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Controls.HorizontalHeaderView {
                id: headerView
                Layout.fillWidth: true
                syncView: tableView
                clip: true
                implicitHeight: 34

                delegate: Rectangle {
                    required property string display
                    color: Kirigami.Theme.backgroundColor
                    implicitHeight: headerView.implicitHeight

                    Controls.Label {
                        anchors {
                            left: parent.left; right: parent.right
                            verticalCenter: parent.verticalCenter
                            leftMargin: 6; rightMargin: 6
                        }
                        text: display
                        elide: Text.ElideRight
                        //clip: true
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.9
                    }

                    Rectangle {//Vertical separator
                        anchors { top: parent.top; bottom: parent.bottom; right: parent.right }
                        width: 1
                        color: Kirigami.Theme.separatorColor
                    }
                }
            }

            Kirigami.Separator { Layout.fillWidth: true }

            TableView {
                id: tableView
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                columnSpacing: 0
                rowSpacing: 0
                model: newSearch1

                property int selectedRow: -1

                ScrollBar.vertical:   ScrollBar { policy: ScrollBar.AsNeeded }
                ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AsNeeded }

                columnWidthProvider: function(column) {
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
                        case 10: return 60   // Width
                        case 11: return 60   // Height
                        case 12: return 80   // Duration
                        case 13: return 80   // Video Width
                        case 14: return 80   // Video Height
                        case 15: return 90   // Audio Duration
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
                            if (display === undefined || display === null) return ""
                            if (column === 1)  return formatFileSize(Number(display))
                            if (column === 12 || column === 15) return formatDuration(Number(display))
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
                            if (mouse.button === Qt.RightButton) {
                                let fileName = String(newSearch1.data(newSearch1.index(row, 0), Qt.DisplayRole) ?? "")
                                let folder   = String(newSearch1.data(newSearch1.index(row, 3), Qt.DisplayRole) ?? "")
                                resultContextMenu.openForRow(row, fileName, folder)
                            }
                        }
                        onDoubleClicked: {
                            let fileName = String(newSearch1.data(newSearch1.index(row, 0), Qt.DisplayRole) ?? "")
                            let folder   = String(newSearch1.data(newSearch1.index(row, 3), Qt.DisplayRole) ?? "")
                            appManager1.openFile(folder + "/" + fileName)
                        }
                    }
                }
            }
        }
    }

    // ── Context menu ─────────────────────────────────────────────────────
    Controls.Menu {
        id: resultContextMenu
        property string fileName: ""
        property string folder:   ""
        property string fullPath: ""

        function openForRow(r, name, dir) {
            fileName = name
            folder   = dir
            fullPath = dir + "/" + name
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
            icon.name: "document-open-folder"
            enabled: resultContextMenu.folder !== ""
            onTriggered: appManager1.openFolder(resultContextMenu.folder)
        }
        Controls.MenuSeparator {}
        Controls.MenuItem {
            text: qsTr("Copy file name")
            icon.name: "edit-copy"
            onTriggered: {
                appManager1.copyToClipboard(resultContextMenu.fileName)
                showPassiveNotification(qsTr("File name copied to clipboard"))
            }
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
            text: qsTr("Copy full path")
            icon.name: "edit-copy"
            onTriggered: {
                appManager1.copyToClipboard(resultContextMenu.fullPath)
                showPassiveNotification(qsTr("Full path copied to clipboard"))
            }
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

        Controls.Label { text: qsTr("") }

        Item { Layout.fillWidth: true }
    }

    Kirigami.Dialog {
        id: batchConfirmDialog
        property string batchAction: ""
        title: batchAction === "trash" ? qsTr("Move to Trash") : qsTr("Delete Files")
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        padding: Kirigami.Units.largeSpacing
        preferredWidth: Kirigami.Units.gridUnit * 28

        contentItem: Controls.Label {
            text: batchConfirmDialog.batchAction === "trash"
                  ? qsTr("Move all %1 result(s) to trash?").arg(newSearch1.properties.filesFoundNumber ?? 0)
                  : qsTr("Permanently delete all %1 result(s)? This cannot be undone.").arg(newSearch1.properties.filesFoundNumber ?? 0)
            wrapMode: Text.WordWrap
        }

        onAccepted: {
            let n = 0
            if (batchAction === "trash")
                n = appManager1.batchMoveSearchResultsToTrash()
            else
                n = appManager1.batchDeleteSearchResults()
            showPassiveNotification(
                batchAction === "trash"
                ? qsTr("%1 file(s) moved to trash").arg(n)
                : qsTr("%1 file(s) deleted").arg(n)
            )
        }
    }

    // ── Refresh when a new search runs ───────────────────────────────────
    ListModel { id: emptyModel }

    Connections {
        target: root
        function onSearchTriggered() {
            tableView.selectedRow = -1
            tableView.model = emptyModel
            tableView.model = newSearch1
            tableView.forceLayout()
        }
    }
}
