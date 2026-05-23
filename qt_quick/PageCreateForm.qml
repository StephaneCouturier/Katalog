import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import QtQuick.Dialogs

ColumnLayout {
    id: pageCreateForm
    spacing: 0

    // Signals - dialogs live in Main.qml to guarantee proper window overlay
    signal validationError(string message)
    signal emptyDirConfirmNeeded()

    // Called from Main.qml Create action
    function triggerCreate() {
        if (create_lineEdit_NewCatalogName.text.trim() === "") {
            validationError(qsTr("Provide a name for this new catalog."))
            return
        }
        if (create_lineEdit_NewCatalogPath.text.trim() === "") {
            validationError(qsTr("Provide a path for this new catalog."))
            return
        }
        if (create_storageComboBox.selectedDeviceId <= 0) {
            validationError(qsTr("Select a Storage for this new catalog.\n(Selection panel on the left and dropdown list)"))
            return
        }
        if (appManager1.isDirectoryEmpty(create_lineEdit_NewCatalogPath.text.trim())) {
            emptyDirConfirmNeeded()
            return
        }
        doCreate()
    }

    // Called from Main.qml Stop action
    function triggerStop() {
        appManager1.stopCatalogCreation()
    }

    // Called by triggerCreate() and by Main.qml empty-dir confirmation dialog
    function doCreate() {
        var excludes = []
        for (var i = 0; i < pendingExcludesModel.count; i++)
            excludes.push(pendingExcludesModel.get(i).folderPath)

        var error = appManager1.createCatalog(
            create_lineEdit_NewCatalogName.text.trim(),
            create_lineEdit_NewCatalogPath.text.trim(),
            create_storageComboBox.selectedDeviceId,
            create_comboBox_FileType.currentValue,
            create_checkBox_IncludeSubDir.checked,
            create_comboBox_IncludeHidden.currentIndex === 1,
            false,
            false,
            create_comboBox_Metadata.currentValue,
            create_comboBox_Checksum.currentValue,
            excludes
        )
        if (error !== "")
            validationError(error)
    }

    // ── Per-catalog pending exclude folders ────────────────────────────────────
    ListModel { id: pendingExcludesModel }

    // ── Global exclude directories (live from appManager) ─────────────────────
    property var  globalExcludes: []
    property bool globalParamsExpanded: true

    Connections {
        target: appManager1
        function onExcludeDirectoriesChanged() {
            globalExcludes = appManager1.getExcludeDirectories()
        }
    }

    Component.onCompleted: {
        globalExcludes = appManager1.getExcludeDirectories()
    }

    // ── Folder dialogs ─────────────────────────────────────────────────────────
    FolderDialog {
        id: sourcePathDialog
        onAccepted: create_lineEdit_NewCatalogPath.text = appManager1.pathFromFileUrl(selectedFolder.toString())
    }
    FolderDialog {
        id: perCatalogExcludeDialog
        onAccepted: create_lineEdit_NewExcludeFolder.text = appManager1.pathFromFileUrl(selectedFolder.toString())
    }
    FolderDialog {
        id: globalExcludeDialog
        onAccepted: create_lineEdit_FolderToExclude.text = appManager1.pathFromFileUrl(selectedFolder.toString())
    }

    // ════════════════════════════════════════════════════════════════════════════
    // Master GridLayout - 2 columns shared across all sections
    // ════════════════════════════════════════════════════════════════════════════
    GridLayout {
        Layout.fillWidth: true
        Layout.margins: Kirigami.Units.gridUnit
        columns: 2
        columnSpacing: Kirigami.Units.largeSpacing
        rowSpacing: Kirigami.Units.smallSpacing

        // ── Section 1: Required ───────────────────────────────────────────────
        Kirigami.Heading {
            level: 3
            text: qsTr("Catalog definition")
            color: Kirigami.Theme.linkColor
            Layout.columnSpan: 2
            Layout.topMargin: Kirigami.Units.smallSpacing
        }

        Controls.Label { text: qsTr("Source path"); opacity: 0.7 }
        RowLayout {
            Layout.fillWidth: true
            Controls.TextField {
                id: create_lineEdit_NewCatalogPath
                Layout.fillWidth: true
                placeholderText: qsTr("Path to index")
            }
            Controls.Button {
                icon.name: "folder-open"
                onClicked: {
                    var p = create_lineEdit_NewCatalogPath.text.trim()
                    if (p !== "") sourcePathDialog.currentFolder = appManager1.pathToFileUrl(p)
                    sourcePathDialog.open()
                }
            }
        }

        Controls.Label { text: qsTr("Storage"); opacity: 0.7 }
        DeviceTreeComboBox {
            id: create_storageComboBox
            storageOnly: true
            hideCatalogs: true
            Layout.fillWidth: true
        }

        Controls.Label { text: qsTr("Catalog name"); opacity: 0.7 }
        RowLayout {
            Layout.fillWidth: true
            Controls.TextField {
                id: create_lineEdit_NewCatalogName
                Layout.fillWidth: true
                placeholderText: qsTr("New catalog name")
            }
            Controls.Button {
                icon.name: "tools-wizard"
                Controls.ToolTip.text: qsTr("Generate name from path")
                Controls.ToolTip.visible: hovered
                onClicked: {
                    var generated = create_lineEdit_NewCatalogPath.text
                    generated = generated.replace(/\//g, "_").replace(/:/g, "_")
                    create_lineEdit_NewCatalogName.text = generated
                }
            }
        }

        Kirigami.Separator { Layout.fillWidth: true; Layout.columnSpan: 2; Layout.topMargin: Kirigami.Units.largeSpacing }

        // ── Section 2: Catalog Options ────────────────────────────────────────
        Kirigami.Heading {
            level: 3
            text: qsTr("Content options")
            color: Kirigami.Theme.linkColor
            Layout.columnSpan: 2
            Layout.topMargin: Kirigami.Units.smallSpacing
        }

        Controls.Label { text: qsTr("File type"); opacity: 0.7 }
        Controls.ComboBox {
            id: create_comboBox_FileType
            Layout.fillWidth: true
            textRole: "text"
            valueRole: "value"
            displayText: ""
            model: [
                { text: qsTr("All"),   value: "All",   iconName: "folder"                  },
                { text: qsTr("Audio"), value: "Audio", iconName: "audio-x-mpeg"             },
                { text: qsTr("Image"), value: "Image", iconName: "image-jpeg"               },
                { text: qsTr("Text"),  value: "Text",  iconName: "view-list-text"           },
                { text: qsTr("Video"), value: "Video", iconName: "video-mp4"                },
                { text: qsTr("Other"), value: "Other", iconName: "document-open"            },
                { text: qsTr("None"),  value: "None",  iconName: "application-x-zerosize"  }
            ]
            currentIndex: 0
            delegate: Controls.ItemDelegate {
                width: create_comboBox_FileType.width
                text: modelData.text
                icon.name: modelData.iconName
                highlighted: create_comboBox_FileType.highlightedIndex === index
            }
            contentItem: RowLayout {
                function positionToRectangle(pos) { return Qt.rect(0, 0, 0, 0) }
                property int selectionStart: 0
                spacing: Kirigami.Units.smallSpacing
                Item { width: Kirigami.Units.smallSpacing }
                Kirigami.Icon {
                    source: create_comboBox_FileType.currentIndex >= 0
                            ? create_comboBox_FileType.model[create_comboBox_FileType.currentIndex].iconName : ""
                    implicitWidth:  Kirigami.Units.iconSizes.small
                    implicitHeight: Kirigami.Units.iconSizes.small
                }
                Controls.Label {
                    text: create_comboBox_FileType.currentIndex >= 0
                          ? create_comboBox_FileType.model[create_comboBox_FileType.currentIndex].text : ""
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        Controls.Label { text: qsTr("Include subdirectories"); opacity: 0.7 }
        Controls.CheckBox {
            id: create_checkBox_IncludeSubDir
            checked: true
        }

        Controls.Label { text: qsTr("Include hidden files"); opacity: 0.7 }
        Controls.ComboBox {
            id: create_comboBox_IncludeHidden
            Layout.fillWidth: true
            model: [ qsTr("None"), qsTr("All") ]
            currentIndex: 0
        }

        Controls.Label { text: qsTr("Include metadata"); opacity: 0.7 }
        Controls.ComboBox {
            id: create_comboBox_Metadata
            Layout.fillWidth: true
            textRole: "text"
            valueRole: "value"
            model: [
                { text: qsTr("None"),           value: "None"          },
                { text: qsTr("Media Basic"),     value: "MediaBasic"    },
                { text: qsTr("Media Extended"),  value: "MediaExtended" },
                { text: qsTr("Full Extended"),   value: "FullExtended"  }
            ]
            currentIndex: 0
        }

        Controls.Label { text: qsTr("Include checksum"); opacity: 0.7 }
        Controls.ComboBox {
            id: create_comboBox_Checksum
            Layout.fillWidth: true
            textRole: "text"
            valueRole: "value"
            model: [
                { text: qsTr("None"),  value: "None"   },
                { text: "SHA-256",     value: "SHA256" }
            ]
            currentIndex: 0
        }

        //Folder exclusion (catalog only)
        Controls.Label { text: qsTr("Folder to exclude"); opacity: 0.7 }
        RowLayout {
            Layout.fillWidth: true
            Controls.TextField {
                id: create_lineEdit_NewExcludeFolder
                Layout.fillWidth: true
                placeholderText: qsTr("Path to exclude")
            }
            Controls.Button {
                icon.name: "folder-open"
                onClicked: {
                    var p = create_lineEdit_NewCatalogPath.text.trim()
                    if (p !== "") perCatalogExcludeDialog.currentFolder = appManager1.pathToFileUrl(p)
                    perCatalogExcludeDialog.open()
                }
            }
            Controls.Button {
                icon.name: "list-add"
                text: qsTr("Add")
                onClicked: {
                    var p = create_lineEdit_NewExcludeFolder.text.trim()
                    if (p === "") return
                    for (var i = 0; i < pendingExcludesModel.count; i++)
                        if (pendingExcludesModel.get(i).folderPath === p) return
                    pendingExcludesModel.append({ folderPath: p })
                    create_lineEdit_NewExcludeFolder.text = ""
                }
            }
        }

        // Pending exclude folders list
        Item { Layout.preferredWidth: 1 }
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2
            Repeater {
                model: pendingExcludesModel
                delegate: RowLayout {
                    Layout.fillWidth: true
                    Controls.Label {
                        text: folderPath
                        elide: Text.ElideMiddle
                        Layout.fillWidth: true
                    }
                    Controls.Button {
                        icon.name: "edit-delete"
                        flat: true
                        onClicked: pendingExcludesModel.remove(index)
                    }
                }
            }
        }

        Kirigami.Separator { Layout.fillWidth: true; Layout.columnSpan: 2; Layout.topMargin: Kirigami.Units.largeSpacing }


        // Global Parameters (collapsible) ────────────────────────
        RowLayout {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.smallSpacing

            Kirigami.Heading {
                level: 3
                text: qsTr("Global Parameters")
                color: Kirigami.Theme.linkColor
                Layout.fillWidth: true
            }
            Controls.Button {
                id: create_button_ToggleGlobalParams
                icon.name: globalParamsExpanded ? "go-up" : "go-down"
                flat: true
                onClicked: globalParamsExpanded = !globalParamsExpanded
            }
        }

        Controls.Label {
            text: qsTr("Global exclude directory")
            opacity: 0.7
            visible: globalParamsExpanded
        }
        RowLayout {
            Layout.fillWidth: true
            visible: globalParamsExpanded
            Controls.TextField {
                id: create_lineEdit_FolderToExclude
                Layout.fillWidth: true
                placeholderText: qsTr("Path to exclude globally")
            }
            Controls.Button {
                icon.name: "folder-open"
                onClicked: {
                    var p = create_lineEdit_NewCatalogPath.text.trim()
                    if (p !== "") globalExcludeDialog.currentFolder = appManager1.pathToFileUrl(p)
                    globalExcludeDialog.open()
                }
            }
            Controls.Button {
                icon.name: "list-add"
                text: qsTr("Add")
                onClicked: {
                    var p = create_lineEdit_FolderToExclude.text.trim()
                    if (p !== "") {
                        appManager1.addExcludeDirectory(p)
                        create_lineEdit_FolderToExclude.text = ""
                    }
                }
            }
        }

        // Global excludes list
        Item {
            visible: globalParamsExpanded && globalExcludes.length > 0
            Layout.preferredWidth: 1
        }
        ColumnLayout {
            Layout.fillWidth: true
            visible: globalParamsExpanded && globalExcludes.length > 0
            spacing: 2
            Repeater {
                model: globalExcludes
                delegate: RowLayout {
                    Layout.fillWidth: true
                    Controls.Label {
                        text: modelData
                        elide: Text.ElideMiddle
                        Layout.fillWidth: true
                    }
                    Controls.Button {
                        icon.name: "edit-delete"
                        flat: true
                        onClicked: appManager1.removeExcludeDirectory(modelData)
                    }
                }
            }
        }

        // Bottom spacer
        Item { Layout.columnSpan: 2; Layout.preferredHeight: Kirigami.Units.gridUnit }
    }
}
