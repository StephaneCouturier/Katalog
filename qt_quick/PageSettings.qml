import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import QtQuick.Dialogs

Kirigami.ScrollablePage {
    id: pageSettingsRoot
    property Kirigami.Action escapeAction: escapeCloseAction  // Esc (KBS-F1)
    title: qsTr("Settings")

    // Set to true when opened from Open Collection > Hosted Db menu
    property bool showHostedForm: false

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
        hostField.text         = appManager1.getHostName()
        dbNameField.text       = appManager1.getDatabaseName()
        portField.value        = appManager1.getDatabasePort()
        userField.text         = appManager1.getDatabaseUserName()
        passwordField.text     = appManager1.getDatabasePassword()
        autoConnectBox.checked = appManager1.getHostedAutoConnect()
        importUpdateSourceCombo.model = appManager1.getImportSourcePaths()
    }

    // ── Dialogs ────────────────────────────────────────────────────────

    // Quality Check report, SpecQualityCheck.md phase 1: read-only, reports only.
    Kirigami.Dialog {
        id: qualityCheckDialog
        property var results: []   // QVariantList of {check, rows, error}

        readonly property var checkTitles: [
            qsTr("Storage devices linked to a missing storage"),
            qsTr("Storage devices linked to a storage with a different name"),
            qsTr("Storage not linked to any device"),
            qsTr("Storage IDs used by more than one storage"),
            qsTr("Storage names used by more than one storage"),
            qsTr("Catalog devices linked to a missing catalog"),
            qsTr("Catalogs not linked to any device"),
            qsTr("Devices with a missing parent device")
        ]

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
            case 2: return qsTr("Device") + " " + r[0] + " \u201C" + r[1] + "\u201D \u2192 " + qsTr("Storage") + " " + r[2] + " \u201C" + r[3] + "\u201D"
            case 3: return qsTr("Storage") + " " + r[0] + " \u201C" + r[1] + "\u201D"
            case 4: return qsTr("ID") + " " + r[0] + " \u00B7 " + qsTr("Storage") + " " + r[1] + " \u201C" + r[2] + "\u201D"
            case 5: return qsTr("Storage") + " " + r[1] + " \u201C" + r[0] + "\u201D"
            case 6: return qsTr("Device") + " " + r[0] + " \u201C" + r[1] + "\u201D \u2192 " + qsTr("Catalog") + " " + r[2]
            case 7: return qsTr("Catalog") + " " + r[0] + " \u201C" + r[1] + "\u201D"
            case 8: return qsTr("Device") + " " + r[0] + " \u201C" + r[1] + "\u201D \u2192 " + qsTr("Device") + " " + r[2]
            }
            return r.join(" \u00B7 ")
        }

        function sectionTitle(res) {
            return res.check + ". " + checkTitles[res.check - 1] + " (" + res.rows.length + ")"
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
                            font.bold: true
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

    footer: RowLayout {
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

    // Single GridLayout so column 0 width is shared across all sections
    GridLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: Kirigami.Units.gridUnit
        anchors.rightMargin: Kirigami.Units.gridUnit
        columns: 2
        columnSpacing: Kirigami.Units.largeSpacing
        rowSpacing: Kirigami.Units.smallSpacing

        // ── Collection & Database ──────────────────────────────────────
        Controls.Label {
            font.bold: true; text: qsTr("Collection & Database")
            Layout.columnSpan: 2
            Layout.topMargin: Kirigami.Units.smallSpacing
            color: Kirigami.Theme.linkColor
        }

        Controls.Label { text: qsTr("Database Mode"); opacity: 0.7; Layout.topMargin: Kirigami.Units.largeSpacing }
        Controls.Label {
            text: {
                var m = appManager1.databaseMode
                if (m === "Memory") return qsTr("Memory")
                if (m === "File")   return qsTr("File")
                if (m === "Hosted") return qsTr("Hosted")
                return "—"
            }
            font.bold: true ; Layout.topMargin: Kirigami.Units.largeSpacing
        }

        Controls.Label { text: qsTr("Collection"); opacity: 0.7 }
        RowLayout {
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

        Controls.Label { text: qsTr("Database Version"); opacity: 0.7; Layout.alignment: Qt.AlignVCenter }
        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing
            Controls.Label {
                text: appManager1.databaseSchemaVersion || "—"
                Layout.fillWidth: true
            }
            // Quality Check, SpecQualityCheck.md QCK-F13
            Controls.Button {
                text: qsTr("Quality check")
                icon.name: "tools-check-spelling"
                onClicked: qualityCheckDialog.run()
            }
        }

        // ── Images folder ──────────────────────────────────────────────
        Controls.Label { text: qsTr("Images folder"); opacity: 0.7; Layout.alignment: Qt.AlignVCenter }
        RowLayout {
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

        // ── Hosted database fields (only when Hosted mode) ─────────────
        Controls.Label {
            text: qsTr("Host Name"); opacity: 0.7
            visible: appManager1.databaseMode === "Hosted" || showHostedForm
            Layout.topMargin: Kirigami.Units.largeSpacing * 2
        }
        Controls.TextField {
            id: hostField
            placeholderText: "localhost"
            Layout.fillWidth: true
            visible: appManager1.databaseMode === "Hosted" || showHostedForm
            Layout.topMargin: Kirigami.Units.largeSpacing * 2
        }

        Controls.Label {
            text: qsTr("Database Name"); opacity: 0.7
            visible: appManager1.databaseMode === "Hosted" || showHostedForm
        }
        Controls.TextField {
            id: dbNameField
            placeholderText: "katalog"
            Layout.fillWidth: true
            visible: appManager1.databaseMode === "Hosted" || showHostedForm
        }

        Controls.Label {
            text: qsTr("Port"); opacity: 0.7
            visible: appManager1.databaseMode === "Hosted" || showHostedForm
        }
        Controls.SpinBox {
            id: portField
            from: 1; to: 65535; value: 3306
            visible: appManager1.databaseMode === "Hosted" || showHostedForm
        }

        Controls.Label {
            text: qsTr("User Name"); opacity: 0.7
            visible: appManager1.databaseMode === "Hosted" || showHostedForm
        }
        Controls.TextField {
            id: userField
            placeholderText: "katalog_user"
            Layout.fillWidth: true
            visible: appManager1.databaseMode === "Hosted" || showHostedForm
        }

        Controls.Label {
            text: qsTr("Password"); opacity: 0.7
            visible: appManager1.databaseMode === "Hosted" || showHostedForm
        }
        Controls.TextField {
            id: passwordField
            echoMode: TextInput.Password
            Layout.fillWidth: true
            visible: appManager1.databaseMode === "Hosted" || showHostedForm
        }

        Controls.Label {
            text: qsTr("Startup:"); opacity: 0.7
            visible: appManager1.databaseMode === "Hosted" || showHostedForm
        }
        Controls.CheckBox {
            id: autoConnectBox
            text: qsTr("Connect automatically on startup")
            onCheckedChanged: appManager1.setHostedAutoConnect(checked)
            visible: appManager1.databaseMode === "Hosted" || showHostedForm
        }

        Controls.Label {
            text: qsTr("phpMyAdmin URL"); opacity: 0.7
            visible: appManager1.databaseMode === "Hosted" || showHostedForm
        }
        Controls.TextField {
            id: phpMyAdminUrlField
            text: appManager1.getPhpMyAdminUrl()
            Layout.fillWidth: true
            visible: appManager1.databaseMode === "Hosted" || showHostedForm
            onEditingFinished: appManager1.setPhpMyAdminUrl(text)
        }

        Controls.Label {
            text: " "
            visible: appManager1.databaseMode === "Hosted" || showHostedForm
        }
        Controls.Button {
            text: qsTr("Connect")
            icon.name: "network-connect"
            enabled: hostField.text.length > 0 && dbNameField.text.length > 0
            visible: appManager1.databaseMode === "Hosted" || showHostedForm
            onClicked: {
                appManager1.openCollectionHosted(
                    hostField.text, dbNameField.text, portField.value,
                    userField.text, passwordField.text
                )
            }
        }

        // ── Separator ──────────────────────────────────────────────────
        Kirigami.Separator { Layout.fillWidth: true; Layout.columnSpan: 2; Layout.topMargin: Kirigami.Units.largeSpacing * 2 }

        // ── Import ─────────────────────────────────────────────────────
        Controls.Label { font.bold: true; text: qsTr("Collection Import & Synchronization"); Layout.columnSpan: 2; color: Kirigami.Theme.linkColor }

        Controls.Label { text: qsTr("Data mode"); opacity: 0.7; Layout.alignment: Qt.AlignVCenter; Layout.topMargin: Kirigami.Units.largeSpacing }
        Controls.ComboBox {
            id: importModeCombo
            model: ["Katalog / " + qsTr("File"), "Katalog / " + qsTr("Memory"), "VVV / " + qsTr("Tab Separated Values")]
            Layout.fillWidth: true
            onCurrentIndexChanged: importPathField.text = ""
            Layout.topMargin: Kirigami.Units.largeSpacing
        }

        Controls.Label { text: qsTr("Source"); opacity: 0.7; Layout.alignment: Qt.AlignVCenter }
        RowLayout {
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

        Controls.Label { text: qsTr("Device"); opacity: 0.7; Layout.alignment: Qt.AlignVCenter }
        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing
            DeviceTreeComboBox {
                id: importDeviceCombo
                Layout.fillWidth: true
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

        Controls.Label { text: qsTr("Update"); opacity: 0.7; Layout.alignment: Qt.AlignVCenter }
        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing
            Controls.ComboBox {
                id: importUpdateSourceCombo
                Layout.fillWidth: true
                enabled: importModeCombo.currentIndex < 2 && !appManager1.importIsRunning
            }
            Controls.Button {
                text: qsTr("Update")
                icon.name: "view-refresh"
                enabled: importModeCombo.currentIndex < 2 && importUpdateSourceCombo.count > 0 && !appManager1.importIsRunning
                onClicked: appManager1.updateAllImportsFromSource(importUpdateSourceCombo.currentText)
            }
        }

        Controls.Label { visible: importErrorMessage.visible }
        Kirigami.InlineMessage {
            id: importErrorMessage
            Layout.fillWidth: true
            type: Kirigami.MessageType.Error
            visible: false
            showCloseButton: true
        }

        // Update report — file-count and size changes after a Collection Update.
        Controls.Label { visible: importReportMessage.visible }
        Kirigami.InlineMessage {
            id: importReportMessage
            Layout.fillWidth: true
            Layout.columnSpan: 1
            type: Kirigami.MessageType.Positive
            text: appManager1.importReportText
            visible: appManager1.importReportText.length > 0
            showCloseButton: true
        }

        // ── Separator ──────────────────────────────────────────────────
        Kirigami.Separator { Layout.fillWidth: true; Layout.columnSpan: 2; Layout.topMargin: Kirigami.Units.largeSpacing * 2 }

        // ── Application ────────────────────────────────────────────────
        Controls.Label { font.bold: true; text: qsTr("Application"); Layout.columnSpan: 2; color: Kirigami.Theme.linkColor }

        // Row labels are centred on the first line of their column (the first
        // control), not on the whole column.
        Controls.Label { text: qsTr("Version"); opacity: 0.7; Layout.alignment: Qt.AlignTop
                         Layout.topMargin: Kirigami.Units.largeSpacing + Math.max(0, (releaseNotesButton.height - implicitHeight) / 2) }
        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            Layout.topMargin: Kirigami.Units.largeSpacing
            Flow {
                spacing: Kirigami.Units.largeSpacing
                Layout.fillWidth: true
                // Same height as the button beside them, text centred on it.
                Controls.Label { text: About.version; font.bold: true
                                 height: releaseNotesButton.height; verticalAlignment: Text.AlignVCenter }
                Controls.Label { text: appManager1.appReleaseDate; opacity: 0.7
                                 height: releaseNotesButton.height; verticalAlignment: Text.AlignVCenter }
                Controls.Button {
                    id: releaseNotesButton
                    text: qsTr("Release Notes")
                    icon.name: "view-list-text"
                    //onClicked: Qt.openUrlExternally("https://github.com/StephaneCouturier/Katalog/releases")
                    onClicked: Qt.openUrlExternally("https://stephanecouturier.github.io/Katalog/docs/Development-Roadmap#katalog-3")
                }
            }
            Controls.CheckBox {
                text: qsTr("Check for a new version on startup")
                checked: appManager1.checkVersionChoice
                onCheckedChanged: appManager1.checkVersionChoice = checked
            }
        }

        Controls.Label { text: qsTr("Behavior"); opacity: 0.7; Layout.alignment: Qt.AlignTop
                         Layout.topMargin: Kirigami.Units.largeSpacing * 2 + Math.max(0, (caseSensitiveCheckBox.height - implicitHeight) / 2) }
        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            Layout.topMargin: Kirigami.Units.largeSpacing * 2
            Controls.CheckBox {
                id: caseSensitiveCheckBox
                text: qsTr("File sorting is Case Sensitive")
                checked: appManager1.fileSortCaseSensitive
                onCheckedChanged: appManager1.fileSortCaseSensitive = checked
                Controls.ToolTip.visible: hovered
                Controls.ToolTip.text: qsTr("If enabled, the sorting will respect case sensitive sorting, so as to have this order AA, AB, AC, Aa, Ab, Ac")
            }
            Controls.CheckBox {
                text: qsTr("Refresh device status when returning to the application")
                checked: appManager1.refreshDeviceStatusOnActivation
                onCheckedChanged: appManager1.refreshDeviceStatusOnActivation = checked
            }
            Controls.CheckBox {
                text: qsTr("Allow deleting files from Katalog")
                checked: appManager1.allowFileDeletion
                onCheckedChanged: appManager1.allowFileDeletion = checked
            }
        }

        Controls.Label { text: qsTr("Theme"); opacity: 0.7; Layout.alignment: Qt.AlignTop
                         Layout.topMargin: Kirigami.Units.largeSpacing * 2 + Math.max(0, (themeComboBox.height - implicitHeight) / 2) }
        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            Layout.topMargin: Kirigami.Units.largeSpacing * 2
            RowLayout {
                spacing: Kirigami.Units.largeSpacing
                Controls.ComboBox {
                    id: themeComboBox
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
                    // combo below.
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
                }
            }
            // K2's own option, on K2's own key, so a choice made in either
            // version is read by the other (THM-F7 / THM-C8). K2 applies it to
            // its eight tree views; K3 consumes it in the Devices cards for now
            // (THM-C10).
            Controls.CheckBox {
                text: qsTr("Use bigger icon size")
                checked: appManager1.biggerIconSize
                onToggled: appManager1.biggerIconSize = checked
            }
            // App-wide text size (TYP-F9): 0.8 to 1.2, step 0.1.
            RowLayout {
                spacing: Kirigami.Units.smallSpacing
                Controls.Label { text: qsTr("Text size") }
                Controls.ToolButton {
                    icon.name: "zoom-out"
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
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 8
                }
                Controls.ToolButton {
                    icon.name: "zoom-in"
                    enabled: textSizeSlider.value < textSizeSlider.to
                    onClicked: pageSettingsRoot.textScaleEdited(
                                   Math.min(textSizeSlider.to, textSizeSlider.value + textSizeSlider.stepSize))
                }
            }
        }

        Controls.Label { text: qsTr("Language"); opacity: 0.7; Layout.alignment: Qt.AlignTop
                         Layout.topMargin: Kirigami.Units.largeSpacing * 2 + Math.max(0, (languageComboBox.height - implicitHeight) / 2) }
        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            Layout.topMargin: Kirigami.Units.largeSpacing * 2

            Controls.ComboBox {
                id: languageComboBox
                Layout.fillWidth: true
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
                    width: languageComboBox.width
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
                // height. The popup opens upward (bottom anchored above the
                // field), so cap its height to the space above the field minus a
                // header clearance — this keeps the top below the page header
                // bar. The list scrolls when taller than the cap.
                Binding {
                    target: languageComboBox.popup
                    property: "height"
                    value: Math.min(languageComboBox.popup.contentItem.implicitHeight
                                    + languageComboBox.popup.topPadding
                                    + languageComboBox.popup.bottomPadding,
                                    languageComboBox.mapToItem(null, 0, 0).y
                                    - Kirigami.Units.gridUnit * 3)
                }
            }
        }

        Controls.Label { text: qsTr("Settings file"); opacity: 0.7; Layout.topMargin: Kirigami.Units.largeSpacing * 2 }
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.largeSpacing * 2
            spacing: Kirigami.Units.smallSpacing
            Controls.Label {
                text: appManager1.getSettingsFilePath() || qsTr("(none)")
                elide: Text.ElideMiddle
                Layout.fillWidth: true
            }
            Controls.Button {
                text: qsTr("Open")
                icon.name: "document-edit"
                onClicked: appManager1.openSettingsFile()
            }
        }
        /*
        // ── Separator ──────────────────────────────────────────────────
        Kirigami.Separator { Layout.fillWidth: true; Layout.columnSpan: 2; Layout.topMargin: Kirigami.Units.largeSpacing * 2}

        // ── Search ────────────────────────────────────────────────────
        Controls.Label { font.bold: true; text: qsTr("Search"); Layout.columnSpan: 2; color: Kirigami.Theme.linkColor }

        Controls.Label { text: qsTr("Layout"); opacity: 0.7; }
        Controls.CheckBox {
            text: qsTr("Keep Selection visible with Search and Results")
            checked: appManager1.searchKeepsSelection
            onCheckedChanged: appManager1.searchKeepsSelection = checked
        }
        */
    }
}
