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
        property int  savedWidth:    1080
        property int  savedHeight:   720
        property bool drawerPinned:  false
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
                icon.name: "document-open-data"
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
                    icon.name: "server-database"
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
                    icon.name: "network-server"
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
                icon.name: "select"
                text: "Selection"
                onTriggered: { appManager1.setLastPage("Selection"); root.showPage(pageSelection) }
            },
            Kirigami.Action {
                icon.name: "search"
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
                icon.name: "lastfm-tag"
                text: "Tags"
                onTriggered: { appManager1.setLastPage("Tags"); root.showPage(pageTags) }
            },
            Kirigami.Action {
                icon.name: "backup"
                text: "Backup"
                onTriggered: { appManager1.setLastPage("Backup"); root.showPage(pageBackup) }
            },
            Kirigami.Action {
                icon.name: "settings-configure"
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
                icon.name: "application-exit-symbolic"
                shortcut: StandardKey.Quit
                onTriggered: Qt.quit()
            }
        ]

        footer: Column {
            spacing: Kirigami.Units.smallSpacing
            padding: Kirigami.Units.largeSpacing

            Row {
                width: parent.width - Kirigami.Units.largeSpacing * 2
                spacing: Kirigami.Units.smallSpacing

                Controls.ToolButton {
                    icon.name: windowSettings.drawerPinned ? "window-unpin" : "pin"
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

                Kirigami.Icon {
                    source: "zoom-out"
                    width: Kirigami.Units.iconSizes.small
                    height: Kirigami.Units.iconSizes.small
                    anchors.verticalCenter: parent.verticalCenter
                }

                Controls.Slider {
                    id: cardSizeSlider
                    from: 0.7
                    to: 1.5
                    value: 1.0
                    stepSize: 0.1
                    width: parent.width - 60
                    onValueChanged: root.cardScale = value
                }

                Kirigami.Icon {
                    source: "zoom-in"
                    width: Kirigami.Units.iconSizes.small
                    height: Kirigami.Units.iconSizes.small
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
    }

    // When searchKeepsSelection is on, prevent Kirigami from scrolling
    // away from col 0 (Selection) while Search+Results are open.
    Connections {
        target: pageStack
        function onCurrentIndexChanged() {
            if (appManager1.searchKeepsSelection
                    && pageStack.depth >= 3
                    && pageStack.currentIndex !== 0) {
                pageStack.currentIndex = 0
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
    pageStack.initialPage: [ pageSelection, pageSearch ]

    Component.onCompleted: {
        width  = windowSettings.savedWidth
        height = windowSettings.savedHeight
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
        title: "Katalog 3 - Alpha1 Version"
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
                icon.name: "search"
                onTriggered: {
                    root.searchTriggered()
                    pageSearchForm.executeSearch()
                    // Remove any stale col-2+ pages (old Results)
                    while (pageStack.depth > 2) {
                        let p = pageStack.get(pageStack.depth - 1)
                        pageStack.removePage(p)
                        p.visible = false
                    }
                    // Use insertPage (not push) to append Results at the end without
                    // Kirigami auto-changing currentIndex asynchronously via its animation.
                    pageSearchResults.visible = true
                    pageStack.insertPage(pageStack.depth, pageSearchResults)
                    // Anchor view: keepSelection=0 (wide screens see Selection+Search+Results);
                    // default=depth-1 (2-col screens see Search+Results).
                    pageStack.currentIndex = appManager1.searchKeepsSelection ? 0 : (pageStack.depth - 1)
                }
            },
            Kirigami.Action {
                text: qsTr("Reset")
                icon.name: "edit-clear-all"
                onTriggered: pageSearchForm.resetSearch()
            },
            Kirigami.Action {
                text: qsTr("Close")
                icon.name: "view-close"
                onTriggered: root.closeFeaturePage(pageSearch)
            }
        ]

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
                    pageStack.currentIndex = 1  // back to Search
                }
            }
        ]

        PageSearchResultsForm {
            id: pageSearchResultsForm
            anchors.fill: parent
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

        actions: [
            Kirigami.Action {
                text: "Close"
                icon.name: "view-close"
                onTriggered: root.closeFeaturePage(pageCreate)
            }
        ]

        PageCreateForm {
            id: pageCreate_formLayout_Create
        }
    }

    //Pages - Statistics
    Kirigami.ScrollablePage {
        id: pageStatistics
        visible: false
        title: "Statistics"

        actions: [
            /*Kirigami.Action {
                text: "Batch process"
                icon.name: "document-export"
                onTriggered: showPassiveNotification("Batch process clicked, no action")
            },*/
            Kirigami.Action {
                text: "Close"
                icon.name: "view-close"
                onTriggered: root.closeFeaturePage(pageStatistics)
            }
        ]

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            text: "Statistics — coming soon"
            icon.name: "view-statistics"
        }
    }

    //Pages - Tags
    Kirigami.ScrollablePage {
        id: pageTags
        visible: false
        title: "Tags"

        actions: [
            /*Kirigami.Action {
                text: "Batch process"
                icon.name: "document-export"
                onTriggered: showPassiveNotification("Batch process clicked, no action")
            },*/
            Kirigami.Action {
                text: "Close"
                icon.name: "view-close"
                onTriggered: root.closeFeaturePage(pageTags)
            }
        ]

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            text: "Tags — coming soon"
            icon.name: "lastfm-tag"
        }
    }

    //Pages - Backup
    Kirigami.ScrollablePage {
        id: pageBackup
        visible: false
        title: "Backup"

        actions: [
            Kirigami.Action {
                text: "Close"
                icon.name: "view-close"
                onTriggered: root.closeFeaturePage(pageBackup)
            }
        ]

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            text: "Backup — coming soon"
            icon.name: "backup"
        }
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
