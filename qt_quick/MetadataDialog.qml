import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

// Extended metadata viewer, shared by the Search results and Explore pages.
// Call showFor(catalogId, fileName, folderPath): it fetches the parsed fields and
// the raw JSON, and reports when a file has no extended metadata.
Kirigami.Dialog {
    id: metadataDialog
    property string   jsonText:    ""
    property var      fields:      []   // QVariantList of {label, value}
    property string   fileLabel:   ""

    // Fetch and show the metadata for one file, or report that it has none.
    function showFor(catalogId, fileName, folderPath) {
        let f = appManager1.getFileMetadataParsedFields(catalogId, fileName, folderPath)
        if (f.length > 0) {
            metadataDialog.fields    = f
            metadataDialog.jsonText  = appManager1.getFileMetadataJson(catalogId, fileName, folderPath)
            metadataDialog.fileLabel = fileName
            metadataDialog.open()
        } else {
            applicationWindow().showPassiveNotification(qsTr("No extended metadata available"))
        }
    }

    title: qsTr("Extended Metadata")
    width: 500
    preferredHeight: Kirigami.Units.gridUnit * 32

    standardButtons: Kirigami.Dialog.Close

    customFooterActions: [
        Kirigami.Action {
            text: qsTr("Copy JSON")
            icon.name: "edit-copy"
            onTriggered: {
                appManager1.copyToClipboard(metadataDialog.jsonText)
                applicationWindow().showPassiveNotification(qsTr("Metadata JSON copied to clipboard"))
            }
        }
    ]

    contentItem: ColumnLayout {
        spacing: 0

        // File name header
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: fileNameLabel.implicitHeight + Kirigami.Units.smallSpacing * 2
            color: Kirigami.Theme.alternateBackgroundColor
            radius: 4

            Controls.Label {
                id: fileNameLabel
                anchors {
                    left: parent.left; right: parent.right
                    verticalCenter: parent.verticalCenter
                    leftMargin: Kirigami.Units.smallSpacing * 2
                    rightMargin: Kirigami.Units.smallSpacing * 2
                }
                text: metadataDialog.fileLabel
                font.bold: true
                elide: Text.ElideMiddle
            }
        }

        Kirigami.Separator { Layout.fillWidth: true }

        // Column headers
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 28
            color: Kirigami.Theme.backgroundColor

            Controls.Label {
                x: 8; y: 0
                width: 152
                height: parent.height
                text: qsTr("Field")
                font.bold: true
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.9
                verticalAlignment: Text.AlignVCenter
            }
            Rectangle {
                x: 160; y: 4
                width: 1; height: parent.height - 8
                color: Kirigami.Theme.separatorColor ?? "transparent"; opacity: 0.5
            }
            Controls.Label {
                x: 169; y: 0
                width: parent.width - x - 8
                height: parent.height
                text: qsTr("Value")
                font.bold: true
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.9
                verticalAlignment: Text.AlignVCenter
            }
        }

        Kirigami.Separator { Layout.fillWidth: true }

        // Scrollable table
        Controls.ScrollView {
            id: metadataScrollView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            ListView {
                id: metadataListView
                width: metadataScrollView.availableWidth
                model: metadataDialog.fields
                clip: true

                delegate: Rectangle {
                    required property var   modelData
                    required property int   index
                    width: ListView.view.width

                    readonly property int   colWidth: 160
                    readonly property int   hPad:     8
                    implicitHeight: Math.max(28, valueText.implicitHeight + 10)

                    readonly property bool darkTheme: Kirigami.Theme.backgroundColor.hslLightness < 0.5
                    color: index % 2 === 0
                           ? (darkTheme ? Kirigami.Theme.backgroundColor : "#ffffff")
                           : (darkTheme ? "#161b1d" : "#f5f5f5")

                    Controls.Label {
                        x: hPad
                        y: 5
                        width: colWidth - hPad
                        text: modelData.label
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.9
                        wrapMode: Text.NoWrap
                        elide: Text.ElideRight
                        opacity: 0.8
                        color: Kirigami.Theme.textColor
                    }

                    Rectangle {
                        x: colWidth
                        y: 0
                        width: 1
                        height: parent.height
                        color: Kirigami.Theme.separatorColor ?? Kirigami.Theme.textColor
                        opacity: 0.4
                    }

                    Controls.Label {
                        id: valueText
                        x: colWidth + 1 + hPad
                        y: 5
                        width: parent.width - x - hPad
                        text: modelData.value
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.9
                        wrapMode: Text.WrapAnywhere
                        color: Kirigami.Theme.textColor
                    }

                    Kirigami.Separator {
                        anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
                        opacity: 0.3
                    }
                }
            }
        }
    }
}
