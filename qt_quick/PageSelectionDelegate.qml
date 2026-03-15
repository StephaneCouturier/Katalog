import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import Qt.labs.platform

Kirigami.AbstractCard {
    id: card

    anchors.left: parent.left
    anchors.leftMargin: model.level * Kirigami.Units.gridUnit
    anchors.right: parent.right

    property bool isSelected: appManager1.selectedDeviceId === model.deviceId

    TapHandler {
        onTapped: appManager1.selectDeviceById(model.deviceId)
    }

    contentItem: Item {
        implicitWidth: deviceDelegateLayout.implicitWidth
        implicitHeight: deviceDelegateLayout.implicitHeight

        GridLayout {
            id: deviceDelegateLayout
            anchors {
                left: parent.left
                top: parent.top
                right: parent.right
            }
            rowSpacing: Kirigami.Units.largeSpacing
            columnSpacing: Kirigami.Units.largeSpacing
            columns: root.wideScreen ? 4 : 2

            ColumnLayout {
                RowLayout {
                    Kirigami.Icon {
                        source: model.type === "Virtual" ? "drive-multidisk" :
                                model.type === "Storage" ? "drive-harddisk" :
                                model.isActive ? "preferences-devices-drive-optical-check" : "drive-optical"
                    }

                    Kirigami.Heading {
                        Layout.fillWidth: true
                        level: 2
                        text: model.name
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.cardScale
                    }
                }
                Kirigami.Separator {
                    Layout.fillWidth: true
                    visible: appManager1.showDeviceInfo && model.description.length > 0
                }

                Controls.Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: model.description
                    visible: appManager1.showDeviceInfo && model.description.length > 0
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.cardScale * 0.8
                }
            }

            Controls.ToolButton {
                icon.name: "go-up"
                Layout.columnSpan: 1
                visible: model.hasChildren && !model.isCollapsed
                onClicked: appManager1.collapseDevice(model.deviceId)
                Controls.ToolTip.text: "Collapse"
                Controls.ToolTip.visible: hovered
            }
            Controls.ToolButton {
                icon.name: "go-down"
                Layout.columnSpan: 2
                visible: model.hasChildren && model.isCollapsed
                onClicked: appManager1.expandDevice(model.deviceId)
                Controls.ToolTip.text: "Expand"
                Controls.ToolTip.visible: hovered
            }
        }
        Rectangle {
            anchors.fill: parent
            anchors.margins: -8  // Remove the negative margins
            color: "transparent"
            border.color: Kirigami.Theme.highlightColor
            border.width: 2
            radius: Kirigami.Units.cornerRadius
            visible: card.isSelected
            z: 999
        }
    }
}
