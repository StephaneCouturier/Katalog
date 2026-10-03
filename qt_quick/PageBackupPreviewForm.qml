import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import QtQuick.Controls   // required for ScrollBar attached property (namespace alias prevents it)
import org.kde.kirigami as Kirigami

Kirigami.Page {
    id: root
    property Kirigami.Action escapeAction: escapeCloseAction  // Esc (KBS-F1)
    title: qsTr("Backup Preview")
    padding: 0

    property int mappingId: -1
    property var previewData: null

    // Header labels for the resizable preview table (BKP-F15). Same strings as the
    // former Repeater header row, kept in this file so no translation slot changes.
    readonly property var columnHeaders: [qsTr("Status"), qsTr("File Name"), qsTr("Path"), qsTr("Size")]

    function doPreview() {
        // The compare already ran on the Backup page (BKP-F16); read its result and the
        // populated table model — no compare happens on this page.
        root.previewData = appManager1.lastBackupPreviewSummary()
        if (root.previewData && root.previewData.mappingName)
            root.title = qsTr("Preview") + " - " + root.previewData.mappingName
    }

    function closeLayer() {
        pageStack.layers.pop()
    }

    // The Backup page runs any pre-preview catalog update and only pushes this page once
    // the report is ready (BKP-F14), so the preview is computed synchronously here.
    Component.onCompleted: doPreview()

    actions: [
        Kirigami.Action {
            text:        qsTr("Export to CSV")
            icon.name:   "document-export"
            displayHint: Kirigami.DisplayHint.KeepVisible
            enabled:     root.previewData && (root.previewData.filesToCopy.length + root.previewData.fileConflicts.length) > 0
            onTriggered: {
                var path = appManager1.exportLastBackupPreviewToCsv()
                exportMsg.text    = path
                exportMsg.visible = true
            }
        },
        Kirigami.Action {
            id: escapeCloseAction
            text:        qsTr("Close")
            icon.name:   "view-close"
            displayHint: Kirigami.DisplayHint.KeepVisible
            onTriggered: root.closeLayer()
        }
    ]

    ColumnLayout {
        anchors.fill: parent
        spacing: Kirigami.Units.smallSpacing

        // Computing state (momentary — preview is synchronous)
        Kirigami.PlaceholderMessage {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
            visible: root.previewData === null
            text: qsTr("Computing preview…")
            icon.name: "view-refresh"
        }

        // Error state
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            type:    Kirigami.MessageType.Error
            visible: root.previewData !== null && root.previewData.error !== ""
            text:    root.previewData ? root.previewData.error : ""
        }

        // Export feedback
        Kirigami.InlineMessage {
            id: exportMsg
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            type: Kirigami.MessageType.Positive
            visible: false
            showCloseButton: true
        }

        // Summary
        Kirigami.InlineMessage {
            id: summaryMsg
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            type: Kirigami.MessageType.Information
            showCloseButton: false
            visible: root.previewData !== null && root.previewData.hasData
            text: {
                if (!root.previewData) return ""
                var pd = root.previewData
                var copyLabel  = pd.isArchive ? qsTr("Move") : qsTr("Copy")
                var copyCount  = pd.filesToCopy  ? pd.filesToCopy.length  : 0
                var confCount  = pd.fileConflicts ? pd.fileConflicts.length : 0
                return copyLabel + ": " + copyCount + " " + qsTr("file(s)")
                    + " · " + qsTr("Conflicts: %1").arg(confCount)
                    + " · " + qsTr("Already in target: %1").arg(pd.skippedCount || 0)
                    + (pd.sourceActive ? "" : " - ⚠ " + qsTr("source offline"))
                    + (pd.targetActive ? "" : " - ⚠ " + qsTr("target offline"))
            }
        }

        // Space warning
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            type: {
                if (!root.previewData) return Kirigami.MessageType.Information
                if (root.previewData.spaceStatus === "Insufficient") return Kirigami.MessageType.Error
                return Kirigami.MessageType.Warning
            }
            visible: root.previewData !== null
                     && (root.previewData.spaceStatus === "Low" || root.previewData.spaceStatus === "Insufficient")
            text: {
                if (!root.previewData) return ""
                if (root.previewData.spaceStatus === "Insufficient")
                    return qsTr("Insufficient disk space - Required: %1, Available: %2")
                        .arg(appManager1.formatDataSize(root.previewData.spaceRequired))
                        .arg(appManager1.formatDataSize(root.previewData.spaceAvailable))
                return qsTr("Low target space - %1 remaining after operation")
                    .arg(appManager1.formatDataSize(root.previewData.spaceAvailable - root.previewData.spaceRequired))
            }
            showCloseButton: false
        }

        // ── Resizable file table: Status | File Name | Path | Size (BKP-F15) ──
        // Column widths are user-adjustable by dragging the header dividers,
        // consistent with the search-results and explore tables.
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            visible: root.previewData !== null && root.previewData.hasData
                     && (root.previewData.filesToCopy.length + root.previewData.fileConflicts.length) > 0

            Controls.HorizontalHeaderView {
                id: previewHeaderView
                anchors { top: parent.top; left: parent.left; right: parent.right }
                syncView: previewTableView
                clip: true
                implicitHeight: 34
                resizableColumns: true

                delegate: Rectangle {
                    required property int column
                    color: Kirigami.Theme.backgroundColor
                    implicitHeight: previewHeaderView.implicitHeight
                    // Full, unelided width, read by the double-click fit (CDT-F4).
                    implicitWidth: 12 + headerLabel.implicitWidth

                    Controls.Label {
                        id: headerLabel
                        anchors {
                            left: parent.left; right: parent.right
                            verticalCenter: parent.verticalCenter
                            leftMargin: 6; rightMargin: 6
                        }
                        text: root.columnHeaders[column] !== undefined ? root.columnHeaders[column] : ""
                        elide: Text.ElideRight
                        font.bold: true
                        opacity: 0.7
                        color: Kirigami.Theme.textColor
                    }

                    Rectangle { // Vertical separator
                        anchors { top: parent.top; bottom: parent.bottom; right: parent.right }
                        width: 1
                        color: Kirigami.Theme.separatorColor ?? "transparent"
                    }

                    // This header does not sort; its only tap action is the
                    // divider double-click fit (CDT-F4).
                    TapHandler {
                        onTapped: function(eventPoint) {
                            applicationWindow().headerTapped(previewTableView, previewHeaderView, column,
                                                             parent.width, eventPoint.position.x)
                        }
                    }
                }
            }

            Kirigami.Separator {
                id: previewHeaderSep
                anchors { top: previewHeaderView.bottom; left: parent.left; right: parent.right }
            }

            TableView {
                id: previewTableView
                anchors { top: previewHeaderSep.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }
                columnSpacing: 0
                rowSpacing: 0
                clip: true
                model: appManager1.backupPreviewModel

                ScrollBar.vertical:   ScrollBar { policy: ScrollBar.AsNeeded }
                ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AsNeeded }

                rowHeightProvider: function(row) { return 30 }

                // Keyboard navigation (CDT-F5–F11), shared in Main.qml. There is
                // no row action and no row menu here (CDT-F7/F8): the keys move
                // the selection, type-ahead matches File Name (CDT-F9) and
                // Left/Right scroll (CDT-F11).
                property int selectedRow: -1
                activeFocusOnTab: true
                Keys.onPressed: function(event) {
                    applicationWindow().tableKeyPressed(previewTableView, event, {
                        nameAt: function(r) {
                            let m = appManager1.backupPreviewModel
                            return m.data(m.index(r, 1), Qt.DisplayRole)   // File Name
                        }
                    })
                }

                columnWidthProvider: function(column) {
                    let w = previewTableView.explicitColumnWidth(column)
                    if (w >= 0) return w
                    switch (column) {
                        case 0: return 110  // Status
                        case 1: return 220  // File Name
                        case 2: return 360  // Path
                        case 3: return 110  // Size
                    }
                    return 120
                }

                delegate: Rectangle {
                    required property int    row
                    required property int    column
                    required property string fileName
                    required property string folderPath
                    required property string fileSizeStr
                    required property bool   isConflict
                    implicitHeight: 30
                    // Full, unelided content width, read by the double-click fit (CDT-F4).
                    implicitWidth: 12 + cellLabel.implicitWidth

                    readonly property bool darkTheme: Kirigami.Theme.backgroundColor.hslLightness < 0.5
                    color: previewTableView.selectedRow === row
                           ? applicationWindow().selectionHighlightColor
                           : row % 2 === 0
                             ? (darkTheme ? Kirigami.Theme.backgroundColor : "#ffffff")
                             : (darkTheme ? "#161b1d" : "#e9f7fc")

                    // Column separator
                    Rectangle {
                        visible: column > 0
                        anchors { top: parent.top; bottom: parent.bottom; left: parent.left }
                        width: 1
                        color: Kirigami.Theme.separatorColor ?? Kirigami.Theme.textColor
                        opacity: applicationWindow().columnSeparatorOpacity
                    }

                    Controls.Label {
                        id: cellLabel
                        anchors {
                            left: parent.left; right: parent.right
                            verticalCenter: parent.verticalCenter
                            leftMargin: 6; rightMargin: 6
                        }
                        elide: Text.ElideRight
                        horizontalAlignment: column === 3 ? Text.AlignRight : Text.AlignLeft
                        opacity: column === 2 ? 0.7 : 1.0
                        color: previewTableView.selectedRow === row
                               ? Kirigami.Theme.highlightedTextColor
                               : column === 0
                                 ? (isConflict ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.positiveTextColor)
                                 : Kirigami.Theme.textColor
                        text: {
                            switch (column) {
                                case 0: return isConflict ? qsTr("Conflict")
                                                          : (root.previewData && root.previewData.isArchive ? qsTr("Move") : qsTr("Copy"))
                                case 1: return fileName
                                case 2: return folderPath
                                case 3: return fileSizeStr
                            }
                            return ""
                        }
                    }

                    // Selects the row and gives the table keyboard focus (CDT-F5).
                    TapHandler {
                        onTapped: {
                            previewTableView.selectedRow = row
                            previewTableView.forceActiveFocus()
                        }
                    }
                }
            }
        }

        // Empty preview (in sync)
        Kirigami.PlaceholderMessage {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: root.previewData !== null
                     && root.previewData.filesToCopy.length === 0
                     && root.previewData.fileConflicts.length === 0
            text: (root.previewData && root.previewData.isArchive) ? qsTr("Nothing to move")
                                                                   : qsTr("Nothing to copy")
            explanation: qsTr("No files need to be copied or moved.")
            icon.name: "checkmark"
        }
    }
}
