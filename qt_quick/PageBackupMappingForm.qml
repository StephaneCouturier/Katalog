import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

ScrollablePageFitted {
    id: root
    property Kirigami.Action escapeAction: escapeCloseAction  // Esc (KBS-F1)

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
            includeEmptyDirsCheck.checked = mappingData.includeEmptyDirs
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
                sourceDriveCheck.checked,
                includeEmptyDirsCheck.checked)
            : appManager1.updateBackupMapping(
                editMappingId,
                nameField.text.trim(),
                typeCombo.currentValue,
                sourceCombo.selectedDeviceId,
                targetCombo.selectedDeviceId,
                strict,
                conflictModeCombo.currentValue,
                sourceDriveCheck.checked,
                includeEmptyDirsCheck.checked)
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
            id: escapeCloseAction
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

        GridLayout {
            Layout.fillWidth: true
            columns: 2
            columnSpacing: Kirigami.Units.largeSpacing
            rowSpacing: Kirigami.Units.smallSpacing

            // Source
            RowLayout {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                Layout.topMargin: Kirigami.Units.smallSpacing
                spacing: Kirigami.Units.largeSpacing
                Controls.Label { text: qsTr("Source"); color: Kirigami.Theme.linkColor }
                Kirigami.Separator { Layout.fillWidth: true; Layout.alignment: Qt.AlignVCenter }
            }

            Controls.Label { text: qsTr("Source catalog"); opacity: 0.7 }
            DeviceTreeComboBox {
                id: sourceCombo
                Layout.preferredWidth: Kirigami.Units.gridUnit * 24
                Layout.maximumWidth: Kirigami.Units.gridUnit * 24  // set width, shrinks only to fit (CBX-C7)
                catalogOnly: true
            }

            // Target
            RowLayout {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                Layout.topMargin: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
                spacing: Kirigami.Units.largeSpacing
                Controls.Label { text: qsTr("Target"); color: Kirigami.Theme.linkColor }
                Kirigami.Separator { Layout.fillWidth: true; Layout.alignment: Qt.AlignVCenter }
            }

            Controls.Label { text: qsTr("Target catalog"); opacity: 0.7 }
            DeviceTreeComboBox {
                id: targetCombo
                Layout.preferredWidth: Kirigami.Units.gridUnit * 24
                Layout.maximumWidth: Kirigami.Units.gridUnit * 24  // set width, shrinks only to fit (CBX-C7)
                catalogOnly: true
            }

            // Options
            RowLayout {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                Layout.topMargin: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
                spacing: Kirigami.Units.largeSpacing
                Controls.Label { text: qsTr("Options"); color: Kirigami.Theme.linkColor }
                Kirigami.Separator { Layout.fillWidth: true; Layout.alignment: Qt.AlignVCenter }
            }

            // Name + auto-generate
            Controls.Label { text: qsTr("Name"); opacity: 0.7 }
            RowLayout {
                Controls.TextField {
                    id: nameField
                    placeholderText: qsTr("e.g. Docs → NAS_Docs")
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 20
                }
                IconButton {
                    icon.name: "system-run"
                    Controls.ToolTip.text: qsTr("Auto-generate name")
                    Controls.ToolTip.visible: hovered
                    onClicked: root.generateName()
                }
            }

            // Type
            Controls.Label { text: qsTr("Type"); opacity: 0.7 }
            ComboBoxFitted {
                id: typeCombo
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

            Controls.Label { text: qsTr("Directories"); opacity: 0.7 }
            Controls.CheckBox {
                id: includeEmptyDirsCheck
                text: qsTr("Include empty")
                checked: true
            }

            Controls.Label { text: qsTr("Strict copy"); opacity: 0.7 }
            Controls.CheckBox {
                id: strictCopyCheck
                text: qsTr("Mirror folder structure exactly (default)")
                checked: true
            }

            Controls.Label { text: qsTr("On conflict"); opacity: 0.7 }
            ComboBoxFitted {
                id: conflictModeCombo
                textRole:  "text"
                valueRole: "value"
                model: [
                    { value: "RenameOldest", text: qsTr("Rename oldest - rename target, copy source") },
                    { value: "Skip",         text: qsTr("Skip - leave target untouched") }
                ]
            }

            Controls.Label { text: qsTr("Source mode"); opacity: 0.7 }
            Controls.CheckBox {
                id: sourceDriveCheck
                text: qsTr("Scan source drive directly (requires connected source)")
            }
        }
    }
}
