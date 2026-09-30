import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import QtQuick.Dialogs

ColumnLayout {
    id: root
    spacing: 0

    property var    tagEntries: []
    property var    tagNames:   []
    property string selectedTagFilter: ""

    function reload() {
        tagNames   = appManager1.getTagNames()
        tagEntries = appManager1.getTagEntries(root.selectedTagFilter)
    }

    Connections {
        target: appManager1
        function onTagsChanged() { root.reload() }
    }

    Component.onCompleted: reload()

    FolderDialog {
        id: tagFolderDialog
        onAccepted: folderPathField.text = appManager1.pathFromFileUrl(selectedFolder.toString())
    }

    Kirigami.InlineMessage {
        id: tagValidationMessage
        Layout.fillWidth: true
        Layout.leftMargin:  Kirigami.Units.gridUnit
        Layout.rightMargin: Kirigami.Units.gridUnit
        Layout.topMargin:   Kirigami.Units.gridUnit
        type: Kirigami.MessageType.Warning
        showCloseButton: true
        visible: false
    }

    // ═══ Add a tag ════════════════════════════════════════════════════════════
    GridLayout {
        Layout.fillWidth: true
        Layout.margins: Kirigami.Units.gridUnit
        columns: 2
        columnSpacing: Kirigami.Units.largeSpacing
        rowSpacing: Kirigami.Units.smallSpacing

        RowLayout {
            Layout.columnSpan: 2
            Layout.topMargin: Kirigami.Units.smallSpacing
            Layout.fillWidth: true
            spacing: Kirigami.Units.largeSpacing
            Controls.Label { text: qsTr("Add a tag"); color: Kirigami.Theme.linkColor }
            Kirigami.Separator { Layout.fillWidth: true; Layout.alignment: Qt.AlignVCenter }
        }

        Controls.Label { text: qsTr("Folder"); opacity: 0.7 }
        RowLayout {
            Layout.fillWidth: true
            Controls.TextField {
                id: folderPathField
                Layout.fillWidth: true
                placeholderText: qsTr("Folder path")
                onTextChanged: tagValidationMessage.visible = false
            }
            IconButton {
                icon.name: "folder-open"
                text: qsTr("Select a folder")
                onClicked: {
                    var p = folderPathField.text.trim()
                    tagFolderDialog.currentFolder = appManager1.pathToFileUrl(
                        p !== "" ? p : appManager1.getCollectionBrowsePath())
                    tagFolderDialog.open()
                }
            }
        }

        Controls.Label { text: qsTr("Tag"); opacity: 0.7 }
        RowLayout {
            Layout.fillWidth: true
            ComboBoxFitted {
                id: tagNameCombo
                Layout.fillWidth: true
                Layout.maximumWidth: Number.POSITIVE_INFINITY  // stretches with the form (CBX-C7)
                editable: true
                model: root.tagNames
                onEditTextChanged: tagValidationMessage.visible = false
            }
            Controls.Button {
                icon.name: "tag"
                text: qsTr("Tag the folder")
                onClicked: {
                    var folder = folderPathField.text.trim()
                    var name   = tagNameCombo.editText.trim()
                    if (folder === "") {
                        tagValidationMessage.text = qsTr("Select or enter a folder to tag.")
                        tagValidationMessage.visible = true
                        return
                    }
                    if (name === "") {
                        tagValidationMessage.text = qsTr("Select or enter a tag name.")
                        tagValidationMessage.visible = true
                        return
                    }
                    appManager1.createTag(name, folder)
                    folderPathField.text = ""
                    tagNameCombo.currentIndex = -1
                }
            }
        }
    }

    // ═══ Current folders and tags ════════════════════════════════════════════
    GridLayout {
        Layout.fillWidth: true
        Layout.margins: Kirigami.Units.gridUnit
        columns: 2
        columnSpacing: Kirigami.Units.largeSpacing
        rowSpacing: Kirigami.Units.smallSpacing

        RowLayout {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.smallSpacing
            spacing: Kirigami.Units.largeSpacing
            Controls.Label { text: qsTr("Current folders and tags"); color: Kirigami.Theme.linkColor }
            Kirigami.Separator { Layout.fillWidth: true; Layout.alignment: Qt.AlignVCenter }
        }
        // Tag filter on the row below the title, above the Tag column (PGL-F7)
        Item { Layout.fillWidth: true }
        ComboBoxFitted {
            id: filterCombo
            model: [qsTr("All")].concat(root.tagNames)
            onActivated: {
                root.selectedTagFilter = currentIndex === 0 ? "" : currentText
                root.reload()
            }
        }

        // Column headers
        Controls.Label {
            text: qsTr("Folder")
            opacity: 0.7
            Layout.fillWidth: true
        }
        Controls.Label {
            text: qsTr("Tag")
            opacity: 0.7
        }

        Kirigami.Separator { Layout.fillWidth: true; Layout.columnSpan: 2 }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.columnSpan: 2
            spacing: 2

            Repeater {
                model: root.tagEntries
                delegate: RowLayout {
                    Layout.fillWidth: true
                    Controls.Label {
                        text: modelData.folder
                        elide: Text.ElideMiddle
                        Layout.fillWidth: true
                    }
                    Controls.Label {
                        text: modelData.name
                        Layout.preferredWidth: 120
                        horizontalAlignment: Text.AlignLeft
                    }
                    IconButton {
                        icon.name: "edit-delete"
                        text: qsTr("Remove this tag")
                        flat: true
                        onClicked: appManager1.deleteTag(modelData.tagId)
                    }
                }
            }

            Controls.Label {
                visible: root.tagEntries.length === 0
                text: qsTr("No tags")
                opacity: 0.5
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: Kirigami.Units.gridUnit
            }
        }
    }

    Item { Layout.fillHeight: true }
}
