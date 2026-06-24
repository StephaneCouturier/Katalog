import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

// A button + popup that shows the device tree with indentation.
// Use selectedDeviceId (int) and selectedDeviceName (string) to read the selection.
// Call resetSelection() to clear back to the current app-selected device.
// Call selectById(id) to pre-select a specific device by ID.
// Set storageOnly: true to restrict selection to Storage-type devices only
// (ancestor groups are shown as non-selectable context, pre-selection uses getDefaultStorageId()).
// Set catalogOnly: true to restrict selection to Catalog-type devices only
// (Storage and Virtual ancestors shown as non-selectable context, matching K2's backup
// device tree where catalogs are grouped under their Storage/Virtual parent; no auto-reset
// on selection change).
// Set hideCatalogs: true to hide Catalog-type devices from the list entirely.
Controls.Button {
    id: control

    property int    selectedDeviceId:   0
    property string selectedDeviceName: ""
    property string selectedDeviceType: ""
    property bool   storageOnly:        false
    property bool   catalogOnly:        false
    property bool   hideCatalogs:       false
    property bool   hideStorages:       false
    // Restrict selectable parents to a single device group (0=Physical, 1=Virtual);
    // -1 disables the filter. Mirrors K2's loadParentsList, which only offers parents
    // in the same group so a device can never be re-parented across groups.
    property int    groupFilter:        -1
    // Hide a single device (e.g. the one being edited) so it can't be its own parent.
    property int    excludeDeviceId:    -1

    // Source model — defaults to the shared device list
    property var sourceModel: appManager1.deviceListModel

    // ── Helpers ───────────────────────────────────────────────────────────
    // DeviceListModel roles: TypeRole=257, NameRole=258, DeviceIdRole=261
    function _findDevice(targetId) {
        for (var i = 0; i < sourceModel.rowCount(); i++) {
            var idx = sourceModel.index(i, 0)
            if (sourceModel.data(idx, 261) === targetId)
                return { name: sourceModel.data(idx, 258),
                         type: sourceModel.data(idx, 257) }
        }
        return null
    }

    function _applyDevice(id) {
        // id <= 0 means "no parent / root". Without this, selectById(0) would be a
        // no-op (no device has id 0) and leave the selection pre-filled by
        // resetSelection() — so editing a root device would silently re-parent it
        // under the app-selected device on save. Mirrors K2 (parentID 0 == root).
        if (id <= 0) {
            selectedDeviceId   = 0
            selectedDeviceName = ""
            selectedDeviceType = ""
            return
        }
        var dev = _findDevice(id)
        if (dev) {
            if (storageOnly && dev.type !== "Storage") {
                selectedDeviceId   = 0
                selectedDeviceName = ""
                selectedDeviceType = ""
                return
            }
            if (catalogOnly && dev.type !== "Catalog") {
                selectedDeviceId   = 0
                selectedDeviceName = ""
                selectedDeviceType = ""
                return
            }
            selectedDeviceId   = id
            selectedDeviceName = dev.name
            selectedDeviceType = dev.type
        }
    }

    function resetSelection() {
        if (catalogOnly) return   // catalogOnly pickers start empty; caller uses selectById()
        if (storageOnly)
            _applyDevice(appManager1.getDefaultStorageId())
        else
            _applyDevice(appManager1.selectedDeviceId)
    }

    function selectById(id) { _applyDevice(id) }

    // When true, the selection tracks the app-selected device (default — used by
    // pickers on Create/Search/etc.). The parent picker in the device editor sets
    // this false: its value must come only from selectById()/the user's dropdown
    // choice, never silently follow what is selected on the Selection page.
    property bool followAppSelection: true

    // Initialise from the currently selected device, and follow changes
    Component.onCompleted: if (followAppSelection) resetSelection()

    Connections {
        target: appManager1
        enabled: control.followAppSelection
        function onSelectedDeviceChanged() { control.resetSelection() }
        function onDeviceListRefreshed()   { control.resetSelection() }
    }

    // ── Button appearance ─────────────────────────────────────────────────
    implicitWidth: 200
    leftPadding:  Kirigami.Units.smallSpacing * 2
    rightPadding: Kirigami.Units.smallSpacing * 2

    onClicked: popup.open()

    // Left-aligned text with a drop-down arrow on the right
    contentItem: RowLayout {
        // KDE desktop style's MobileCursor calls positionToRectangle on contentItem expecting a TextInput
        function positionToRectangle(pos) { return Qt.rect(0, 0, 0, 0) }
        property int selectionStart: 0
        spacing: Kirigami.Units.smallSpacing

        Kirigami.Icon {
            source: control.selectedDeviceType === "Virtual" ? "drive-multidisk"
                  : control.selectedDeviceType === "Storage" ? "drive-harddisk"
                  : control.selectedDeviceName.length > 0    ? "media-optical"
                  :                                            "drive-harddisk"
            implicitWidth:  Kirigami.Units.iconSizes.small
            implicitHeight: Kirigami.Units.iconSizes.small
            Layout.alignment: Qt.AlignVCenter
        }

        Controls.Label {
            text:               control.selectedDeviceName.length > 0
                                    ? control.selectedDeviceName
                                    : control.storageOnly  ? qsTr("Select a Storage")
                                    : control.catalogOnly  ? qsTr("Select a Catalog")
                                    :                        qsTr("Select")
            elide:              Text.ElideRight
            horizontalAlignment: Text.AlignLeft
            verticalAlignment:   Text.AlignVCenter
            Layout.fillWidth:   true
        }

        Kirigami.Icon {
            source: "go-down"
            implicitWidth:  Kirigami.Units.iconSizes.small
            implicitHeight: Kirigami.Units.iconSizes.small
            opacity: 0.6
            Layout.alignment: Qt.AlignVCenter
        }
    }

    // ── Drop-down popup ───────────────────────────────────────────────────
    Controls.Popup {
        id: popup

        y:      control.height + 2
        width:  Math.max(control.width, 250)
        padding: 2

        contentHeight: Math.min(listView.contentHeight, 300)

        closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutside

        background: Rectangle {
            color:        popup.palette.base
            border.color: Kirigami.Theme.separatorColor ?? "transparent"
            border.width: 1
            radius:       4
        }

        contentItem: Controls.ScrollView {
            clip: true

            ListView {
                id: listView
                model: control.sourceModel
                clip:  true

                delegate: Controls.ItemDelegate {
                    required property string name
                    required property int    deviceId
                    required property int    level
                    required property string type
                    required property int    groupId

                    visible:     !(control.hideCatalogs && type === "Catalog")
                                 && !(control.hideStorages && type === "Storage")
                                 && (control.groupFilter < 0 || groupId === control.groupFilter)
                                 && deviceId !== control.excludeDeviceId
                    height:      visible ? implicitHeight : 0
                    width:       ListView.view.width
                    leftPadding: Kirigami.Units.smallSpacing + level * Kirigami.Units.gridUnit
                    highlighted: control.selectedDeviceId === deviceId
                    enabled:     (!control.storageOnly  || type === "Storage")
                                 && (!control.catalogOnly || type === "Catalog")

                    background: Rectangle {
                        color: control.selectedDeviceId === deviceId
                               ? Kirigami.Theme.highlightColor
                               : hovered ? Kirigami.Theme.alternateBackgroundColor : "transparent"
                        radius: 3
                    }

                    contentItem: RowLayout {
                        spacing: Kirigami.Units.smallSpacing
                        Kirigami.Icon {
                            source: type === "Virtual"  ? "drive-multidisk"
                                  : type === "Storage"  ? "drive-harddisk"
                                  :                       "media-optical"
                            implicitWidth:  Kirigami.Units.iconSizes.small
                            implicitHeight: Kirigami.Units.iconSizes.small
                            color: control.selectedDeviceId === deviceId
                                   ? Kirigami.Theme.highlightedTextColor
                                   : Kirigami.Theme.textColor
                        }
                        Controls.Label {
                            text:  name
                            color: control.selectedDeviceId === deviceId
                                   ? Kirigami.Theme.highlightedTextColor
                                   : Kirigami.Theme.textColor
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }

                    onClicked: {
                        control.selectedDeviceId   = deviceId
                        control.selectedDeviceName = name
                        control.selectedDeviceType = type
                        popup.close()
                    }
                }
            }
        }
    }
}
