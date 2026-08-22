import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

Kirigami.AbstractCard {
    id: card

    required property var modelData
    required property int index

    property real delegateCardScale: 1.0

    signal editRequested(int deviceId)
    signal exploreRequested(int deviceId)
    signal splitSubDirRequested(int deviceId, string deviceName)
    signal splitFileTypeRequested(int deviceId, string deviceName, bool deviceActive)
    signal verifyRequested(int deviceId, string deviceName)
    signal unassignRequested(int deviceId, int parentId, string deviceName)
    signal deleteRequested(int deviceId, string deviceName, string deviceType)
    signal addVirtualChildRequested(int parentId)
    signal addStorageChildRequested(int parentId)
    signal assignCatalogRequested(int virtualDeviceId)
    signal filelightRequested(int deviceId)

    readonly property int    devId:        modelData.deviceId
    readonly property int    devParent:    modelData.parentId
    readonly property string devName:      modelData.name
    readonly property string devType:      modelData.type
    readonly property string devPath:      modelData.path
    readonly property string devParentType: modelData.parentType ?? ""
    readonly property int    devLevel:     modelData.level
    readonly property bool   devActive:    modelData.active
    readonly property int    devGroupId:   modelData.groupId ?? 0

    // Unassign: only Catalog and Storage devices assigned to a Virtual Group (groupId != 0).
    // Virtual devices are never unassigned - they can only be deleted.
    readonly property bool canUnassign: devType !== "Virtual" && devGroupId !== 0 && devPath !== "EXPORT"

    anchors.left:       parent ? parent.left  : undefined
    anchors.leftMargin: devLevel * Kirigami.Units.gridUnit
    anchors.right:      parent ? parent.right : undefined

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
            text: card.devName
            enabled: false
            font.bold: true
        }

        // K2 has NO separator between the header and the first action items.
        // Update: Catalog only if active; Storage and Virtual always
        Controls.MenuItem {
            text: qsTr("Update")
            icon.name: "media-playlist-repeat"
            visible: (card.devType === "Catalog" && card.devActive)
                     || card.devType === "Storage"
                     || card.devType === "Virtual"
            // Always enabled: while an operation runs the request is queued
            // rather than dropped (SpecOperationQueue.md).
            onTriggered: appManager1.updateDevice(card.devId)
        }
        Controls.MenuItem {
            text: qsTr("Explore")
            icon.name: "view-list-tree"
            visible: card.devType === "Catalog"
            onTriggered: card.exploreRequested(card.devId)
        }
        Controls.MenuItem {
            text: qsTr("Edit")
            icon.name: "document-edit-sign"
            onTriggered: card.editRequested(card.devId)
        }
        Controls.MenuItem {
            text: qsTr("Open folder")
            icon.name: "document-open-folder"
            visible: card.devPath.length > 0 && card.devPath !== "EXPORT"
            onTriggered: appManager1.openDeviceFolder(card.devId)
        }

        // Storage: Filelight with NO separator before it (K2 structure)
        Controls.MenuItem {
            text: qsTr("Filelight")
            icon.name: "view-statistics"
            visible: card.devType === "Storage" && card.devActive
            onTriggered: card.filelightRequested(card.devId)
        }

        // Catalog section: separator, Verify, separator+Filelight only when active, separator, Splits
        Controls.MenuSeparator { visible: card.devType === "Catalog" }
        Controls.MenuItem {
            text: qsTr("Verify Checksums")
            icon.name: "document-properties"
            visible: card.devType === "Catalog"
            onTriggered: card.verifyRequested(card.devId, card.devName)
        }
        Controls.MenuSeparator { visible: card.devType === "Catalog" && card.devActive }
        Controls.MenuItem {
            text: qsTr("Filelight")
            icon.name: "view-statistics"
            visible: card.devType === "Catalog" && card.devActive
            onTriggered: card.filelightRequested(card.devId)
        }
        Controls.MenuSeparator { visible: card.devType === "Catalog" }
        Controls.MenuItem {
            text: qsTr("Split catalog by sub-directory")
            icon.name: "edit-cut"
            visible: card.devType === "Catalog"
            onTriggered: card.splitSubDirRequested(card.devId, card.devName)
        }
        Controls.MenuItem {
            text: qsTr("Split catalog by file type")
            icon.name: "edit-cut"
            visible: card.devType === "Catalog"
            onTriggered: card.splitFileTypeRequested(card.devId, card.devName, card.devActive)
        }

        // Virtual section: separator, then virtual device actions
        Controls.MenuSeparator { visible: card.devType === "Virtual" }
        Controls.MenuItem {
            text: qsTr("Add Virtual device")
            icon.name: "document-new"
            visible: card.devType === "Virtual"
            onTriggered: card.addVirtualChildRequested(card.devId)
        }
        Controls.MenuItem {
            text: qsTr("Add Storage device")
            icon.name: "document-new"
            visible: card.devType === "Virtual" && card.devGroupId === 0
            onTriggered: card.addStorageChildRequested(card.devId)
        }
        Controls.MenuItem {
            text: qsTr("Assign selected catalog")
            icon.name: "document-new"
            visible: card.devType === "Virtual" && card.devGroupId !== 0
            enabled: appManager1.selectedDeviceType === "Catalog"
            onTriggered: card.assignCatalogRequested(card.devId)
        }

        // Separator before Unassign/Delete (always visible after all type-specific items)
        Controls.MenuSeparator {}
        Controls.MenuItem {
            text: card.devType === "Storage" ? qsTr("Unassign this storage") : qsTr("Unassign this catalog")
            icon.name: "edit-cut"
            visible: card.canUnassign
            onTriggered: card.unassignRequested(card.devId, card.devParent, card.devName)
        }
        Controls.MenuItem {
            text: card.devType === "Catalog" ? qsTr("Delete this catalog")
                : card.devType === "Storage" ? qsTr("Delete this storage")
                : qsTr("Delete")
            icon.name: "edit-delete"
            visible: !card.canUnassign
            onTriggered: card.deleteRequested(card.devId, card.devName, card.devType)
        }
    }

    contentItem: Item {
        implicitWidth:  deviceDelegateLayout.implicitWidth
        implicitHeight: deviceDelegateLayout.implicitHeight

        RowLayout {
            id: deviceDelegateLayout
            anchors {
                left:  parent.left
                top:   parent.top
                right: parent.right
            }
            spacing: Kirigami.Units.largeSpacing

            Kirigami.Icon {
                source: {
                    if (card.devType === "Storage") return "drive-harddisk"
                    if (card.devType === "Catalog")
                        return card.devActive ? "media-optical-blu-ray" : "media-optical"
                    return "drive-multidisk"
                }
                implicitWidth:  Kirigami.Units.iconSizes.medium
                implicitHeight: Kirigami.Units.iconSizes.medium
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                Kirigami.Heading {
                    Layout.fillWidth: true
                    level: 2
                    text: card.devName
                    elide: Text.ElideRight
                    maximumLineCount: 1
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * card.delegateCardScale
                }

                Controls.Label {
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * card.delegateCardScale * 0.8
                    text: {
                        var d = card.modelData
                        var parts = []
                        if (d.type !== "Virtual") parts.push(d.type)
                        if (d.fileCount > 0)
                            parts.push(Number(d.fileCount).toLocaleString(Qt.locale(), "f", 0) + " " + qsTr("files")
                                       + "  " + appManager1.formatDataSize(d.totalFileSize))
                        if (d.type === "Storage" && d.freeSpace > 0)
                            parts.push(qsTr("free") + ": " + appManager1.formatDataSize(d.freeSpace))
                        if (d.dateUpdated && d.dateUpdated.length > 0)
                            parts.push(d.dateUpdated)
                        return parts.join("  ·  ")
                    }
                    visible: text.length > 0
                }
            }

            Controls.ToolButton {
                icon.name: "application-menu"
                onClicked: contextMenu.popup()
                Controls.ToolTip.text: qsTr("Actions")
                Controls.ToolTip.visible: hovered
            }
        }
    }
}
