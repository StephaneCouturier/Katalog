import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import QtQuick.Dialogs as Dialogs
import Qt.labs.platform
import QtCore
import Katalog 3.0

// Provides basic features needed for all kirigami applications
Kirigami.ApplicationWindow {
    // Unique identifier to reference this object
    id: root

    // Global activity panel: running job progress plus the queue behind it.
    // Anchored to pageStack rather than used as the window footer: the window
    // footer spans the whole window, so a pinned (non-modal) global drawer sits
    // on top of its left edge. pageStack already excludes the drawer, so
    // matching its geometry puts the panel beside the drawer, not under it.
    OperationQueueView {
        id: activityPanel
        anchors.left:   root.pageStack.left
        anchors.right:  root.pageStack.right
        anchors.bottom: root.pageStack.bottom
        z: 1
    }

    // Re-probe device active status when the user comes back to Katalog, so a
    // drive connected or removed meanwhile shows correctly. Off by default and
    // gated on the mount table, so this is normally a no-op — see
    // SpecDeviceActiveStatus.md. Application-level rather than per-window, so a
    // future secondary window would not change the behaviour.
    Connections {
        target: Qt.application
        function onStateChanged() {
            if (Qt.application.state === Qt.ApplicationActive)
                appManager1.refreshDeviceActiveOnActivation()
        }
    }

    Settings {
        id: windowSettings
        property int  savedWidth:     900
        property int  savedHeight:    600
        property bool drawerPinned:   false
        property real savedCardScale: 1.0
    }

    signal searchTriggered()
    property real cardScale: 1.0

    // One metric for every small icon button on the Selection page — the panel
    // header and the device cards alike — so the two read as one set and the
    // buttons never set the card height.
    readonly property real selectionButton: Kirigami.Units.iconSizes.small
                                            + Kirigami.Units.smallSpacing
    property bool _firstRunCreateNew: false

    // Selection ("Filters" panel in K2) can be collapsed beside an open feature
    // page — the K2 ShowHideFilters portage (mainwindow_tab_filters.cpp). It is
    // removed from the stack when hidden so the feature page becomes column 0 and
    // keeps Kirigami's global-drawer button; it is forced back in whenever no
    // feature page is open, so the window is never blank.
    // featureOpen = is any non-Selection page currently in the stack.
    property bool featureOpen: false

    // The feature page currently open (null when only Selection is shown). Kept
    // reactive so context menus can adapt to the active page — e.g. the Selection
    // card menu hides its "Search" shortcut while Search/Results is already open.
    property var openFeaturePage: null

    function indexOfPage(page) {
        for (var i = 0; i < pageStack.depth; i++)
            if (pageStack.get(i) === page) return i
        return -1
    }

    function selectionInStack() {
        return pageStack.depth > 0 && pageStack.get(0) === pageSelection
    }

    function recomputeFeatureOpen() {
        for (var i = 0; i < pageStack.depth; i++)
            if (pageStack.get(i) !== pageSelection) { featureOpen = true; return }
        featureOpen = false
    }

    // Remove every page except Selection (static page objects are reused, so we
    // also hide them — matches the existing close/re-push pattern).
    function removeAllFeaturePages() {
        for (var i = pageStack.depth - 1; i >= 0; i--) {
            let p = pageStack.get(i)
            if (p !== pageSelection) {
                pageStack.removePage(p)
                p.visible = false
            }
        }
        featureOpen = false
    }

    // Enforce the invariant: Selection in stack at index 0 iff shown OR no feature
    // page open. Call after any change to the stack or to showSelectionPage.
    function syncSelectionVisibility() {
        let shouldShow = appManager1.showSelectionPage || !featureOpen
        let inStack = selectionInStack()
        if (shouldShow && !inStack) {
            // Selection must occupy column 0, before the feature page(s). Inserting
            // at index 0 with insertPage() drops the pages after it on this Kirigami
            // build (the feature page would vanish), so rebuild the stack from the
            // existing page objects — they are static/reused, so state is preserved.
            let pages = []
            for (var i = 0; i < pageStack.depth; i++)
                pages.push(pageStack.get(i))
            for (var j = pages.length - 1; j >= 0; j--)
                pageStack.removePage(pages[j])
            pageSelection.visible = true
            pageStack.push(pageSelection)
            for (var k = 0; k < pages.length; k++) {
                pages[k].visible = true
                pageStack.push(pages[k])
            }
            pageStack.currentIndex = pageStack.depth - 1
        } else if (!shouldShow && inStack) {
            pageStack.removePage(pageSelection)
            pageSelection.visible = false
        }
    }

    // ── Page navigation ────────────────────────────────────────────────────
    //
    // Stack layout:
    //   col 0 : pageSelection — the home column; collapsible (see syncSelectionVisibility).
    //           When hidden with a feature page open it is removed from the stack,
    //           so the feature page takes col 0. No Close button.
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
            // Navigate home: drop feature pages. Selection reappears (it is now the
            // only page) without changing the user's show/hide preference.
            removeAllFeaturePages()
            syncSelectionVisibility()
            pageStack.currentIndex = 0
            openFeaturePage = null
            return
        }

        // If the feature page is already in the stack, navigate to it
        // (preserves pages above it, e.g. SearchResults stays when going to Search).
        let existing = indexOfPage(page)
        if (existing >= 0) {
            pageStack.currentIndex = existing
            openFeaturePage = page
            return
        }

        // Not in stack: clear other feature pages, push this one, then collapse
        // Selection if it is hidden so the feature page becomes column 0.
        removeAllFeaturePages()
        page.visible = true
        pageStack.push(page)
        featureOpen = true
        syncSelectionVisibility()
        pageStack.currentIndex = indexOfPage(page)
        openFeaturePage = page
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
        // Clear any remaining feature pages (e.g. SearchResults if Search was closed)
        removeAllFeaturePages()
        // No feature page left → Selection must reappear so the window is not blank.
        syncSelectionVisibility()
        pageStack.currentIndex = Math.max(0, pageStack.depth - 1)
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
                text: qsTr("Open...")
                icon.name: "document-open"
                Kirigami.Action {
                    icon.name: "folder"
                    text: qsTr("Collection Folder...")
                    onTriggered: {
                        var p = appManager1.getCollectionFolder()
                        if (p.length > 0)
                            memoryFolderDialog.currentFolder = appManager1.pathToFileUrl(p)
                        memoryFolderDialog.open()
                    }
                }
                Kirigami.Action {
                    icon.name: "network-server-database"
                    text: qsTr("SQLite Database...")
                    onTriggered: {
                        var p = appManager1.getDatabaseFilePath()
                        if (p.length > 0) {
                            var slash = p.lastIndexOf("/")
                            var folder = slash >= 0 ? p.substring(0, slash) : p
                            var parentSlash = folder.lastIndexOf("/")
                            databaseFileDialog.currentFolder = appManager1.pathToFileUrl(parentSlash > 0 ? folder.substring(0, parentSlash) : folder)
                        }
                        databaseFileDialog.open()
                    }
                }
                Kirigami.Action {
                    icon.name: "network-workgroup"
                    text: qsTr("Hosted Database...")
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
                text: qsTr("New...")
                icon.name: "document-new"
                Kirigami.Action {
                    icon.name: "network-server-database"
                    text: qsTr("SQLite Database...")
                    onTriggered: {
                        var p = appManager1.getNewCollectionDefaultPath()
                        newDatabaseFileDialog.currentFile = appManager1.pathToFileUrl(p)
                        newDatabaseFileDialog.open()
                    }
                }
            },
            Kirigami.Action {
                separator: true
            },
            Kirigami.Action {
                icon.name: "edit-select"
                text: qsTr("Selection")
                onTriggered: { appManager1.setLastPage("Selection"); root.showPage(pageSelection) }
            },
            Kirigami.Action {
                icon.name: "edit-find"
                text: qsTr("Search")
                onTriggered: { appManager1.setLastPage("Search"); root.showPage(pageSearch) }
            },
            Kirigami.Action {
                icon.name: "drive-multidisk"
                text: qsTr("Devices")
                onTriggered: { appManager1.setLastPage("Devices"); root.showPage(pageDevices) }
            },
            Kirigami.Action {
                icon.name: "view-list-tree"
                text: qsTr("Explore")
                onTriggered: { appManager1.setLastPage("Explore"); root.showPage(pageExplore) }
            },
            Kirigami.Action {
                icon.name: "journal-new"
                text: qsTr("Create")
                onTriggered: { appManager1.setLastPage("Create"); root.showPage(pageCreate) }
            },
            Kirigami.Action {
                icon.name: "backup"
                text: qsTr("Backup")
                onTriggered: { appManager1.setLastPage("Backup"); root.showPage(pageBackup) }
            },
            Kirigami.Action {
                icon.name: "view-statistics"
                text: qsTr("Statistics")
                onTriggered: { appManager1.setLastPage("Statistics"); root.showPage(pageStatistics) }
            },
            Kirigami.Action {
                icon.name: "tag"
                text: qsTr("Tags")
                onTriggered: { appManager1.setLastPage("Tags"); root.showPage(pageTags) }
            },
            Kirigami.Action {
                icon.name: "configure"
                text: qsTr("Settings")
                onTriggered: { appManager1.setLastPage("Settings"); root.showLayer(settingsPageComponent) }
            },
            Kirigami.Action {
                separator: true
            },
            Kirigami.Action {
                icon.name: "help-about"
                text: qsTr("Documentation")
                onTriggered: Qt.openUrlExternally("https://stephanecouturier.github.io/Katalog/");
            },
            Kirigami.Action {
                text: qsTr("About") //i18n("About")
                icon.name: "help-about"
                onTriggered: root.showLayer(aboutPage)
            },
            Kirigami.Action {
                text: qsTr("Quit")
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
                    onToggled: windowSettings.drawerPinned = checked
                }

                Controls.Label {
                    text: windowSettings.drawerPinned ? qsTr("Drawer pinned") : qsTr("Drawer floating")
                    anchors.verticalCenter: parent.verticalCenter
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.9
                    opacity: 0.7
                }
            }

            Row {
                width: parent.width - Kirigami.Units.largeSpacing * 2
                spacing: Kirigami.Units.smallSpacing

                Controls.ToolButton {
                    // Collapse/restore the Selection column beside the open feature
                    // page (K2 ShowHideFilters portage). Disabled when Selection is
                    // the only page — there is nothing to collapse to.
                    icon.name: appManager1.showSelectionPage ? "window-unpin" : "window-pin"
                    checkable: true
                    checked: !appManager1.showSelectionPage
                    enabled: root.featureOpen
                    onToggled: appManager1.showSelectionPage = !checked
                }

                Controls.Label {
                    text: appManager1.showSelectionPage ? qsTr("Selection shown") : qsTr("Selection hidden")
                    anchors.verticalCenter: parent.verticalCenter
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.9
                    opacity: 0.7
                }
            }

            Controls.Label {
                text: qsTr("Card text size")
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
                    pageSearchForm.restoreLastSearch()
                    if (appManager1.isFirstRun) {
                        appManager1.clearFirstRun()
                        if (root._firstRunCreateNew)
                            firstRunReadyDialog.open()
                    }
                } else {
                    showPassiveNotification("✗ " + message, "warning")
                }
            }
            // Collapse/restore Selection when the user toggles it, keeping the
            // open feature page in view.
            function onShowSelectionPageChanged() {
                root.syncSelectionVisibility()
                if (pageStack.depth > 0)
                    pageStack.currentIndex = pageStack.depth - 1
            }
        }

    //Pages ---------------------------------------------------------------------
    pageStack.initialPage: pageSelection

    Component.onCompleted: {
        width         = windowSettings.savedWidth
        height        = windowSettings.savedHeight
        root.cardScale = windowSettings.savedCardScale

        if (appManager1.isFirstRun) {
            firstRunWelcomeDialog.open()
            return
        }

        if (appManager1.shouldShowAlphaWarning())
            alphaWarningDialog.open()

        pageSearchForm.restoreLastSearch()

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
        title: "Katalog 3 - Alpha3 Version"
        modal: true
        anchors.centerIn: parent
        width: Math.min(520, parent.width - Kirigami.Units.largeSpacing * 4)

        Controls.Label {
            width: parent.width
            wrapMode: Text.WordWrap
            text: qsTr("This is an early alpha version of Katalog intended to support development and gather feedback.\n\n")
        }

        footer: Controls.DialogButtonBox {
            Controls.CheckBox {
                id: doNotShowAgain
                text: qsTr("Do not show again")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.ResetRole
            }
            Controls.Button {
                text: qsTr("OK")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
            onAccepted: {
                if (doNotShowAgain.checked)
                    appManager1.setAlphaWarningShown()
                alphaWarningDialog.close()
            }
        }
    }

    // First-run welcome dialog
    Controls.Dialog {
        id: firstRunWelcomeDialog
        title: "Katalog"
        modal: true
        anchors.centerIn: parent
        width: Math.min(520, parent.width - Kirigami.Units.largeSpacing * 4)
        contentItem: Controls.Label {
            width: firstRunWelcomeDialog.availableWidth
            wrapMode: Text.WordWrap
            textFormat: Text.RichText
            text: qsTr("<br/><b>Welcome to Katalog!</b><br/><br/>It seems this is the first run.<br/><br/>The following Settings have been applied:<br/> - Language: <b>%1</b><br/> - Theme: <b>%2</b><br/><br/>You can change these in the tab %3.")
                      .arg(Qt.locale().name)
                      .arg(qsTr("System"))
                      .arg(qsTr("Settings"))
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("Open existing...")
                onClicked: {
                    root._firstRunCreateNew = false
                    firstRunWelcomeDialog.close()
                    databaseFileDialog.open()
                }
            }
            Controls.Button {
                text: qsTr("Create new...")
                onClicked: {
                    root._firstRunCreateNew = true
                    firstRunWelcomeDialog.close()
                    var p = appManager1.getNewCollectionDefaultPath()
                    newDatabaseFileDialog.currentFile = appManager1.pathToFileUrl(p)
                    newDatabaseFileDialog.open()
                }
            }
        }
    }

    // First-run ready dialog (shown after the collection is created)
    Controls.Dialog {
        id: firstRunReadyDialog
        title: "Katalog"
        modal: true
        anchors.centerIn: parent
        width: Math.min(520, parent.width - Kirigami.Units.largeSpacing * 4)
        contentItem: Controls.Label {
            width: firstRunReadyDialog.availableWidth
            wrapMode: Text.WordWrap
            textFormat: Text.RichText
            text: qsTr("<br/><b>Ready to create a file catalog:</b><br/><br/>")
                  + qsTr("1- Select an entire drive or directory, <br/>2- select options, and <br/>3- click 'Create'<br/>")
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("OK")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
            onAccepted: {
                firstRunReadyDialog.close()
                appManager1.setLastPage("Create")
                root.showPage(pageCreate)
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
                } else if (pageDeviceEdit.fromDevicesPage) {
                    pageDeviceEdit.fromDevicesPage = false
                    root.showPage(pageDevices)
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
                text: (pageDeviceEdit_form.deviceType === "Catalog"
                       ? qsTr("The catalog source path changed.")
                       : qsTr("The storage path changed."))
                      + "\n\nOld path: " + editStoragePathDialog.previousPath
                      + "\nNew path: " + editStoragePathDialog.newPath
                      + "\n\n" + qsTr("How should the catalog indexes be updated?")
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

    // Device page snapshot dialog
    Controls.Dialog {
        id: devSnapshotDialog
        property var data: null
        title: "Katalog"
        modal: true
        anchors.centerIn: parent
        width: Math.min(500, root.width - Kirigami.Units.largeSpacing * 4)
        contentItem: Controls.Label {
            textFormat: Text.RichText
            wrapMode: Text.WordWrap
            text: {
                if (!devSnapshotDialog.data) return ""
                var d = devSnapshotDialog.data
                var fc    = (d.newCatalogFileCount   || 0)
                var dfc   = (d.deltaCatalogFileCount || 0)
                var fs    = appManager1.formatDataSize(d.newCatalogFileSize   || 0)
                var dfs   = appManager1.formatDataSizeDelta(Math.abs(d.deltaCatalogFileSize  || 0))
                var spc   = appManager1.formatDataSize(d.newStorageFreeSpace  || 0)
                var dspc  = appManager1.formatDataSizeDelta(Math.abs(d.deltaStorageFree      || 0))
                var stot  = appManager1.formatDataSize(d.newStorageTotalSpace || 0)
                var dstot = appManager1.formatDataSizeDelta(Math.abs(d.deltaStorageTotal     || 0))
                return "<br/>" + qsTr("A snapshot of this collection was recorded:") +
                    "<table>" +
                    "<tr><td><br/><b>" + qsTr("Catalogs") + "</b></td><td></td><td></td></tr>" +
                    "<tr><td>" + qsTr("Number of files:") + " </td><td style='text-align:right;'><b> " + Number(fc).toLocaleString(Qt.locale(), "f", 0) + " </b></td><td>  (" + qsTr("added:") + " <b> " + ((dfc >= 0) ? "+" : "") + Number(dfc).toLocaleString(Qt.locale(), "f", 0) + " </b>)</td></tr>" +
                    "<tr><td>" + qsTr("Total file size:") + " </td><td style='text-align:right;'><b> " + fs + " </b></td><td>  (" + qsTr("added:") + " <b> " + ((d.deltaCatalogFileSize >= 0) ? "+" : "") + dfs + " </b>)</td></tr>" +
                    "<tr><td><br/><b>" + qsTr("Storage") + "</b></td><td></td><td></td></tr>" +
                    "<tr><td>" + qsTr("Storage free space:") + " </td><td style='text-align:right;'><b> " + spc + " </b></td><td>  (" + qsTr("added:") + " <b> " + ((d.deltaStorageFree >= 0) ? "+" : "") + dspc + " </b>)</td></tr>" +
                    "<tr><td>" + qsTr("Storage total space:") + " </td><td style='text-align:right;'><b> " + stot + " </b></td><td>  (" + qsTr("added:") + " <b> " + ((d.deltaStorageTotal >= 0) ? "+" : "") + dstot + " </b>)</td></tr>" +
                    "</table>"
            }
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("OK")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
            onAccepted: devSnapshotDialog.close()
        }
    }

    // Update All Active — pre-confirmation dialog
    Controls.Dialog {
        id: devUpdateAllDialog
        title: "Katalog"
        modal: true
        anchors.centerIn: parent
        width: Math.min(440, root.width - Kirigami.Units.largeSpacing * 4)
        contentItem: Controls.Label {
            wrapMode: Text.WordWrap
            text: qsTr("Do you want a the summary of updates for each catalog?")
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("Yes")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.YesRole
                onClicked: { devUpdateAllDialog.close(); appManager1.updateAllActiveDevices(true) }
            }
            Controls.Button {
                text: qsTr("No")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.NoRole
                onClicked: { devUpdateAllDialog.close(); appManager1.updateAllActiveDevices(false) }
            }
            Controls.Button {
                text: qsTr("Cancel")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.RejectRole
                onClicked: devUpdateAllDialog.close()
            }
        }
    }

    // Device update result dialog — shown after single update or each catalog in "All active" with Yes
    Controls.Dialog {
        id: devUpdateReportDialog
        property var report: ({})
        title: "Katalog"
        modal: true
        anchors.centerIn: parent
        width: Math.min(460, root.width - Kirigami.Units.largeSpacing * 4)
        contentItem: Controls.Label {
            wrapMode: Text.WordWrap
            textFormat: Text.RichText
            text: {
                var r = devUpdateReportDialog.report
                if (!r || !r.deviceType) return ""
                var msg = ""

                if (r.deviceType === "Catalog") {
                    msg += "<table>"
                    msg += "<tr><td>" + qsTr("Catalog updated: ") + "</td><td align='center'><b>" + r.deviceName + "</b></td></tr>"
                    msg += "<tr><td>" + qsTr("Path: ")            + "</td><td><b>" + r.devicePath + "</b></td></tr>"
                    msg += "</table><br/><table>"
                    msg += "<tr><td>" + qsTr("Number of files: ") + "</td><td align='right'><b>" + Number(r.fileCount || 0).toLocaleString(Qt.locale(), "f", 0) + "</b></td>"
                    msg += "<td>&nbsp;&nbsp;" + qsTr("(added: ") + "</td><td align='right'><b>" + Number(r.filesAdded || 0).toLocaleString(Qt.locale(), "f", 0) + "</b>)</td></tr>"
                    msg += "<tr><td>" + qsTr("Total file size: ") + "</td><td align='right'><b>" + appManager1.formatDataSize(r.totalSize || 0) + "</b></td>"
                    msg += "<td>&nbsp;&nbsp;" + qsTr("(added: ") + "</td><td align='right'><b>" + appManager1.formatDataSizeDelta(r.sizeAdded || 0) + "</b>)</td></tr>"
                    msg += "</table>"
                    if (r.storageUpdated) {
                        msg += "<br/><table>"
                        msg += "<tr><td>" + qsTr("Storage updated: ") + "</td><td align='center'><b>" + (r.storageName || "") + "</b></td></tr>"
                        msg += "<tr><td>" + qsTr("Path: ")            + "</td><td><b>" + (r.storagePath || "") + "</b></td></tr>"
                        msg += "</table><br/><table>"
                        msg += "<tr><td>" + qsTr("Used Space: ")  + "</td><td align='right'><b>" + appManager1.formatDataSize(r.storageUsed  || 0) + "</b></td>"
                        msg += "<td>&nbsp;&nbsp;" + qsTr("(added: ") + "</td><td align='right'><b>" + appManager1.formatDataSizeDelta(r.storageUsedAdded  || 0) + "</b>)</td></tr>"
                        msg += "<tr><td>" + qsTr("Free Space: ")  + "</td><td align='right'><b>" + appManager1.formatDataSize(r.storageFree  || 0) + "</b></td>"
                        msg += "<td>&nbsp;&nbsp;" + qsTr("(added: ") + "</td><td align='right'><b>" + appManager1.formatDataSizeDelta(r.storageFreeAdded  || 0) + "</b>)</td></tr>"
                        msg += "<tr><td>" + qsTr("Total Space: ") + "</td><td align='right'><b>" + appManager1.formatDataSize(r.storageTotal || 0) + "</b></td>"
                        msg += "<td>&nbsp;&nbsp;" + qsTr("(added: ") + "</td><td align='right'><b>" + appManager1.formatDataSizeDelta(r.storageTotalAdded || 0) + "</b>)</td></tr>"
                        msg += "</table>"
                    }

                } else if (r.deviceType === "Storage") {
                    msg += "<table>"
                    msg += "<tr><td>" + qsTr("Storage updated: ") + "</td><td align='center'><b>" + r.deviceName + "</b></td></tr>"
                    msg += "<tr><td>" + qsTr("Path: ")            + "</td><td><b>" + r.devicePath + "</b></td></tr>"
                    msg += "</table><br/><table>"
                    msg += "<tr><td>" + qsTr("Used Space: ")  + "</td><td align='right'><b>" + appManager1.formatDataSize(r.storageUsed  || 0) + "</b></td>"
                    msg += "<td>&nbsp;&nbsp;" + qsTr("(added: ") + "</td><td align='right'><b>" + appManager1.formatDataSizeDelta(r.storageUsedAdded  || 0) + "</b>)</td></tr>"
                    msg += "<tr><td>" + qsTr("Free Space: ")  + "</td><td align='right'><b>" + appManager1.formatDataSize(r.storageFree  || 0) + "</b></td>"
                    msg += "<td>&nbsp;&nbsp;" + qsTr("(added: ") + "</td><td align='right'><b>" + appManager1.formatDataSizeDelta(r.storageFreeAdded  || 0) + "</b>)</td></tr>"
                    msg += "<tr><td>" + qsTr("Total Space: ") + "</td><td align='right'><b>" + appManager1.formatDataSize(r.storageTotal || 0) + "</b></td>"
                    msg += "<td>&nbsp;&nbsp;" + qsTr("(added: ") + "</td><td align='right'><b>" + appManager1.formatDataSizeDelta(r.storageTotalAdded || 0) + "</b>)</td></tr>"
                    msg += "</table>"

                } else if (r.deviceType === "Virtual") {
                    msg += "<table>"
                    msg += "<tr><td>" + qsTr("Virtual device updated: ") + "</td><td align='center'><b>" + r.deviceName + "</b></td></tr>"
                    msg += "</table><br/><table>"
                    msg += "<tr><td>" + qsTr("Number of files: ") + "</td><td align='right'><b>" + Number(r.fileCount || 0).toLocaleString(Qt.locale(), "f", 0) + "</b></td>"
                    msg += "<td>&nbsp;&nbsp;" + qsTr("(added: ") + "</td><td align='right'><b>" + Number(r.filesAdded || 0).toLocaleString(Qt.locale(), "f", 0) + "</b>)</td></tr>"
                    msg += "<tr><td>" + qsTr("Total file size: ") + "</td><td align='right'><b>" + appManager1.formatDataSize(r.totalSize || 0) + "</b></td>"
                    msg += "<td>&nbsp;&nbsp;" + qsTr("(added: ") + "</td><td align='right'><b>" + appManager1.formatDataSizeDelta(r.sizeAdded || 0) + "</b>)</td></tr>"
                    msg += "</table><br/>"
                    msg += qsTr("Catalogs updated:") + " <b>" + (r.catalogsUpdated || 0) + "</b> (" + (r.catalogsSkipped || 0) + " " + qsTr("skipped") + ")<br/>"
                    if (r.storageUpdated) {
                        msg += "<br/><table>"
                        msg += "<tr><td>" + qsTr("Storage") + "</td></tr>"
                        msg += "<tr><td>" + qsTr("Used Space: ")  + "</td><td align='right'><b>" + appManager1.formatDataSize(r.storageUsed  || 0) + "</b></td>"
                        msg += "<td>&nbsp;&nbsp;" + qsTr("(added: ") + "</td><td align='right'><b>" + appManager1.formatDataSizeDelta(r.storageUsedAdded  || 0) + "</b>)</td></tr>"
                        msg += "<tr><td>" + qsTr("Free Space: ")  + "</td><td align='right'><b>" + appManager1.formatDataSize(r.storageFree  || 0) + "</b></td>"
                        msg += "<td>&nbsp;&nbsp;" + qsTr("(added: ") + "</td><td align='right'><b>" + appManager1.formatDataSizeDelta(r.storageFreeAdded  || 0) + "</b>)</td></tr>"
                        msg += "<tr><td>" + qsTr("Total Space: ") + "</td><td align='right'><b>" + appManager1.formatDataSize(r.storageTotal || 0) + "</b></td>"
                        msg += "<td>&nbsp;&nbsp;" + qsTr("(added: ") + "</td><td align='right'><b>" + appManager1.formatDataSizeDelta(r.storageTotalAdded || 0) + "</b>)</td></tr>"
                        msg += "</table>"
                    }

                } else if (r.deviceType === "list") {
                    msg += "<table><br/>"
                    msg += qsTr("Selected active catalogs are updated.") + "&nbsp;<br/>"
                    msg += "<tr><td>" + qsTr("Number of files: ") + "</td><td align='right'><b>" + Number(r.fileCount || 0).toLocaleString(Qt.locale(), "f", 0) + "</b></td>"
                    msg += "<td>&nbsp;&nbsp;" + qsTr("(added: ") + "</td><td align='right'><b>" + Number(r.filesAdded || 0).toLocaleString(Qt.locale(), "f", 0) + "</b>)</td></tr>"
                    msg += "<tr><td>" + qsTr("Total file size: ") + "</td><td align='right'><b>" + appManager1.formatDataSize(r.totalSize || 0) + "</b></td>"
                    msg += "<td>&nbsp;&nbsp;" + qsTr("(added: ") + "</td><td align='right'><b>" + appManager1.formatDataSizeDelta(r.sizeAdded || 0) + "</b>)</td></tr>"
                    msg += "</table><br/>"
                    msg += qsTr("Catalogs updated:") + " <b>" + (r.catalogsUpdated || 0) + "</b> (" + (r.catalogsSkipped || 0) + " " + qsTr("skipped") + ")<br/>"
                    if (r.storageUpdated) {
                        msg += "<br/><table>"
                        msg += "<tr><td>" + qsTr("Used Space: ")  + "</td><td align='right'><b>" + appManager1.formatDataSize(r.storageUsed  || 0) + "</b></td>"
                        msg += "<td>&nbsp;&nbsp;" + qsTr("(added: ") + "</td><td align='right'><b>" + appManager1.formatDataSizeDelta(r.storageUsedAdded  || 0) + "</b>)</td></tr>"
                        msg += "<tr><td>" + qsTr("Free Space: ")  + "</td><td align='right'><b>" + appManager1.formatDataSize(r.storageFree  || 0) + "</b></td>"
                        msg += "<td>&nbsp;&nbsp;" + qsTr("(added: ") + "</td><td align='right'><b>" + appManager1.formatDataSizeDelta(r.storageFreeAdded  || 0) + "</b>)</td></tr>"
                        msg += "<tr><td>" + qsTr("Total Space: ") + "</td><td align='right'><b>" + appManager1.formatDataSize(r.storageTotal || 0) + "</b></td>"
                        msg += "<td>&nbsp;&nbsp;" + qsTr("(added: ") + "</td><td align='right'><b>" + appManager1.formatDataSizeDelta(r.storageTotalAdded || 0) + "</b>)</td></tr>"
                        msg += "</table>"
                    }
                }
                return msg
            }
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("OK")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
            onAccepted: {
                devUpdateReportDialog.close()
                appManager1.acknowledgeUpdateReport()
            }
        }
    }

    Connections {
        target: appManager1
        function onDeviceUpdateReportReady(report) {
            devUpdateReportDialog.report = report
            devUpdateReportDialog.open()
        }
    }

    // Create empty-source confirmation (Yes/No decision → modal)
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
                    text: qsTr("Close")
                    icon.name: "view-close"
                    onTriggered: pageStack.layers.pop()
                }
            ]
        }
    }

    //Pages - Selection
    Kirigami.ScrollablePage {
        id: pageSelection
        title: qsTr("Selection")
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
                    Controls.ToolTip.text: qsTr("Show device info")
                    Controls.ToolTip.visible: hovered
                }
                Controls.ToolButton {
                    icon.name: "go-up"
                    icon.width:  Kirigami.Units.iconSizes.small
                    icon.height: Kirigami.Units.iconSizes.small
                    implicitWidth:  root.selectionButton
                    implicitHeight: root.selectionButton
                    padding: 0
                    enabled: appManager1.canCollapseDevices
                    onClicked: appManager1.collapseDevices()
                    Controls.ToolTip.text: qsTr("Collapse one level")
                    Controls.ToolTip.visible: hovered
                }
                Controls.ToolButton {
                    icon.name: "go-down"
                    icon.width:  Kirigami.Units.iconSizes.small
                    icon.height: Kirigami.Units.iconSizes.small
                    implicitWidth:  root.selectionButton
                    implicitHeight: root.selectionButton
                    padding: 0
                    enabled: appManager1.canExpandDevices
                    onClicked: appManager1.expandDevices()
                    Controls.ToolTip.text: qsTr("Expand one level")
                    Controls.ToolTip.visible: hovered
                }
            }
        }

        Kirigami.CardsListView {
            id: selectionListView1
            model: appManager1.deviceFilterModel
            delegate: PageSelectionDelegate {}
            topMargin: Kirigami.Units.smallSpacing
            spacing: Kirigami.Units.largeSpacing
        }
    }

    //Pages - Search
    Kirigami.ScrollablePage {
        id: pageSearch
        title: qsTr("Search")

        // Run the search and show the results page. Shared by the Search action button
        // and the Enter key in the search text field so both behave identically.
        function runSearch() {
            if (appManager1.searchIsRunning)
                return
            root.searchTriggered()
            pageSearchForm.executeSearch()
            // If the results page is already the top of the stack, do NOT remove and
            // re-push it: removing and immediately re-pushing the same static page object
            // in one event loop tick corrupts the QML component context and crashes.
            // The onSearchTriggered Connections in PageSearchResultsForm resets the model.
            let resultsAtTop = pageStack.depth > 0
                               && pageStack.get(pageStack.depth - 1) === pageSearchResults
            if (resultsAtTop) {
                pageStack.currentIndex = pageStack.depth - 1
            } else {
                // Remove any stale pages beyond Search, then push Results fresh.
                let searchIdx = indexOfPage(pageSearch)
                while (pageStack.depth - 1 > searchIdx) {
                    let p = pageStack.get(pageStack.depth - 1)
                    pageStack.removePage(p)
                    p.visible = false
                }
                // Ensure currentIndex points to Search before pushing.
                // Kirigami's push() truncates pages forward of currentIndex, so if
                // the user back-navigated without removing pages, push() would drop Search.
                pageStack.currentIndex = searchIdx
                pageSearchResults.visible = true
                pageStack.push(pageSearchResults)
            }
        }

        actions: [
            Kirigami.Action {
                text:        qsTr("Search")
                icon.name:   "edit-find"
                displayHint: Kirigami.DisplayHint.KeepVisible
                enabled:     !appManager1.searchIsRunning
                onTriggered: pageSearch.runSearch()
            },
            Kirigami.Action {
                text:        appManager1.searchIsPaused ? qsTr("Resume") : qsTr("Pause")
                icon.name:   appManager1.searchIsPaused ? "media-playback-start" : "media-playback-pause"
                displayHint: Kirigami.DisplayHint.KeepVisible
                enabled:     appManager1.searchIsRunning
                onTriggered: appManager1.searchIsPaused ? appManager1.resumeSearch() : appManager1.pauseSearch()
            },
            Kirigami.Action {
                text:        qsTr("Stop")
                icon.name:   "process-stop"
                displayHint: Kirigami.DisplayHint.KeepVisible
                enabled:     appManager1.searchIsRunning
                onTriggered: appManager1.stopSearch()
            },
            Kirigami.Action {
                text:        qsTr("Reset")
                icon.name:   "edit-clear-history"
                displayHint: Kirigami.DisplayHint.KeepVisible
                enabled:     !appManager1.searchIsRunning
                onTriggered: pageSearchForm.resetSearch()
            },
            Kirigami.Action {
                text:        qsTr("History")
                icon.name:   "view-history"
                displayHint: Kirigami.DisplayHint.KeepVisible
                enabled:     !appManager1.searchIsRunning
                onTriggered: pageSearchForm.openHistorySheet()
            },
            Kirigami.Action {
                text:        qsTr("Close")
                icon.name:   "view-close"
                displayHint: Kirigami.DisplayHint.KeepVisible
                onTriggered: {
                    pageStack.removePage(pageSearch)
                    pageSearch.visible = false
                    // Results stays in the stack if present; navigate to it or Selection
                    root.recomputeFeatureOpen()
                    root.syncSelectionVisibility()
                    pageStack.currentIndex = Math.max(0, pageStack.depth - 1)
                }
            }
        ]

        footer: RowLayout {
            visible: appManager1.searchIsRunning || appManager1.searchStatusText.length > 0
            spacing: Kirigami.Units.smallSpacing
            Controls.BusyIndicator {
                running: appManager1.searchIsRunning && !appManager1.searchIsPaused
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
            onSearchRequested: pageSearch.runSearch()
        }
    }

    //Pages - SearchResults
    Kirigami.Page {
        id: pageSearchResults
        visible: false
        title: {
            let n = Number(newSearch1.properties.filesFoundNumber ?? 0).toLocaleString(Qt.locale(), "f", 0)
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
                    root.recomputeFeatureOpen()
                    root.syncSelectionVisibility()
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
                root.recomputeFeatureOpen()
                root.syncSelectionVisibility()
                pageStack.currentIndex = Math.max(0, pageStack.depth - 1)
            }
        }
    }

    //Pages - Devices
    Kirigami.Page {
        id: pageDevices
        visible: false
        padding: 0
        title: qsTr("Devices")
        actions: [
            Kirigami.Action {
                text:        qsTr("All active")
                icon.name:   "media-playlist-repeat"
                displayHint: Kirigami.DisplayHint.KeepVisible
                // Always enabled: requests are queued while an operation runs
                // (SpecOperationQueue.md).
                onTriggered: devUpdateAllDialog.open()
            },
            Kirigami.Action {
                text:        qsTr("Stop")
                icon.name:   "process-stop"
                displayHint: Kirigami.DisplayHint.KeepVisible
                enabled:     appManager1.deviceUpdateIsRunning
                onTriggered: appManager1.stopDeviceUpdate()
            },
            Kirigami.Action {
                text: qsTr("Snapshot")
                icon.name: "camera-photo"
                enabled: !appManager1.deviceUpdateIsRunning
                onTriggered: {
                    var result = appManager1.recordDevicesSnapshot()
                    devSnapshotDialog.data = result
                    devSnapshotDialog.open()
                }
            },
            Kirigami.Action {
                text: qsTr("Insert Virtual Group")
                icon.name: "folder-new"
                enabled: !appManager1.deviceUpdateIsRunning
                onTriggered: {
                    var newId = appManager1.addDeviceVirtual(0)
                    if (newId > 0) {
                        pageDeviceEdit.fromDevicesPage = true
                        pageDeviceEdit_form.deviceId = newId
                        pageDeviceEdit_form.loadDevice()
                        root.showPage(pageDeviceEdit)
                    }
                }
            },
            Kirigami.Action {
                text: qsTr("Add Storage")
                icon.name: "drive-harddisk"
                enabled: !appManager1.deviceUpdateIsRunning
                onTriggered: {
                    var newId = appManager1.addDeviceStorage(0)
                    if (newId > 0) {
                        pageDeviceEdit.fromDevicesPage = true
                        pageDeviceEdit_form.deviceId = newId
                        pageDeviceEdit_form.loadDevice()
                        root.showPage(pageDeviceEdit)
                    }
                }
            },
            // Kirigami.Action {
            //     text:        qsTr("Gentle stop")
            //     icon.name:   "media-playback-stop"
            //     displayHint: Kirigami.DisplayHint.KeepVisible
            //     enabled:     appManager1.deviceUpdateIsRunning
            //     onTriggered: appManager1.gentleStopDeviceUpdate()
            // },
            Kirigami.Action {
                text: qsTr("Close")
                icon.name: "view-close"
                onTriggered: root.closeFeaturePage(pageDevices)
            }
        ]

        // Update progress is reported by the global activity panel, next to
        // the page area — see OperationQueueView.qml.

        PageDevicesView {
            id: pageDevicesView
            cardScale: root.cardScale
            onEditDeviceRequested: (deviceId) => {
                pageDeviceEdit.fromDevicesPage = true
                pageDeviceEdit_form.deviceId = deviceId
                pageDeviceEdit_form.loadDevice()
                root.showPage(pageDeviceEdit)
            }
            onExploreDeviceRequested: (deviceId) => {
                appManager1.setLastPage("Explore")
                exploreFolders.openByDeviceId(deviceId)
                root.showPage(pageExplore)
            }
        }
    }

    //Pages - Explore
    Kirigami.Page {
        id: pageExplore
        visible: false
        title: qsTr("Explore")
        padding: 0

        actions: [
            Kirigami.Action {
                text: qsTr("Close")
                icon.name: "view-close"
                onTriggered: root.closeFeaturePage(pageExplore)
            }
        ]

        RowLayout {
            anchors.fill: parent
            spacing: 0

            PageExploreFolders {
                id: exploreFolders
                Layout.preferredWidth: 280
                Layout.minimumWidth:   180
                Layout.fillHeight: true

                onFolderClicked: function(folderPath) {
                    exploreFiles.currentFolderPath = folderPath
                }
                onCatalogOpenRequested: function(info) {
                    exploreFiles.catalogName  = info.name  ?? ""
                    exploreFiles.catalogPath  = info.path  ?? ""
                    exploreFiles.catalogId    = info.externalId ?? 0
                }
            }

            Kirigami.Separator { Layout.fillHeight: true }

            PageExploreFiles {
                id: exploreFiles
                Layout.fillWidth: true
                Layout.fillHeight: true

                onFolderNavigated: function(folderPath) {
                    exploreFolders.selectedFolderPath = folderPath
                    exploreFiles.currentFolderPath    = folderPath
                }
            }
        }
    }

    //Pages - Create
    Kirigami.ScrollablePage {
        id: pageCreate
        visible: false
        title: qsTr("Create")
        Connections {
            target: appManager1
            function onCatalogCreationCompleted(success, report) {
                if (pageCreate.visible) {
                    // On success `report` is non-empty only when the catalog was
                    // kept with an unfinished metadata/checksum scan; it already
                    // explains what happened, so it replaces the generic text.
                    showPassiveNotification(
                        success ? (report.length > 0 ? report : qsTr("Catalog created successfully."))
                                : qsTr("Catalog creation failed: ") + report,
                        (success && report.length === 0) ? "short" : "long"
                    )
                    if (success)
                        appManager1.refreshDeviceList()
                }
            }
        }

        Connections {
            target: pageCreate_formLayout_Create
            function onEmptyDirConfirmNeeded() {
                createEmptyDirDialog.open()
            }
        }

        actions: [
            Kirigami.Action {
                text: qsTr("Create")
                icon.name: "document-save"
                // Always enabled: a creation requested while another operation
                // runs is queued rather than refused (SpecOperationQueue.md).
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

        // Creation progress is reported by the global activity panel in the
        // window footer, alongside the queue — see OperationQueueView.qml.

        PageCreateForm {
            id: pageCreate_formLayout_Create
        }
    }

    //Pages - Device Edit
    Kirigami.ScrollablePage {
        id: pageDeviceEdit
        visible: false
        title: qsTr("Edit Device")
        property bool fromDevicesPage: false
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
                if (pageDeviceEdit.fromDevicesPage) {
                    pageDeviceEdit.fromDevicesPage = false
                    root.showPage(pageDevices)
                } else {
                    root.closeFeaturePage(pageDeviceEdit)
                }
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
                text: qsTr("Cancel")
                icon.name: "view-close"
                onTriggered: {
                    if (pageDeviceEdit.fromDevicesPage) {
                        pageDeviceEdit.fromDevicesPage = false
                        root.showPage(pageDevices)
                    } else {
                        root.closeFeaturePage(pageDeviceEdit)
                    }
                }
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
        title: qsTr("Statistics")
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
        title: qsTr("Tags")
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
        title: qsTr("Backup")

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
                text:      qsTr("LuckyBackup")
                tooltip: qsTr("Generate LuckyBackup profile")
                icon.name: "document-save"
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

        // Preparation progress shown on the Backup page while "Update catalogs" runs and/or
        // while the preview compare runs (BKP-F14/BKP-F16) — same StatusBarMessageBuilder
        // messages as the other screens. The compare can be cancelled from here.
        footer: RowLayout {
            id: backupPrepFooter
            readonly property bool   prepRunning: appManager1.catalogUpdateForBackupRunning
                                                  || appManager1.backupPreviewRunning
            readonly property string prepStatus:  appManager1.backupPreviewRunning
                                                  ? appManager1.backupPreviewStatusText
                                                  : appManager1.catalogUpdateForBackupStatusText
            visible: prepRunning || prepStatus.length > 0
            spacing: Kirigami.Units.smallSpacing
            Controls.BusyIndicator {
                running: backupPrepFooter.prepRunning
                visible: backupPrepFooter.prepRunning
                implicitWidth:  Kirigami.Units.gridUnit * 1.5
                implicitHeight: Kirigami.Units.gridUnit * 1.5
                Layout.leftMargin: Kirigami.Units.smallSpacing
            }
            Controls.Label {
                Layout.fillWidth: true
                Layout.margins: Kirigami.Units.smallSpacing
                text: backupPrepFooter.prepStatus
                textFormat: Text.StyledText
                elide: Text.ElideRight
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.85
            }
            Controls.Button {
                visible: appManager1.backupPreviewRunning
                text:      qsTr("Cancel")
                icon.name: "process-stop"
                Layout.rightMargin: Kirigami.Units.smallSpacing
                onClicked: appManager1.stopBackupPreview()
            }
        }

        PageBackupForm {
            id: backupPageForm
            cardScale: root.cardScale
            onRequestAddMapping:        root.showLayer(backupMappingFormComponent)
            // The whole preview flow (optional catalog update → cancellable compare) runs in
            // AppManager on the Backup page; the footer shows progress. The Preview page opens
            // only when the report is ready (BKP-F14/BKP-F16).
            onRequestPreviewMapping: (mappingId) => appManager1.startBackupPreview(mappingId)
            onRequestEditMapping: (mappingData) => root.showLayer(backupMappingFormComponent, { editMappingId: mappingData.mappingId, mappingData: mappingData })
        }

        // Preview compare finished — open the Preview page now that the report is ready.
        // Not fired for a cancelled or failed preview.
        Connections {
            target: appManager1
            function onBackupPreviewReady(mappingId, success, cancelled) {
                if (success)
                    root.showLayer(backupPreviewFormComponent, { mappingId: mappingId })
            }
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
        title: qsTr("Select Collection Folder")
        onAccepted: {
            var path = appManager1.pathFromFileUrl(selectedFolder.toString())
            appManager1.openCollectionMemory(path)
        }
    }

    Dialogs.FileDialog {
        id: databaseFileDialog
        title: qsTr("Open SQLite Database")
        nameFilters: ["SQLite databases (*.db *.sqlite *.sqlite3)", "All files (*)"]
        onAccepted: {
            var path = appManager1.pathFromFileUrl(selectedFile.toString())
            appManager1.setDatabaseFilePath(path)
        }
    }

    Dialogs.FileDialog {
        id: newDatabaseFileDialog
        title: qsTr("Select the database to create and open:")
        fileMode: Dialogs.FileDialog.SaveFile
        nameFilters: ["SQLite databases (*.db)", "All files (*)"]
        onAccepted: {
            var path = appManager1.pathFromFileUrl(selectedFile.toString())
            appManager1.createNewSQLiteCollection(path)
        }
    }
}
