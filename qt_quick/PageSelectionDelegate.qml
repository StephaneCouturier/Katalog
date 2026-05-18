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

    TapHandler {
        acceptedButtons: Qt.RightButton
        onTapped: contextMenu.popup()
    }

    TapHandler {
        onLongPressed: contextMenu.popup()
    }

    Controls.Menu {
        id: contextMenu

        Controls.MenuItem {
            text: model.name
            enabled: false
            font.bold: true
        }
        Controls.MenuSeparator {}

        Controls.MenuItem {
            text: qsTr("Explore")
            icon.name: "view-list-tree"
            visible: model.type === "Catalog"
            height: visible ? implicitHeight : 0
            onTriggered: {
                appManager1.setLastPage("Explore")
                exploreFolders.openByDeviceId(model.deviceId)
                root.showPage(pageExplore)
            }
        }

        Controls.MenuItem {
            text: qsTr("Open folder")
            icon.name: "document-open"
            onTriggered: appManager1.openDeviceFolder(model.deviceId)
        }

        Controls.MenuItem {
            text: qsTr("Edit")
            icon.name: "document-edit"
            onTriggered: {
                pageDeviceEdit.fromDevicesPage = false
                pageDeviceEdit_form.deviceId = model.deviceId
                pageDeviceEdit_form.loadDevice()
                root.showPage(pageDeviceEdit)
            }
        }
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
                                model.isActive ? "media-optical-blu-ray" : "media-optical"
                    }

                    Kirigami.Heading {
                        Layout.fillWidth: true
                        level: 2
                        text: model.name
                        elide: Text.ElideRight
                        maximumLineCount: 1
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
            color: Qt.rgba(Kirigami.Theme.highlightColor.r,
                           Kirigami.Theme.highlightColor.g,
                           Kirigami.Theme.highlightColor.b, 0.12)
            border.color: Kirigami.Theme.highlightColor
            border.width: 2
            radius: Kirigami.Units.cornerRadius
            visible: card.isSelected
            z: 999
        }
    }
}
