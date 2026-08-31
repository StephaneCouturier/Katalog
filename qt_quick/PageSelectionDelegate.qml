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
    topPadding:    Kirigami.Units.mediumSpacing
    bottomPadding: Kirigami.Units.mediumSpacing
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
            // Select first, so the highlighted card and the action agree
            // (SpecSelection.md SEL-F3). Update, Open folder and Edit below
            // deliberately do NOT select (SEL-F4).
            onTriggered: {
                appManager1.selectDeviceById(model.deviceId)
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
            // Selects first, as Search does (SEL-F3).
            onTriggered: {
                appManager1.selectDeviceById(model.deviceId)
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
                Layout.fillWidth: true

                // Icon and name come from the shared component, so this card and
                // the selected-device reminder above the list cannot drift apart
                // (SpecSelection.md SEL-C1).
                DeviceIdentity {
                    Layout.fillWidth: true
                    deviceType:     model.type
                    deviceName:     model.name
                    deviceIsActive: model.isActive
                    fontScale:      root.cardScale
                    // The selected card is filled with the highlight colour, so
                    // the name needs the colour meant to sit on it.
                    nameColor:      card.isSelected ? Kirigami.Theme.highlightedTextColor
                                                    : Kirigami.Theme.textColor
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
                    color: card.isSelected ? Kirigami.Theme.highlightedTextColor
                                           : Kirigami.Theme.textColor
                }
            }

            // One button, always in the layout, rather than two that come and
            // go. A Catalog can never hold children, so its slot was simply
            // absent and its card came out shorter than a Storage or Virtual
            // one; reserving the space — as the Explore folder tree does —
            // makes every card the same height. Both labels already existed.
            Controls.ToolButton {
                Layout.columnSpan: 1
                icon.name: model.isCollapsed ? "go-down" : "go-up"
                icon.width:  Kirigami.Units.iconSizes.small
                icon.height: Kirigami.Units.iconSizes.small
                implicitWidth:  root.selectionButton
                implicitHeight: root.selectionButton
                padding: 0
                opacity: model.hasChildren ? 1.0 : 0.0
                enabled: model.hasChildren
                onClicked: {
                    if (model.isCollapsed)
                        appManager1.expandDevice(model.deviceId)
                    else
                        appManager1.collapseDevice(model.deviceId)
                }
                Controls.ToolTip.text: model.isCollapsed ? qsTr("Expand") : qsTr("Collapse")
                Controls.ToolTip.visible: hovered
            }
        }
        Rectangle {
            anchors.fill: parent
            // The frame has to reach the card's own edges, and the card is not
            // padded evenly — tight top and bottom, wider left and right. Each
            // side therefore cancels the padding on that side; a single uniform
            // margin left the frame short on the left and right.
            anchors.topMargin:    -card.topPadding
            anchors.bottomMargin: -card.bottomPadding
            anchors.leftMargin:   -card.leftPadding
            anchors.rightMargin:  -card.rightPadding
            // Filled with the border's own blue rather than a 12% wash: at that
            // strength it was indistinguishable from the tinted page behind it.
            color: Kirigami.Theme.highlightColor
            border.color: Kirigami.Theme.highlightColor
            border.width: 2
            radius: Kirigami.Units.cornerRadius
            visible: card.isSelected
            // Behind the content, not over it. As an overlay at z: 999 a solid
            // fill would simply hide the card; only the old 12% alpha let the
            // name show through.
            z: -1
        }
    }
}
