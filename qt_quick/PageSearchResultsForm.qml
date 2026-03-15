import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

ColumnLayout {
    id: pageSearchResults_column
    spacing: 0

    // ── Helpers ──────────────────────────────────────────────────────────
    function formatFileSize(size) {
        let n = Number(size)
        if (!n || n <= 0)                        return ""
        if (n < 1024)                            return n + " B"
        if (n < 1024 * 1024)                     return (n / 1024).toFixed(1) + " KB"
        if (n < 1024 * 1024 * 1024)              return (n / (1024 * 1024)).toFixed(1) + " MB"
        if (n < 1024 * 1024 * 1024 * 1024)       return (n / (1024 * 1024 * 1024)).toFixed(2) + " GB"
        return (n / (1024 * 1024 * 1024 * 1024)).toFixed(2) + " TB"
    }
    function formatDate(dateStr) {
        let s = String(dateStr ?? "")
        return s.length >= 10 ? s.substring(0, 10) : s
    }

    // ── Summary bar ──────────────────────────────────────────────────────
    Rectangle {
        Layout.fillWidth: true
        implicitHeight: summaryRow.implicitHeight + Kirigami.Units.largeSpacing * 2
        color: Kirigami.Theme.alternateBackgroundColor

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

            Item { Layout.fillWidth: true }

            Controls.Label {
                text: {
                    let d1 = formatDate(newSearch1.properties.filesFoundMinDate)
                    let d2 = formatDate(newSearch1.properties.filesFoundMaxDate)
                    if (d1 && d1 === d2) return d1
                    if (d1 && d2)        return d1 + " – " + d2
                    return ""
                }
                opacity: 0.7
                font.pointSize: Kirigami.Theme.smallFont.pointSize
            }
        }
    }

    Kirigami.Separator { Layout.fillWidth: true }

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

                columnWidthProvider: function(column) {
                    switch (column) {
                        case 0: return 280   // Name
                        case 1: return 90    // Size
                        case 2: return 140   // Date
                        case 3: return 380   // Folder
                        case 4: return 160   // Catalog
                    }
                    return 120
                }

                delegate: Rectangle {
                    required property int     row
                    required property int     column
                    required property var     display
                    implicitHeight: 26
                    color: tableView.selectedRow === row
                           ? Kirigami.Theme.highlightColor
                           : (row % 2 === 0
                              ? Kirigami.Theme.backgroundColor
                              : Kirigami.Theme.alternateBackgroundColor)

                    // Column separator
                    Rectangle {
                        visible: column > 0
                        anchors { top: parent.top; bottom: parent.bottom; left: parent.left }
                        width: 1
                        color: Kirigami.Theme.separatorColor
                        opacity: 0.4
                    }

                    Controls.Label {
                        anchors {
                            left: parent.left; right: parent.right
                            verticalCenter: parent.verticalCenter
                            leftMargin:  column === 0 ? 8 : 6
                            rightMargin: 4
                        }
                        text: {
                            if (!display) return ""
                            if (column === 1) return formatFileSize(Number(display))
                            return String(display)
                        }
                        color: tableView.selectedRow === row
                               ? Kirigami.Theme.highlightedTextColor
                               : Kirigami.Theme.textColor
                        // Elide folder path from left to keep filename visible
                        elide: column === 3 ? Text.ElideLeft : Text.ElideRight
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
