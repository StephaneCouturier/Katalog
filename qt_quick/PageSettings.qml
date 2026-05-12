import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import QtQuick.Dialogs

Kirigami.ScrollablePage {
    id: pageSettingsRoot
    title: qsTr("Settings")

    // Set to true when opened from Open Collection > Hosted Db menu
    property bool showHostedForm: false
    property string importStatus: ""

    function loadImportSource(path) {
        var devices = appManager1.openImportSource(path)
        importDeviceModel.clear()
        if (devices.length === 0) {
            pageSettingsRoot.importStatus = qsTr("Error: could not open collection")
        } else {
            for (var i = 0; i < devices.length; i++)
                importDeviceModel.append(devices[i])
            pageSettingsRoot.importStatus = ""
        }
    }

    Connections {
        target: appManager1
        function onDatabaseModeChanged() {
            if (appManager1.databaseMode !== "Hosted")
                showHostedForm = false
        }
        function onImportSourceChanged() {
            importUpdateSourceCombo.model = appManager1.getImportSourcePaths()
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
            loadImportSource(path)
        }
    }

    FolderDialog {
        id: importFolderDialog
        onAccepted: {
            var path = appManager1.pathFromFileUrl(selectedFolder.toString())
            importPathField.text = path
            loadImportSource(path)
        }
    }

    ListModel { id: importDeviceModel }

    actions: [
        Kirigami.Action {
            text: qsTr("Close")
            icon.name: "view-close"
            onTriggered: pageStack.layers.pop()
        }
    ]

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
            model: [qsTr("File"), qsTr("Memory")]
            Layout.fillWidth: true
        }

        Controls.Label { text: qsTr("Import"); opacity: 0.7; Layout.alignment: Qt.AlignVCenter }
        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing
            Controls.TextField {
                id: importPathField
                placeholderText: qsTr("Path")
                Layout.fillWidth: true
            }
            Controls.Button {
                text: qsTr("Select")
                icon.name: "edit-select"
                onClicked: {
                    if (importModeCombo.currentIndex === 0) {
                        importFileDialog.currentFolder = appManager1.pathToFileUrl(appManager1.getCollectionFolder())
                        importFileDialog.open()
                    } else {
                        importFolderDialog.currentFolder = appManager1.pathToFileUrl(appManager1.getCollectionFolder())
                        importFolderDialog.open()
                    }
                }
            }
        }

        Controls.Label { opacity: 0.7; Layout.alignment: Qt.AlignVCenter }
        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing
            Controls.ComboBox {
                id: importDeviceCombo
                Layout.fillWidth: true
                model: importDeviceModel
                textRole: "name"
                valueRole: "id"
                enabled: importDeviceModel.count > 0
            }
            Controls.Button {
                text: qsTr("Import")
                icon.name: "document-import"
                enabled: importDeviceModel.count > 0
                onClicked: {
                    var result = appManager1.importDevice(importDeviceCombo.currentValue)
                    pageSettingsRoot.importStatus = result === "" ? qsTr("Completed") : result
                }
            }
            Controls.Label {
                text: pageSettingsRoot.importStatus
                visible: pageSettingsRoot.importStatus !== ""
                color: pageSettingsRoot.importStatus === qsTr("Completed")
                       ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.negativeTextColor
            }
        }

        Controls.Label { text: qsTr("Update"); opacity: 0.7; Layout.alignment: Qt.AlignVCenter }
        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing
            Controls.ComboBox {
                id: importUpdateSourceCombo
                Layout.fillWidth: true
            }
            Controls.Button {
                text: qsTr("Update")
                icon.name: "view-refresh"
                enabled: importUpdateSourceCombo.count > 0
                onClicked: {
                    var result = appManager1.updateAllImportsFromSource(importUpdateSourceCombo.currentText)
                    pageSettingsRoot.importStatus = result === "" ? qsTr("Completed") : result
                }
            }
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
        Controls.Button {
            text: qsTr("Open")
            icon.name: "document-edit"
            onClicked: appManager1.openSettingsFile()
            Layout.topMargin: Kirigami.Units.largeSpacing * 2
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
