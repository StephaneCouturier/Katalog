import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

Item {
    id: root
    anchors.fill: parent

    signal editDeviceRequested(int deviceId)
    signal exploreDeviceRequested(int deviceId)

    property real   cardScale: 1.0
    property string viewFilter: "All"
    property bool   filterFromSelection: appManager1.deviceFilterFromSelection
    property var    devices: []
    property string operationDeviceName: ""
    property bool   operationDeviceActive: false

    function refreshDevices() {
        var scope = filterFromSelection ? appManager1.selectedDeviceId : 0
        devices = appManager1.getDeviceList(viewFilter, scope)
    }

    // Create a child device and open it for editing, as K2 does
    // (mainwindow_tab_device_pr.cpp addDeviceVirtual/addDeviceStorage, both
    // ending in editDevice()). Lives on the page rather than in the card's
    // handler: creating a device refreshes the list and destroys that card.
    function createChildAndEdit(parentId, kind) {
        var newId = (kind === "Storage") ? appManager1.addDeviceStorage(parentId)
                                         : appManager1.addDeviceVirtual(parentId)
        console.log("createChildAndEdit: kind=" + kind + " parentId=" + parentId
                    + " newId=" + newId)
        if (newId > 0)
            root.editDeviceRequested(newId)
        else
            console.warn("createChildAndEdit: no device created, editor not opened")
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
                    text: qsTr("Storage list")
                    checkable: true; checked: root.viewFilter === "Storage"
                    Controls.ButtonGroup.group: filterGroup
                    onClicked: { root.viewFilter = "Storage"; root.refreshDevices() }
                }
                Controls.ToolButton {
                    text: qsTr("Catalogs list")
                    checkable: true; checked: root.viewFilter === "Catalogs"
                    Controls.ButtonGroup.group: filterGroup
                    onClicked: { root.viewFilter = "Catalogs"; root.refreshDevices() }
                }

                Item { Layout.fillWidth: true }

                Controls.CheckBox {
                    text: qsTr("Filter from Selection")
                    checked: root.filterFromSelection
                    onToggled: {
                        root.filterFromSelection = checked
                        appManager1.deviceFilterFromSelection = checked
                        root.refreshDevices()
                    }
                }
            }
        }

        Kirigami.Separator { Layout.fillWidth: true }

        // Summary bar - shown for Catalogs and Storage list views (matches K2 CatalogStats/StorageStats)
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: summaryRow.implicitHeight + Kirigami.Units.smallSpacing * 2
            color: Kirigami.Theme.alternateBackgroundColor
            visible: root.viewFilter !== "All"

            RowLayout {
                id: summaryRow
                anchors {
                    left: parent.left; right: parent.right
                    verticalCenter: parent.verticalCenter
                    leftMargin: Kirigami.Units.largeSpacing
                    rightMargin: Kirigami.Units.largeSpacing
                }
                spacing: Kirigami.Units.largeSpacing

                // Catalogs summary
                Controls.Label {
                    visible: root.viewFilter === "Catalogs"
                    text: qsTr("Catalogs") + ":  <b>" + root.devices.length + "</b>"
                    textFormat: Text.RichText
                }
                Controls.ToolSeparator { visible: root.viewFilter === "Catalogs" }
                Controls.Label {
                    visible: root.viewFilter === "Catalogs"
                    text: qsTr("Total File Size") + ":  <b>" + appManager1.formatDataSize(
                        root.devices.reduce(function(s, d) { return s + (d.totalFileSize || 0) }, 0)) + "</b>"
                    textFormat: Text.RichText
                }
                Controls.ToolSeparator { visible: root.viewFilter === "Catalogs" }
                Controls.Label {
                    visible: root.viewFilter === "Catalogs"
                    text: qsTr("Total Number of Files") + ":  <b>" + Number(root.devices.reduce(
                        function(s, d) { return s + (d.fileCount || 0) }, 0)).toLocaleString(Qt.locale(), "f", 0) + "</b>"
                    textFormat: Text.RichText
                }

                // Storage summary
                Controls.Label {
                    visible: root.viewFilter === "Storage"
                    text: qsTr("Devices") + ":  <b>" + root.devices.length + "</b>"
                    textFormat: Text.RichText
                }
                Controls.ToolSeparator { visible: root.viewFilter === "Storage" }
                Controls.Label {
                    visible: root.viewFilter === "Storage"
                    text: qsTr("Total Space") + ":  <b>" + appManager1.formatDataSize(
                        root.devices.reduce(function(s, d) { return s + (d.totalSpace || 0) }, 0)) + "</b>"
                    textFormat: Text.RichText
                }
                Controls.ToolSeparator { visible: root.viewFilter === "Storage" }
                Controls.Label {
                    id: storageUsedLabel
                    visible: root.viewFilter === "Storage"
                    text: {
                        var used = root.devices.reduce(function(s, d) { return s + ((d.totalSpace || 0) - (d.freeSpace || 0)) }, 0)
                        return qsTr("Used") + ":  <b>" + appManager1.formatDataSize(used) + "</b>"
                    }
                    textFormat: Text.RichText
                }
                Controls.ToolSeparator { visible: root.viewFilter === "Storage" }
                Controls.Label {
                    id: storageFreeLabel
                    visible: root.viewFilter === "Storage"
                    text: {
                        var free  = root.devices.reduce(function(s, d) { return s + (d.freeSpace  || 0) }, 0)
                        var total = root.devices.reduce(function(s, d) { return s + (d.totalSpace || 0) }, 0)
                        var pct   = total > 0 ? Math.round(free / total * 100) : 0
                        return qsTr("Free") + ":  <b>" + appManager1.formatDataSize(free) + "  (" + pct + "%)</b>"
                    }
                    textFormat: Text.RichText
                }

                Item { Layout.fillWidth: true }
            }
        }
        Kirigami.Separator { Layout.fillWidth: true; visible: root.viewFilter !== "All" }

        // Device list
        Kirigami.CardsListView {
            id: deviceList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
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

                onEditRequested:    (id) => root.editDeviceRequested(id)
                onExploreRequested: (id) => root.exploreDeviceRequested(id)

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

                // The work is done by a function on the page, not here: adding a
                // device refreshes the list and destroys this delegate, and any
                // statement left in a handler running on it is abandoned. The
                // page's own frame survives that.
                onAddVirtualChildRequested: (parentId) => root.createChildAndEdit(parentId, "Virtual")
                onAddStorageChildRequested: (parentId) => root.createChildAndEdit(parentId, "Storage")

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
