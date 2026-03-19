import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: pageSettingsRoot
    title: "Settings"

    // Set to true when opened from Open Collection > Hosted Db menu
    property bool showHostedForm: false

    Connections {
        target: appManager1
        function onDatabaseModeChanged() {
            if (appManager1.databaseMode !== "Hosted")
                showHostedForm = false
        }
    }

    // Populate hosted fields when the page is created (dynamic instantiation via Component)
    Component.onCompleted: {
        hostField.text         = appManager1.getHostName()
        dbNameField.text       = appManager1.getDatabaseName()
        portField.value        = appManager1.getDatabasePort()
        userField.text         = appManager1.getDatabaseUserName()
        passwordField.text     = appManager1.getDatabasePassword()
        autoConnectBox.checked = appManager1.getHostedAutoConnect()
    }

    actions: [
        Kirigami.Action {
            text: "Close"
            icon.name: "view-close"
            onTriggered: pageStack.layers.pop()
        }
    ]

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing

        // ── Current connection status ──────────────────────────────────
        Kirigami.FormLayout {
            Layout.fillWidth: true

            Controls.Label {
                Kirigami.FormData.label: "Database Mode:"
                text: appManager1.databaseMode || "—"
                font.bold: true
            }

            Controls.Label {
                Kirigami.FormData.label: "Collection:"
                text: {
                    var mode = appManager1.databaseMode
                    if (mode === "Memory") return appManager1.getCollectionFolder() || "(none)"
                    if (mode === "File")   return appManager1.getDatabaseFilePath() || "(none)"
                    if (mode === "Hosted") return appManager1.getHostName() + "/" + appManager1.getDatabaseName()
                    return "—"
                }
                elide: Text.ElideMiddle
                Layout.fillWidth: true
            }
        }

        Kirigami.Separator { Layout.fillWidth: true }

        // ── Search layout ──────────────────────────────────────────────
        Kirigami.Heading {
            level: 3
            text: "Search"
        }

        Kirigami.FormLayout {
            Layout.fillWidth: true

            Controls.CheckBox {
                Kirigami.FormData.label: "Layout:"
                text: "Keep Selection visible with Search and Results"
                checked: appManager1.searchKeepsSelection
                onCheckedChanged: appManager1.searchKeepsSelection = checked
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
            visible: appManager1.databaseMode === "Hosted" || showHostedForm
        }

        // ── Hosted database ────────────────────────────────────────────
        Kirigami.Heading {
            level: 3
            text: "Hosted Database"
            visible: appManager1.databaseMode === "Hosted" || showHostedForm
        }

        Kirigami.FormLayout {
            Layout.fillWidth: true
            visible: appManager1.databaseMode === "Hosted" || showHostedForm

            Controls.TextField {
                id: hostField
                Kirigami.FormData.label: "Host name:"
                placeholderText: "localhost"
            }

            Controls.TextField {
                id: dbNameField
                Kirigami.FormData.label: "Database name:"
                placeholderText: "katalog"
            }

            Controls.SpinBox {
                id: portField
                Kirigami.FormData.label: "Port:"
                from: 1
                to: 65535
                value: 3306
            }

            Controls.TextField {
                id: userField
                Kirigami.FormData.label: "User name:"
                placeholderText: "katalog_user"
            }

            Controls.TextField {
                id: passwordField
                Kirigami.FormData.label: "Password:"
                echoMode: TextInput.Password
            }

            Controls.CheckBox {
                id: autoConnectBox
                Kirigami.FormData.label: "Startup:"
                text: "Connect automatically on startup"
                onCheckedChanged: appManager1.setHostedAutoConnect(checked)
            }

            Controls.Button {
                Kirigami.FormData.label: " "
                text: "Connect"
                icon.name: "network-connect"
                enabled: hostField.text.length > 0 && dbNameField.text.length > 0
                onClicked: {
                    appManager1.openCollectionHosted(
                        hostField.text,
                        dbNameField.text,
                        portField.value,
                        userField.text,
                        passwordField.text
                    )
                }
            }
        }
    }
}
