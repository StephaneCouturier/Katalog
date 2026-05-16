import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

Item {
    id: root
    anchors.fill: parent

    signal editDeviceRequested(int deviceId)

    property real   cardScale: 1.0
    property string viewFilter: "All"
    property bool   filterFromSelection: false
    property var    devices: []
    property string operationDeviceName: ""
    property bool   operationDeviceActive: false

    function refreshDevices() {
        var scope = filterFromSelection ? appManager1.selectedDeviceId : 0
        devices = appManager1.getDeviceList(viewFilter, scope)
    }

    Connections {
        target: appManager1
        function onDeviceListChanged()    { root.refreshDevices() }
        function onSelectedDeviceChanged() {
            if (root.filterFromSelection) root.refreshDevices()
        }
        function onSplitCompleted(success, error) {
            root.refreshDevices()
            if (!success && error.length > 0) {
                operationResultLabel.text = error
                operationResultDialog.open()
            }
        }
    }

    Component.onCompleted: refreshDevices()

    // ── Dialogs ────────────────────────────────────────────────────────────

    Controls.Dialog {
        id: splitSubDirConfirmDialog
        property int deviceId: 0
        title: qsTr("Split Catalog")
        modal: true
        anchors.centerIn: parent
        width: Math.min(420, root.width - Kirigami.Units.largeSpacing * 4)
        contentItem: Controls.Label {
            text: qsTr("Split <b>%1</b> into sub-catalogs by sub-directory?<br/><br/>This will create one sub-catalog per immediate sub-directory and remove the original catalog. This operation cannot be undone.").arg(root.operationDeviceName)
            textFormat: Text.RichText
            wrapMode: Text.WordWrap
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("Split")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
            Controls.Button {
                text: qsTr("Cancel")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.RejectRole
            }
            onAccepted: {
                var id = splitSubDirConfirmDialog.deviceId
                splitSubDirConfirmDialog.close()
                var err = appManager1.splitCatalogBySubDirectory(id)
                if (err.length > 0) {
                    operationResultLabel.text = err
                    operationResultDialog.open()
                } else {
                    root.refreshDevices()
                }
            }
            onRejected: splitSubDirConfirmDialog.close()
        }
    }

    Controls.Dialog {
        id: splitFileTypeDialog
        property int  deviceId:    0
        property bool deviceActive: false
        title: qsTr("Split Catalog")
        modal: true
        anchors.centerIn: parent
        width: Math.min(420, root.width - Kirigami.Units.largeSpacing * 4)
        contentItem: ColumnLayout {
            spacing: Kirigami.Units.largeSpacing
            Controls.Label {
                text: qsTr("Split <b>%1</b> into sub-catalogs by file type:").arg(root.operationDeviceName)
                textFormat: Text.RichText
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing
                Controls.Button {
                    text: qsTr("Verify then Split")
                    icon.name: "dialog-ok"
                    Layout.fillWidth: true
                    enabled: splitFileTypeDialog.deviceActive
                    onClicked: {
                        var id = splitFileTypeDialog.deviceId
                        splitFileTypeDialog.close()
                        appManager1.splitCatalogByFileType(id, true)
                    }
                }
                Controls.Button {
                    text: qsTr("Split without verifying")
                    icon.name: "edit-cut"
                    Layout.fillWidth: true
                    onClicked: {
                        var id = splitFileTypeDialog.deviceId
                        splitFileTypeDialog.close()
                        appManager1.splitCatalogByFileType(id, false)
                    }
                }
            }
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("Cancel")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.RejectRole
            }
            onRejected: splitFileTypeDialog.close()
        }
    }

    Controls.Dialog {
        id: verifyConfirmDialog
        property int deviceId: 0
        title: qsTr("Verify Checksums")
        modal: true
        anchors.centerIn: parent
        width: Math.min(420, root.width - Kirigami.Units.largeSpacing * 4)
        contentItem: Controls.Label {
            text: qsTr("Verify checksums for <b>%1</b>?").arg(root.operationDeviceName)
            textFormat: Text.RichText
            wrapMode: Text.WordWrap
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("Verify")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
            Controls.Button {
                text: qsTr("Cancel")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.RejectRole
            }
            onAccepted: {
                var id = verifyConfirmDialog.deviceId
                verifyConfirmDialog.close()
                var result = appManager1.verifyDeviceChecksums(id)
                if (result.noChecksums) {
                    verifyResultLabel.text = qsTr("No checksums are stored for this catalog.")
                } else {
                    verifyResultLabel.text =
                        qsTr("Total: %1").arg(result.total) + "\n" +
                        qsTr("Verified: %1").arg(result.verified) + "\n" +
                        qsTr("Mismatches: %1").arg(result.mismatches) + "\n" +
                        qsTr("Missing: %1").arg(result.missing)
                    if (result.mismatches > 0 && result.mismatchedFiles && result.mismatchedFiles.length > 0)
                        verifyResultLabel.text += "\n\n" + result.mismatchedFiles.join("\n")
                }
                verifyResultDialog.open()
            }
            onRejected: verifyConfirmDialog.close()
        }
    }

    Controls.Dialog {
        id: verifyResultDialog
        title: qsTr("Checksum Verification")
        modal: true
        anchors.centerIn: parent
        width: Math.min(460, root.width - Kirigami.Units.largeSpacing * 4)
        contentItem: Controls.Label {
            id: verifyResultLabel
            wrapMode: Text.WordWrap
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("OK")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
            onAccepted: verifyResultDialog.close()
        }
    }

    Controls.Dialog {
        id: unassignDialog
        property int deviceId: 0
        property int parentId: 0
        title: qsTr("Unassign Device")
        modal: true
        anchors.centerIn: parent
        width: Math.min(420, root.width - Kirigami.Units.largeSpacing * 4)
        contentItem: Controls.Label {
            text: qsTr("Unassign <b>%1</b> from its parent device?").arg(root.operationDeviceName)
            textFormat: Text.RichText
            wrapMode: Text.WordWrap
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("Unassign")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
            Controls.Button {
                text: qsTr("Cancel")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.RejectRole
            }
            onAccepted: {
                unassignDialog.close()
                var err = appManager1.unassignDevice(unassignDialog.deviceId, unassignDialog.parentId)
                if (err.length > 0) {
                    operationResultLabel.text = err
                    operationResultDialog.open()
                }
            }
            onRejected: unassignDialog.close()
        }
    }

    Controls.Dialog {
        id: deleteConfirmDialog
        property int    deviceId:   0
        property string deviceType: ""
        title: qsTr("Delete Device")
        modal: true
        anchors.centerIn: parent
        width: Math.min(420, root.width - Kirigami.Units.largeSpacing * 4)
        contentItem: Controls.Label {
            text: qsTr("Do you want to <b>delete</b> this %1 device?<br/><br/>Name: <b>%2</b>")
                  .arg(deleteConfirmDialog.deviceType)
                  .arg(root.operationDeviceName)
            textFormat: Text.RichText
            wrapMode: Text.WordWrap
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("Yes")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
            Controls.Button {
                text: qsTr("Cancel")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.RejectRole
            }
            onAccepted: {
                deleteConfirmDialog.close()
                var err = appManager1.deleteDevice(deleteConfirmDialog.deviceId)
                if (err.length > 0) {
                    operationResultLabel.text = err
                    operationResultDialog.open()
                }
            }
            onRejected: deleteConfirmDialog.close()
        }
    }

    Controls.Dialog {
        id: operationResultDialog
        title: "Katalog"
        modal: true
        anchors.centerIn: parent
        width: Math.min(420, root.width - Kirigami.Units.largeSpacing * 4)
        contentItem: Controls.Label {
            id: operationResultLabel
            wrapMode: Text.WordWrap
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("OK")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
            onAccepted: operationResultDialog.close()
        }
    }

    // ── Layout ──────────────────────────────────────────────────────────────

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Filter bar
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: filterRow.implicitHeight + Kirigami.Units.smallSpacing * 2
            color: Kirigami.Theme.alternateBackgroundColor

            RowLayout {
                id: filterRow
                anchors {
                    left: parent.left; right: parent.right
                    verticalCenter: parent.verticalCenter
                    leftMargin: Kirigami.Units.largeSpacing
                    rightMargin: Kirigami.Units.largeSpacing
                }
                spacing: Kirigami.Units.smallSpacing

                Controls.ButtonGroup { id: filterGroup }

                Controls.ToolButton {
                    text: qsTr("Device tree")
                    checkable: true; checked: root.viewFilter === "All"
                    Controls.ButtonGroup.group: filterGroup
                    onClicked: { root.viewFilter = "All"; root.refreshDevices() }
                }
                Controls.ToolButton {
                    text: qsTr("Catalogs list")
                    checkable: true; checked: root.viewFilter === "Catalogs"
                    Controls.ButtonGroup.group: filterGroup
                    onClicked: { root.viewFilter = "Catalogs"; root.refreshDevices() }
                }
                Controls.ToolButton {
                    text: qsTr("Storage list")
                    checkable: true; checked: root.viewFilter === "Storage"
                    Controls.ButtonGroup.group: filterGroup
                    onClicked: { root.viewFilter = "Storage"; root.refreshDevices() }
                }

                Item { Layout.fillWidth: true }

                Controls.CheckBox {
                    text: qsTr("Filter from Selection")
                    checked: root.filterFromSelection
                    onToggled: {
                        root.filterFromSelection = checked
                        root.refreshDevices()
                    }
                }
            }
        }

        Kirigami.Separator { Layout.fillWidth: true }

        // Device list
        Kirigami.CardsListView {
            id: deviceList
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: root.devices
            topMargin: Kirigami.Units.smallSpacing

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                visible: deviceList.count === 0 && !appManager1.deviceUpdateIsRunning
                text: qsTr("No devices")
                icon.name: "drive-multidisk"
            }

            delegate: PageDevicesViewDelegate {
                delegateCardScale: root.cardScale

                onEditRequested: (id) => root.editDeviceRequested(id)

                onSplitSubDirRequested: (id, name) => {
                    root.operationDeviceName = name
                    splitSubDirConfirmDialog.deviceId = id
                    splitSubDirConfirmDialog.open()
                }

                onSplitFileTypeRequested: (id, name, active) => {
                    root.operationDeviceName = name
                    root.operationDeviceActive = active
                    splitFileTypeDialog.deviceId = id
                    splitFileTypeDialog.deviceActive = active
                    splitFileTypeDialog.open()
                }

                onVerifyRequested: (id, name) => {
                    root.operationDeviceName = name
                    verifyConfirmDialog.deviceId = id
                    verifyConfirmDialog.open()
                }

                onUnassignRequested: (id, pid, name) => {
                    root.operationDeviceName = name
                    unassignDialog.deviceId = id
                    unassignDialog.parentId = pid
                    unassignDialog.open()
                }

                onDeleteRequested: (id, name, type) => {
                    var check = appManager1.checkDeviceDeleteAllowed(id)
                    if (!check.allowed) {
                        operationResultLabel.text = check.errorMessage
                        operationResultDialog.open()
                        return
                    }
                    root.operationDeviceName = name
                    deleteConfirmDialog.deviceId   = id
                    deleteConfirmDialog.deviceType = type
                    deleteConfirmDialog.open()
                }

                onAddVirtualChildRequested: (parentId) => {
                    var newId = appManager1.addDeviceVirtual(parentId)
                    if (newId > 0) root.editDeviceRequested(newId)
                }

                onAddStorageChildRequested: (parentId) => {
                    var newId = appManager1.addDeviceStorage(parentId)
                    if (newId > 0) root.editDeviceRequested(newId)
                }

                onAssignCatalogRequested: (virtualId) => {
                    var err = appManager1.assignCatalogToDevice(appManager1.selectedDeviceId, virtualId)
                    if (err.length > 0) {
                        operationResultLabel.text = err
                        operationResultDialog.open()
                    }
                }

                onFilelightRequested: (id) => {
                    appManager1.launchFilelight(id)
                }
            }
        }
    }
}
