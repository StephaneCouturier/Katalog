import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import Qt.labs.platform

Kirigami.AbstractCard {
    id: card

    anchors.left: parent.left
    anchors.leftMargin: type === "Storage" ? 15 :
                        type === "Catalog" ? 30 :
                        0
    anchors.right: parent.right

    property bool isSelected: appManager1.getSelectedDeviceId() === model.deviceId

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
                    visible: model.description.length > 0
                }

                Controls.Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: model.description
                    visible: model.description.length > 0
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * root.cardScale * 0.8
                }
            }

            Controls.Button {
                Layout.alignment: Qt.AlignRight
                Layout.columnSpan: 2
                icon.name: "labplot-zoom-select"
                onClicked: {
                    console.log("Select clicked for device:", model.name, "ID:", model.deviceId)

                    // Update the selected device through AppManager method
                    appManager1.selectDeviceById(model.deviceId)

                    // Show confirmation with the updated device name
                    //showPassiveNotification("Selected device: " + appManager1.getSelectedDeviceName())
                    console.log("Selected device updated to:", appManager1.getSelectedDeviceName())

                    // Force update of selection state for all cards
                    selectionListView1.model.dataChanged(selectionListView1.model.index(0, 0),
                                                         selectionListView1.model.index(selectionListView1.model.rowCount() - 1, 0))
                }
            }
            Controls.Button {
                Layout.alignment: Qt.AlignRight
                Layout.columnSpan: 1
                icon.name: "document-open"
                Controls.ToolTip.visible: down
                Controls.ToolTip.text: qsTr("Open the device")
                onClicked: showPassiveNotification("Open clicked for: " + model.name)
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
