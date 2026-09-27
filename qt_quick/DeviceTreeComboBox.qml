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
    // Natural width from the widest device name (or prompt), with the icon and
    // arrow either side; it may shrink to fit and elide (SpecComboBoxes.md,
    // CBX-C5 / F1 / F2). A call site that stretches the button sets its own
    // Layout width.
    leftPadding:  Kirigami.Units.smallSpacing * 2
    rightPadding: Kirigami.Units.smallSpacing * 2

    // Bumped when the device list changes, so the widths below are measured again.
    property int _listRevision: 0
    Connections {
        target: control.sourceModel
        function onModelReset()   { control._listRevision++ }
        function onRowsInserted() { control._listRevision++ }
        function onRowsRemoved()  { control._listRevision++ }
        function onDataChanged()  { control._listRevision++ }
    }

    implicitWidth: {
        _listRevision
        var widest = rowMetrics.advanceWidth(contentLabel.text)
        for (var i = 0; i < sourceModel.rowCount(); i++)
            widest = Math.max(widest, rowMetrics.advanceWidth(sourceModel.data(sourceModel.index(i, 0), 258)))
        return Math.ceil(widest) + Kirigami.Units.iconSizes.small * 2
               + Kirigami.Units.smallSpacing * 2 + leftPadding + rightPadding
    }
    Layout.fillWidth: true
    Layout.maximumWidth: implicitWidth

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
            id: contentLabel
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
    // Sized and placed like every other combo box list (SpecComboBoxes.md,
    // CBX-C5): at least as wide as the button, wide enough for the longest
    // name at its indent, inside the window, and opened on the side with room.
    FontMetrics { id: rowMetrics; font: control.font }

    // DeviceListModel roles: NameRole=258, LevelRole=262
    function _widestRow() {
        var widest = 0
        for (var i = 0; i < sourceModel.rowCount(); i++) {
            var idx = sourceModel.index(i, 0)
            widest = Math.max(widest, rowMetrics.advanceWidth(sourceModel.data(idx, 258))
                                      + sourceModel.data(idx, 262) * Kirigami.Units.gridUnit)
        }
        // Row padding and icon, as the delegate lays them out, plus the popup's
        // padding and room for the scroll bar.
        return widest + Kirigami.Units.smallSpacing * 4 + Kirigami.Units.iconSizes.small
               + Kirigami.Units.largeSpacing * 2 + popup.leftPadding + popup.rightPadding
    }

    // Called on open: the list's own height is not known yet, so the side is
    // chosen from an estimate, and the height then follows the list itself.
    function _placePopup() {
        var win = control.Window.window
        if (!win) return
        popup.width = Math.min(win.width, Math.max(control.width, _widestRow()))
        var top     = control.mapToItem(null, 0, 0).y
        var below   = win.height - (top + control.height + 2)
        var above   = top - 2
        var chrome  = popup.topPadding + popup.bottomPadding
        var wanted  = Math.min(300, sourceModel.rowCount()
                                    * (rowMetrics.height + Kirigami.Units.largeSpacing * 2)) + chrome
        popup.downward = below >= wanted || below >= above
        popup.room     = (popup.downward ? below : above) - chrome
    }

    Controls.Popup {
        id: popup

        property bool downward: true
        property real room: 300

        y:      downward ? control.height + 2 : -height - 2
        width:  control.width
        padding: 2
        // Qt then shifts the popup back inside the window instead of letting a
        // wide list run off its side.
        leftMargin:  0
        rightMargin: 0

        contentHeight: Math.min(listView.contentHeight, 300, room)
        onAboutToShow: control._placePopup()

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
                               ? applicationWindow().selectionHighlightColor
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
