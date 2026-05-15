import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root
    title: qsTr("Backup Preview")

    property int mappingId: -1
    property var previewData: null

    function loadPreview() {
        if (root.mappingId < 0) return
        root.previewData = appManager1.previewBackup(root.mappingId)
        if (root.previewData && root.previewData.mappingName)
            root.title = qsTr("Preview") + " — " + root.previewData.mappingName
    }

    function closeLayer() {
        pageStack.layers.pop()
    }

    Component.onCompleted: loadPreview()

    actions: [
        Kirigami.Action {
            text:      qsTr("Export to CSV")
            icon.name: "document-export"
            enabled:   root.previewData && (root.previewData.filesToCopy.length + root.previewData.fileConflicts.length) > 0
            onTriggered: {
                var path = appManager1.exportLastBackupPreviewToCsv()
                exportMsg.text    = path
                exportMsg.visible = true
            }
        },
        Kirigami.Action {
            text:      qsTr("Close")
            icon.name: "view-close"
            onTriggered: root.closeLayer()
        }
    ]

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing
        width: parent.width

        // Loading state
        Kirigami.PlaceholderMessage {
            Layout.fillWidth: true
            visible: root.previewData === null
            text: qsTr("Computing preview…")
            icon.name: "hourglass"
        }

        // Error state
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            type:    Kirigami.MessageType.Error
            visible: root.previewData !== null && root.previewData.error !== ""
            text:    root.previewData ? root.previewData.error : ""
        }

        // Export feedback
        Kirigami.InlineMessage {
            id: exportMsg
            Layout.fillWidth: true
            type: Kirigami.MessageType.Positive
            visible: false
            showCloseButton: true
        }

        // Main content (when data is available)
        ColumnLayout {
            visible: root.previewData !== null && root.previewData.hasData
            spacing: Kirigami.Units.largeSpacing
            Layout.fillWidth: true

            // Summary
            Kirigami.InlineMessage {
                id: summaryMsg
                Layout.fillWidth: true
                type: Kirigami.MessageType.Information
                showCloseButton: false
                text: {
                    if (!root.previewData) return ""
                    var pd = root.previewData
                    var copyLabel  = pd.isArchive ? qsTr("Move") : qsTr("Copy")
                    var copyCount  = pd.filesToCopy  ? pd.filesToCopy.length  : 0
                    var confCount  = pd.fileConflicts ? pd.fileConflicts.length : 0
                    return copyLabel + ": " + copyCount + " " + qsTr("file(s)")
                        + " · " + qsTr("Conflicts: %1").arg(confCount)
                        + " · " + qsTr("Already in target: %1").arg(pd.skippedCount || 0)
                        + (pd.sourceActive ? "" : " — ⚠ " + qsTr("source offline"))
                        + (pd.targetActive ? "" : " — ⚠ " + qsTr("target offline"))
                }
            }

            // Space warning
            Kirigami.InlineMessage {
                Layout.fillWidth: true
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
                        return qsTr("Insufficient disk space — Required: %1, Available: %2")
                            .arg(appManager1.formatDataSize(root.previewData.spaceRequired))
                            .arg(appManager1.formatDataSize(root.previewData.spaceAvailable))
                    return qsTr("Low target space — %1 remaining after operation")
                        .arg(appManager1.formatDataSize(root.previewData.spaceAvailable - root.previewData.spaceRequired))
                }
                showCloseButton: false
            }

            // Files to copy / move
            ColumnLayout {
                visible: root.previewData && root.previewData.filesToCopy && root.previewData.filesToCopy.length > 0
                spacing: Kirigami.Units.smallSpacing
                Layout.fillWidth: true

                Controls.Label {
                    text: root.previewData
                        ? (root.previewData.isArchive ? qsTr("Files to move") : qsTr("Files to copy"))
                            + " (" + (root.previewData.filesToCopy ? root.previewData.filesToCopy.length : 0) + ")"
                        : ""
                    font.bold: true
                }

                // Column headers
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 0
                    Controls.Label { text: qsTr("File Name");  Layout.preferredWidth: Kirigami.Units.gridUnit * 12; font.bold: true; opacity: 0.7 }
                    Controls.Label { text: qsTr("Path");       Layout.fillWidth: true;                              font.bold: true; opacity: 0.7 }
                    Controls.Label { text: qsTr("Size");       Layout.preferredWidth: Kirigami.Units.gridUnit * 7;  font.bold: true; opacity: 0.7 }
                }
                Kirigami.Separator { Layout.fillWidth: true }

                Repeater {
                    model: root.previewData ? root.previewData.filesToCopy : []
                    delegate: RowLayout {
                        required property var modelData
                        Layout.fillWidth: true
                        spacing: 0
                        Controls.Label {
                            text:  modelData.fileName
                            elide: Text.ElideRight
                            Layout.preferredWidth: Kirigami.Units.gridUnit * 12
                        }
                        Controls.Label {
                            text:  modelData.folderPath
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                            opacity: 0.7
                        }
                        Controls.Label {
                            text:  modelData.fileSizeStr
                            horizontalAlignment: Text.AlignRight
                            Layout.preferredWidth: Kirigami.Units.gridUnit * 7
                        }
                    }
                }
            }

            // Conflicts
            ColumnLayout {
                visible: root.previewData && root.previewData.fileConflicts && root.previewData.fileConflicts.length > 0
                spacing: Kirigami.Units.smallSpacing
                Layout.fillWidth: true

                Controls.Label {
                    text: qsTr("Conflicts")
                        + " (" + (root.previewData && root.previewData.fileConflicts ? root.previewData.fileConflicts.length : 0) + ")"
                    font.bold: true
                    color: Kirigami.Theme.neutralTextColor
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 0
                    Controls.Label { text: qsTr("File Name"); Layout.preferredWidth: Kirigami.Units.gridUnit * 12; font.bold: true; opacity: 0.7 }
                    Controls.Label { text: qsTr("Path");      Layout.fillWidth: true;                              font.bold: true; opacity: 0.7 }
                    Controls.Label { text: qsTr("Size");      Layout.preferredWidth: Kirigami.Units.gridUnit * 7;  font.bold: true; opacity: 0.7 }
                }
                Kirigami.Separator { Layout.fillWidth: true }

                Repeater {
                    model: root.previewData ? root.previewData.fileConflicts : []
                    delegate: RowLayout {
                        required property var modelData
                        Layout.fillWidth: true
                        spacing: 0
                        Controls.Label {
                            text:  modelData.fileName
                            elide: Text.ElideRight
                            Layout.preferredWidth: Kirigami.Units.gridUnit * 12
                        }
                        Controls.Label {
                            text:  modelData.folderPath
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                            opacity: 0.7
                        }
                        Controls.Label {
                            text:  modelData.fileSizeStr
                            horizontalAlignment: Text.AlignRight
                            Layout.preferredWidth: Kirigami.Units.gridUnit * 7
                        }
                    }
                }
            }
        }

        // Empty preview (in sync)
        Kirigami.PlaceholderMessage {
            Layout.fillWidth: true
            visible: root.previewData !== null && root.previewData.hasData
                     && root.previewData.filesToCopy.length === 0
                     && root.previewData.fileConflicts.length === 0
            text: qsTr("Everything is in sync")
            explanation: qsTr("No files need to be copied or moved.")
            icon.name: "checkmark"
        }
    }
}
