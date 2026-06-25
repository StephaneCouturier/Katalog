import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    // -1 = creating a new link; >= 0 = editing the link with this mapping id.
    property int editMappingId: -1
    // Initial values used to pre-fill the form when editing (set via showLayer props).
    property var mappingData: null

    title: editMappingId < 0 ? qsTr("Add Link") : qsTr("Edit Link")

    Component.onCompleted: {
        if (editMappingId >= 0 && mappingData) {
            nameField.text          = mappingData.mappingName
            var ti = typeCombo.indexOfValue(mappingData.mappingType)
            if (ti >= 0) typeCombo.currentIndex = ti
            sourceCombo.selectById(mappingData.sourceDeviceId)
            targetCombo.selectById(mappingData.targetDeviceId)
            strictCopyCheck.checked = mappingData.strictCopy
            var ci = conflictModeCombo.indexOfValue(mappingData.conflictMode)
            if (ci >= 0) conflictModeCombo.currentIndex = ci
            sourceDriveCheck.checked = mappingData.sourceDrive
        }
    }

    // Generate a suggested name from source → target names
    function generateName() {
        var srcName = sourceCombo.selectedDeviceName
        var tgtName = targetCombo.selectedDeviceName
        if (srcName && tgtName)
            nameField.text = srcName + " → " + tgtName
    }

    function save() {
        var strict = (typeCombo.currentValue !== "Archive") && strictCopyCheck.checked
        var err = editMappingId < 0
            ? appManager1.createBackupMapping(
                nameField.text.trim(),
                typeCombo.currentValue,
                sourceCombo.selectedDeviceId,
                targetCombo.selectedDeviceId,
                strict,
                conflictModeCombo.currentValue,
                sourceDriveCheck.checked)
            : appManager1.updateBackupMapping(
                editMappingId,
                nameField.text.trim(),
                typeCombo.currentValue,
                sourceCombo.selectedDeviceId,
                targetCombo.selectedDeviceId,
                strict,
                conflictModeCombo.currentValue,
                sourceDriveCheck.checked)
        if (err) {
            errorMessage.text    = err
            errorMessage.visible = true
        } else {
            closeLayer()
        }
    }

    function closeLayer() {
        pageStack.layers.pop()
    }

    actions: [
        Kirigami.Action {
            text:        qsTr("Save")
            icon.name:   "document-save"
            displayHint: Kirigami.DisplayHint.KeepVisible
            onTriggered: root.save()
        },
        Kirigami.Action {
            text:        qsTr("Cancel")
            icon.name:   "dialog-cancel"
            displayHint: Kirigami.DisplayHint.KeepVisible
            onTriggered: root.closeLayer()
        }
    ]

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing
        width: parent.width

        Kirigami.InlineMessage {
            id: errorMessage
            Layout.fillWidth: true
            type: Kirigami.MessageType.Error
            visible: false
            showCloseButton: true
        }

        Kirigami.FormLayout {
            Layout.fillWidth: true

            // Name + auto-generate
            RowLayout {
                Kirigami.FormData.label: qsTr("Name")
                Controls.TextField {
                    id: nameField
                    placeholderText: qsTr("e.g. Docs → NAS_Docs")
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 20
                }
                Controls.Button {
                    icon.name: "system-run"
                    Controls.ToolTip.text: qsTr("Auto-generate name")
                    Controls.ToolTip.visible: hovered
                    onClicked: root.generateName()
                }
            }

            // Type
            Controls.ComboBox {
                id: typeCombo
                Kirigami.FormData.label: qsTr("Type")
                textRole:  "text"
                valueRole: "value"
                model: [
                    { value: "Backup",  text: qsTr("Backup")  },
                    { value: "Archive", text: qsTr("Archive") }
                ]
                onCurrentValueChanged: {
                    // Archive mode disables strict copy
                    strictCopyCheck.enabled = (currentValue !== "Archive")
                    if (currentValue === "Archive") strictCopyCheck.checked = false
                }
            }

            // Source
            Kirigami.Separator {
                Kirigami.FormData.label: qsTr("Source")
                Kirigami.FormData.isSection: true
            }

            DeviceTreeComboBox {
                id: sourceCombo
                Kirigami.FormData.label: qsTr("Source catalog")
                Layout.preferredWidth: Kirigami.Units.gridUnit * 24
                catalogOnly: true
            }

            // Target
            Kirigami.Separator {
                Kirigami.FormData.label: qsTr("Target")
                Kirigami.FormData.isSection: true
            }

            DeviceTreeComboBox {
                id: targetCombo
                Kirigami.FormData.label: qsTr("Target catalog")
                Layout.preferredWidth: Kirigami.Units.gridUnit * 24
                catalogOnly: true
            }

            // Options
            Kirigami.Separator {
                Kirigami.FormData.label: qsTr("Options")
                Kirigami.FormData.isSection: true
            }

            Controls.CheckBox {
                id: strictCopyCheck
                Kirigami.FormData.label: qsTr("Strict copy")
                text: qsTr("Mirror folder structure exactly (default)")
                checked: true
            }

            Controls.ComboBox {
                id: conflictModeCombo
                Kirigami.FormData.label: qsTr("On conflict")
                textRole:  "text"
                valueRole: "value"
                model: [
                    { value: "RenameOldest", text: qsTr("Rename oldest - rename target, copy source") },
                    { value: "Skip",         text: qsTr("Skip - leave target untouched") }
                ]
            }

            Controls.CheckBox {
                id: sourceDriveCheck
                Kirigami.FormData.label: qsTr("Source mode")
                text: qsTr("Scan source drive directly (requires connected source)")
            }
        }
    }
}
