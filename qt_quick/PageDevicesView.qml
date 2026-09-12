import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

Item {
    id: root
    anchors.fill: parent

    signal editDeviceRequested(int deviceId)
    signal exploreDeviceRequested(int deviceId)

    property real   cardScale: 1.0
    // Restored from the collection's stored choice (DVP-F7). A property
    // initializer, deliberately: it is evaluated before the
    // Component.onCompleted load below, so the one load that already happens at
    // startup uses the right view and costs no extra probe (DVP-C6).
    property string viewFilter: appManager1.deviceDisplayContents
    property bool   filterFromSelection: appManager1.deviceFilterFromSelection
    property var    devices: []

    // Collapsed device IDs for the CARD tree, held here and derived from the
    // flat list the loader returns - the technique PageExploreFolders.qml uses
    // (DVP-C19). Independent of the Selection page, which keeps its own state
    // in its own model (DVP-C18), and of the tree table, which keeps its own.
    // Transient: every fresh load opens the tree fully (DVP-F25).
    property var collapsedCardIds: ({})

    // The cards actually shown: every device, minus those under a collapsed
    // one. Each row is tagged with whether it has children and whether it is
    // collapsed, so the delegate needs no second pass over the list.
    readonly property var visibleCards: {
        var all = root.devices
        if (root.viewFilter !== "All")
            return all
        var out = []
        var skipDeeperThan = -1
        for (var i = 0; i < all.length; i++) {
            var d = all[i]
            if (skipDeeperThan >= 0) {
                if (d.level > skipDeeperThan) continue
                skipDeeperThan = -1
            }
            // A row has children when the next one stands a level deeper: the
            // list is depth-first, so that is the whole test.
            var hasKids = i + 1 < all.length && all[i + 1].level > d.level
            var collapsed = hasKids && root.collapsedCardIds[d.deviceId] === true
            var row = Object.assign({}, d)
            row._hasChildren = hasKids
            row._isCollapsed = collapsed
            out.push(row)
            if (collapsed) skipDeeperThan = d.level
        }
        return out
    }

    function toggleCardCollapsed(deviceId) {
        var next = Object.assign({}, root.collapsedCardIds)
        if (next[deviceId] === true) delete next[deviceId]
        else next[deviceId] = true
        root.collapsedCardIds = next
    }
    property string operationDeviceName: ""
    property bool   operationDeviceActive: false

    // Table display mode (SpecDevicesPage DVP-F1). Offered for the Storage and
    // Catalogs lists only; the Device tree keeps its cards until that view is
    // migrated in its own right.
    // All three views render as cards or as a table, on one shared preference
    // (DVP-F1 as amended, DVP-F15). Kept as a property because the filter bar
    // and the table body both read it.
    readonly property bool tableAvailable: true
    property bool displayAsTable: appManager1.deviceDisplayAsTable
    readonly property bool tableMode: tableAvailable && displayAsTable
    property int  tableSortColumn: -1
    property bool tableSortAscending: true

    // Feeds the table from the rows already loaded for the cards. Deliberately
    // separate from refreshDevices(): switching display mode must not reload the
    // list, because a reload re-probes the active status and only opening the
    // page or switching the type filter may do that (DVP-C2 / DAS-O4).
    function refreshTable() {
        if (!tableAvailable) return
        appManager1.populateDeviceTable(viewFilter, devices)
        if (tableSortColumn >= 0)
            appManager1.sortDeviceTable(tableSortColumn, tableSortAscending ? 0 : 1)
    }

    onDevicesChanged:    refreshTable()
    onViewFilterChanged: { tableSortColumn = -1; tableSortAscending = true }

    function refreshDevices() {
        var scope = filterFromSelection ? appManager1.selectedDeviceId : 0
        devices = appManager1.getDeviceList(viewFilter, scope)
        // Fully expanded on every load, matching the tree table (DVP-F25).
        collapsedCardIds = ({})
    }

    // Create a child device and open it for editing, as K2 does
    // (mainwindow_tab_device_pr.cpp addDeviceVirtual/addDeviceStorage, both
    // ending in editDevice()). Lives on the page rather than in the card's
    // handler: creating a device refreshes the list and destroys that card.
    function createChildAndEdit(parentId, kind) {
        var newId = (kind === "Storage") ? appManager1.addDeviceStorage(parentId)
                                         : appManager1.addDeviceVirtual(parentId)
        console.log("createChildAndEdit: kind=" + kind + " parentId=" + parentId
                    + " newId=" + newId)
        if (newId > 0)
            root.editDeviceRequested(newId)
        else
            console.warn("createChildAndEdit: no device created, editor not opened")
    }

    Connections {
        target: appManager1
        function onDeviceListChanged() {
            // Opening another collection reaches this page here, through
            // refreshAllUI(). The stored view is picked up before the list is
            // rebuilt, so the restore rides on the load that happens anyway
            // rather than forcing a second one (DVP-C6). Note this must not
            // move into onDatabaseModeChanged: that signal is emitted after
            // refreshAllUI() has already rebuilt the list
            // (appmanager.cpp:977-978).
            root.viewFilter = appManager1.deviceDisplayContents
            root.refreshDevices()
        }
        // Both display choices are stored in the collection's own .ini, so
        // opening another collection must bring that collection's values up
        // rather than leave the previous one's showing (DVP-F4).
        function onDatabaseModeChanged() {
            root.displayAsTable = appManager1.deviceDisplayAsTable
        }
        function onSelectedDeviceChanged() {
            if (root.filterFromSelection) root.refreshDevices()
        }
        function onSplitCompleted(success, error) {
            root.refreshDevices()
            if (!success && error.length > 0) {
                operationResultLabel.text = error
                operationResultDialog.open()
            }
        }
    }

    Component.onCompleted: refreshDevices()

    // ── Dialogs ────────────────────────────────────────────────────────────

    Controls.Dialog {
        id: splitSubDirConfirmDialog
        property int deviceId: 0
        title: qsTr("Split Catalog")
        modal: true
        anchors.centerIn: parent
        width: Math.min(420, root.width - Kirigami.Units.largeSpacing * 4)
        contentItem: Controls.Label {
            text: qsTr("Split <b>%1</b> into sub-catalogs by sub-directory?<br/><br/>This will create one sub-catalog per immediate sub-directory and remove the original catalog. This operation cannot be undone.").arg(root.operationDeviceName)
            textFormat: Text.RichText
            wrapMode: Text.WordWrap
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("Split")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
            Controls.Button {
                text: qsTr("Cancel")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.RejectRole
            }
            onAccepted: {
                var id = splitSubDirConfirmDialog.deviceId
                splitSubDirConfirmDialog.close()
                var err = appManager1.splitCatalogBySubDirectory(id)
                if (err.length > 0) {
                    operationResultLabel.text = err
                    operationResultDialog.open()
                } else {
                    root.refreshDevices()
                }
            }
            onRejected: splitSubDirConfirmDialog.close()
        }
    }

    Controls.Dialog {
        id: splitFileTypeDialog
        property int  deviceId:    0
        property bool deviceActive: false
        title: qsTr("Split Catalog")
        modal: true
        anchors.centerIn: parent
        width: Math.min(420, root.width - Kirigami.Units.largeSpacing * 4)
        contentItem: ColumnLayout {
            spacing: Kirigami.Units.largeSpacing
            Controls.Label {
                text: qsTr("Split <b>%1</b> into sub-catalogs by file type:").arg(root.operationDeviceName)
                textFormat: Text.RichText
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing
                Controls.Button {
                    text: qsTr("Verify then Split")
                    icon.name: "dialog-ok"
                    Layout.fillWidth: true
                    enabled: splitFileTypeDialog.deviceActive
                    onClicked: {
                        var id = splitFileTypeDialog.deviceId
                        splitFileTypeDialog.close()
                        appManager1.splitCatalogByFileType(id, true)
                    }
                }
                Controls.Button {
                    text: qsTr("Split without verifying")
                    icon.name: "edit-cut"
                    Layout.fillWidth: true
                    onClicked: {
                        var id = splitFileTypeDialog.deviceId
                        splitFileTypeDialog.close()
                        appManager1.splitCatalogByFileType(id, false)
                    }
                }
            }
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("Cancel")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.RejectRole
            }
            onRejected: splitFileTypeDialog.close()
        }
    }

    Controls.Dialog {
        id: verifyConfirmDialog
        property int deviceId: 0
        title: qsTr("Verify Checksums")
        modal: true
        anchors.centerIn: parent
        width: Math.min(420, root.width - Kirigami.Units.largeSpacing * 4)
        contentItem: Controls.Label {
            text: qsTr("Verify checksums for <b>%1</b>?").arg(root.operationDeviceName)
            textFormat: Text.RichText
            wrapMode: Text.WordWrap
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("Verify")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
            Controls.Button {
                text: qsTr("Cancel")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.RejectRole
            }
            onAccepted: {
                var id = verifyConfirmDialog.deviceId
                verifyConfirmDialog.close()
                var result = appManager1.verifyDeviceChecksums(id)
                if (result.noChecksums) {
                    verifyResultLabel.text = qsTr("No checksums are stored for this catalog.")
                } else {
                    verifyResultLabel.text =
                        qsTr("Total: %1").arg(result.total) + "\n" +
                        qsTr("Verified: %1").arg(result.verified) + "\n" +
                        qsTr("Mismatches: %1").arg(result.mismatches) + "\n" +
                        qsTr("Missing: %1").arg(result.missing)
                    if (result.mismatches > 0 && result.mismatchedFiles && result.mismatchedFiles.length > 0)
                        verifyResultLabel.text += "\n\n" + result.mismatchedFiles.join("\n")
                }
                verifyResultDialog.open()
            }
            onRejected: verifyConfirmDialog.close()
        }
    }

    Controls.Dialog {
        id: verifyResultDialog
        title: qsTr("Checksum Verification")
        modal: true
        anchors.centerIn: parent
        width: Math.min(460, root.width - Kirigami.Units.largeSpacing * 4)
        contentItem: Controls.Label {
            id: verifyResultLabel
            wrapMode: Text.WordWrap
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("OK")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
            onAccepted: verifyResultDialog.close()
        }
    }

    Controls.Dialog {
        id: unassignDialog
        property int deviceId: 0
        property int parentId: 0
        title: qsTr("Unassign Device")
        modal: true
        anchors.centerIn: parent
        width: Math.min(420, root.width - Kirigami.Units.largeSpacing * 4)
        contentItem: Controls.Label {
            text: qsTr("Unassign <b>%1</b> from its parent device?").arg(root.operationDeviceName)
            textFormat: Text.RichText
            wrapMode: Text.WordWrap
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("Unassign")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
            Controls.Button {
                text: qsTr("Cancel")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.RejectRole
            }
            onAccepted: {
                unassignDialog.close()
                var err = appManager1.unassignDevice(unassignDialog.deviceId, unassignDialog.parentId)
                if (err.length > 0) {
                    operationResultLabel.text = err
                    operationResultDialog.open()
                }
            }
            onRejected: unassignDialog.close()
        }
    }

    Controls.Dialog {
        id: deleteConfirmDialog
        property int    deviceId:   0
        property string deviceType: ""
        title: qsTr("Delete Device")
        modal: true
        anchors.centerIn: parent
        width: Math.min(420, root.width - Kirigami.Units.largeSpacing * 4)
        contentItem: Controls.Label {
            text: qsTr("Do you want to <b>delete</b> this %1 device?<br/><br/>Name: <b>%2</b>")
                  .arg(deleteConfirmDialog.deviceType)
                  .arg(root.operationDeviceName)
            textFormat: Text.RichText
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
                deleteConfirmDialog.close()
                var err = appManager1.deleteDevice(deleteConfirmDialog.deviceId)
                if (err.length > 0) {
                    operationResultLabel.text = err
                    operationResultDialog.open()
                }
            }
            onRejected: deleteConfirmDialog.close()
        }
    }

    Controls.Dialog {
        id: operationResultDialog
        title: "Katalog"
        modal: true
        anchors.centerIn: parent
        width: Math.min(420, root.width - Kirigami.Units.largeSpacing * 4)
        contentItem: Controls.Label {
            id: operationResultLabel
            wrapMode: Text.WordWrap
        }
        footer: Controls.DialogButtonBox {
            Controls.Button {
                text: qsTr("OK")
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
            onAccepted: operationResultDialog.close()
        }
    }

    // ── Layout ──────────────────────────────────────────────────────────────

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Filter bar
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: filterRow.implicitHeight + Kirigami.Units.smallSpacing * 2
            color: Kirigami.Theme.alternateBackgroundColor

            RowLayout {
                id: filterRow
                anchors {
                    left: parent.left; right: parent.right
                    verticalCenter: parent.verticalCenter
                    leftMargin: Kirigami.Units.largeSpacing
                    rightMargin: Kirigami.Units.largeSpacing
                }
                spacing: Kirigami.Units.smallSpacing

                Controls.ButtonGroup { id: filterGroup }

                Controls.ToolButton {
                    text: qsTr("Device tree")
                    checkable: true; checked: root.viewFilter === "All"
                    Controls.ButtonGroup.group: filterGroup
                    onClicked: {
                        root.viewFilter = "All"
                        appManager1.deviceDisplayContents = "All"
                        root.refreshDevices()
                    }
                }
                Controls.ToolButton {
                    text: qsTr("Storage list")
                    checkable: true; checked: root.viewFilter === "Storage"
                    Controls.ButtonGroup.group: filterGroup
                    onClicked: {
                        root.viewFilter = "Storage"
                        appManager1.deviceDisplayContents = "Storage"
                        root.refreshDevices()
                    }
                }
                Controls.ToolButton {
                    text: qsTr("Catalogs list")
                    checkable: true; checked: root.viewFilter === "Catalogs"
                    Controls.ButtonGroup.group: filterGroup
                    onClicked: {
                        root.viewFilter = "Catalogs"
                        appManager1.deviceDisplayContents = "Catalogs"
                        root.refreshDevices()
                    }
                }

                Item { Layout.fillWidth: true }

                // Display mode (DVP-F1). Shown only for the two list views; the
                // Device tree has no table yet. Neither this nor Full Table
                // reloads the list - both re-render rows already in hand, so no
                // active-status probe is triggered (DVP-C2).
                Controls.ToolSeparator { visible: root.tableAvailable }

                Controls.ButtonGroup { id: displayModeGroup }

                Controls.ToolButton {
                    text: qsTr("Cards")
                    icon.name: "view-list-icons"
                    visible: root.tableAvailable
                    checkable: true; checked: !root.displayAsTable
                    Controls.ButtonGroup.group: displayModeGroup
                    onClicked: {
                        root.displayAsTable = false
                        appManager1.deviceDisplayAsTable = false
                    }
                }
                Controls.ToolButton {
                    text: qsTr("Table")
                    icon.name: "view-list-details"
                    visible: root.tableAvailable
                    checkable: true; checked: root.displayAsTable
                    Controls.ButtonGroup.group: displayModeGroup
                    onClicked: {
                        root.displayAsTable = true
                        appManager1.deviceDisplayAsTable = true
                        root.refreshTable()
                    }
                }

                // K2's own Full Table string (mainwindow.ui:5591), reused
                // verbatim so no translation slot is spent (DVP-C5).
                Controls.CheckBox {
                    text: qsTr("Full Table")
                    visible: root.tableMode
                    checked: appManager1.deviceDisplayFullTable
                    onToggled: {
                        appManager1.deviceDisplayFullTable = checked
                        // The column count changed; TableView caches column
                        // widths and has to be told to measure them again.
                        deviceTable.forceLayout()
                    }
                }

                // Last in the bar, after the whole display-mode cluster rather
                // than between Table and Full Table, which would split that
                // cluster (DVP-F1). The separator carries the cluster's own
                // visibility, so in the Device tree view this checkbox is the
                // only control at the right of the bar.
                Controls.ToolSeparator { visible: root.tableAvailable }

                Controls.CheckBox {
                    text: qsTr("Filter from Selection")
                    checked: root.filterFromSelection
                    onToggled: {
                        root.filterFromSelection = checked
                        appManager1.deviceFilterFromSelection = checked
                        root.refreshDevices()
                    }
                }
            }
        }

        Kirigami.Separator { Layout.fillWidth: true }

        // Summary bar - shown for Catalogs and Storage list views (matches K2 CatalogStats/StorageStats)
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: summaryRow.implicitHeight + Kirigami.Units.smallSpacing * 2
            color: Kirigami.Theme.alternateBackgroundColor
            visible: root.viewFilter !== "All"

            RowLayout {
                id: summaryRow
                anchors {
                    left: parent.left; right: parent.right
                    verticalCenter: parent.verticalCenter
                    leftMargin: Kirigami.Units.largeSpacing
                    rightMargin: Kirigami.Units.largeSpacing
                }
                spacing: Kirigami.Units.largeSpacing

                // Catalogs summary
                Controls.Label {
                    visible: root.viewFilter === "Catalogs"
                    text: qsTr("Catalogs") + ":  <b>" + root.devices.length + "</b>"
                    textFormat: Text.RichText
                }
                Controls.ToolSeparator { visible: root.viewFilter === "Catalogs" }
                Controls.Label {
                    visible: root.viewFilter === "Catalogs"
                    text: qsTr("Total File Size") + ":  <b>" + appManager1.formatDataSize(
                        root.devices.reduce(function(s, d) { return s + (d.totalFileSize || 0) }, 0)) + "</b>"
                    textFormat: Text.RichText
                }
                Controls.ToolSeparator { visible: root.viewFilter === "Catalogs" }
                Controls.Label {
                    visible: root.viewFilter === "Catalogs"
                    text: qsTr("Total Number of Files") + ":  <b>" + Number(root.devices.reduce(
                        function(s, d) { return s + (d.fileCount || 0) }, 0)).toLocaleString(Qt.locale(), "f", 0) + "</b>"
                    textFormat: Text.RichText
                }

                // Storage summary
                Controls.Label {
                    visible: root.viewFilter === "Storage"
                    text: qsTr("Devices") + ":  <b>" + root.devices.length + "</b>"
                    textFormat: Text.RichText
                }
                Controls.ToolSeparator { visible: root.viewFilter === "Storage" }
                Controls.Label {
                    visible: root.viewFilter === "Storage"
                    text: qsTr("Total Space") + ":  <b>" + appManager1.formatDataSize(
                        root.devices.reduce(function(s, d) { return s + (d.totalSpace || 0) }, 0)) + "</b>"
                    textFormat: Text.RichText
                }
                Controls.ToolSeparator { visible: root.viewFilter === "Storage" }
                Controls.Label {
                    id: storageUsedLabel
                    visible: root.viewFilter === "Storage"
                    text: {
                        var used = root.devices.reduce(function(s, d) { return s + ((d.totalSpace || 0) - (d.freeSpace || 0)) }, 0)
                        return qsTr("Used") + ":  <b>" + appManager1.formatDataSize(used) + "</b>"
                    }
                    textFormat: Text.RichText
                }
                Controls.ToolSeparator { visible: root.viewFilter === "Storage" }
                Controls.Label {
                    id: storageFreeLabel
                    visible: root.viewFilter === "Storage"
                    text: {
                        var free  = root.devices.reduce(function(s, d) { return s + (d.freeSpace  || 0) }, 0)
                        var total = root.devices.reduce(function(s, d) { return s + (d.totalSpace || 0) }, 0)
                        var pct   = total > 0 ? Math.round(free / total * 100) : 0
                        return qsTr("Free") + ":  <b>" + appManager1.formatDataSize(free) + "  (" + pct + "%)</b>"
                    }
                    textFormat: Text.RichText
                }

                Item { Layout.fillWidth: true }
            }
        }
        Kirigami.Separator { Layout.fillWidth: true; visible: root.viewFilter !== "All" }

        // Device list
        Kirigami.CardsListView {
            id: deviceList
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: !root.tableMode
            clip: true
            model: root.visibleCards
            topMargin: Kirigami.Units.smallSpacing
            // The Selection list's own two values, copied rather than guessed:
            // that list is the one in Main.qml (the CardsListView holding
            // PageSelectionDelegate), not the unused PageSelectionView.qml.
            // Leaving spacing unset is not the same thing - Kirigami's own
            // default is wider than largeSpacing (DVP-F19).
            spacing: Kirigami.Units.largeSpacing

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                visible: deviceList.count === 0 && !appManager1.deviceUpdateIsRunning
                text: qsTr("No devices")
                icon.name: "drive-multidisk"
            }

            delegate: PageDevicesViewDelegate {
                delegateCardScale: root.cardScale
                viewFilter:        root.viewFilter

                onCollapseToggleRequested: (id) => root.toggleCardCollapsed(id)

                onEditRequested:    (id) => root.editDeviceRequested(id)
                onExploreRequested: (id) => root.exploreDeviceRequested(id)

                onSplitSubDirRequested: (id, name) => {
                    root.operationDeviceName = name
                    splitSubDirConfirmDialog.deviceId = id
                    splitSubDirConfirmDialog.open()
                }

                onSplitFileTypeRequested: (id, name, active) => {
                    root.operationDeviceName = name
                    root.operationDeviceActive = active
                    splitFileTypeDialog.deviceId = id
                    splitFileTypeDialog.deviceActive = active
                    splitFileTypeDialog.open()
                }

                onVerifyRequested: (id, name) => {
                    root.operationDeviceName = name
                    verifyConfirmDialog.deviceId = id
                    verifyConfirmDialog.open()
                }

                onUnassignRequested: (id, pid, name) => {
                    root.operationDeviceName = name
                    unassignDialog.deviceId = id
                    unassignDialog.parentId = pid
                    unassignDialog.open()
                }

                onDeleteRequested: (id, name, type) => {
                    var check = appManager1.checkDeviceDeleteAllowed(id)
                    if (!check.allowed) {
                        operationResultLabel.text = check.errorMessage
                        operationResultDialog.open()
                        return
                    }
                    root.operationDeviceName = name
                    deleteConfirmDialog.deviceId   = id
                    deleteConfirmDialog.deviceType = type
                    deleteConfirmDialog.open()
                }

                // The work is done by a function on the page, not here: adding a
                // device refreshes the list and destroys this delegate, and any
                // statement left in a handler running on it is abandoned. The
                // page's own frame survives that.
                onAddVirtualChildRequested: (parentId) => root.createChildAndEdit(parentId, "Virtual")
                onAddStorageChildRequested: (parentId) => root.createChildAndEdit(parentId, "Storage")

                onAssignCatalogRequested: (virtualId) => {
                    var err = appManager1.assignCatalogToDevice(appManager1.selectedDeviceId, virtualId)
                    if (err.length > 0) {
                        operationResultLabel.text = err
                        operationResultDialog.open()
                    }
                }

                onFilelightRequested: (id) => {
                    appManager1.launchFilelight(id)
                }
            }
        }

        // ── Table display (DVP-F1) ─────────────────────────────────────────
        // Header and rows follow the idiom already used on the Explore page.
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: root.tableMode
            clip: true

            Controls.HorizontalHeaderView {
                id: deviceTableHeader
                anchors { top: parent.top; left: parent.left; right: parent.right }
                syncView: deviceTable
                clip: true
                implicitHeight: 34
                resizableColumns: true

                delegate: Rectangle {
                    required property int    column
                    required property string display
                    color: Kirigami.Theme.backgroundColor
                    implicitHeight: deviceTableHeader.implicitHeight

                    Controls.Label {
                        anchors {
                            left: parent.left; right: sortMark.left
                            verticalCenter: parent.verticalCenter
                            leftMargin: 6; rightMargin: 2
                        }
                        text: display
                        elide: Text.ElideRight
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.9
                        color: Kirigami.Theme.textColor
                    }

                    Controls.Label {
                        id: sortMark
                        anchors { right: parent.right; verticalCenter: parent.verticalCenter; rightMargin: 4 }
                        text: root.tableSortColumn === column ? (root.tableSortAscending ? "▲" : "▼") : ""
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.7
                        color: Kirigami.Theme.textColor
                    }

                    Rectangle { // Vertical separator
                        anchors { top: parent.top; bottom: parent.bottom; right: parent.right }
                        width: 1
                        color: Kirigami.Theme.separatorColor ?? "transparent"
                    }

                    TapHandler {
                        onTapped: {
                            deviceTable.selectedRow = -1
                            if (root.tableSortColumn === column) {
                                root.tableSortAscending = !root.tableSortAscending
                            } else {
                                root.tableSortColumn    = column
                                root.tableSortAscending = true
                            }
                            appManager1.sortDeviceTable(root.tableSortColumn,
                                                        root.tableSortAscending ? 0 : 1)
                        }
                    }
                }
            }

            Kirigami.Separator {
                id: deviceTableHeaderSep
                anchors { top: deviceTableHeader.bottom; left: parent.left; right: parent.right }
            }

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                visible: deviceTable.rows === 0 && !appManager1.deviceUpdateIsRunning
                text: qsTr("No devices")
                icon.name: "drive-multidisk"
            }

            TableView {
                id: deviceTable
                anchors { top: deviceTableHeaderSep.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }
                columnSpacing: 0
                rowSpacing: 0
                clip: true
                model: appManager1.deviceTableModel

                property int selectedRow: -1

                // Qualified: this file imports QtQuick.Controls under the
                // Controls namespace, so the bare type name is not in scope.
                Controls.ScrollBar.vertical:   Controls.ScrollBar { policy: Controls.ScrollBar.AsNeeded }
                Controls.ScrollBar.horizontal: Controls.ScrollBar { policy: Controls.ScrollBar.AsNeeded }

                rowHeightProvider: function(row) { return 30 }
                columnWidthProvider: function(column) {
                    let w = deviceTable.explicitColumnWidth(column)
                    if (w >= 0) return w
                    return appManager1.deviceTableColumnWidth(column)
                }

                delegate: Rectangle {
                    required property int    row
                    required property int    column
                    required property var    display
                    required property string alignment
                    required property string iconName
                    required property bool   isBoolean
                    required property bool   tick
                    required property bool   bold
                    required property bool   italic
                    required property bool   dimmed
                    required property int    deviceId
                    required property var    rowData
                    // Tree rows only; zero and false everywhere else, so the
                    // two flat views lay out exactly as they did (DVP-F11).
                    required property int    level
                    required property bool   hasChildren
                    required property bool   expanded

                    readonly property real treeIndent: level * Kirigami.Units.gridUnit

                    implicitHeight: 30

                    color: deviceTable.selectedRow === row
                           ? applicationWindow().selectionHighlightColor
                           : (row % 2 === 0
                              ? applicationWindow().rowBaseColor
                              : applicationWindow().rowStripeColor)

                    Rectangle { // Column separator
                        visible: column > 0
                        anchors { top: parent.top; bottom: parent.bottom; left: parent.left }
                        width: 1
                        color: Kirigami.Theme.separatorColor
                        opacity: 0.4
                    }

                    // Expand/collapse, on tree rows that have children. Icon
                    // only - K2's tree has no text here either, so nothing to
                    // translate (DVP-C12).
                    Kirigami.Icon {
                        id: treeBranchIcon
                        visible: hasChildren
                        // Above the row-wide MouseArea below, which is declared
                        // after this icon and would otherwise sit on top of it
                        // and swallow every click on the chevron.
                        z: 2
                        anchors {
                            left: parent.left
                            verticalCenter: parent.verticalCenter
                            leftMargin: 4 + treeIndent
                        }
                        source: expanded ? "go-down-symbolic" : "go-next-symbolic"
                        implicitWidth:  Kirigami.Units.iconSizes.small
                        implicitHeight: Kirigami.Units.iconSizes.small
                        color: deviceTable.selectedRow === row
                               ? Kirigami.Theme.highlightedTextColor
                               : Kirigami.Theme.textColor
                        isMask: true

                        MouseArea {
                            anchors.fill: parent
                            // A little wider than the icon: the chevron is small
                            // and this is the one control the tree is driven by.
                            anchors.margins: -4
                            // Ahead of the row's own handler, so opening a
                            // branch does not also select the device.
                            onClicked: appManager1.toggleDeviceTableRow(row)
                        }
                    }

                    // Device icon, in the Name cell only, as K2 draws it.
                    Kirigami.Icon {
                        id: deviceRowIcon
                        visible: iconName.length > 0
                        anchors {
                            left: parent.left
                            verticalCenter: parent.verticalCenter
                            // Past the chevron, and past the space kept for one
                            // even when this row has no children, so every name
                            // at the same depth starts at the same x.
                            leftMargin: 6 + treeIndent
                                        + (root.viewFilter === "All" && column === 0
                                           ? Kirigami.Units.iconSizes.small + 4 : 0)
                        }
                        source: iconName
                        implicitWidth:  Kirigami.Units.iconSizes.small
                        implicitHeight: Kirigami.Units.iconSizes.small
                    }

                    // Boolean columns carry a tick rather than a word (DVP-F5).
                    Kirigami.Icon {
                        visible: isBoolean && tick
                        anchors { left: parent.left; verticalCenter: parent.verticalCenter; leftMargin: 6 }
                        source: "dialog-ok-apply"
                        implicitWidth:  Kirigami.Units.iconSizes.small
                        implicitHeight: Kirigami.Units.iconSizes.small
                    }

                    Controls.Label {
                        visible: !isBoolean
                        anchors {
                            left: deviceRowIcon.visible ? deviceRowIcon.right : parent.left
                            right: parent.right
                            verticalCenter: parent.verticalCenter
                            leftMargin:  deviceRowIcon.visible ? 4 : 6
                            rightMargin: 6
                        }
                        text: display === undefined || display === null ? "" : String(display)
                        horizontalAlignment: alignment === "right" ? Text.AlignRight : Text.AlignLeft
                        elide: Text.ElideRight
                        clip: true
                        font.bold:   bold
                        font.italic: italic
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.9
                        color: deviceTable.selectedRow === row
                               ? Kirigami.Theme.highlightedTextColor
                               : (dimmed ? Kirigami.Theme.disabledTextColor
                                         : Kirigami.Theme.textColor)
                    }

                    MouseArea {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        onClicked: function(mouse) {
                            deviceTable.selectedRow = row
                            if (mouse.button === Qt.RightButton)
                                openRowMenu()
                        }
                        onPressAndHold: {
                            deviceTable.selectedRow = row
                            openRowMenu()
                        }

                        // The row menu is the card's menu, unchanged (DVP-F6):
                        // the same delegate component is reused, driven by this
                        // row's data rather than by a card's.
                        function openRowMenu() {
                            tableRowMenu.modelData = rowData
                            tableRowMenu.popup()
                        }
                    }
                }
            }
        }

    }

    // Row context menu for the table. PageDevicesViewDelegate owns the one menu
    // definition; the table borrows it through this hidden instance rather than
    // repeating it, so the two can never drift apart (DVP-F6). The menu opens at
    // the cursor, so the host card being invisible does not affect placement.
    PageDevicesViewDelegate {
        id: tableRowMenu
        visible: false
        index: -1
        // Fully shaped, so none of the delegate's typed properties is ever fed
        // an undefined before the first row is picked.
        modelData: ({ deviceId: 0, parentId: 0, name: "", type: "",
                      path: "", parentType: "", level: 0,
                      active: false, groupId: 0 })

        function popup() { openContextMenu() }

        onEditRequested:    (id) => root.editDeviceRequested(id)
        onExploreRequested: (id) => root.exploreDeviceRequested(id)

        onSplitSubDirRequested: (id, name) => {
            root.operationDeviceName = name
            splitSubDirConfirmDialog.deviceId = id
            splitSubDirConfirmDialog.open()
        }
        onSplitFileTypeRequested: (id, name, active) => {
            root.operationDeviceName = name
            root.operationDeviceActive = active
            splitFileTypeDialog.deviceId = id
            splitFileTypeDialog.deviceActive = active
            splitFileTypeDialog.open()
        }
        onVerifyRequested: (id, name) => {
            root.operationDeviceName = name
            verifyConfirmDialog.deviceId = id
            verifyConfirmDialog.open()
        }
        onUnassignRequested: (id, pid, name) => {
            root.operationDeviceName = name
            unassignDialog.deviceId = id
            unassignDialog.parentId = pid
            unassignDialog.open()
        }
        onDeleteRequested: (id, name, type) => {
            var check = appManager1.checkDeviceDeleteAllowed(id)
            if (!check.allowed) {
                operationResultLabel.text = check.errorMessage
                operationResultDialog.open()
                return
            }
            root.operationDeviceName = name
            deleteConfirmDialog.deviceId   = id
            deleteConfirmDialog.deviceType = type
            deleteConfirmDialog.open()
        }
        onAddVirtualChildRequested: (parentId) => root.createChildAndEdit(parentId, "Virtual")
        onAddStorageChildRequested: (parentId) => root.createChildAndEdit(parentId, "Storage")
        onAssignCatalogRequested: (virtualId) => {
            var err = appManager1.assignCatalogToDevice(appManager1.selectedDeviceId, virtualId)
            if (err.length > 0) {
                operationResultLabel.text = err
                operationResultDialog.open()
            }
        }
        onFilelightRequested: (id) => appManager1.launchFilelight(id)
    }
}
