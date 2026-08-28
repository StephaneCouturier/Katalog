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

    // Tightened to fit more devices on screen. The vertical padding is what
    // stacks up over a long list, so it is trimmed hardest; the horizontal
    // padding is left alone, because narrowing the text against the card edge
    // would look cramped without buying any rows.
    topPadding:    Kirigami.Units.smallSpacing
    bottomPadding: Kirigami.Units.smallSpacing
    leftPadding:   Kirigami.Units.largeSpacing
    rightPadding:  Kirigami.Units.largeSpacing

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

        // Quick jump to the Search page without opening the drawer. Hidden when
        // Search or Results is already the open feature page (the Selection panel
        // stays visible as a side column beside those pages).
        Controls.MenuItem {
            text:      qsTr("Search")
            icon.name: "edit-find"
            visible:   root.openFeaturePage !== pageSearch
                       && root.openFeaturePage !== pageSearchResults
            height:    visible ? implicitHeight : 0
            onTriggered: {
                appManager1.setLastPage("Search")
                root.showPage(pageSearch)
            }
        }

        Controls.MenuItem {
            text:      qsTr("Update")
            icon.name: "media-playlist-repeat"
            // Match K2 (mainwindow_tab_filters.cpp): Update is available for Storage and
            // Virtual devices unconditionally, and for Catalogs only when active. The C++
            // updateDevice() → updateDeviceHierarchy() path already cascades sub-catalogs.
            visible:   model.type === "Storage" || model.type === "Virtual"
                       || (model.type === "Catalog" && model.isActive)
            height:    visible ? implicitHeight : 0
            // Always enabled: while an operation runs the request is queued
            // rather than dropped (SpecOperationQueue.md).
            onTriggered: appManager1.updateDevice(model.deviceId)
        }

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
            rowSpacing: Kirigami.Units.smallSpacing
            columnSpacing: Kirigami.Units.smallSpacing
            columns: root.wideScreen ? 4 : 2

            ColumnLayout {
                RowLayout {
                    Kirigami.Icon {
                        source: model.type === "Virtual" ? "drive-multidisk" :
                                model.type === "Storage" ? "drive-harddisk" :
                                model.isActive ? "media-optical-blu-ray" : "media-optical"
                        // Sized to the text beside it rather than to the icon
                        // theme's default, which is taller than the name line
                        // and would set the card height on its own.
                        implicitWidth:  Kirigami.Units.iconSizes.small
                        implicitHeight: Kirigami.Units.iconSizes.small
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
                // A ToolButton's default height is taller than the card's text,
                // so left unbounded it, rather than the content, would set the
                // card height.
                implicitWidth:  Kirigami.Units.iconSizes.medium + Kirigami.Units.largeSpacing
                implicitHeight: Kirigami.Units.iconSizes.medium + Kirigami.Units.smallSpacing
                padding: 0
                visible: model.hasChildren && !model.isCollapsed
                onClicked: appManager1.collapseDevice(model.deviceId)
                Controls.ToolTip.text: qsTr("Collapse")
                Controls.ToolTip.visible: hovered
            }
            Controls.ToolButton {
                icon.name: "go-down"
                Layout.columnSpan: 2
                implicitWidth:  Kirigami.Units.iconSizes.medium + Kirigami.Units.largeSpacing
                implicitHeight: Kirigami.Units.iconSizes.medium + Kirigami.Units.smallSpacing
                padding: 0
                visible: model.hasChildren && model.isCollapsed
                onClicked: appManager1.expandDevice(model.deviceId)
                Controls.ToolTip.text: qsTr("Expand")
                Controls.ToolTip.visible: hovered
            }
        }
        Rectangle {
            anchors.fill: parent
            // Tracks the card's padding rather than a fixed 8px: with the
            // padding trimmed, a fixed overhang would spill past the card edge
            // onto the neighbouring card.
            anchors.margins: -Kirigami.Units.smallSpacing
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
