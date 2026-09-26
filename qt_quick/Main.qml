import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import QtQuick.Dialogs as Dialogs
import Qt.labs.platform
import QtCore
import QtQml.Models
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
        //Quitting the application does not always close the window first —
        //Cmd+Q on macOS does not — so the geometry is saved here too.
        function onAboutToQuit() {
            root.saveWindowGeometry()
        }
    }

    Settings {
        id: windowSettings
        property bool drawerPinned:   false
        property real savedCardScale: 1.0
    }

    // Window geometry ---------------------------------------------------------
    // Kept in Katalog's own settings file through AppManager, not in the
    // Settings block above: that one writes to the platform-native store, which
    // a portable build has no business using. The tracked values are the
    // windowed ones, so a maximized window comes back maximized and
    // un-maximizes to the size it had before.
    property bool _geometryRestored: false
    property int  _windowedX:      0
    property int  _windowedY:      0
    property int  _windowedWidth:  900
    property int  _windowedHeight: 600

    function trackWindowedGeometry() {
        //Only once the saved geometry has been applied: the sizes the platform
        //reports while the window is still being set up are transient, and
        //recording them used to overwrite the stored value before it was read.
        if (!root._geometryRestored || root.visibility !== Window.Windowed)
            return

        root._windowedX      = root.x
        root._windowedY      = root.y
        root._windowedWidth  = root.width
        root._windowedHeight = root.height
    }

    function saveWindowGeometry() {
        appManager1.saveWindowGeometry(root._windowedX, root._windowedY,
                                       root._windowedWidth, root._windowedHeight,
                                       root.visibility === Window.Maximized)
    }

    signal searchTriggered()
    property real cardScale: 1.0

    // App-wide text size (TYP-F7): scales the application font, which
    // Kirigami.Theme.defaultFont follows, so no QML item multiplies by it again.
    // The window font carries it to the controls too: Qt Quick Controls take
    // their default font per control type from the platform theme, not from
    // the application font, so buttons would not follow otherwise.
    onCardScaleChanged: appManager1.setTextScale(cardScale)

    // A page hidden while the text size changes gets the new font values but
    // keeps drawing with the old ones (style-drawn controls do not repaint while
    // hidden). When such a page is shown again, the size is nudged and restored
    // in the same frame, which makes every item redraw at the current size.
    property var _pageTextScale: new Map()
    function refreshTextIfStale(page) {
        let shownAt = _pageTextScale.get(page)
        _pageTextScale.set(page, cardScale)
        // A page never shown yet counts as stale too: hidden pages are drawn
        // at creation, before the saved or changed size reaches them.
        if (shownAt !== cardScale) {
            appManager1.setTextScale(cardScale + 0.01)
            appManager1.setTextScale(cardScale)
        }
    }

    // The top line of the window (drawer collection name, page title, page
    // toolbar buttons) keeps the system size: the text-size setting does not
    // apply to it. Kirigami builds the page header itself, rebuilds it every
    // time the page is shown (its loader is only active while the page is
    // visible), and creates the toolbar buttons asynchronously afterwards. So
    // the header is pinned each time Kirigami creates it. Items get the system
    // size explicitly (whole-font
    // assignment, replacing Kirigami's own font binding); headings keep
    // Kirigami's level factors.
    Instantiator {
        model: [pageSelection, pageSearch, pageSearchResults, pageDevices, pageExplore,
                pageCreate, pageDeviceEdit, pageStatistics, pageTags, pageBackup]
        delegate: Connections {
            required property var modelData
            target: modelData
            function onGlobalToolBarItemChanged() { root.pinHeaderFont(modelData.globalToolBarItem) }
        }
    }
    Connections {
        target: pageStack.layers.currentItem
        ignoreUnknownSignals: true
        function onGlobalToolBarItemChanged() { root.pinHeaderFont(pageStack.layers.currentItem.globalToolBarItem) }
    }
    // A layer's header is created during the push, before the connection above
    // retargets to it, so it is also pinned once the layer becomes current.
    Connections {
        target: pageStack.layers
        function onCurrentItemChanged() {
            Qt.callLater(function() {
                let layer = pageStack.layers.currentItem
                if (layer && pageStack.layers.depth > 1)
                    root.pinHeaderFont(layer.globalToolBarItem)
            })
        }
    }
    function pinHeaderFont(item) {
        if (!item)
            return
        const base = appManager1.systemTextPointSize
        const headingFactor = {1: 1.35, 2: 1.20, 3: 1.15, 4: 1.10}
        if (item.font !== undefined) {
            let factor = ("level" in item) ? (headingFactor[item.level] || 1.0) : 1.0
            if (item.font.pointSize !== base * factor) {
                let f = item.font
                f.pointSize = base * factor
                item.font = f
            }
        }
        for (let i = 0; i < item.children.length; ++i)
            pinHeaderFont(item.children[i])
    }

    font.pointSize: Kirigami.Theme.defaultFont.pointSize

    // The height the platform's own toolbar takes, measured rather than assumed:
    // Breeze and Fusion pad differently, so any fixed number is right on one
    // platform and wrong on the other. Used by the drawer's collection row so it
    // matches the page toolbar beside it on every platform.
    readonly property real toolBarHeight: toolBarProbe.implicitHeight

    Controls.ToolBar {
        id: toolBarProbe
        visible: false
        enabled: false
        Controls.ToolButton {
            icon.name: "go-up"
            display: Controls.AbstractButton.IconOnly
        }
    }

    // Height of a header strip row. Selection, the Explore directory tree and the
    // Explore file list each have two of them, and they are defined here rather
    // than per page so the three line up when they share the screen.
    readonly property real headerRowHeight: Kirigami.Units.gridUnit * 2

    // The desktop's own background tells us which way round we are.
    //
    // Read inside each colour binding rather than through a shared property. As
    // its own property it was a second hop: changing the Plasma scheme updated
    // highlightColor, which re-ran the colour bindings before this had caught up
    // with the new backgroundColor, so they used the previous light/dark answer
    // and nothing re-triggered them — the band stayed wrong until a restart.
    // Called from a binding, it makes both colours a direct dependency.
    function isDarkDesktop() {
        return Kirigami.Theme.backgroundColor.hslLightness < 0.5
    }

    // The desktop's View background — the surface its own file managers and list
    // views paint on. It needs a probe of its own because everything else here
    // inherits the Window colour set, which is the grey; the View set is the
    // near-white (#fcfcfc under Breeze light) or near-black (#1b1e20 under Breeze
    // dark) one row of a file list is meant to sit on. Taken from the scheme
    // rather than hardcoded to white/black so a scheme that uses neither still
    // gets its own surface (SpecTheme THM-F1, THM-C6).
    readonly property color viewBackgroundColor:
        viewColorProbe.Kirigami.Theme.backgroundColor

    // Must NOT be visible: false. A hidden item gets no resolved palette from
    // the KDE platform theme and Kirigami.Theme.backgroundColor then reads back
    // opaque black, which painted the first row of every file list black on a
    // light desktop. It is a bare 0x0 Item with no children, so it paints
    // nothing regardless. Note this only misbehaves under the real platform
    // theme — with QT_QPA_PLATFORM=offscreen a hidden probe still returns the
    // right colour, so it cannot be caught by an offscreen run.
    Item {
        id: viewColorProbe
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false
    }

    // The two tinted surfaces, defined once for every theme so they always agree.
    // In both cases the logo band is the darker of the two, so it reads as a
    // header above the page.
    // Both surfaces are built from the desktop's ACCENT
    // (Kirigami.Theme.highlightColor), not from its background. The background is
    // a grey, so lightening or darkening it only ever yields more grey; the
    // accent is what carries the desktop's hue — blue under Breeze, green under
    // the openSUSE set — and that is what should show through here.
    //
    // Light desktop: the accent is laid over a near-white background at two
    // strengths, so more accent reads as deeper. Dark desktop: alpha over
    // near-black would make more accent read as *lighter*, so the two are
    // darkened solids of the accent instead, which keeps the band the darker of
    // the pair either way.
    // Theme 2, "Desktop Theme (gray)", takes the same two surfaces from the
    // desktop's BACKGROUND rather than its accent: no hue at all, just a step
    // away from the window colour, for anyone who finds the tinted version too
    // strong.
    // Theme 1, "Katalog Colors", no longer forces the brand blues — the brand
    // palette was retired from K3 and theme 1 renders exactly as theme 0, not as
    // theme 2 (SpecTheme THM-C2). The stored value and its Settings entry are
    // kept, so no string and no translation slot moves (THM-C3).
    readonly property color selectionPageColor:
        appManager1.themeId === 2
        ? (root.isDarkDesktop() ? Qt.lighter(Kirigami.Theme.backgroundColor, 1.25)
                                : Qt.darker(Kirigami.Theme.backgroundColor, 1.06))
        : (root.isDarkDesktop() ? Qt.darker(Kirigami.Theme.highlightColor, 2.6)
                            : Qt.rgba(Kirigami.Theme.highlightColor.r,
                                      Kirigami.Theme.highlightColor.g,
                                      Kirigami.Theme.highlightColor.b, 0.12))

    // Selection highlight. One definition for every list in the application —
    // Selection cards, search results, explore files and folders, device combos —
    // and every theme hands it back to the desktop (SpecTheme THM-C2).
    readonly property color selectionHighlightColor: Kirigami.Theme.highlightColor

    // The two file-list row surfaces, for the Search results list and the Explore
    // file list. Even rows take the desktop's View background — its brightest
    // surface on a light scheme, its darkest on a dark one — and odd rows the
    // Window background beside it, so the pair is two neighbouring steps of the
    // desktop's own scale rather than a tint of its own (SpecTheme THM-F1/F2),
    // the same pair under every stored Theme value (THM-F4). Defined here and
    // consumed by three delegates - the Search results list, the Explore file
    // list and, since THM-F8, the Explore folder list - none of which may
    // re-derive either colour inline (THM-C1 / THM-C11). Until THM-F8 this
    // comment already claimed the folder list as a consumer while the spec
    // covered only the two file lists and the folder list consumed neither:
    // the comment was an intention, not a requirement (THM-C12).
    readonly property color rowBaseColor:   root.viewBackgroundColor
    readonly property color rowStripeColor: Kirigami.Theme.backgroundColor

    // Always at least as dark as a selected card, and always darker than the
    // page beneath it. On a light desktop that is the full accent — the very
    // colour a selected card is filled with; on a dark one the page is already a
    // darkened accent, so the band is darkened further still.
    readonly property color logoBandColor:
        appManager1.themeId === 2
          // Grey: on a dark desktop the page was lightened, so the band stays at
          // the plain background; on a light one it steps further down.
          ? (root.isDarkDesktop() ? Kirigami.Theme.backgroundColor
                                  : Qt.darker(Kirigami.Theme.backgroundColor, 1.18))
          : (root.isDarkDesktop() ? Qt.darker(Kirigami.Theme.highlightColor, 3.8)
                              : Kirigami.Theme.highlightColor)

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
            refreshTextIfStale(pageSelection)
            pageStack.push(pageSelection)
            for (var k = 0; k < pages.length; k++) {
                pages[k].visible = true
                refreshTextIfStale(pages[k])
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
        refreshTextIfStale(page)
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

    // Esc goes back one step (SpecKeyboardShortcuts.md): it triggers the page's
    // own Close/Cancel action (its escapeAction) on the open layer, else on the
    // rightmost page. An open popup keeps its Esc: Qt does not deliver a window
    // shortcut past it (KBS-F4).
    Shortcut {
        sequence: "Esc"
        onActivated: {
            let page = pageStack.layers.depth > 1 ? pageStack.layers.currentItem
                                                  : pageStack.lastItem
            if (page && page.escapeAction && page.escapeAction.enabled)
                page.escapeAction.trigger()
        }
    }

    // Global Drawer
    globalDrawer: Kirigami.GlobalDrawer {
        //isMenu: true
        modal: !windowSettings.drawerPinned
        width: 200

        header: ColumnLayout {
            spacing: 0

            // The collection is the first thing in the drawer. No application logo
            // above it: the window already carries one, and the band's height was
            // styled differently by each platform's controls. Its colour is kept
            // here, so the drawer still opens on the brand surface.
            Rectangle {
                Layout.fillWidth: true
                // Kirigami's own global-toolbar height, which is the number the
                // page toolbar beside this drawer is actually laid out with —
                // authoritative on every platform. The measured probe is only a
                // fallback for the case where that property is unset.
                Layout.preferredHeight: {
                    var h = 0
                    try { h = pageStack.globalToolBar.preferredHeight } catch (e) { h = 0 }
                    return h > 0 ? h : root.toolBarHeight
                }
                color: root.logoBandColor

                RowLayout {
                    anchors {
                        left: parent.left; right: parent.right
                        verticalCenter: parent.verticalCenter
                        leftMargin: Kirigami.Units.largeSpacing
                        rightMargin: Kirigami.Units.largeSpacing
                    }
                    // Both take the colour meant to sit on a filled surface: the
                    // row is painted with the brand band colour, so the ordinary
                    // text colour would be unreadable on it.
                    Kirigami.Icon {
                        source: appManager1.currentCollectionIconName
                        color: Kirigami.Theme.highlightedTextColor
                        isMask: true
                        Layout.preferredWidth:  Kirigami.Units.iconSizes.smallMedium
                        Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                        Layout.alignment: Qt.AlignVCenter
                    }
                    Controls.Label {
                        text: appManager1.currentCollectionDisplayName
                        font.bold: true
                        font.pointSize: appManager1.systemTextPointSize   // top line keeps the system size
                        elide: Text.ElideRight
                        color: Kirigami.Theme.highlightedTextColor
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                    }
                }
            }
        }

        actions: [
            // Kirigami.Action {
            //     separator: true
            // },
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
                Kirigami.Action {
                    visible: appManager1.recentCollections.length > 5
                    icon.name: appManager1.recentCollections.length > 5 ? appManager1.recentCollections[5].iconName : ""
                    text:      appManager1.recentCollections.length > 5 ? appManager1.recentCollections[5].displayName : ""
                    onTriggered: appManager1.openRecentCollection(appManager1.recentCollections[5])
                }
                Kirigami.Action {
                    visible: appManager1.recentCollections.length > 6
                    icon.name: appManager1.recentCollections.length > 6 ? appManager1.recentCollections[6].iconName : ""
                    text:      appManager1.recentCollections.length > 6 ? appManager1.recentCollections[6].displayName : ""
                    onTriggered: appManager1.openRecentCollection(appManager1.recentCollections[6])
                }
                Kirigami.Action {
                    visible: appManager1.recentCollections.length > 7
                    icon.name: appManager1.recentCollections.length > 7 ? appManager1.recentCollections[7].iconName : ""
                    text:      appManager1.recentCollections.length > 7 ? appManager1.recentCollections[7].displayName : ""
                    onTriggered: appManager1.openRecentCollection(appManager1.recentCollections[7])
                }
                Kirigami.Action {
                    visible: appManager1.recentCollections.length > 8
                    icon.name: appManager1.recentCollections.length > 8 ? appManager1.recentCollections[8].iconName : ""
                    text:      appManager1.recentCollections.length > 8 ? appManager1.recentCollections[8].displayName : ""
                    onTriggered: appManager1.openRecentCollection(appManager1.recentCollections[8])
                }
                Kirigami.Action {
                    visible: appManager1.recentCollections.length > 9
                    icon.name: appManager1.recentCollections.length > 9 ? appManager1.recentCollections[9].iconName : ""
                    text:      appManager1.recentCollections.length > 9 ? appManager1.recentCollections[9].displayName : ""
                    onTriggered: appManager1.openRecentCollection(appManager1.recentCollections[9])
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

                IconButton { flat: true;
                    icon.name: windowSettings.drawerPinned ? "window-unpin" : "window-pin"
                    checkable: true
                    checked: windowSettings.drawerPinned
                    onToggled: windowSettings.drawerPinned = checked
                }

                Controls.Label {
                    text: windowSettings.drawerPinned ? qsTr("Drawer pinned") : qsTr("Drawer floating")
                    anchors.verticalCenter: parent.verticalCenter
                    opacity: 0.7
                }
            }

            Row {
                width: parent.width - Kirigami.Units.largeSpacing * 2
                spacing: Kirigami.Units.smallSpacing

                IconButton { flat: true;
                    // Collapse/restore the Selection column beside the open feature
                    // page (K2 ShowHideFilters portage). Disabled when Selection is
                    // the only page — there is nothing to collapse to.
                    icon.name: appManager1.showSelectionPage ? "window-unpin" : "window-pin"
                    checkable: true
                    // Pressed while Selection is shown, matching the drawer button
                    // above: both read "this panel is held open".
                    checked: appManager1.showSelectionPage
                    enabled: root.featureOpen
                    onToggled: appManager1.showSelectionPage = checked
                }

                Controls.Label {
                    text: appManager1.showSelectionPage ? qsTr("Selection shown") : qsTr("Selection hidden")
                    anchors.verticalCenter: parent.verticalCenter
                    opacity: 0.7
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
        //The position is asked for unconditionally: a platform that does not let
        //an application place its own window ignores it and places the window
        //itself. It is only asked for if it still lands on an existing screen.
        var savedGeometry = appManager1.getWindowGeometry()
        width  = savedGeometry.width
        height = savedGeometry.height
        if (savedGeometry.hasPosition) {
            x = savedGeometry.x
            y = savedGeometry.y
        }
        root._windowedX      = root.x
        root._windowedY      = root.y
        root._windowedWidth  = root.width
        root._windowedHeight = root.height
        if (savedGeometry.maximized)
            visibility = Window.Maximized
        root._geometryRestored = true

        // Clamped to the setting's range (TYP-F10): earlier builds allowed 0.7 to 1.3.
        root.cardScale = Math.min(1.2, Math.max(0.8, windowSettings.savedCardScale))

        if (appManager1.isFirstRun) {
            firstRunWelcomeDialog.open()
            return
        }

        if (appManager1.shouldShowAlphaWarning())
            alphaWarningDialog.open()

        pageSearchForm.restoreLastSearch()

        console.warn("PROBE toolBarProbe.implicitHeight =", toolBarProbe.implicitHeight)
        console.warn("PROBE gridUnit =", Kirigami.Units.gridUnit)
        console.warn("PROBE headerRowHeight =", root.headerRowHeight)
        try {
            console.warn("PROBE gtb.preferredHeight =", pageStack.globalToolBar.preferredHeight)
            console.warn("PROBE gtb.minimumHeight   =", pageStack.globalToolBar.minimumHeight)
            console.warn("PROBE gtb.maximumHeight   =", pageStack.globalToolBar.maximumHeight)
        } catch (e) { console.warn("PROBE gtb unavailable:", e) }

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
        // Headers that already exist before the connections above could see them.
        for (let i = 0; i < pageStack.depth; ++i)
            root.pinHeaderFont(pageStack.get(i).globalToolBarItem)
    }

    onXChanged:      root.trackWindowedGeometry()
    onYChanged:      root.trackWindowedGeometry()
    onWidthChanged:  root.trackWindowedGeometry()
    onHeightChanged: root.trackWindowedGeometry()

    //Saved when the window goes away rather than on every resize step.
    onClosing: root.saveWindowGeometry()

    Controls.Dialog {
        id: alphaWarningDialog
        title: "Katalog 3 - Beta2 Version"
        modal: true
        anchors.centerIn: parent
        width: Math.min(520, parent.width - Kirigami.Units.largeSpacing * 4)

        ColumnLayout {
            width: parent.width
            // One blank line between the message and the link. Set here rather
            // than with newlines in the text, so the spacing can be changed
            // without touching a string that is translated into 30 languages.
            spacing: Kirigami.Units.gridUnit

            Controls.Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                // The trailing newlines the source string carries are dropped at
                // display time: the gap is the layout's job now. The argument to
                // qsTr() is untouched, so every existing translation still
                // matches it exactly.
                text: qsTr("This is a beta version of Katalog intended to support development and gather feedback.\n\n")
                          .replace(/\n+$/, "")
            }

            // Where to find news and support for the beta. The visible text is a
            // shortened form of the address, so it is not prose and carries no
            // qsTr(): a URL is the same in every language. The link points at
            // the announcements CATEGORY, not at one post - a category exists
            // before anything is published there and survives posts being
            // renamed or deleted, whereas this address is frozen into every
            // released build and can never be corrected.
            Controls.Label {
                Layout.fillWidth: true
                wrapMode: Text.WrapAnywhere
                textFormat: Text.StyledText
                linkColor: Kirigami.Theme.linkColor
                text: "<a href=\"https://github.com/StephaneCouturier/Katalog/discussions/categories/announcements\">Github: Katalog/discussions/categories/announcements</a>"
                onLinkActivated: (link) => Qt.openUrlExternally(link)
                // And one blank line between the link and the buttons below.
                Layout.bottomMargin: Kirigami.Units.gridUnit

                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
            }
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

        // Set when the user accepts; consumed once the dialog is off screen.
        property int pendingRescanDeviceId: 0
        onClosed: {
            if (pendingRescanDeviceId > 0) {
                var id = pendingRescanDeviceId
                pendingRescanDeviceId = 0
                appManager1.triggerDeviceRescan(id)
            }
        }
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
                // Started from onClosed, not here. The scan blocks the GUI thread
                // for as long as it runs, so anything begun before the dialog has
                // finished closing leaves it frozen on screen. callLater was not
                // enough — one turn does not cover the close animation.
                editCatalogUpdateDialog.pendingRescanDeviceId = pageDeviceEdit_form.deviceId
                editCatalogUpdateDialog.close()
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
                      + "\n\n" + qsTr("Old path:") + " " + editStoragePathDialog.previousPath
                      + "\n" + qsTr("New path:") + " " + editStoragePathDialog.newPath
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
            property Kirigami.Action escapeAction: aboutPageEscapeAction  // Esc (KBS-F1)
            aboutData: About
            actions: [
                // Filing a bug means quoting versions the user would otherwise
                // have to hunt for; one action puts them all on the clipboard.
                // The block itself is assembled in AppManager and deliberately
                // carries no paths, no host, and no credentials — see
                // SpecAbout.md.
                Kirigami.Action {
                    text: qsTr("Copy version and system information")
                    icon.name: "edit-copy"
                    onTriggered: {
                        appManager1.copyToClipboard(appManager1.systemInformation())
                        showPassiveNotification(
                            qsTr("Version and system information copied to clipboard"))
                    }
                },
                Kirigami.Action {
                    id: aboutPageEscapeAction
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

        // Tinted rather than plain: white cards on a white page gave no contrast.
        // Under either theme — Katalog Colors forces the brand blue, Desktop
        // Theme steps away from the desktop's own background. See
        // root.selectionPageColor, which keeps this in step with the logo band.
        background: Rectangle {
            color: root.selectionPageColor
        }
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

        header: ColumnLayout {
            width: parent.width
            spacing: 0

            // A reminder of what is currently selected, at the very top and with
            // no label of its own — the icon and type styling say what it is
            // (SpecSelection.md SEL-F5). Always present, including the All state,
            // so the rows below never shift as the selection changes.
            Controls.ItemDelegate {
                id: selectionReminder
                Layout.fillWidth: true
                Layout.leftMargin:  Kirigami.Units.gridUnit
                Layout.rightMargin: Kirigami.Units.gridUnit
                Layout.preferredHeight: root.headerRowHeight
                topPadding: 0
                bottomPadding: 0

                // The row paints its own surface instead of taking the active Qt
                // Quick Controls style's (THM-F10, the rule THM-F9 set for the
                // Explore lists). Under Fusion — the style every packaged build
                // resolves to — ItemDelegate's background is an opaque #ffffff
                // rectangle that paints unconditionally, which covered the tinted
                // Selection page surface; under org.kde.desktop the same delegate
                // is transparent until hovered, so a development build never
                // showed it. Idle is fully transparent so selectionPageColor shows
                // through; the hover tint stays because the row is clickable
                // (SEL-F6), and comes from Kirigami.Theme, never a literal
                // (THM-C13). Not a list row: no stripe and no highlighted state,
                // it mirrors the selection rather than being selectable.
                background: Rectangle {
                    color: selectionReminder.hovered ? Kirigami.Theme.hoverColor
                                                     : "transparent"
                }

                // Scrolls the list to the selected card — the "where is it?" half
                // of the problem this reminder solves. It deliberately does NOT
                // change or clear the selection: it sits directly above the filter
                // field, and a misclick that widened the search scope would be
                // silent (SEL-F6, SEL-C7).
                onClicked: {
                    var row = appManager1.selectionRowForDevice(appManager1.selectedDeviceId)
                    if (row >= 0)
                        selectionListView1.positionViewAtIndex(row, ListView.Contain)
                }

                contentItem: DeviceIdentity {
                    deviceType: appManager1.selectedDeviceId > 0
                                ? appManager1.selectedDeviceType : "All"
                    deviceName: appManager1.selectedDeviceId > 0
                                ? appManager1.selectedDeviceName : qsTr("All")
                    deviceIsActive: appManager1.selectedDeviceIsActive
                }
            }

            Item {
            Layout.fillWidth: true
            Layout.preferredHeight: root.headerRowHeight
            RowLayout {
                anchors {
                    left: parent.left
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    leftMargin: Kirigami.Units.gridUnit
                    rightMargin: Kirigami.Units.gridUnit
                }
                spacing: Kirigami.Units.smallSpacing
                // Filters the device list, not files: a filter icon and
                // placeholder keep it from reading as a file search.
                // ActionTextField, not SearchField: SearchField draws its own
                // magnifier, which cannot be replaced. Both icons are plain
                // Icons, not actions, so they carry no button frame or tooltip:
                // the filter icon is styled like SearchField's magnifier, the
                // clear icon like the Search page's (SearchTermList.qml).
                Kirigami.ActionTextField {
                    id: deviceSearchField
                    Layout.fillWidth: true
                    placeholderText: qsTr("Name") + "..."
                    focusSequence: StandardKey.Find
                    inputMethodHints: Qt.ImhNoPredictiveText
                    Accessible.searchEdit: true
                    leftPadding: filterIcon.width + Kirigami.Units.smallSpacing * 3
                    rightPadding: text.length > 0
                                  ? Kirigami.Units.iconSizes.small + Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
                                  : undefined
                    onTextChanged: appManager1.setDeviceFilter(text)

                    Kirigami.Icon {
                        id: filterIcon
                        anchors { left: parent.left; leftMargin: Kirigami.Units.smallSpacing * 2; verticalCenter: parent.verticalCenter }
                        implicitWidth:  Kirigami.Units.iconSizes.sizeForLabels
                        implicitHeight: Kirigami.Units.iconSizes.sizeForLabels
                        color: deviceSearchField.placeholderTextColor
                        source: "view-filter"
                    }

                    Kirigami.Icon {
                        anchors { right: parent.right; rightMargin: Kirigami.Units.smallSpacing * 2; verticalCenter: parent.verticalCenter }
                        source: parent.LayoutMirroring.enabled ? "edit-clear-locationbar-ltr" : "edit-clear-locationbar-rtl"
                        implicitWidth: Kirigami.Units.iconSizes.small
                        implicitHeight: Kirigami.Units.iconSizes.small
                        visible: deviceSearchField.text.length > 0
                        opacity: deviceClearTap.pressed ? 0.5 : 1.0
                        Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration } }
                        HoverHandler { cursorShape: Qt.ArrowCursor }
                        TapHandler {
                            id: deviceClearTap
                            onTapped: {
                                deviceSearchField.clear()
                                deviceSearchField.forceActiveFocus()
                            }
                        }
                    }
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
        }

        Kirigami.CardsListView {
            id: selectionListView1
            model: appManager1.deviceFilterModel
            delegate: PageSelectionDelegate {}
            // Without this, delegates keep painting outside the viewport while
            // the list scrolls — which is how card icons ended up drawn over the
            // search history dialog.
            clip: true
            topMargin: Kirigami.Units.smallSpacing
            spacing: Kirigami.Units.largeSpacing
        }
    }

    //Pages - Search
    Kirigami.ScrollablePage {
        id: pageSearch
        // Hidden until pushed, like every other feature page: when the app opens
        // on another page, Search is never pushed and would otherwise stay drawn
        // on the window behind the page stack.
        visible: false
        property Kirigami.Action escapeAction: pageSearchEscapeAction  // Esc (KBS-F1)
        title: qsTr("Search")

        // Run the search and show the results page. Shared by the Search action button
        // and the Enter key in the search text field so both behave identically.
        function runSearch() {
            if (appManager1.searchIsRunning)
                return
            // A catalog update or creation runs synchronously on this thread and
            // keeps the UI alive by pumping the event loop, so the Search action
            // stays clickable while one is in progress. Starting a search there
            // would run it nested inside that job's call stack, re-entrantly, on
            // the same database connection. Refuse instead.
            if (appManager1.deviceUpdateIsRunning || appManager1.catalogIsCreating)
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
                root.refreshTextIfStale(pageSearchResults)
                pageStack.push(pageSearchResults)
            }
        }

        actions: [
            Kirigami.Action {
                text:        qsTr("Search")
                icon.name:   "edit-find"
                displayHint: Kirigami.DisplayHint.KeepVisible
                enabled:     !appManager1.searchIsRunning
                             && !appManager1.deviceUpdateIsRunning
                             && !appManager1.catalogIsCreating
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
                id: pageSearchEscapeAction
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


        PageSearchForm {
            id: pageSearchForm
            onSearchRequested: pageSearch.runSearch()
        }
    }

    //Pages - SearchResults
    Kirigami.Page {
        id: pageSearchResults
        property Kirigami.Action escapeAction: pageSearchResultsEscapeAction  // Esc (KBS-F1)
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
                id: pageSearchResultsEscapeAction
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
        property Kirigami.Action escapeAction: pageDevicesEscapeAction  // Esc (KBS-F1)
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
                enabled: !appManager1.deviceUpdateIsRunning && !appManager1.searchIsRunning
                onTriggered: {
                    var result = appManager1.recordDevicesSnapshot()
                    devSnapshotDialog.data = result
                    devSnapshotDialog.open()
                }
            },
            Kirigami.Action {
                text: qsTr("Insert Virtual Group")
                icon.name: "folder-new"
                enabled: !appManager1.deviceUpdateIsRunning && !appManager1.searchIsRunning
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
                enabled: !appManager1.deviceUpdateIsRunning && !appManager1.searchIsRunning
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
                id: pageDevicesEscapeAction
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
                console.log("Main: onEditDeviceRequested deviceId=" + deviceId)
                pageDeviceEdit.fromDevicesPage = true
                pageDeviceEdit_form.deviceId = deviceId
                pageDeviceEdit_form.loadDevice()
                root.showPage(pageDeviceEdit)
                console.log("Main: showPage(pageDeviceEdit) done")
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
        property Kirigami.Action escapeAction: pageExploreEscapeAction  // Esc (KBS-F1)
        visible: false
        title: qsTr("Explore")
        padding: 0

        actions: [
            Kirigami.Action {
                id: pageExploreEscapeAction
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
        property Kirigami.Action escapeAction: pageCreateEscapeAction  // Esc (KBS-F1)
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
                id: pageCreateEscapeAction
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
        property Kirigami.Action escapeAction: pageDeviceEditEscapeAction  // Esc (KBS-F1)
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
                id: pageDeviceEditEscapeAction
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
        property Kirigami.Action escapeAction: pageStatisticsEscapeAction  // Esc (KBS-F1)
        visible: false
        title: qsTr("Statistics")
        padding: 0

        actions: [
            Kirigami.Action {
                id: pageStatisticsEscapeAction
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
        property Kirigami.Action escapeAction: pageTagsEscapeAction  // Esc (KBS-F1)
        visible: false
        title: qsTr("Tags")
        actions: [
            Kirigami.Action {
                id: pageTagsEscapeAction
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
        property Kirigami.Action escapeAction: pageBackupEscapeAction  // Esc (KBS-F1)
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
                id: pageBackupEscapeAction
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
        PageSettings {
            textScale: root.cardScale
            onTextScaleEdited: function(value) {
                value = Math.round(value * 10) / 10   // keep exact steps (0.8 … 1.2)
                root.cardScale = value
                windowSettings.savedCardScale = value
            }
        }
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
