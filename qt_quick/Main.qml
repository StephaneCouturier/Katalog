import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import QtQuick.Dialogs as Dialogs
import Qt.labs.platform
import Qt.labs.settings 1.0
import Katalog 3.0

// Provides basic features needed for all kirigami applications
Kirigami.ApplicationWindow {
    // Unique identifier to reference this object
    id: root

    Settings {
        id: windowSettings
        property int  savedWidth:     900
        property int  savedHeight:    600
        property bool drawerPinned:   false
        property real savedCardScale: 1.0
    }

    signal searchTriggered()
    property real cardScale: 1.0

    // ── Page navigation ────────────────────────────────────────────────────
    //
    // Stack layout:
    //   col 0 : pageSelection  — permanent, never removed, no Close button
    //   col 1 : one active feature page at a time (Search by default)
    //           Switching feature pages replaces col 1, so Search is hidden
    //           when Devices / Explore / etc. is open — matching K2 tab behaviour.
    //   col 2 : SearchResults — only when Search is at col 1
    //
    // Layer stack (pageStack.layers, overlay):
    //   Settings, About — one at a time via showLayer().
    //   layers.replace() breaks Kirigami's page header, so we pop then push
    //   after the pop animation completes (tracked via layers.onBusyChanged).

    // Pending layer state — used to sequence pop → push
    property var  _pendingLayerComponent:   null
    property var  _pendingLayerProperties:  null

    Connections {
        target: pageStack.layers
        function onBusyChanged() {
            if (!pageStack.layers.busy && root._pendingLayerComponent !== null) {
                let comp  = root._pendingLayerComponent
                let props = root._pendingLayerProperties
                root._pendingLayerComponent  = null
                root._pendingLayerProperties = null
                if (props)
                    pageStack.layers.push(comp, props)
                else
                    pageStack.layers.push(comp)
            }
            // Re-layout backup cards after the layer animation fully completes.
            // Having two Connections blocks with the same signal can cause one to be
            // silently dropped, so this is merged here rather than in a second block.
            if (!pageStack.layers.busy && pageStack.layers.depth === 1 && pageStack.currentItem === pageBackup)
                Qt.callLater(function() { backupPageForm.refresh() })
        }
    }

    // Navigate to a main-stack page.
    // Always closes any open layer overlay first.
    // pageSelection : clears col 1+, goes to col 0.
    // Any other page: if already in stack → navigate there (preserving pages above it,
    //                 e.g. SearchResults stays when navigating to Search).
    //                 If not in stack → clear col 1+, push at col 1.
    function showPage(page) {
        // Close any open layer so main content is visible
        if (pageStack.layers.depth > 1)
            pageStack.layers.pop()

        if (page === pageSelection) {
            while (pageStack.depth > 1) {
                let p = pageStack.get(pageStack.depth - 1)
                pageStack.removePage(p)
                p.visible = false
            }
            return
        }

        // If page is already in the stack, navigate to it (preserves pages above)
        for (var i = 1; i < pageStack.depth; i++) {
            if (pageStack.get(i) === page) {
                pageStack.currentIndex = i
                return
            }
        }

        // Not in stack: clear col 1+, push at col 1
        while (pageStack.depth > 1) {
            let p = pageStack.get(pageStack.depth - 1)
            pageStack.removePage(p)
            p.visible = false
        }
        page.visible = true
        pageStack.push(page)
    }

    // Show a layer overlay (Settings, About).
    // If another layer is already open: pop it first, then push the new one
    // once the pop animation ends (avoids the broken-header bug from replace()).
    function showLayer(component, properties) {
        if (pageStack.layers.depth > 1) {
            _pendingLayerComponent  = component
            _pendingLayerProperties = properties || null
            pageStack.layers.pop()
        } else {
            if (properties)
                pageStack.layers.push(component, properties)
            else
                pageStack.layers.push(component)
        }
    }

    // Close a col-1 feature page: remove it and return to Selection only.
    // Called from the Close button of every col-1 page for consistent behaviour.
    function closeFeaturePage(page) {
        pageStack.removePage(page)
        page.visible = false
        // Clear any remaining col 1+ pages (e.g. SearchResults if Search was closed)
        while (pageStack.depth > 1) {
            let p = pageStack.get(pageStack.depth - 1)
            pageStack.removePage(p)
            p.visible = false
        }
    }

    // Global Drawer
    globalDrawer: Kirigami.GlobalDrawer {
        //isMenu: true
        modal: !windowSettings.drawerPinned
        width: 200

        header: Item {
            implicitHeight: Kirigami.Units.iconSizes.smallMedium + Kirigami.Units.largeSpacing * 2.5

            RowLayout {
                anchors {
                    left: parent.left; right: parent.right
                    verticalCenter: parent.verticalCenter
                    leftMargin: Kirigami.Units.largeSpacing
                    rightMargin: Kirigami.Units.largeSpacing
                }
                Kirigami.Icon {
                    source: appManager1.currentCollectionIconName
                    Layout.preferredWidth:  Kirigami.Units.iconSizes.smallMedium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                    Layout.alignment: Qt.AlignVCenter
                }
                Controls.Label {
                    text: appManager1.currentCollectionDisplayName
                    font.bold: true
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter
                }
            }
        }

        actions: [
            Kirigami.Action {
                separator: true
            },
            Kirigami.Action {
                text: "Open Collection..."
                icon.name: "document-open"
                Kirigami.Action {
                    icon.name: "folder"
                    text: "Collection Folder..."
                    onTriggered: {
                        var p = appManager1.getCollectionFolder()
                        if (p.length > 0)
                            memoryFolderDialog.currentFolder = "file://" + p
                        memoryFolderDialog.open()
                    }
                }
                Kirigami.Action {
                    icon.name: "network-server-database"
                    text: "SQLite Database..."
                    onTriggered: {
                        var p = appManager1.getDatabaseFilePath()
                        if (p.length > 0) {
                            var slash = p.lastIndexOf("/")
                            var folder = slash >= 0 ? p.substring(0, slash) : p
                            var parentSlash = folder.lastIndexOf("/")
                            databaseFileDialog.currentFolder = "file://" + (parentSlash > 0 ? folder.substring(0, parentSlash) : folder)
                        }
                        databaseFileDialog.open()
                    }
                }
                Kirigami.Action {
                    icon.name: "network-workgroup"
                    text: "Hosted Database..."
                    onTriggered: root.showLayer(settingsPageComponent, { showHostedForm: true })
                }
                Kirigami.Action {
                    separator: true
                    visible: appManager1.recentCollections.length > 0
                }
                Kirigami.Action {
                    visible: appManager1.recentCollections.length > 0
                    icon.name: appManager1.recentCollections.length > 0 ? appManager1.recentCollections[0].iconName : ""
                    text:      appManager1.recentCollections.length > 0 ? appManager1.recentCollections[0].displayName : ""
                    onTriggered: appManager1.openRecentCollection(appManager1.recentCollections[0])
                }
                Kirigami.Action {
                    visible: appManager1.recentCollections.length > 1
                    icon.name: appManager1.recentCollections.length > 1 ? appManager1.recentCollections[1].iconName : ""
                    text:      appManager1.recentCollections.length > 1 ? appManager1.recentCollections[1].displayName : ""
                    onTriggered: appManager1.openRecentCollection(appManager1.recentCollections[1])
                }
                Kirigami.Action {
                    visible: appManager1.recentCollections.length > 2
                    icon.name: appManager1.recentCollections.length > 2 ? appManager1.recentCollections[2].iconName : ""
                    text:      appManager1.recentCollections.length > 2 ? appManager1.recentCollections[2].displayName : ""
                    onTriggered: appManager1.openRecentCollection(appManager1.recentCollections[2])
                }
                Kirigami.Action {
                    visible: appManager1.recentCollections.length > 3
                    icon.name: appManager1.recentCollections.length > 3 ? appManager1.recentCollections[3].iconName : ""
                    text:      appManager1.recentCollections.length > 3 ? appManager1.recentCollections[3].displayName : ""
                    onTriggered: appManager1.openRecentCollection(appManager1.recentCollections[3])
                }
                Kirigami.Action {
                    visible: appManager1.recentCollections.length > 4
                    icon.name: appManager1.recentCollections.length > 4 ? appManager1.recentCollections[4].iconName : ""
                    text:      appManager1.recentCollections.length > 4 ? appManager1.recentCollections[4].displayName : ""
                    onTriggered: appManager1.openRecentCollection(appManager1.recentCollections[4])
                }
            },
            Kirigami.Action {
                separator: true
            },
            Kirigami.Action {
                icon.name: "edit-select"
                text: "Selection"
                onTriggered: { appManager1.setLastPage("Selection"); root.showPage(pageSelection) }
            },
            Kirigami.Action {
                icon.name: "edit-find"
                text: "Search"
                onTriggered: { appManager1.setLastPage("Search"); root.showPage(pageSearch) }
            },
            Kirigami.Action {
                icon.name: "drive-multidisk"
                text: "Devices"
                onTriggered: { appManager1.setLastPage("Devices"); root.showPage(pageDevices) }
            },
            Kirigami.Action {
                icon.name: "view-list-tree"
                text: "Explore"
                onTriggered: { appManager1.setLastPage("Explore"); root.showPage(pageExplore) }
            },
            Kirigami.Action {
                icon.name: "journal-new"
                text: "Create"
                onTriggered: { appManager1.setLastPage("Create"); root.showPage(pageCreate) }
            },
            Kirigami.Action {
                icon.name: "view-statistics"
                text: "Statistics"
                onTriggered: { appManager1.setLastPage("Statistics"); root.showPage(pageStatistics) }
            },
            Kirigami.Action {
                icon.name: "tag"
                text: "Tags"
                onTriggered: { appManager1.setLastPage("Tags"); root.showPage(pageTags) }
            },
            Kirigami.Action {
                icon.name: "backup"
                text: "Backup"
                onTriggered: { appManager1.setLastPage("Backup"); root.showPage(pageBackup) }
            },
            Kirigami.Action {
                icon.name: "configure"
                text: "Settings"
                onTriggered: { appManager1.setLastPage("Settings"); root.showLayer(settingsPageComponent) }
            },
            Kirigami.Action {
                separator: true
            },
            Kirigami.Action {
                icon.name: "help-about"
                text: "Documentation"
                onTriggered: Qt.openUrlExternally("https://stephanecouturier.github.io/Katalog/");
            },
            Kirigami.Action {
                text: "About" //i18n("About")
                icon.name: "help-about"
                onTriggered: root.showLayer(aboutPage)
            },
            Kirigami.Action {
                text: "Quit"
                icon.name: "application-exit"
                shortcut: StandardKey.Quit
                onTriggered: Qt.quit()
            }
        ]

        footer: Column {
            Controls.MenuSeparator { width: parent.width - 20}

            spacing: Kirigami.Units.smallSpacing
            padding: Kirigami.Units.largeSpacing

            Row {
                width: parent.width - Kirigami.Units.largeSpacing * 2
                spacing: Kirigami.Units.smallSpacing

                Controls.ToolButton {
                    icon.name: windowSettings.drawerPinned ? "window-unpin" : "window-pin"
                    checkable: true
                    checked: windowSettings.drawerPinned
                    Controls.ToolTip.text: windowSettings.drawerPinned ? "Unpin drawer" : "Pin drawer open"
                    Controls.ToolTip.visible: hovered
                    onToggled: windowSettings.drawerPinned = checked
                }

                Controls.Label {
                    text: windowSettings.drawerPinned ? "Drawer pinned" : "Drawer floating"
                    anchors.verticalCenter: parent.verticalCenter
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.9
                    opacity: 0.7
                }
            }

            Controls.Label {
                text: "Card text size"
                //font.bold: true
                width: parent.width
            }

            Row {
                width: parent.width
                spacing: Kirigami.Units.smallSpacing

                Controls.ToolButton {
                    icon.name: "zoom-out"
                    implicitWidth:  Kirigami.Units.iconSizes.small + Kirigami.Units.smallSpacing * 2
                    implicitHeight: Kirigami.Units.iconSizes.small + Kirigami.Units.smallSpacing * 2
                    onClicked: cardSizeSlider.value = Math.max(cardSizeSlider.from,
                                   cardSizeSlider.value - cardSizeSlider.stepSize)
                }

                Controls.Slider {
                    id: cardSizeSlider
                    from: 0.7
                    to: 1.3
                    value: windowSettings.savedCardScale
                    stepSize: 0.1
                    width: parent.width - 80
                    onValueChanged: {
                        root.cardScale = value
                        windowSettings.savedCardScale = value
                    }
                }

                Controls.ToolButton {
                    icon.name: "zoom-in"
                    implicitWidth:  Kirigami.Units.iconSizes.small + Kirigami.Units.smallSpacing * 2
                    implicitHeight: Kirigami.Units.iconSizes.small + Kirigami.Units.smallSpacing * 2
                    onClicked: cardSizeSlider.value = Math.min(cardSizeSlider.to,
                                   cardSizeSlider.value + cardSizeSlider.stepSize)
                }
            }
        }
    }

    // Database status notification
        Connections {
            target: appManager1
            function onDatabaseConnectionChanged(success, message) {
                if (success) {
                    showPassiveNotification("✓ " + message, "positive")
                    root.showPage(pageSelection)  // clear col 2, go to Selection
                } else {
                    showPassiveNotification("✗ " + message, "warning")
                }
            }
        }

    //Pages ---------------------------------------------------------------------
    pageStack.initialPage: pageSelection

    Component.onCompleted: {
        width         = windowSettings.savedWidth
        height        = windowSettings.savedHeight
        root.cardScale = windowSettings.savedCardScale
        if (appManager1.shouldShowAlphaWarning())
            alphaWarningDialog.open()

        // Restore last active page
        var last = appManager1.getLastPage()
        if (last === "Settings") {
            root.showLayer(settingsPageComponent)
        } else {
            var pageMap = {
                "Search":     pageSearch,
                "Devices":    pageDevices,
                "Explore":    pageExplore,
                "Create":     pageCreate,
                "Statistics": pageStatistics,
                "Tags":       pageTags,
                "Backup":     pageBackup
            }
            if (last !== "Selection" && pageMap[last])
                root.showPage(pageMap[last])
        }
    }

    onWidthChanged:  windowSettings.savedWidth  = width
    onHeightChanged: windowSettings.savedHeight = height

    Controls.Dialog {
        id: alphaWarningDialog
        title: "Katalog 3 - Alpha2 Version"
        modal: true
        anchors.centerIn: parent
        width: Math.min(520, parent.width - Kirigami.Units.largeSpacing * 4)

        Controls.Label {
            width: parent.width
            wrapMode: Text.WordWrap
            text: "This is an early alpha version of Katalog intended to support development and gather feedback.\n\n" +
                  "Features available (read-only):\n" +
                  "  • Open a collection\n" +
                  "  • Device selection\n" +
                  "  • Search\n\n" +
                  "All other features are not yet available in this version."
        }

        footer: Controls.DialogButtonBox {
            Controls.CheckBox {
                id: doNotShowAgain
                text: "Do not show again"
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.ResetRole
            }
            Controls.Button {
                text: "OK"
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
            onAccepted: {
                if (doNotShowAgain.checked)
                    appManager1.setAlphaWarningShown()
                alphaWarningDialog.close()
            }
        }
    }

    // Edit device dialogs
    Controls.Dialog {
        id: editDeleteDeviceDialog
        property string deviceName: ""
        property string deviceType: ""
        property int    deviceId:   0
        title: "Katalog"
        modal: true
        anchors.centerIn: parent
        width: Math.min(460, root.width - Kirigami.Units.largeSpacing * 4)
        contentItem: Controls.Label {
            width: editDeleteDeviceDialog.availableWidth
            wrapMode: Text.WordWrap
            textFormat: Text.RichText
            text: qsTr("Do you want to <b>delete</b> this %1 device?<br/><br/>Name: <b>%2</b>")
                  .arg(editDeleteDeviceDialog.deviceType)
                  .arg(editDeleteDeviceDialog.deviceName)
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("Yes")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
            Controls.Button {
                text: qsTr("Cancel")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.RejectRole
            }
            onAccepted: {
                editDeleteDeviceDialog.close()
                var err = appManager1.deleteDevice(editDeleteDeviceDialog.deviceId)
                if (err !== "") {
                    editValidationDialog.message = err
                    editValidationDialog.open()
                } else {
                    root.closeFeaturePage(pageDeviceEdit)
                }
            }
            onRejected: editDeleteDeviceDialog.close()
        }
    }

    Controls.Dialog {
        id: editValidationDialog
        property alias message: editValidationLabel.text
        title: "Katalog"
        modal: true
        anchors.centerIn: parent
        width: Math.min(460, root.width - Kirigami.Units.largeSpacing * 4)
        contentItem: Controls.Label {
            id: editValidationLabel
            width: editValidationDialog.availableWidth
            wrapMode: Text.WordWrap
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("OK")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
            onAccepted: editValidationDialog.close()
        }
    }

    Controls.Dialog {
        id: editCatalogConfirmDialog
        property bool rescanNeeded: false
        property bool pathChanged:  false
        title: "Katalog"
        modal: true
        anchors.centerIn: parent
        width: Math.min(520, root.width - Kirigami.Units.largeSpacing * 4)
        contentItem: Controls.Label {
            id: editCatalogConfirmLabel
            width: editCatalogConfirmDialog.availableWidth
            wrapMode: Text.WordWrap
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("Yes")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
            Controls.Button {
                text: qsTr("Cancel")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.RejectRole
            }
            onAccepted: {
                editCatalogConfirmDialog.close()
                pageDeviceEdit_form.confirmCatalogSave(editCatalogConfirmDialog.rescanNeeded,
                                                       editCatalogConfirmDialog.pathChanged)
            }
            onRejected: editCatalogConfirmDialog.close()
        }
    }

    Controls.Dialog {
        id: editCatalogUpdateDialog
        title: "Katalog"
        modal: true
        anchors.centerIn: parent
        width: Math.min(460, root.width - Kirigami.Units.largeSpacing * 4)
        contentItem: Controls.Label {
            width: editCatalogUpdateDialog.availableWidth
            wrapMode: Text.WordWrap
            text: qsTr("Update the catalog content with the new criteria?")
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("Yes")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
            Controls.Button {
                text: qsTr("No")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.RejectRole
            }
            onAccepted: {
                editCatalogUpdateDialog.close()
                appManager1.triggerDeviceRescan(pageDeviceEdit_form.deviceId)
                pageDeviceEdit_form.finalizeSave()
            }
            onRejected: { editCatalogUpdateDialog.close(); pageDeviceEdit_form.finalizeSave() }
        }
    }

    Controls.Dialog {
        id: editStoragePathDialog
        property string previousPath: ""
        property string newPath: ""
        title: "Katalog"
        modal: true
        anchors.centerIn: parent
        width: Math.min(520, root.width - Kirigami.Units.largeSpacing * 4)
        contentItem: Column {
            spacing: Kirigami.Units.largeSpacing
            Controls.Label {
                width: editStoragePathDialog.availableWidth
                wrapMode: Text.WordWrap
                text: qsTr("The source path changed.\n\nOld path: %1\nNew path: %2\n\nHow should the catalog indexes be updated?")
                      .arg(editStoragePathDialog.previousPath)
                      .arg(editStoragePathDialog.newPath)
            }
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("Replace path root")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
            Controls.Button {
                id: editPathRescanBtn
                text: qsTr("Full re-index")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.ApplyRole
            }
            Controls.Button {
                text: qsTr("Skip")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.RejectRole
            }
            onAccepted: {
                editStoragePathDialog.close()
                appManager1.triggerStoragePathReplace(pageDeviceEdit_form.deviceId,
                                                      editStoragePathDialog.previousPath,
                                                      editStoragePathDialog.newPath)
                pageDeviceEdit_form.finalizeSave()
            }
            onRejected: { editStoragePathDialog.close(); pageDeviceEdit_form.finalizeSave() }
        }
        Connections {
            target: editStoragePathDialog.footer
            function onClicked(button) {
                if (button === editPathRescanBtn) {
                    editStoragePathDialog.close()
                    appManager1.triggerDeviceRescan(pageDeviceEdit_form.deviceId)
                    pageDeviceEdit_form.finalizeSave()
                }
            }
        }
    }

    // Create validation dialogs
    Controls.Dialog {
        id: createValidationDialog
        property alias message: createValidationLabel.text
        title: "Katalog"
        modal: true
        anchors.centerIn: parent
        width: Math.min(460, root.width - Kirigami.Units.largeSpacing * 4)
        contentItem: Controls.Label {
            id: createValidationLabel
            width: createValidationDialog.availableWidth
            wrapMode: Text.WordWrap
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("OK")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
            onAccepted: createValidationDialog.close()
        }
    }

    Controls.Dialog {
        id: createEmptyDirDialog
        title: "Katalog"
        modal: true
        anchors.centerIn: parent
        width: Math.min(460, root.width - Kirigami.Units.largeSpacing * 4)
        contentItem: Controls.Label {
            width: createEmptyDirDialog.availableWidth
            wrapMode: Text.WordWrap
            text: qsTr("The source folder does not contain any file.\nThis could mean that the source is empty or the device is not mounted to this folder.\nDo you want to save it anyway (the catalog would be empty)?")
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("Yes")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
            Controls.Button {
                text: qsTr("No")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.RejectRole
            }
            onAccepted: { createEmptyDirDialog.close(); pageCreate_formLayout_Create.doCreate() }
            onRejected: createEmptyDirDialog.close()
        }
    }

    Component {
        id: aboutPage
        Kirigami.AboutPage {
            aboutData: About
            actions: [
                Kirigami.Action {
                    text: "Close"
                    icon.name: "view-close"
                    onTriggered: pageStack.layers.pop()
                }
            ]
        }
    }

    //Pages - Selection
    Kirigami.ScrollablePage {
        id: pageSelection
        title: "Selection"

        property string deviceType: "Storage"

        Connections {
            target: appManager1
            function onSelectedDeviceChanged(deviceId) {
                console.log("QML received selectedDeviceChanged signal for device ID:", deviceId)

                // Force refresh of the selection list view to update visual states
                if (selectionListView1 && selectionListView1.model) {
                    // Trigger a visual update of all delegates
                    selectionListView1.model.dataChanged(
                        selectionListView1.model.index(0, 0),
                        selectionListView1.model.index(selectionListView1.model.rowCount() - 1, 0)
                    )
                }
            }
        }

        header: Item {
            width: parent.width
            height: deviceSearchField.implicitHeight + Kirigami.Units.smallSpacing * 5
            RowLayout {
                anchors {
                    left: parent.left
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    leftMargin: Kirigami.Units.gridUnit
                    rightMargin: Kirigami.Units.gridUnit
                }
                spacing: Kirigami.Units.smallSpacing
                Kirigami.SearchField {
                    id: deviceSearchField
                    Layout.fillWidth: true
                    onTextChanged: appManager1.setDeviceFilter(text)
                }
                Controls.CheckBox {
                    checked: appManager1.showDeviceInfo
                    onToggled: appManager1.showDeviceInfo = checked
                    Controls.ToolTip.text: "Show device info"
                    Controls.ToolTip.visible: hovered
                }
                Controls.ToolButton {
                    icon.name: "go-up"
                    enabled: appManager1.canCollapseDevices
                    onClicked: appManager1.collapseDevices()
                    Controls.ToolTip.text: "Collapse one level"
                    Controls.ToolTip.visible: hovered
                }
                Controls.ToolButton {
                    icon.name: "go-down"
                    enabled: appManager1.canExpandDevices
                    onClicked: appManager1.expandDevices()
                    Controls.ToolTip.text: "Expand one level"
                    Controls.ToolTip.visible: hovered
                }
            }
        }

        Kirigami.CardsListView {
            id: selectionListView1
            model: appManager1.deviceFilterModel
            delegate: PageSelectionDelegate {}
            topMargin: Kirigami.Units.smallSpacing
        }
    }

    //Pages - Search
    Kirigami.ScrollablePage {
        id: pageSearch
        title: qsTr("Search")

        actions: [
            Kirigami.Action {
                text: qsTr("Search")
                icon.name: "edit-find"
                onTriggered: {
                    root.searchTriggered()
                    pageSearchForm.executeSearch()
                    // Remove any stale Results page, then push fresh
                    while (pageStack.depth > 2) {
                        let p = pageStack.get(pageStack.depth - 1)
                        pageStack.removePage(p)
                        p.visible = false
                    }
                    // Ensure currentIndex points to Search (depth-1) before pushing.
                    // Kirigami's push() truncates pages forward of currentIndex, so if
                    // the user back-navigated to Selection (currentIndex=0) without
                    // removing pages, push() would drop Search and land on [Selection, Results].
                    pageStack.currentIndex = pageStack.depth - 1
                    pageSearchResults.visible = true
                    pageStack.push(pageSearchResults)
                }
            },
            Kirigami.Action {
                text: qsTr("Reset")
                icon.name: "edit-clear-history"
                onTriggered: pageSearchForm.resetSearch()
            },
            Kirigami.Action {
                text: qsTr("History")
                icon.name: "view-history"
                onTriggered: pageSearchForm.openHistorySheet()
            },
            Kirigami.Action {
                text: qsTr("Close")
                icon.name: "view-close"
                onTriggered: {
                    pageStack.removePage(pageSearch)
                    pageSearch.visible = false
                    // Results stays in the stack if present; navigate to it or Selection
                    pageStack.currentIndex = Math.max(0, pageStack.depth - 1)
                }
            }
        ]

        footer: RowLayout {
            visible: appManager1.searchIsRunning || appManager1.searchStatusText.length > 0
            spacing: Kirigami.Units.smallSpacing
            Controls.BusyIndicator {
                running: appManager1.searchIsRunning
                visible: appManager1.searchIsRunning
                implicitWidth:  Kirigami.Units.gridUnit * 1.5
                implicitHeight: Kirigami.Units.gridUnit * 1.5
                Layout.leftMargin: Kirigami.Units.smallSpacing
            }
            Controls.Label {
                Layout.fillWidth: true
                Layout.margins: Kirigami.Units.smallSpacing
                text: appManager1.searchStatusText
                textFormat: Text.StyledText
                elide: Text.ElideRight
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.85
            }
        }

        PageSearchForm {
            id: pageSearchForm
        }
    }

    //Pages - SearchResults
    Kirigami.Page {
        id: pageSearchResults
        visible: false
        title: {
            let n = newSearch1.properties.filesFoundNumber ?? 0
            if (newSearch1.properties.searchOnDuplicates)
                return qsTr("Duplicates (%1)").arg(n)
            if (newSearch1.properties.searchOnDifferences)
                return qsTr("Differences (%1)").arg(n)
            return qsTr("Results")  //return qsTr("Results (%1)").arg(n)
        }
        padding: 0

        actions: [
            Kirigami.Action {
                text: qsTr("Close")
                icon.name: "view-close"
                onTriggered: {
                    pageStack.removePage(pageSearchResults)
                    pageSearchResults.visible = false
                    // Go to Search if still in stack, otherwise Selection
                    pageStack.currentIndex = Math.max(0, pageStack.depth - 1)
                }
            }
        ]

        footer: RowLayout {
            visible: appManager1.searchStatusText.length > 0
            spacing: Kirigami.Units.smallSpacing
            Controls.Label {
                Layout.fillWidth: true
                Layout.margins: Kirigami.Units.smallSpacing
                text: appManager1.searchStatusText
                textFormat: Text.StyledText
                elide: Text.ElideRight
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.85
            }
        }

        PageSearchResultsForm {
            id: pageSearchResultsForm
            width:  pageSearchResults.availableWidth
            height: pageSearchResults.availableHeight
            onCloseRequested: {
                pageStack.removePage(pageSearchResults)
                pageSearchResults.visible = false
                pageStack.currentIndex = Math.max(0, pageStack.depth - 1)
            }
        }
    }

    //Pages - Devices
    Kirigami.ScrollablePage {
        id: pageDevices
        visible: false
        title: "Devices"

        actions: [
            /*Kirigami.Action {
                text: "Batch process"
                icon.name: "document-export"
                onTriggered: showPassiveNotification("Batch process clicked, no action")
            },*/
            Kirigami.Action {
                text: "Close"
                icon.name: "view-close"
                onTriggered: root.closeFeaturePage(pageDevices)
            }
        ]

        PageDevicesView {
            id: pageDevicesView
        }
    }

    //Pages - Explore
    Kirigami.ScrollablePage {
        id: pageExplore
        visible: false
        title: "Explore"

        actions: [
            /*Kirigami.Action {
                text: "Batch process"
                icon.name: "document-export"
                onTriggered: showPassiveNotification("Batch process clicked, no action")
            },*/
            Kirigami.Action {
                text: "Close"
                icon.name: "view-close"
                onTriggered: root.closeFeaturePage(pageExplore)
            }
        ]

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            text: "Explore — coming soon"
            icon.name: "view-list-tree"
        }
    }

    //Pages - Create
    Kirigami.ScrollablePage {
        id: pageCreate
        visible: false
        title: "Create"

        Connections {
            target: appManager1
            function onCatalogCreationCompleted(success, report) {
                if (pageCreate.visible) {
                    showPassiveNotification(
                        success ? qsTr("Catalog created successfully.") : qsTr("Catalog creation failed: ") + report,
                        success ? "short" : "long"
                    )
                    if (success)
                        appManager1.refreshDeviceList()
                }
            }
        }

        Connections {
            target: pageCreate_formLayout_Create
            function onValidationError(message) {
                createValidationDialog.message = message
                createValidationDialog.open()
            }
            function onEmptyDirConfirmNeeded() {
                createEmptyDirDialog.open()
            }
        }

        actions: [
            Kirigami.Action {
                text: qsTr("Create")
                icon.name: "document-save"
                enabled: !appManager1.catalogIsCreating
                onTriggered: pageCreate_formLayout_Create.triggerCreate()
            },
            Kirigami.Action {
                text: qsTr("Stop")
                icon.name: "process-stop"
                enabled: appManager1.catalogIsCreating
                onTriggered: pageCreate_formLayout_Create.triggerStop()
            },
            Kirigami.Action {
                text: qsTr("Close")
                icon.name: "view-close"
                onTriggered: root.closeFeaturePage(pageCreate)
            }
        ]

        footer: RowLayout {
            visible: appManager1.catalogIsCreating || appManager1.catalogStatusText.length > 0
            spacing: Kirigami.Units.smallSpacing
            Controls.BusyIndicator {
                running: appManager1.catalogIsCreating
                visible: appManager1.catalogIsCreating
                implicitWidth:  Kirigami.Units.gridUnit * 1.5
                implicitHeight: Kirigami.Units.gridUnit * 1.5
                Layout.leftMargin: Kirigami.Units.smallSpacing
            }
            Controls.Label {
                Layout.fillWidth: true
                Layout.margins: Kirigami.Units.smallSpacing
                text: appManager1.catalogStatusText
                textFormat: Text.StyledText
                elide: Text.ElideRight
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.85
            }
        }

        PageCreateForm {
            id: pageCreate_formLayout_Create
        }
    }

    //Pages - Device Edit
    Kirigami.ScrollablePage {
        id: pageDeviceEdit
        visible: false
        title: qsTr("Edit Device")

        Connections {
            target: pageDeviceEdit_form
            function onSaveError(message) {
                editValidationDialog.message = message
                editValidationDialog.open()
            }
            function onCatalogConfirmNeeded(message, rescanNeeded, pathChanged) {
                editCatalogConfirmLabel.text = qsTr("Save changes to the catalog definition?\n\n%1\n\n(The catalog must be updated to reflect these changes)").arg(message)
                editCatalogConfirmDialog.rescanNeeded = rescanNeeded
                editCatalogConfirmDialog.pathChanged  = pathChanged
                editCatalogConfirmDialog.open()
            }
            function onCatalogUpdateContentNeeded() {
                editCatalogUpdateDialog.open()
            }
            function onStoragePathChangeNeeded(previousPath, newPath) {
                editStoragePathDialog.previousPath = previousPath
                editStoragePathDialog.newPath = newPath
                editStoragePathDialog.open()
            }
            function onSaveCompleted() {
                root.closeFeaturePage(pageDeviceEdit)
            }
        }

        actions: [
            Kirigami.Action {
                text: qsTr("Save")
                icon.name: "document-save"
                onTriggered: pageDeviceEdit_form.triggerSave()
            },
            Kirigami.Action {
                text: qsTr("Delete")
                icon.name: "edit-delete"
                onTriggered: {
                    var check = appManager1.checkDeviceDeleteAllowed(pageDeviceEdit_form.deviceId)
                    if (!check.allowed) {
                        editValidationDialog.message = check.errorMessage
                        editValidationDialog.open()
                    } else {
                        var d = appManager1.getDeviceDetails(pageDeviceEdit_form.deviceId)
                        editDeleteDeviceDialog.deviceId   = pageDeviceEdit_form.deviceId
                        editDeleteDeviceDialog.deviceType = pageDeviceEdit_form.deviceType
                        editDeleteDeviceDialog.deviceName = d.name
                        editDeleteDeviceDialog.open()
                    }
                }
            },
            Kirigami.Action {
                text: qsTr("Close")
                icon.name: "view-close"
                onTriggered: root.closeFeaturePage(pageDeviceEdit)
            }
        ]

        PageDeviceEditForm {
            id: pageDeviceEdit_form
        }
    }

    //Pages - Statistics
    Kirigami.Page {
        id: pageStatistics
        visible: false
        title: "Statistics"
        padding: 0

        actions: [
            Kirigami.Action {
                text: qsTr("Close")
                icon.name: "view-close"
                onTriggered: root.closeFeaturePage(pageStatistics)
            }
        ]

        PageStatisticsForm {
            anchors.fill: parent
        }
    }

    //Pages - Tags
    Kirigami.ScrollablePage {
        id: pageTags
        visible: false
        title: "Tags"

        actions: [
            Kirigami.Action {
                text: qsTr("Close")
                icon.name: "view-close"
                onTriggered: root.closeFeaturePage(pageTags)
            }
        ]

        PageTagsForm {}
    }

    //Pages - Backup
    Kirigami.ScrollablePage {
        id: pageBackup
        visible: false
        title: qsTr("BackUp")

        onVisibleChanged: {
            if (visible)
                Qt.callLater(function() { backupPageForm.refresh() })
        }

        actions: [
            Kirigami.Action {
                text:      qsTr("Add")
                icon.name: "list-add"
                onTriggered: root.showLayer(backupMappingFormComponent)
            },
            Kirigami.Action {
                text:      qsTr("Generate LuckyBackup profile")
                icon.name: "application-x-executable"
                enabled:   backupPageForm.mappings.length > 0
                onTriggered: {
                    var ids = backupPageForm.mappings.map(function(m) { return m.mappingId })
                    appManager1.generateLuckyBackupProfile(ids)
                }
            },
            Kirigami.Action {
                text:      qsTr("Close")
                icon.name: "view-close"
                onTriggered: root.closeFeaturePage(pageBackup)
            }
        ]

        PageBackupForm {
            id: backupPageForm
            onRequestAddMapping:        root.showLayer(backupMappingFormComponent)
            onRequestPreviewMapping: (mappingId) => root.showLayer(backupPreviewFormComponent, { mappingId: mappingId })
        }
    }

    Component {
        id: backupMappingFormComponent
        PageBackupMappingForm {}
    }

    Component {
        id: backupPreviewFormComponent
        PageBackupPreviewForm {}
    }

    //Pages - Settings
    Component {
        id: settingsPageComponent
        PageSettings {}
    }

    // Dialogs - triggered from Open Collection menu
    Dialogs.FolderDialog {
        id: memoryFolderDialog
        title: "Select Collection Folder"
        onAccepted: {
            var path = selectedFolder.toString().replace("file://", "")
            appManager1.openCollectionMemory(path)
        }
    }

    Dialogs.FileDialog {
        id: databaseFileDialog
        title: "Open SQLite Database"
        nameFilters: ["SQLite databases (*.db *.sqlite *.sqlite3)", "All files (*)"]
        onAccepted: {
            var path = selectedFile.toString().replace("file://", "")
            appManager1.setDatabaseFilePath(path)
        }
    }
}
