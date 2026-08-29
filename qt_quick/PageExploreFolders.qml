import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

// Left panel of the Explore page: folder tree for the active explore catalog.
// Entry point: openByDeviceId(deviceId) called from context menus.
Item {
    id: root

    signal folderClicked(string folderPath)
    signal catalogOpenRequested(var catalogInfo)

    property string catalogName: ""
    property string catalogPath: ""
    property int    folderCount: 0

    // Total size of the whole catalog, computed live from the file records — an
    // empty folder path means "the whole catalog" — so it always reconciles with
    // the per-folder figures in the file list rather than drifting from a stored
    // total refreshed only at catalog update (SpecExplore.md EXP-F13/F14).
    property real   catalogTotalSize: 0
    property string selectedFolderPath: ""

    // Row buttons are sized to sit inside the height the text already needs, so
    // they never drive the row height. A row without sub-directories keeps the
    // button in the layout and merely fades it: a button that comes and goes
    // re-lays the row out, which moves everything on it and flickers the tree.
    readonly property real rowButton: Kirigami.Units.iconSizes.small + Kirigami.Units.smallSpacing

    // How far each level of the tree steps in.
    readonly property real indentStep: Kirigami.Units.gridUnit

    // The whole tree, in the depth-first order the core returns it. The visible
    // rows are derived from it, so collapsing never loses anything.
    property var allFolders: []

    // Collapsed rows, keyed by full path. Transient view state: it is never
    // persisted, and it is rebuilt from scratch each time a catalog is opened.
    property var collapsedPaths: ({})

    property bool canCollapse: false
    property bool canExpand: false

    // Set while a catalog is being opened, so the reveal handler does not run
    // against the tree that is on its way out.
    property bool _loading: false

    // Open a catalog by device ID (called from Devices/Selection context menus)
    function openByDeviceId(deviceId) {
        _applyOpenResult(appManager1.exploreOpenCatalog(deviceId))
    }

    function _applyOpenResult(info) {
        root._loading = true
        if (!info.success) {
            root.catalogName = ""
            root.catalogPath = ""
            root.folderCount = 0
            root.catalogTotalSize = 0
            root.allFolders  = []
            root.collapsedPaths = ({})
            folderListModel.clear()
            _updateAbility()
            root._loading = false
            return
        }
        root.catalogName        = info.name
        root.catalogPath        = info.path
        root.folderCount        = info.folderCount
        root.catalogTotalSize   = appManager1.getExploreFolderStats("").totalSize
        root.selectedFolderPath = info.path

        // The catalog itself is the root of the tree; the folders hang under it,
        // so every folder level is pushed down by one.
        var rows = [{ name: info.name, fullPath: info.path, level: 0 }]
        var folders = appManager1.getExploreFolders()
        for (var i = 0; i < folders.length; i++) {
            rows.push({
                name:     folders[i].name,
                fullPath: folders[i].fullPath,
                level:    folders[i].level + 1
            })
        }
        root.allFolders = rows

        _collapseToInitialDepth()
        _rebuild()

        root._loading = false
        folderListView.currentIndex = 0
        root.folderClicked(info.path)
        root.catalogOpenRequested(info)
    }

    // A row has children when the row after it steps one level deeper. The list
    // is depth-first, so that is the whole test — the tree needs no other shape.
    function _hasChildren(index) {
        return index + 1 < root.allFolders.length
               && root.allFolders[index + 1].level > root.allFolders[index].level
    }

    // How many ranks stand open when a catalog is loaded. K2 opens the root and
    // its immediate children only (expandToDepth(0)); Katalog 3 opens one rank
    // more, because a catalog's real content usually starts a folder or two
    // below the root and the first thing one did with the K2 tree was open it.
    readonly property int initialOpenDepth: 2

    function _collapseToInitialDepth() {
        var collapsed = ({})
        for (var i = 0; i < root.allFolders.length; i++)
            if (root.allFolders[i].level >= root.initialOpenDepth && _hasChildren(i))
                collapsed[root.allFolders[i].fullPath] = true
        root.collapsedPaths = collapsed
    }

    // A row is hidden when a row above it is collapsed. The tree is depth-first,
    // so one pass with a "skip until the level comes back" mark is enough.
    function _rebuild() {
        folderListModel.clear()
        var skipDeeperThan = -1
        var selectedRow = -1
        for (var i = 0; i < root.allFolders.length; i++) {
            var folder = root.allFolders[i]
            if (skipDeeperThan >= 0) {
                if (folder.level > skipDeeperThan)
                    continue
                skipDeeperThan = -1
            }
            var hasChildren = _hasChildren(i)
            var isCollapsed = hasChildren && root.collapsedPaths[folder.fullPath] === true
            if (folder.fullPath === root.selectedFolderPath)
                selectedRow = folderListModel.count
            folderListModel.append({
                name:        folder.name,
                fullPath:    folder.fullPath,
                level:       folder.level,
                hasChildren: hasChildren,
                isCollapsed: isCollapsed
            })
            if (isCollapsed)
                skipDeeperThan = folder.level
        }
        if (selectedRow >= 0)
            folderListView.currentIndex = selectedRow
        _updateAbility()
    }

    // The header buttons are disabled when they would do nothing, so the tree
    // says what it can still do rather than answering a press with nothing.
    function _updateAbility() {
        var collapsable = false
        for (var i = 0; i < folderListModel.count; i++) {
            var row = folderListModel.get(i)
            if (row.hasChildren && !row.isCollapsed) {
                collapsable = true
                break
            }
        }
        root.canCollapse = collapsable

        var expandable = false
        for (var path in root.collapsedPaths) {
            if (root.collapsedPaths[path] === true) {
                expandable = true
                break
            }
        }
        root.canExpand = expandable
    }

    function toggleCollapsed(folderPath) {
        if (root.collapsedPaths[folderPath] === true)
            delete root.collapsedPaths[folderPath]
        else
            root.collapsedPaths[folderPath] = true
        _rebuild()
    }

    function collapseAll() {
        var collapsed = ({})
        for (var i = 0; i < root.allFolders.length; i++)
            if (_hasChildren(i))
                collapsed[root.allFolders[i].fullPath] = true
        root.collapsedPaths = collapsed
        _rebuild()
    }

    function expandAll() {
        root.collapsedPaths = ({})
        _rebuild()
    }

    // Close the deepest rank that is currently open: one press peels one layer
    // off the bottom, which is what "one level" means to the eye.
    function collapseOneLevel() {
        var deepest = -1
        for (var i = 0; i < folderListModel.count; i++) {
            var row = folderListModel.get(i)
            if (row.hasChildren && !row.isCollapsed)
                deepest = Math.max(deepest, row.level)
        }
        if (deepest < 0)
            return
        for (var j = 0; j < folderListModel.count; j++) {
            var candidate = folderListModel.get(j)
            if (candidate.level === deepest && candidate.hasChildren)
                root.collapsedPaths[candidate.fullPath] = true
        }
        _rebuild()
    }

    // Open the shallowest closed rank, so expanding undoes collapsing.
    function expandOneLevel() {
        var shallowest = -1
        for (var i = 0; i < root.allFolders.length; i++) {
            var folder = root.allFolders[i]
            if (root.collapsedPaths[folder.fullPath] !== true)
                continue
            if (shallowest < 0 || folder.level < shallowest)
                shallowest = folder.level
        }
        if (shallowest < 0)
            return
        for (var j = 0; j < root.allFolders.length; j++)
            if (root.allFolders[j].level === shallowest)
                delete root.collapsedPaths[root.allFolders[j].fullPath]
        _rebuild()
    }

    // Navigating into a sub-folder from the file list selects a row that may sit
    // under a closed one. Open its ancestors rather than highlight a row nobody
    // can see.
    function _revealSelected() {
        for (var i = 0; i < folderListModel.count; i++)
            if (folderListModel.get(i).fullPath === root.selectedFolderPath) {
                folderListView.currentIndex = i
                return
            }

        var index = -1
        for (var j = 0; j < root.allFolders.length; j++)
            if (root.allFolders[j].fullPath === root.selectedFolderPath) {
                index = j
                break
            }
        if (index < 0)
            return

        var level = root.allFolders[index].level
        for (var k = index - 1; k >= 0 && level > 0; k--) {
            if (root.allFolders[k].level < level) {
                delete root.collapsedPaths[root.allFolders[k].fullPath]
                level = root.allFolders[k].level
            }
        }
        _rebuild()
    }

    onSelectedFolderPathChanged: {
        if (!root._loading)
            _revealSelected()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Directories count header
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin:  Kirigami.Units.smallSpacing
            Layout.rightMargin: Kirigami.Units.smallSpacing
            Layout.topMargin:   Kirigami.Units.smallSpacing
            spacing: Kirigami.Units.smallSpacing

            Controls.Label {
                text: qsTr("Directories")
            }
            Controls.Label {
                text: root.folderCount.toLocaleString(Qt.locale(), "f", 0)
                font.bold: true
            }

            Item { Layout.preferredWidth: Kirigami.Units.largeSpacing }

            // The catalog's total size, beside the catalog-wide directory count
            // that is already here. No label of its own: a formatted data size
            // names its own unit.
            Controls.Label {
                text: appManager1.formatDataSizeDelta(root.catalogTotalSize)
                font.bold: true
                visible: root.catalogTotalSize > 0
            }

            Item { Layout.fillWidth: true }
        }

        // Depth controls, on their own row: the panel is narrow enough that they
        // would push the count off the end if they shared its row.
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin:  Kirigami.Units.smallSpacing
            Layout.rightMargin: Kirigami.Units.smallSpacing
            Layout.bottomMargin: Kirigami.Units.smallSpacing
            spacing: 0

            Controls.ToolButton {
                icon.name: "go-up"
                text: qsTr("Collapse one level")
                display: Controls.AbstractButton.IconOnly
                enabled: root.canCollapse
                onClicked: root.collapseOneLevel()
                Controls.ToolTip.text: text
                Controls.ToolTip.visible: hovered
            }
            Controls.ToolButton {
                icon.name: "go-down"
                text: qsTr("Expand one level")
                display: Controls.AbstractButton.IconOnly
                enabled: root.canExpand
                onClicked: root.expandOneLevel()
                Controls.ToolTip.text: text
                Controls.ToolTip.visible: hovered
            }
            Controls.ToolButton {
                icon.name: "collapse-all"
                text: qsTr("Collapse all")
                display: Controls.AbstractButton.IconOnly
                enabled: root.canCollapse
                onClicked: root.collapseAll()
                Controls.ToolTip.text: text
                Controls.ToolTip.visible: hovered
            }
            Controls.ToolButton {
                icon.name: "expand-all"
                text: qsTr("Expand all")
                display: Controls.AbstractButton.IconOnly
                enabled: root.canExpand
                onClicked: root.expandAll()
                Controls.ToolTip.text: text
                Controls.ToolTip.visible: hovered
            }

            Item { Layout.fillWidth: true }
        }

        Kirigami.Separator { Layout.fillWidth: true }

        // Folder tree list
        Controls.ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Controls.ScrollBar.vertical.policy:   Controls.ScrollBar.AsNeeded
            Controls.ScrollBar.horizontal.policy: Controls.ScrollBar.AsNeeded

            ListView {
                id: folderListView
                model: ListModel { id: folderListModel }

                delegate: Controls.ItemDelegate {
                    id: folderDelegate
                    width: folderListView.width
                    highlighted: model.fullPath === root.selectedFolderPath

                    contentItem: RowLayout {
                        spacing: Kirigami.Units.smallSpacing

                        // Depth is drawn rather than modelled: the list is already
                        // in tree order, so indenting is all the view needs to do.
                        Item {
                            implicitWidth: model.level * root.indentStep
                            implicitHeight: 1
                        }

                        // Shown whenever the directory has something inside it:
                        // whether a branch can be opened is a property of the
                        // tree, not of where the pointer happens to be.
                        Controls.ToolButton {
                            padding: 0
                            implicitWidth:  root.rowButton
                            implicitHeight: root.rowButton
                            icon.width:  Kirigami.Units.iconSizes.small
                            icon.height: Kirigami.Units.iconSizes.small
                            opacity: model.hasChildren ? 1.0 : 0.0
                            enabled: model.hasChildren
                            icon.name: model.isCollapsed ? "go-next" : "go-down"
                            text: model.isCollapsed ? qsTr("Expand") : qsTr("Collapse")
                            display: Controls.AbstractButton.IconOnly
                            onClicked: root.toggleCollapsed(model.fullPath)
                            Controls.ToolTip.text: text
                            Controls.ToolTip.visible: hovered
                        }

                        Kirigami.Icon {
                            source: "folder"
                            implicitWidth:  Kirigami.Units.iconSizes.small
                            implicitHeight: Kirigami.Units.iconSizes.small
                        }

                        Controls.Label {
                            text: model.name
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                            color: folderDelegate.highlighted ? Kirigami.Theme.highlightedTextColor
                                                              : Kirigami.Theme.textColor
                        }
                    }

                    onClicked: {
                        root.selectedFolderPath = model.fullPath
                        folderListView.currentIndex = index
                        root.folderClicked(model.fullPath)
                    }
                }
            }
        }
    }
}
