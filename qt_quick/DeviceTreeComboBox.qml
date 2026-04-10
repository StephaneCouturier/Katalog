import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

// A button + popup that shows the device tree with indentation.
// Use selectedDeviceId (int) and selectedDeviceName (string) to read the selection.
// Call resetSelection() to clear back to the current app-selected device.
Controls.Button {
    id: control

    property int    selectedDeviceId:   0
    property string selectedDeviceName: ""
    property string selectedDeviceType: ""

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
        var dev = _findDevice(id)
        if (dev) {
            selectedDeviceId   = id
            selectedDeviceName = dev.name
            selectedDeviceType = dev.type
        }
    }

    function resetSelection() {
        _applyDevice(appManager1.selectedDeviceId)
    }

    // Initialise from the currently selected device, and follow changes
    Component.onCompleted: resetSelection()

    Connections {
        target: appManager1
        function onSelectedDeviceChanged() { control.resetSelection() }
    }

    // ── Button appearance ─────────────────────────────────────────────────
    implicitWidth: 200
    leftPadding:  Kirigami.Units.smallSpacing * 2
    rightPadding: Kirigami.Units.smallSpacing * 2

    onClicked: popup.open()

    // Left-aligned text with a drop-down arrow on the right
    contentItem: RowLayout {
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
                                    : qsTr("Select")
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
            color:        Kirigami.Theme.backgroundColor
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

                    width:       ListView.view.width
                    leftPadding: Kirigami.Units.smallSpacing + level * Kirigami.Units.gridUnit
                    highlighted: control.selectedDeviceId === deviceId

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
