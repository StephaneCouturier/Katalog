import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import QtQuick.Dialogs

ScrollablePageFitted {
    id: pageSettingsRoot
    property Kirigami.Action escapeAction: escapeCloseAction  // Esc (KBS-F1)
    title: qsTr("Settings")

    // Set to true when opened from Open Collection > Hosted Db menu
    property bool showHostedForm: false
    // Recent Hosted entry refused by the host check: pre-fills the form (HST-F5)
    property var refusedHostedEntry: null

    // Text-size setting (TYP-F2 / TYP-F9): owned and persisted by Main.qml.
    property real textScale: 1.0
    signal textScaleEdited(real value)

    Connections {
        target: appManager1
        function onDatabaseModeChanged() {
            if (appManager1.databaseMode !== "Hosted")
                showHostedForm = false
        }
        function onImportSourceChanged() {
            importUpdateSourceCombo.model = appManager1.getImportSourcePaths()
            importDeviceCombo.selectById(0)
        }
        function onImageFolderPathChanged() {
            imageFolderField.text = appManager1.imageFolderPath
        }
    }

    Component.onCompleted: {
        settingsTabBar.currentIndex = showHostedForm ? 1 : 0   // SET-F4 / SET-F5
        hostField.text         = appManager1.getHostName()
        dbNameField.text       = appManager1.getDatabaseName()
        portField.value        = appManager1.getDatabasePort()
        userField.text         = appManager1.getDatabaseUserName()
        passwordField.text     = appManager1.getDatabasePassword()
        importUpdateSourceCombo.model = appManager1.getImportSourcePaths()
        if (refusedHostedEntry) {
            hostField.text     = refusedHostedEntry.hostName
            dbNameField.text   = refusedHostedEntry.dbName
            portField.value    = refusedHostedEntry.port
            userField.text     = refusedHostedEntry.userName
            passwordField.text = refusedHostedEntry.password
            invalidHostMessage.visible = true
        }
    }

    // FormLayout never sizes an item below its preferred width, so an item
    // whose natural width follows its content (checkbox text, combo entries,
    // button rows) asks for no more than the form has: it keeps its natural
    // width when that fits, and wraps or shrinks to the page otherwise (PGL-F16).
    // Not a breakpoint: nothing switches at a given width (PGL-C11).
    // Rounded up: a fractional text width rounded down by the layout wraps the last word.
    function fitWidth(natural) { return Math.min(Math.ceil(natural), formColumn.width) }

    // Settings Connect: empty host becomes localhost (HST-F1), then the host
    // checks (HST-F2..F4) before anything is saved or connected
    function requestHostedConnect() {
        var host = hostField.text.trim()
        if (host === "")
            host = "localhost"
        hostField.text = host
        var check = applicationWindow().requestHostedConnect({
            hostName: host, dbName: dbNameField.text, port: portField.value,
            userName: userField.text, password: passwordField.text })
        invalidHostMessage.visible = (check === "invalid")
    }

    // ── Dialogs ────────────────────────────────────────────────────────

    // Quality Check report, SpecQualityCheck.md phase 1: read-only, reports only.
    Kirigami.Dialog {
        id: qualityCheckDialog
        property var results: []   // QVariantList of {check, rows, error}

        // Keyed by check number: checks 2 and 5 are retired and the others keep
        // their numbers (SpecQualityCheck.md).
        readonly property var checkTitles: ({
            1: qsTr("Storage devices linked to a missing storage"),
            3: qsTr("Storage not linked to any device"),
            4: qsTr("Storage IDs used by more than one storage"),
            6: qsTr("Catalog devices linked to a missing catalog"),
            7: qsTr("Catalogs not linked to any device"),
            8: qsTr("Devices with a missing parent device")
        })

        readonly property int issueCount: {
            var n = 0
            for (var i = 0; i < results.length; ++i)
                n += results[i].rows.length + (results[i].error ? 1 : 0)
            return n
        }

        function run() {
            results = appManager1.runQualityChecks()
            open()
        }

        // One line per row, in the column order of the check's SELECT (core).
        function formatRow(check, r) {
            switch (check) {
            case 1: return qsTr("Device") + " " + r[0] + " \u201C" + r[1] + "\u201D \u2192 " + qsTr("Storage") + " " + r[2]
            case 3: return qsTr("Storage") + " " + r[0] + " \u00B7 " + qsTr("ID") + " " + r[1]
            case 4: return qsTr("ID") + " " + r[0] + " \u00B7 " + qsTr("Storage") + " " + r[1] + " \u201C" + r[2] + "\u201D"
            case 6: return qsTr("Device") + " " + r[0] + " \u201C" + r[1] + "\u201D \u2192 " + qsTr("Catalog") + " " + r[2]
            case 7: return qsTr("Catalog") + " " + r[0] + " \u201C" + r[1] + "\u201D"
            case 8: return qsTr("Device") + " " + r[0] + " \u201C" + r[1] + "\u201D \u2192 " + qsTr("Device") + " " + r[2]
            }
            return r.join(" \u00B7 ")
        }

        // Numbered by position, so the retired checks leave no gap in the report.
        function sectionTitle(res) {
            var position = 0
            for (var i = 0; i < results.length; ++i)
                if (results[i].check === res.check) position = i + 1
            return position + ". " + checkTitles[res.check] + " (" + res.rows.length + ")"
        }

        // Plain-text report for the clipboard (QCK-F16): same content as shown.
        function reportText() {
            var lines = [qsTr("Quality check"), qsTr("Collection") + ": " + collectionLabel(), ""]
            for (var i = 0; i < results.length; ++i) {
                var res = results[i]
                lines.push(sectionTitle(res))
                if (res.error)
                    lines.push("    " + qsTr("Error: ") + res.error)
                else if (res.rows.length === 0)
                    lines.push("    " + qsTr("Nothing found"))
                for (var j = 0; j < res.rows.length; ++j)
                    lines.push("    " + formatRow(res.check, res.rows[j]))
                lines.push("")
            }
            if (issueCount === 0)
                lines.push(qsTr("No issues found."))
            return lines.join("\n")
        }

        function collectionLabel() {
            var mode = appManager1.databaseMode
            if (mode === "Memory") return appManager1.getCollectionFolder()
            if (mode === "File")   return appManager1.getDatabaseFilePath()
            if (mode === "Hosted") return appManager1.getHostName() + "/" + appManager1.getDatabaseName()
            return "—"
        }

        title: qsTr("Quality check")
        width: Math.min(applicationWindow().width - Kirigami.Units.gridUnit * 4, Kirigami.Units.gridUnit * 40)
        preferredHeight: Kirigami.Units.gridUnit * 32
        standardButtons: Kirigami.Dialog.Close

        // Copy is an extra action, not the accept button: leading slot, as in
        // MetadataDialog (SpecValidationRules D4).
        footerLeadingComponent: Controls.Button {
            text: qsTr("Copy to Clipboard")
            icon.name: "edit-copy"
            onClicked: {
                appManager1.copyToClipboard(qualityCheckDialog.reportText())
                applicationWindow().showPassiveNotification(qsTr("Copied"))
            }
        }

        contentItem: Controls.ScrollView {
            id: qualityCheckScrollView
            clip: true

            ColumnLayout {
                width: qualityCheckScrollView.availableWidth
                spacing: Kirigami.Units.smallSpacing

                Controls.Label {
                    text: qualityCheckDialog.collectionLabel()
                    elide: Text.ElideMiddle
                    opacity: 0.7
                    Layout.fillWidth: true
                    Layout.margins: Kirigami.Units.largeSpacing
                    Layout.bottomMargin: 0
                }

                // Overall answer for a clean collection (QCK-F7)
                Kirigami.InlineMessage {
                    visible: qualityCheckDialog.issueCount === 0
                    type: Kirigami.MessageType.Positive
                    text: qsTr("No issues found.")
                    Layout.fillWidth: true
                    Layout.leftMargin: Kirigami.Units.largeSpacing
                    Layout.rightMargin: Kirigami.Units.largeSpacing
                }

                Repeater {
                    model: qualityCheckDialog.results
                    delegate: ColumnLayout {
                        id: checkSection
                        required property var modelData
                        Layout.fillWidth: true
                        Layout.leftMargin: Kirigami.Units.largeSpacing
                        Layout.rightMargin: Kirigami.Units.largeSpacing
                        Layout.topMargin: Kirigami.Units.smallSpacing
                        spacing: 2

                        Controls.Label {

                            text: qualityCheckDialog.sectionTitle(checkSection.modelData)
                            wrapMode: Text.WordWrap
                            // Check 4 is informational (QCK-F8): not coloured as a defect
                            color: checkSection.modelData.error || (checkSection.modelData.rows.length > 0 && checkSection.modelData.check !== 4)
                                   ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
                            Layout.fillWidth: true
                        }
                        // Per-check clean answer (QCK-F15)
                        Controls.Label {
                            visible: !checkSection.modelData.error && checkSection.modelData.rows.length === 0
                            text: qsTr("Nothing found")
                            opacity: 0.7
                            Layout.leftMargin: Kirigami.Units.largeSpacing
                        }
                        Controls.Label {
                            visible: !!checkSection.modelData.error
                            text: qsTr("Error: ") + checkSection.modelData.error
                            wrapMode: Text.WordWrap
                            color: Kirigami.Theme.negativeTextColor
                            Layout.fillWidth: true
                            Layout.leftMargin: Kirigami.Units.largeSpacing
                        }
                        Repeater {
                            model: checkSection.modelData.rows
                            delegate: Controls.Label {
                                required property var modelData
                                text: qualityCheckDialog.formatRow(checkSection.modelData.check, modelData)
                                wrapMode: Text.WordWrap
                                textFormat: Text.PlainText
                                Layout.fillWidth: true
                                Layout.leftMargin: Kirigami.Units.largeSpacing
                            }
                        }
                    }
                }
            }
        }
    }

    FolderDialog {
        id: imageFolderDialog
        onAccepted: appManager1.imageFolderPath = appManager1.pathFromFileUrl(selectedFolder.toString())
    }

    // Collection export (SpecCollectionExport.md CEX-F3 / CEX-F4)
    FileDialog {
        id: exportFileDialog
        title: qsTr("Select")
        fileMode: FileDialog.SaveFile
        onAccepted: appManager1.exportToSQLiteFile(appManager1.pathFromFileUrl(selectedFile.toString()))
    }

    FolderDialog {
        id: exportFolderDialog
        title: qsTr("Select a folder")
        onAccepted: appManager1.exportToMemoryMode(appManager1.pathFromFileUrl(selectedFolder.toString()))
    }

    Connections {
        target: appManager1
        function onExportCompleted(targetPath) {
            applicationWindow().showPassiveNotification(qsTr("Export") + ": " + targetPath)
        }
    }

    FileDialog {
        id: importFileDialog
        fileMode: FileDialog.OpenFile
        onAccepted: {
            var path = appManager1.pathFromFileUrl(selectedFile.toString())
            importPathField.text = path
            if (importModeCombo.currentIndex < 2)
                appManager1.openImportSource(path)
        }
    }

    FolderDialog {
        id: importFolderDialog
        onAccepted: {
            var path = appManager1.pathFromFileUrl(selectedFolder.toString())
            importPathField.text = path
            appManager1.openImportSource(path)
        }
    }

    actions: [
        Kirigami.Action {
            id: escapeCloseAction
            text:        qsTr("Close")
            icon.name:   "view-close"
            displayHint: Kirigami.DisplayHint.KeepVisible
            onTriggered: pageStack.layers.pop()
        }
    ]

    footer: ColumnLayout {
        visible: importStatusRow.visible || exportStatusRow.visible
        spacing: 0
        RowLayout {
            id: importStatusRow
            visible: appManager1.importIsRunning || appManager1.importStatusText.length > 0
            spacing: Kirigami.Units.smallSpacing
            Controls.BusyIndicator {
                running: appManager1.importIsRunning
                visible: appManager1.importIsRunning
                implicitWidth:  Kirigami.Units.gridUnit * 1.5
                implicitHeight: Kirigami.Units.gridUnit * 1.5
                Layout.leftMargin: Kirigami.Units.smallSpacing
            }
            Controls.Label {
                Layout.fillWidth: true
                Layout.margins: Kirigami.Units.smallSpacing
                text: appManager1.importStatusText
                elide: Text.ElideRight
            }
        }
        // Collection export progress and result (CEX-F9 to CEX-F11)
        RowLayout {
            id: exportStatusRow
            visible: appManager1.exportIsRunning || appManager1.exportStatusText.length > 0
            spacing: Kirigami.Units.smallSpacing
            Controls.BusyIndicator {
                running: appManager1.exportIsRunning
                visible: appManager1.exportIsRunning
                implicitWidth:  Kirigami.Units.gridUnit * 1.5
                implicitHeight: Kirigami.Units.gridUnit * 1.5
                Layout.leftMargin: Kirigami.Units.smallSpacing
            }
            Controls.Label {
                Layout.fillWidth: true
                Layout.margins: Kirigami.Units.smallSpacing
                text: appManager1.exportStatusText
                elide: Text.ElideRight
            }
            Controls.Button {
                text: qsTr("Cancel")
                icon.name: "dialog-cancel"
                visible: appManager1.exportIsRunning
                Layout.rightMargin: Kirigami.Units.smallSpacing
                onClicked: appManager1.cancelExport()
            }
        }
    }

    // Two tabs: Application first, Collection second (SET-F1). The drawer
    // Settings entry and the startup restore open Application (SET-F4);
    // Open... > Hosted Database... opens Collection (SET-F5). Not persisted (SET-C3).
    header: Controls.TabBar {
        id: settingsTabBar
        // Equal shares of the full width on every style: Breeze sizes tab
        // buttons to their text, Fusion already shares the width.
        Controls.TabButton { text: qsTr("Application"); width: settingsTabBar.width / settingsTabBar.count }
        Controls.TabButton { text: qsTr("Collection");  width: settingsTabBar.width / settingsTabBar.count }
    }

    // Label/field rows are laid out by Kirigami.FormLayout (PGL-F15/F17): the
    // label sits beside its field when there is room and above it when the page
    // is narrow, as FormLayout decides — no width threshold of ours (PGL-C11).
    // Long checkbox texts wrap and button rows wrap (PGL-F16).
    ColumnLayout {
        id: formColumn
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: Kirigami.Units.gridUnit
        anchors.rightMargin: Kirigami.Units.gridUnit

        // Collection tab (SET-F3). Its FormLayouts are twinned so all sections
        // share one label column (PGL-C13).
        ColumnLayout {
            visible: settingsTabBar.currentIndex === 1
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            // ── Collection & Database ──────────────────────────────────────
            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: Kirigami.Units.smallSpacing
                spacing: Kirigami.Units.largeSpacing
                Controls.Label { text: qsTr("Collection & Database"); color: applicationWindow().titleColor }
                Kirigami.Separator { Layout.fillWidth: true; Layout.alignment: Qt.AlignVCenter }
            }

            Kirigami.FormLayout {
                id: collectionForm
                Layout.fillWidth: true
                // Full page width (PGL-F20): in wide mode Kirigami sizes its grid to
                // implicitWidth and centres it, so the full width keeps it on the
                // left (PGL-F19) and lets the path fields stretch.
                implicitWidth: formColumn.width
                twinFormLayouts: [hostedForm, importForm]

                Controls.Label {
                    Kirigami.FormData.label: qsTr("Database Mode")
                    text: {
                        var m = appManager1.databaseMode
                        if (m === "Memory") return qsTr("Memory")
                        if (m === "File")   return qsTr("File")
                        if (m === "Hosted") return qsTr("Hosted")
                        return "—"
                    }
                }

                RowLayout {
                    Kirigami.FormData.label: qsTr("Collection")
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing
                    Controls.Label {
                        text: {
                            var mode = appManager1.databaseMode
                            if (mode === "Memory") return appManager1.getCollectionFolder() || qsTr("(none)")
                            if (mode === "File")   return appManager1.getDatabaseFilePath() || qsTr("(none)")
                            if (mode === "Hosted") return appManager1.getHostName() + "/" + appManager1.getDatabaseName()
                            return "—"
                        }
                        elide: Text.ElideMiddle
                        Layout.fillWidth: true
                        // The elided path must not size the form (PGL-F16)
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 6
                    }
                    Controls.Button {
                        text: qsTr("Edit")
                        icon.name: "document-edit"
                        visible: appManager1.databaseMode === "File"
                        onClicked: appManager1.openDatabaseFile()
                    }
                    Controls.Button {
                        text: qsTr("Edit")
                        icon.name: "document-edit"
                        visible: appManager1.databaseMode === "Memory"
                        onClicked: appManager1.openFolder(appManager1.getCollectionFolder())
                    }
                    Controls.Button {
                        text: qsTr("Edit")
                        icon.name: "document-edit"
                        visible: appManager1.databaseMode === "Hosted"
                        onClicked: Qt.openUrlExternally(appManager1.getPhpMyAdminUrl()
                                                        + encodeURIComponent(appManager1.getDatabaseName()))
                    }
                }

                RowLayout {
                    Kirigami.FormData.label: qsTr("Database Version")
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing
                    Controls.Label {
                        text: appManager1.databaseSchemaVersion || "—"
                        Layout.fillWidth: true
                    }
                    // Quality Check, SpecQualityCheck.md QCK-F13
                    Controls.Button {
                        text: qsTr("Quality check")
                        icon.name: "checkmark"
                        onClicked: qualityCheckDialog.run()
                    }
                }

                // ── Export to another mode (SpecCollectionExport.md CEX-F1, CEX-F12) ──
                // A Flow: the second button moves to the next line when both do not fit (PGL-F16).
                Flow {
                    id: exportRow
                    readonly property bool exportAllowed: !appManager1.exportIsRunning
                                                          && !appManager1.searchIsRunning
                                                          && !appManager1.catalogIsCreating
                                                          && !appManager1.deviceUpdateIsRunning
                    Layout.fillWidth: true
                    Layout.preferredWidth: pageSettingsRoot.fitWidth(childrenRect.width)
                    spacing: Kirigami.Units.smallSpacing
                    Controls.Button {
                        text: qsTr("Export to SQLite file")
                        icon.name: "document-export"
                        visible: appManager1.databaseMode === "Memory" || appManager1.databaseMode === "Hosted"
                        enabled: exportRow.exportAllowed
                        onClicked: {
                            // Browse folder: Hosted mode has no collection folder
                            var folder = appManager1.getCollectionBrowsePath()
                            exportFileDialog.currentFolder = appManager1.pathToFileUrl(folder)
                            exportFileDialog.selectedFile  = appManager1.pathToFileUrl(folder + "/export.db")
                            exportFileDialog.open()
                        }
                    }
                    Controls.Button {
                        text: qsTr("Export to Memory mode (csv)")
                        icon.name: "document-export"
                        visible: appManager1.databaseMode === "File" || appManager1.databaseMode === "Hosted"
                        enabled: exportRow.exportAllowed
                        onClicked: {
                            exportFolderDialog.currentFolder = appManager1.pathToFileUrl(appManager1.getCollectionBrowsePath())
                            exportFolderDialog.open()
                        }
                    }
                }

                // ── Images folder ──────────────────────────────────────────
                RowLayout {
                    Kirigami.FormData.label: qsTr("Images folder")
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing
                    Controls.TextField {
                        id: imageFolderField
                        text: appManager1.imageFolderPath
                        Layout.fillWidth: true
                        onEditingFinished: appManager1.imageFolderPath = text
                    }
                    Controls.Button {
                        text: qsTr("Select")
                        icon.name: "edit-select"
                        onClicked: {
                            imageFolderDialog.currentFolder = appManager1.pathToFileUrl(imageFolderField.text || appManager1.getCollectionFolder())
                            imageFolderDialog.open()
                        }
                    }
                }
            }

            // ── Hosted database fields (only when Hosted mode) ─────────────
            // Refused host (HST-F4): blocking validation error at the top of the form
            Kirigami.InlineMessage {
                id: invalidHostMessage
                Layout.fillWidth: true
                Layout.topMargin: Kirigami.Units.largeSpacing
                type: Kirigami.MessageType.Warning
                showCloseButton: true
                visible: false
                text: qsTr("Invalid hostname.") + "\n"
                      + qsTr("Only local or private network allowed.") + "\n"
                      + qsTr("Valid:") + " localhost, 127.x.x.x, 192.168.x.x, 10.x.x.x, 172.16-31.x.x"
            }

            Kirigami.FormLayout {
                id: hostedForm
                Layout.fillWidth: true
                // Full page width (PGL-F20): in wide mode Kirigami sizes its grid to
                // implicitWidth and centres it, so the full width keeps it on the
                // left (PGL-F19) and lets the path fields stretch.
                implicitWidth: formColumn.width
                Layout.topMargin: Kirigami.Units.largeSpacing
                visible: appManager1.databaseMode === "Hosted" || showHostedForm
                twinFormLayouts: [collectionForm, importForm]

                Controls.TextField {
                    id: hostField
                    Kirigami.FormData.label: qsTr("Host Name")
                    Layout.fillWidth: true
                    onTextEdited: invalidHostMessage.visible = false
                }
                Controls.TextField {
                    id: dbNameField
                    Kirigami.FormData.label: qsTr("Database Name")
                    Layout.fillWidth: true
                }
                Controls.SpinBox {
                    id: portField
                    Kirigami.FormData.label: qsTr("Port")
                    from: 1; to: 65535; value: 3306
                }
                Controls.TextField {
                    id: userField
                    Kirigami.FormData.label: qsTr("User Name")
                    Layout.fillWidth: true
                }
                Controls.TextField {
                    id: passwordField
                    Kirigami.FormData.label: qsTr("Password")
                    echoMode: TextInput.Password
                    Layout.fillWidth: true
                }
                Controls.TextField {
                    id: phpMyAdminUrlField
                    Kirigami.FormData.label: qsTr("phpMyAdmin URL")
                    text: appManager1.getPhpMyAdminUrl()
                    Layout.fillWidth: true
                    onEditingFinished: appManager1.setPhpMyAdminUrl(text)
                }
                Controls.Button {
                    text: qsTr("Connect")
                    icon.name: "network-connect"
                    enabled: dbNameField.text.length > 0
                    onClicked: pageSettingsRoot.requestHostedConnect()
                }
            }

            // ── Import ─────────────────────────────────────────────────────
            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: Kirigami.Units.largeSpacing * 2
                spacing: Kirigami.Units.largeSpacing
                Controls.Label { text: qsTr("Collection Import & Synchronization"); color: applicationWindow().titleColor }
                Kirigami.Separator { Layout.fillWidth: true; Layout.alignment: Qt.AlignVCenter }
            }

            Kirigami.FormLayout {
                id: importForm
                Layout.fillWidth: true
                // Full page width (PGL-F20): in wide mode Kirigami sizes its grid to
                // implicitWidth and centres it, so the full width keeps it on the
                // left (PGL-F19) and lets the path fields stretch.
                implicitWidth: formColumn.width
                twinFormLayouts: [collectionForm, hostedForm]

                ComboBoxFitted {
                    id: importModeCombo
                    Kirigami.FormData.label: qsTr("Data mode")
                    model: ["Katalog / " + qsTr("File"), "Katalog / " + qsTr("Memory"), "VVV / " + qsTr("Tab Separated Values")]
                    Layout.fillWidth: true
                    Layout.maximumWidth: Number.POSITIVE_INFINITY  // stretches with the form (CBX-C7)
                    Layout.preferredWidth: pageSettingsRoot.fitWidth(implicitWidth)
                    onCurrentIndexChanged: importPathField.text = ""
                }

                RowLayout {
                    Kirigami.FormData.label: qsTr("Source")
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing
                    Controls.TextField {
                        id: importPathField
                        placeholderText: importModeCombo.currentIndex === 2 ? qsTr("VVV export file (.tsv)") : qsTr("Path")
                        Layout.fillWidth: true
                        readOnly: true
                    }
                    Controls.Button {
                        text: qsTr("Select")
                        icon.name: "edit-select"
                        onClicked: {
                            if (importModeCombo.currentIndex === 0) {
                                importFileDialog.currentFolder = appManager1.pathToFileUrl(appManager1.getCollectionFolder())
                                importFileDialog.open()
                            } else if (importModeCombo.currentIndex === 1) {
                                importFolderDialog.currentFolder = appManager1.pathToFileUrl(appManager1.getCollectionFolder())
                                importFolderDialog.open()
                            } else {
                                importFileDialog.currentFolder = appManager1.pathToFileUrl(appManager1.getCollectionFolder())
                                importFileDialog.open()
                            }
                        }
                    }
                }

                RowLayout {
                    Kirigami.FormData.label: qsTr("Device")
                    Layout.fillWidth: true
                    Layout.preferredWidth: pageSettingsRoot.fitWidth(implicitWidth)
                    spacing: Kirigami.Units.smallSpacing
                    DeviceTreeComboBox {
                        id: importDeviceCombo
                        Layout.fillWidth: true
                        Layout.maximumWidth: Number.POSITIVE_INFINITY  // stretches with the form (CBX-C7)
                        sourceModel: appManager1.importSourceDeviceModel
                        // The import source picker must not track the app-selected device:
                        // source-collection IDs are independent, so following the app
                        // selection could auto-select the wrong source device by ID collision.
                        followAppSelection: false
                        enabled: importModeCombo.currentIndex < 2 && !appManager1.importIsRunning
                    }
                    Controls.Button {
                        text: qsTr("Import")
                        icon.name: "document-import"
                        enabled: !appManager1.importIsRunning && (
                            importModeCombo.currentIndex === 2
                                ? importPathField.text.length > 0
                                : importDeviceCombo.selectedDeviceName.length > 0
                        )
                        onClicked: {
                            if (importModeCombo.currentIndex === 2) {
                                var err = appManager1.importFromVVV(importPathField.text)
                                if (err.length > 0)
                                    importErrorMessage.text = err
                                importErrorMessage.visible = err.length > 0
                            } else {
                                appManager1.importDevice(importDeviceCombo.selectedDeviceId)
                            }
                        }
                    }
                }

                RowLayout {
                    Kirigami.FormData.label: qsTr("Update")
                    Layout.fillWidth: true
                    Layout.preferredWidth: pageSettingsRoot.fitWidth(implicitWidth)
                    spacing: Kirigami.Units.smallSpacing
                    ComboBoxFitted {
                        id: importUpdateSourceCombo
                        Layout.fillWidth: true
                        Layout.maximumWidth: Number.POSITIVE_INFINITY  // stretches with the form (CBX-C7)
                        enabled: importModeCombo.currentIndex < 2 && !appManager1.importIsRunning
                    }
                    Controls.Button {
                        text: qsTr("Update")
                        icon.name: "view-refresh"
                        enabled: importModeCombo.currentIndex < 2 && importUpdateSourceCombo.count > 0 && !appManager1.importIsRunning
                        onClicked: appManager1.updateAllImportsFromSource(importUpdateSourceCombo.currentText)
                    }
                }

                Kirigami.InlineMessage {
                    id: importErrorMessage
                    Layout.fillWidth: true
                    Layout.preferredWidth: pageSettingsRoot.fitWidth(implicitWidth)
                    type: Kirigami.MessageType.Error
                    visible: false
                    showCloseButton: true
                }

                // Update report — file-count and size changes after a Collection Update.
                Kirigami.InlineMessage {
                    id: importReportMessage
                    Layout.fillWidth: true
                    Layout.preferredWidth: pageSettingsRoot.fitWidth(implicitWidth)
                    type: Kirigami.MessageType.Positive
                    text: appManager1.importReportText
                    visible: appManager1.importReportText.length > 0
                    showCloseButton: true
                }
            }
        }

        // Application tab (SET-F2): sections "Interface" and "App information".
        // Its FormLayouts are twinned so both sections share one label column
        // (PGL-C13).
        ColumnLayout {
            visible: settingsTabBar.currentIndex === 0
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            // ── Interface ──────────────────────────────────────────────────
            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: Kirigami.Units.smallSpacing
                spacing: Kirigami.Units.largeSpacing
                Controls.Label { text: qsTr("Interface"); color: applicationWindow().titleColor }
                Kirigami.Separator { Layout.fillWidth: true; Layout.alignment: Qt.AlignVCenter }
            }

            Kirigami.FormLayout {
                id: interfaceForm
                Layout.fillWidth: true
                Layout.maximumWidth: implicitWidth  // left-aligned, never centred (PGL-F19)
                twinFormLayouts: [appInfoForm]

                ComboBoxFitted {
                    id: languageComboBox
                    Kirigami.FormData.label: qsTr("Language")
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 12
                    Layout.maximumWidth: Kirigami.Units.gridUnit * 12  // set width, same as Theme; shrinks only to fit (CBX-C7)
                    rowLeadingWidth: 20 + Kirigami.Units.smallSpacing  // flag
                    textRole: "displayName"
                    valueRole: "code"
                    // Suppress the style's built-in text so only the custom
                    // contentItem (flag + label) renders — otherwise the selected
                    // language name is drawn twice, overlapping.
                    displayText: ""

                    model: appManager1.getLanguageList()

                    contentItem: RowLayout {
                        function positionToRectangle(pos) { return Qt.rect(0, 0, 0, 0) }
                        property int selectionStart: 0
                        spacing: Kirigami.Units.smallSpacing
                        Image {
                            source: languageComboBox.currentIndex >= 0
                                    ? "qrc" + languageComboBox.model[languageComboBox.currentIndex].flagPath
                                    : ""
                            width:  20; height: 14
                            fillMode: Image.PreserveAspectFit
                            sourceSize.width: 20; sourceSize.height: 14
                        }
                        Controls.Label {
                            text: languageComboBox.currentIndex >= 0
                                  ? languageComboBox.model[languageComboBox.currentIndex].displayName
                                  : ""
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                    }

                    delegate: Controls.ItemDelegate {
                        width: ListView.view ? ListView.view.width : languageComboBox.width  // fills the list, which may be wider than the box
                        contentItem: RowLayout {
                            spacing: Kirigami.Units.smallSpacing
                            Image {
                                source: "qrc" + modelData.flagPath
                                width:  20; height: 14
                                fillMode: Image.PreserveAspectFit
                                sourceSize.width: 20; sourceSize.height: 14
                            }
                            Controls.Label {
                                text: modelData.displayName
                                Layout.fillWidth: true
                            }
                        }
                        highlighted: languageComboBox.highlightedIndex === index
                    }

                    Component.onCompleted: {
                        // Resolved by code, never by list position: the order is
                        // arbitrary and its first entry is Bulgarian, so a code that
                        // is not found must not leave the box sitting on index 0 —
                        // one click there stored Bulgarian for good (LNG-C1). A
                        // stored code absent from the list has already been
                        // sanitised to en_US at startup (LNG-F3), so en_US is the
                        // language actually in effect and what this shows (LNG-F5).
                        var lang = appManager1.getCurrentLanguage()
                        var langs = appManager1.getLanguageList()
                        var found = -1
                        var english = -1
                        for (var i = 0; i < langs.length; i++) {
                            if (langs[i].code === lang)    found = i
                            if (langs[i].code === "en_US") english = i
                        }
                        currentIndex = found >= 0 ? found : english
                    }

                    onActivated: {
                        appManager1.setLanguage(currentValue)
                    }

                    // The 30-language list would otherwise span the full window
                    // height. The popup opens below the field, and is pushed up
                    // when it does not fit there, so cap its height to the larger
                    // of the space below the field and the space above it minus a
                    // header clearance — this keeps the top below the page header
                    // bar and tab bar. The list scrolls when taller than the cap.
                    Binding {
                        target: languageComboBox.popup
                        property: "height"
                        value: {
                            var top = languageComboBox.mapToItem(null, 0, 0).y
                            var above = top - Kirigami.Units.gridUnit * 3 - settingsTabBar.height
                            var below = (languageComboBox.Window.window ? languageComboBox.Window.window.height : 0)
                                        - top - languageComboBox.height - Kirigami.Units.gridUnit
                            return Math.min(languageComboBox.popup.contentItem.implicitHeight
                                            + languageComboBox.popup.topPadding
                                            + languageComboBox.popup.bottomPadding,
                                            Math.max(above, below))
                        }
                    }
                }

                ComboBoxFitted {
                    id: themeComboBox
                    Kirigami.FormData.label: qsTr("Theme")
                    // Listed with the two desktop variants together, but the
                    // stored numbers are unchanged: 0 Desktop Theme, 1 Katalog
                    // Colors, 2 Desktop Theme (gray). 0 and 1 are K2's own
                    // values in Settings/Theme, so a choice made there still
                    // means the same thing here.
                    model: [
                        { text: qsTr("Desktop Theme"),        value: 0 },
                        { text: qsTr("Desktop Theme (gray)"), value: 2 },
                        { text: qsTr("Katalog Colors"),       value: 1 }
                    ]
                    textRole: "text"
                    valueRole: "value"
                    // Set once the model exists rather than bound: as a binding,
                    // indexOfValue() ran before the model was ready and returned
                    // -1, leaving the box blank. Same approach as the language
                    // combo above.
                    Component.onCompleted: currentIndex = indexOfValue(appManager1.themeId)
                    onActivated: appManager1.themeId = currentValue

                    // Follow the setting if it changes elsewhere.
                    Connections {
                        target: appManager1
                        function onThemeIdChanged() {
                            themeComboBox.currentIndex =
                                themeComboBox.indexOfValue(appManager1.themeId)
                        }
                    }
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 12
                    Layout.maximumWidth: Kirigami.Units.gridUnit * 12  // set width, shrinks only to fit (CBX-C7)
                }

                // App-wide text size (TYP-F9): 0.8 to 1.2, step 0.1.
                RowLayout {
                    Kirigami.FormData.label: qsTr("Text size")
                    Layout.fillWidth: true
                    Layout.preferredWidth: pageSettingsRoot.fitWidth(implicitWidth)
                    spacing: Kirigami.Units.smallSpacing
                    Controls.ToolButton {
                        icon.name: "zoom-out"
                        Controls.ToolTip.text: qsTr("Decrease text size")
                        Controls.ToolTip.visible: hovered
                        enabled: textSizeSlider.value > textSizeSlider.from
                        onClicked: pageSettingsRoot.textScaleEdited(
                                       Math.max(textSizeSlider.from, textSizeSlider.value - textSizeSlider.stepSize))
                    }
                    Controls.Slider {
                        id: textSizeSlider
                        from: 0.8
                        to: 1.2
                        stepSize: 0.1
                        snapMode: Controls.Slider.SnapAlways
                        value: pageSettingsRoot.textScale
                        onMoved: pageSettingsRoot.textScaleEdited(value)
                        // Shrinks with a narrow page rather than widening it (PGL-C11)
                        Layout.fillWidth: true
                        Layout.maximumWidth: Kirigami.Units.gridUnit * 8
                    }
                    Controls.ToolButton {
                        icon.name: "zoom-in"
                        Controls.ToolTip.text: qsTr("Increase text size")
                        Controls.ToolTip.visible: hovered
                        enabled: textSizeSlider.value < textSizeSlider.to
                        onClicked: pageSettingsRoot.textScaleEdited(
                                       Math.min(textSizeSlider.to, textSizeSlider.value + textSizeSlider.stepSize))
                    }
                    Item { Layout.fillWidth: true }
                }
                // K2's own option, on K2's own key, so a choice made in either
                // version is read by the other (THM-F7 / THM-C8). K2 applies it to
                // its eight tree views; K3 consumes it in the Devices cards for now
                // (THM-C10).
                Controls.CheckBox {
                    Kirigami.FormData.label: qsTr("Icon size")
                    text: qsTr("Use bigger icon size")
                    checked: appManager1.biggerIconSize
                    onToggled: appManager1.biggerIconSize = checked
                    Layout.fillWidth: true
                    Layout.preferredWidth: pageSettingsRoot.fitWidth(implicitWidth)
                    Component.onCompleted: contentItem.wrapMode = Text.WordWrap  // PGL-F16
                }
                // Back to the default window size (WIN-F4, SpecWindowGeometry.md).
                // Icon of the Search Reset action; tooltip = the size, untranslated (WIN-C5).
                Controls.Button {
                    Kirigami.FormData.label: qsTr("Window size")
                    text: qsTr("Reset window size")
                    icon.name: "edit-clear-history"
                    Controls.ToolTip.text: appManager1.getDefaultWindowSizeText()
                    Controls.ToolTip.visible: hovered
                    onClicked: applicationWindow().resetWindowSize()
                }

                Controls.CheckBox {
                    id: caseSensitiveCheckBox
                    Kirigami.FormData.label: qsTr("Behavior")
                    text: qsTr("File sorting is Case Sensitive")
                    checked: appManager1.fileSortCaseSensitive
                    onCheckedChanged: appManager1.fileSortCaseSensitive = checked
                    Controls.ToolTip.visible: hovered
                    Controls.ToolTip.text: qsTr("If enabled, the sorting will respect case sensitive sorting, so as to have this order AA, AB, AC, Aa, Ab, Ac")
                    Layout.fillWidth: true
                    Layout.preferredWidth: pageSettingsRoot.fitWidth(implicitWidth)
                    Component.onCompleted: contentItem.wrapMode = Text.WordWrap  // PGL-F16
                }
                Controls.CheckBox {
                    text: qsTr("Refresh device status when returning to the application")
                    checked: appManager1.refreshDeviceStatusOnActivation
                    onToggled: appManager1.refreshDeviceStatusOnActivation = checked
                    Layout.fillWidth: true
                    Layout.preferredWidth: pageSettingsRoot.fitWidth(implicitWidth)
                    Component.onCompleted: contentItem.wrapMode = Text.WordWrap  // PGL-F16
                }
                Controls.CheckBox {
                    text: qsTr("Authorize actions that delete files (no recovery)")
                    checked: appManager1.allowFileDeletion
                    onToggled: appManager1.allowFileDeletion = checked
                    Layout.fillWidth: true
                    Layout.preferredWidth: pageSettingsRoot.fitWidth(implicitWidth)
                    Component.onCompleted: contentItem.wrapMode = Text.WordWrap  // PGL-F16
                }
            }

            // ── App information ────────────────────────────────────────────
            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: Kirigami.Units.largeSpacing * 2
                spacing: Kirigami.Units.largeSpacing
                Controls.Label { text: qsTr("App information"); color: applicationWindow().titleColor }
                Kirigami.Separator { Layout.fillWidth: true; Layout.alignment: Qt.AlignVCenter }
            }

            Kirigami.FormLayout {
                id: appInfoForm
                Layout.fillWidth: true
                // Full page width (PGL-F20), as on the Collection tab, so the
                // settings file path stretches.
                implicitWidth: formColumn.width
                twinFormLayouts: [interfaceForm]

                Flow {
                    Kirigami.FormData.label: qsTr("Version")
                    Kirigami.FormData.buddyFor: releaseNotesButton
                    spacing: Kirigami.Units.largeSpacing
                    Layout.fillWidth: true
                    Layout.preferredWidth: pageSettingsRoot.fitWidth(childrenRect.width)
                    // Same height as the button beside them, text centred on it.
                    Controls.Label { text: About.version;
                                     height: releaseNotesButton.height; verticalAlignment: Text.AlignVCenter }
                    Controls.Label { text: appManager1.appReleaseDate; opacity: 0.7
                                     height: releaseNotesButton.height; verticalAlignment: Text.AlignVCenter }
                    Controls.Button {
                        id: releaseNotesButton
                        text: qsTr("Release Notes")
                        icon.name: "view-list-text"
                        onClicked: Qt.openUrlExternally("https://github.com/StephaneCouturier/Katalog/releases")
                    }
                }
                Controls.CheckBox {
                    text: qsTr("Check for a new version on startup")
                    checked: appManager1.checkVersionChoice
                    onCheckedChanged: appManager1.checkVersionChoice = checked
                    Layout.fillWidth: true
                    Layout.preferredWidth: pageSettingsRoot.fitWidth(implicitWidth)
                    Component.onCompleted: contentItem.wrapMode = Text.WordWrap  // PGL-F16
                }

                RowLayout {
                    Kirigami.FormData.label: qsTr("Settings file")
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing
                    Controls.Label {
                        text: appManager1.getSettingsFilePath() || qsTr("(none)")
                        elide: Text.ElideMiddle
                        Layout.fillWidth: true
                        // The elided path must not size the form (PGL-F16)
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 6
                    }
                    Controls.Button {
                        text: qsTr("Open")
                        icon.name: "document-edit"
                        onClicked: appManager1.openSettingsFile()
                    }
                }
                /*
                // ── Separator ──────────────────────────────────────────────────
                // ── Search ────────────────────────────────────────────────────
                RowLayout {
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    Layout.topMargin: Kirigami.Units.largeSpacing * 2
                    spacing: Kirigami.Units.largeSpacing
                    Controls.Label { text: qsTr("Search"); color: applicationWindow().titleColor }
                    Kirigami.Separator { Layout.fillWidth: true; Layout.alignment: Qt.AlignVCenter }
                }

                Controls.Label { text: qsTr("Layout"); opacity: 0.7; }
                Controls.CheckBox {
                    text: qsTr("Keep Selection visible with Search and Results")
                    checked: appManager1.searchKeepsSelection
                    onCheckedChanged: appManager1.searchKeepsSelection = checked
                }
                */
            }
        }
    }
}
