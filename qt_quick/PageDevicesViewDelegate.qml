import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

Kirigami.AbstractCard {
    id: card

    required property var modelData
    required property int index

    property real delegateCardScale: 1.0

    // Which of the three views is showing, and this row's place in the tree.
    // The collapse state is the page's, held per page and never shared with the
    // Selection page (DVP-C18).
    property string viewFilter: "All"
    readonly property bool cardHasChildren: modelData._hasChildren === true
    readonly property bool cardIsCollapsed: modelData._isCollapsed === true
    signal collapseToggleRequested(int deviceId)

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

    // The icon size the user chose, shared with K2 (THM-F7). One level of the
    // tree's indent is exactly that size (DVP-F22), so the indent grows and
    // shrinks with the icons.
    readonly property real deviceIconSize: appManager1.biggerIconSize
                                           ? Kirigami.Units.iconSizes.medium
                                           : Kirigami.Units.iconSizes.smallMedium

    // Only the Device tree indents. The Storage and Catalogs lists never show a
    // card's parent, so an indent there marks a hierarchy the user cannot see
    // (DVP-F21).
    readonly property bool showsHierarchy: viewFilter === "All"

    anchors.left:       parent ? parent.left  : undefined
    anchors.leftMargin: showsHierarchy ? devLevel * deviceIconSize : 0
    anchors.right:      parent ? parent.right : undefined

    // Matched to the Selection card, which trims the vertical padding hardest
    // because that is what stacks up over a long list, and leaves the
    // horizontal padding alone (DVP-F19).
    topPadding:    Kirigami.Units.mediumSpacing
    bottomPadding: Kirigami.Units.mediumSpacing
    leftPadding:   Kirigami.Units.largeSpacing
    rightPadding:  Kirigami.Units.largeSpacing

    // Lets the Devices page open this same menu for a table row, so the table
    // and the cards can never offer different actions (SpecDevicesPage DVP-F6).
    function openContextMenu() { contextMenu.popup() }

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

            ColumnLayout {
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                RowLayout {
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing

                    // The Selection page's own name rules - per-type weight,
                    // italic and opacity - taken from the shared component
                    // rather than copied, so the two pages cannot drift
                    // (DVP-F17 / DVP-C14). The icon is suppressed: this card
                    // already draws a larger one in the column to the left.
                    DeviceIdentity {
                        Layout.fillWidth: true
                        // Icon as well as name, from the one component: a second
                        // icon rule in this file is what DVP-C20 removes.
                        showIcon:       true
                        iconSize:       card.deviceIconSize
                        // Cards wrap rather than hide (CDT-F1). Only this
                        // caller opts in: the Selection cards and the reminder
                        // above that list keep their current behaviour until
                        // the user asks otherwise (DVP-C16).
                        nameWraps:      true
                        deviceType:     card.devType
                        deviceName:     card.devName
                        deviceIsActive: card.devActive
                        fontScale:      card.delegateCardScale
                    }

                    // The user's own note, beside the name when there is one.
                    // Sized like the detail line below the name rather than like
                    // the heading, so it annotates the name without competing
                    // with it (SpecDeviceComment.md).
                    Controls.Label {
                        Layout.maximumWidth: parent.width * 0.5
                        visible: text.length > 0
                        text: card.modelData.comment !== undefined
                              ? card.modelData.comment : ""
                        // The half-width cap stays - it is what keeps the note
                        // from crowding the name - and the text wraps inside it
                        // instead of being cut off (CDT-F1 / DVP-C17).
                        wrapMode: Text.Wrap
                        opacity: 0.7
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize * card.delegateCardScale * 0.8
                    }
                }

                // Drawn only when there is a line below it to separate, as on
                // the Selection card (DVP-F23).
                Kirigami.Separator {
                    Layout.fillWidth: true
                    visible: deviceDetailLine.text.length > 0
                }

                Controls.Label {
                    id: deviceDetailLine
                    Layout.fillWidth: true
                    // Lined up with the name above, past the icon, exactly as
                    // the Selection card's second line is (DVP-F26). The
                    // separator above stays full width (DVP-C22).
                    Layout.leftMargin: card.deviceIconSize
                                       + Kirigami.Units.smallSpacing
                    // Wraps onto as many lines as it needs, so the date at the
                    // end stays readable on a narrow card (CDT-F1 / DVP-F18).
                    wrapMode: Text.Wrap
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * card.delegateCardScale * 0.8
                    text: {
                        var d = card.modelData
                        var parts = []
                        if (d.type !== "Virtual") parts.push(d.type)
                        // A catalog states its figures even when both are zero:
                        // an empty catalog is a fact worth reading, and hiding
                        // them made it look like one whose contents are unknown
                        // (DVP-F10). formatDataSizeDelta(), because the other
                        // formatter returns an empty string at zero and would
                        // blank the size here (DVP-C9); above zero the two are
                        // the same call. Catalogs only - Storage and Virtual
                        // cards keep their current behaviour.
                        if (d.type === "Catalog" || d.fileCount > 0)
                            parts.push(Number(d.fileCount).toLocaleString(Qt.locale(), "f", 0) + " " + qsTr("files")
                                       + "  " + appManager1.formatDataSizeDelta(d.totalFileSize))
                        // A device that reports space states all three, in the
                        // order the user reads them (DVP-F16). Shown only when
                        // there is space to report: three zeros would claim a
                        // measurement that was never taken, which is why this is
                        // not the empty-catalog rule of DVP-F10. Each value goes
                        // through the zero-rendering formatter, so a full disk's
                        // free space reads as zero rather than as a blank after
                        // its label (DVP-C13).
                        if ((d.type === "Storage" || d.type === "Virtual") && d.totalSpace > 0) {
                            parts.push(qsTr("used")  + ": " + appManager1.formatDataSizeDelta(d.usedSpace))
                            parts.push(qsTr("free")  + ": " + appManager1.formatDataSizeDelta(d.freeSpace))
                            parts.push(qsTr("total") + ": " + appManager1.formatDataSizeDelta(d.totalSpace))
                        }
                        if (d.dateUpdated && d.dateUpdated.length > 0)
                            parts.push(d.dateUpdated)
                        return parts.join("  ·  ")
                    }
                    visible: text.length > 0
                }
            }

            // The Selection card's own expand/collapse control, same icons and
            // same tooltips (DVP-F24). It keeps its place in the layout when the
            // device has no children, so every card in the view is the same
            // height - the reason the Selection card does it that way. Only the
            // Device tree shows a hierarchy, so only it carries the control.
            Controls.ToolButton {
                visible: card.showsHierarchy
                icon.name: card.cardIsCollapsed ? "go-down" : "go-up"
                icon.width:  Kirigami.Units.iconSizes.small
                icon.height: Kirigami.Units.iconSizes.small
                padding: 0
                opacity: card.cardHasChildren ? 1.0 : 0.0
                enabled: card.cardHasChildren
                onClicked: card.collapseToggleRequested(card.devId)
                Controls.ToolTip.text: card.cardIsCollapsed ? qsTr("Expand") : qsTr("Collapse")
                Controls.ToolTip.visible: hovered
            }

            IconButton { flat: true;
                icon.name: "application-menu"
                onClicked: contextMenu.popup()
                Controls.ToolTip.text: qsTr("Actions")
                Controls.ToolTip.visible: hovered
            }
        }
    }
}
