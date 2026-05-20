import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import QtQuick.Dialogs

Kirigami.ScrollablePage {
    id: pageSettingsRoot
    title: qsTr("Settings")
    titleDelegate: Component { RowLayout { spacing: Kirigami.Units.smallSpacing; Layout.fillWidth: true; Layout.minimumWidth: 0
        Kirigami.Icon { source: "configure"; implicitWidth: Kirigami.Units.iconSizes.smallMedium; implicitHeight: Kirigami.Units.iconSizes.smallMedium }
        Kirigami.Heading { text: pageSettingsRoot.title; maximumLineCount: 1; elide: Text.ElideRight; Layout.fillWidth: true } } }

    // Set to true when opened from Open Collection > Hosted Db menu
    property bool showHostedForm: false

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
            text: qsTr("Close")
            icon.name: "view-close"
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
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.85
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
        Kirigami.Heading {
            level: 3; text: qsTr("Collection & Database")
            Layout.columnSpan: 2
            Layout.topMargin: Kirigami.Units.smallSpacing
            color: Kirigami.Theme.linkColor
        }

        Controls.Label { text: qsTr("Database Mode"); opacity: 0.7 }
        Controls.Label { text: appManager1.databaseMode || "—"; font.bold: true }

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
        }

        Controls.Label { text: qsTr("Database Version"); opacity: 0.7 }
        Controls.Label { text: appManager1.databaseSchemaVersion || "—" }

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
            text: qsTr("Host name"); opacity: 0.7
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
            text: qsTr("Database name:"); opacity: 0.7
            visible: appManager1.databaseMode === "Hosted" || showHostedForm
        }
        Controls.TextField {
            id: dbNameField
            placeholderText: "katalog"
            Layout.fillWidth: true
            visible: appManager1.databaseMode === "Hosted" || showHostedForm
        }

        Controls.Label {
            text: qsTr("Port:"); opacity: 0.7
            visible: appManager1.databaseMode === "Hosted" || showHostedForm
        }
        Controls.SpinBox {
            id: portField
            from: 1; to: 65535; value: 3306
            visible: appManager1.databaseMode === "Hosted" || showHostedForm
        }

        Controls.Label {
            text: qsTr("User name:"); opacity: 0.7
            visible: appManager1.databaseMode === "Hosted" || showHostedForm
        }
        Controls.TextField {
            id: userField
            placeholderText: "katalog_user"
            Layout.fillWidth: true
            visible: appManager1.databaseMode === "Hosted" || showHostedForm
        }

        Controls.Label {
            text: qsTr("Password:"); opacity: 0.7
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

        // ── Import ─────────────────────────────────────────────────────
        Kirigami.Heading {
            level: 4; text: qsTr("Import")
            Layout.columnSpan: 2
            Layout.topMargin: Kirigami.Units.largeSpacing * 2
            color: Kirigami.Theme.disabledTextColor
        }

        Controls.Label { text: qsTr("Data mode"); opacity: 0.7; Layout.alignment: Qt.AlignVCenter }
        Controls.ComboBox {
            id: importModeCombo
            model: [qsTr("Katalog File"), qsTr("Katalog Memory"), qsTr("VVV Tab Separated Values")]
            Layout.fillWidth: true
            onCurrentIndexChanged: importPathField.text = ""
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
                enabled: importModeCombo.currentIndex < 2 && selectedDeviceName.length > 0 && !appManager1.importIsRunning
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

        // ── Separator ──────────────────────────────────────────────────
        Kirigami.Separator { Layout.fillWidth: true; Layout.columnSpan: 2; Layout.topMargin: Kirigami.Units.largeSpacing * 2 }

        // ── Application ────────────────────────────────────────────────
        Kirigami.Heading { level: 3; text: qsTr("Application"); Layout.columnSpan: 2; color: Kirigami.Theme.linkColor; font.bold: true }

        Controls.Label { text: qsTr("Version"); opacity: 0.7; Layout.alignment: Qt.AlignTop }
        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            Flow {
                spacing: Kirigami.Units.largeSpacing
                Layout.fillWidth: true
                Controls.Label { text: About.version; font.bold: true }
                Controls.Label { text: appManager1.appReleaseDate; opacity: 0.7 }
                Controls.Button {
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

/*
        Controls.Label { text: qsTr("Language"); opacity: 0.7; Layout.topMargin: Kirigami.Units.largeSpacing * 2; Layout.alignment: Qt.AlignTop }
        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            Layout.topMargin: Kirigami.Units.largeSpacing * 2

            Controls.ComboBox {
                id: languageComboBox
                Layout.fillWidth: true
                textRole: "displayName"
                valueRole: "code"

                model: appManager1.getLanguageList()

                contentItem: RowLayout {
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
                    var lang = appManager1.getCurrentLanguage()
                    var langs = appManager1.getLanguageList()
                    for (var i = 0; i < langs.length; i++) {
                        if (langs[i].code === lang) { currentIndex = i; break }
                    }
                }

                onActivated: {
                    appManager1.setLanguage(currentValue)
                }
            }
        }
*/
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
        Kirigami.Heading { level: 3; text: qsTr("Search"); Layout.columnSpan: 2; color: Kirigami.Theme.linkColor; }

        Controls.Label { text: qsTr("Layout"); opacity: 0.7; }
        Controls.CheckBox {
            text: qsTr("Keep Selection visible with Search and Results")
            checked: appManager1.searchKeepsSelection
            onCheckedChanged: appManager1.searchKeepsSelection = checked
        }
        */
    }
}
