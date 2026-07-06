import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import QtQuick.Dialogs

ColumnLayout {
    id: root
    spacing: 0

    property int    deviceId:   0
    property string deviceType: ""
    property string originalPath: ""
    property bool   _originalIsFullDevice: false
    property var    storagePictureList:    [""]

    // Signals - dialogs live in Main.qml
    signal saveError(string message)
    signal catalogConfirmNeeded(string message, bool rescanNeeded, bool pathChanged)
    signal catalogUpdateContentNeeded()
    signal storagePathChangeNeeded(string previousPath, string newPath)
    signal saveCompleted()

    function finalizeSave() {
        appManager1.refreshDeviceList()
        saveCompleted()
    }

    function loadDevice() {
        var d = appManager1.getDeviceDetails(deviceId)
        deviceType    = d.type
        originalPath  = d.path

        edit_lineEdit_Name.text = d.name
        // Only offer parents in the same group, and never the device itself —
        // re-parenting can reorganise within a group but never cross groups.
        edit_storageComboBox.groupFilter     = d.groupId
        edit_storageComboBox.excludeDeviceId = deviceId
        edit_storageComboBox.selectById(d.parentId)

        if (d.type !== "Virtual")
            edit_lineEdit_Path.text = d.path

        if (d.type === "Catalog") {
            _originalIsFullDevice = d.isFullDevice

            for (var i = 0; i < edit_comboBox_FileType.model.length; i++) {
                if (edit_comboBox_FileType.model[i].value === d.fileType) {
                    edit_comboBox_FileType.currentIndex = i; break
                }
            }
            edit_checkBox_IncludeSubDir.checked = d.includeSubDir
            edit_comboBox_IncludeHidden.currentIndex = d.includeHidden ? 1 : 0
            for (var j = 0; j < edit_comboBox_Metadata.model.length; j++) {
                if (edit_comboBox_Metadata.model[j].value === d.includeMetadata) {
                    edit_comboBox_Metadata.currentIndex = j; break
                }
            }
            for (var k = 0; k < edit_comboBox_Checksum.model.length; k++) {
                if (edit_comboBox_Checksum.model[k].value === d.includeChecksum) {
                    edit_comboBox_Checksum.currentIndex = k; break
                }
            }

            excludeFoldersModel.clear()
            var folders = d.excludeFolders
            for (var f = 0; f < folders.length; f++)
                excludeFoldersModel.append({ folderPath: folders[f] })
        }

        if (d.type === "Storage") {
            edit_lineEdit_StorageExtId.text   = String(d.storageExtId)
            edit_lineEdit_StorageType.text    = d.storageType
            edit_lineEdit_StorageLabel.text   = d.storageLabel
            edit_lineEdit_StorageFS.text      = d.storageFileSystem
            edit_lineEdit_TotalSpace.text     = String(d.totalSpace)
            edit_lineEdit_FreeSpace.text      = String(d.freeSpace)
            edit_lineEdit_Brand.text          = d.storageBrand
            edit_lineEdit_Model.text          = d.storageModel
            edit_lineEdit_SerialNumber.text   = d.storageSerialNumber
            edit_lineEdit_BuildDate.text      = d.storageBuildDate
            edit_lineEdit_Comment1.text       = d.storageComment1
            edit_lineEdit_Comment2.text       = d.storageComment2
            edit_lineEdit_Comment3.text       = d.storageComment3

            storagePictureList = appManager1.getStoragePictureList()
            var picIdx = storagePictureList.indexOf(d.storagePicturePath)
            edit_comboBox_StoragePicture.currentIndex = picIdx >= 0 ? picIdx : 0
        }
    }

    // Called from Main.qml Save action
    function triggerSave() {
        var name = edit_lineEdit_Name.text.trim()
        if (name === "") {
            saveError(qsTr("Provide a name for this device."))
            return
        }

        var path     = (deviceType !== "Virtual") ? edit_lineEdit_Path.text.trim() : ""
        if (path.length > 1 && path.endsWith("/"))
            path = path.slice(0, -1)
        var parentId = edit_storageComboBox.selectedDeviceId

        var err = appManager1.saveDeviceBasicFields(deviceId, name, parentId, path)
        if (err !== "") { saveError(err); return }

        if (deviceType === "Catalog") {
            var fileType = edit_comboBox_FileType.model[edit_comboBox_FileType.currentIndex].value
            var subdir   = edit_checkBox_IncludeSubDir.checked
            var hidden   = edit_comboBox_IncludeHidden.currentIndex === 1
            var metadata = edit_comboBox_Metadata.model[edit_comboBox_Metadata.currentIndex].value
            var checksum = edit_comboBox_Checksum.model[edit_comboBox_Checksum.currentIndex].value

            var pathChanged = (path !== originalPath && originalPath !== "")
            var check = appManager1.checkCatalogOptionChanges(deviceId, fileType, subdir, hidden, metadata, checksum, _originalIsFullDevice)
            var message = check.message
            if (pathChanged) {
                if (message !== "") message += "\n"
                message += qsTr("Source path: %1 → %2").arg(originalPath).arg(path)
            }
            var rescanNeeded = check.rescanNeeded || pathChanged
            if (check.needsConfirmation || pathChanged) {
                catalogConfirmNeeded(message, rescanNeeded, pathChanged)
                return
            }
            finalizeSave()
            return
        }

        if (deviceType === "Storage") {
            var storageFields = {
                "storageExtId":        parseInt(edit_lineEdit_StorageExtId.text) || 0,
                "storageType":         edit_lineEdit_StorageType.text,
                "storageLabel":        edit_lineEdit_StorageLabel.text,
                "storageFileSystem":   edit_lineEdit_StorageFS.text,
                "totalSpace":          parseInt(edit_lineEdit_TotalSpace.text) || 0,
                "freeSpace":           parseInt(edit_lineEdit_FreeSpace.text) || 0,
                "storageBrand":        edit_lineEdit_Brand.text,
                "storageModel":        edit_lineEdit_Model.text,
                "storageSerialNumber": edit_lineEdit_SerialNumber.text,
                "storageBuildDate":    edit_lineEdit_BuildDate.text,
                "storageComment1":     edit_lineEdit_Comment1.text,
                "storageComment2":     edit_lineEdit_Comment2.text,
                "storageComment3":     edit_lineEdit_Comment3.text,
                "storagePicturePath":  edit_comboBox_StoragePicture.currentIndex >= 0
                                       ? edit_comboBox_StoragePicture.model[edit_comboBox_StoragePicture.currentIndex] : ""
            }
            var storageErr = appManager1.saveStorageDetails(deviceId, storageFields)
            if (storageErr !== "") { saveError(storageErr); return }

            if (path !== originalPath && originalPath !== "") {
                storagePathChangeNeeded(originalPath, path)
                return
            }
        }

        finalizeSave()
    }

    // Called from Main.qml after catalog confirm dialog accepted
    function confirmCatalogSave(rescanNeeded, pathChanged) {
        var fileType = edit_comboBox_FileType.model[edit_comboBox_FileType.currentIndex].value
        var subdir   = edit_checkBox_IncludeSubDir.checked
        var hidden   = edit_comboBox_IncludeHidden.currentIndex === 1
        var metadata = edit_comboBox_Metadata.model[edit_comboBox_Metadata.currentIndex].value
        var checksum = edit_comboBox_Checksum.model[edit_comboBox_Checksum.currentIndex].value
        appManager1.saveCatalogOptions(deviceId, fileType, subdir, hidden, metadata, checksum, _originalIsFullDevice)

        if (pathChanged) {
            storagePathChangeNeeded(originalPath, edit_lineEdit_Path.text.trim())
        } else if (rescanNeeded) {
            catalogUpdateContentNeeded()
        } else {
            finalizeSave()
        }
    }

    Component.onCompleted: loadDevice()

    // ── Folder dialogs ─────────────────────────────────────────────────────────
    FolderDialog {
        id: editPathDialog
        onAccepted: edit_lineEdit_Path.text = appManager1.pathFromFileUrl(selectedFolder.toString())
    }
    FolderDialog {
        id: editExcludeDialog
        onAccepted: edit_lineEdit_NewExcludeFolder.text = appManager1.pathFromFileUrl(selectedFolder.toString())
    }

    ListModel { id: excludeFoldersModel }

    // ═══ Main Form ═════════════════════════════════════════════════════════════
    GridLayout {
        Layout.fillWidth: true
        Layout.margins: Kirigami.Units.gridUnit
        columns: 2
        columnSpacing: Kirigami.Units.largeSpacing
        rowSpacing: Kirigami.Units.smallSpacing

        // ── Section: Identity ─────────────────────────────────────────────────
        Kirigami.Heading {
            level: 3
            text: qsTr("Device")
            color: Kirigami.Theme.linkColor
            Layout.columnSpan: 2
            Layout.topMargin: Kirigami.Units.smallSpacing
        }

        Controls.Label { text: qsTr("Type"); opacity: 0.7 }
        Controls.Label { text: root.deviceType; font.bold: true }

        Controls.Label { text: qsTr("Name"); opacity: 0.7 }
        Controls.TextField {
            id: edit_lineEdit_Name
            Layout.fillWidth: true
        }

        Controls.Label { text: qsTr("Parent device"); opacity: root.deviceId === 1 ? 0.35 : 0.7 }
        DeviceTreeComboBox {
            id: edit_storageComboBox
            storageOnly: root.deviceType === "Catalog"
            hideCatalogs: true
            hideStorages: root.deviceType === "Storage"
            // The editor's parent must reflect only the loaded device + explicit
            // dropdown picks — never silently follow the Selection page.
            followAppSelection: false
            // Device id 1 is the reserved " Physical Group" root — it is the
            // structural top of the Physical hierarchy and must never be re-parented.
            enabled: root.deviceId !== 1
            Layout.fillWidth: true
        }

        // ── Section: Path (Storage + Catalog) ─────────────────────────────────
        Kirigami.Separator {
            Layout.fillWidth: true; Layout.columnSpan: 2
            Layout.topMargin: Kirigami.Units.largeSpacing
            visible: root.deviceType !== "Virtual"
        }
        Kirigami.Heading {
            level: 3
            text: qsTr("Location")
            color: Kirigami.Theme.linkColor
            Layout.columnSpan: 2
            Layout.topMargin: Kirigami.Units.smallSpacing
            visible: root.deviceType !== "Virtual"
        }

        Controls.Label { text: qsTr("Source path"); opacity: 0.7; visible: root.deviceType !== "Virtual" }
        RowLayout {
            Layout.fillWidth: true
            visible: root.deviceType !== "Virtual"
            Controls.TextField {
                id: edit_lineEdit_Path
                Layout.fillWidth: true
            }
            Controls.Button {
                icon.name: "folder-open"
                onClicked: {
                    var p = edit_lineEdit_Path.text.trim()
                    if (p !== "") editPathDialog.currentFolder = appManager1.pathToFileUrl(p)
                    editPathDialog.open()
                }
            }
        }

        // ── Section: Catalog options ───────────────────────────────────────────
        Kirigami.Separator {
            Layout.fillWidth: true; Layout.columnSpan: 2
            Layout.topMargin: Kirigami.Units.largeSpacing
            visible: root.deviceType === "Catalog"
        }
        Kirigami.Heading {
            level: 3
            text: qsTr("Content options")
            color: Kirigami.Theme.linkColor
            Layout.columnSpan: 2
            Layout.topMargin: Kirigami.Units.smallSpacing
            visible: root.deviceType === "Catalog"
        }

        Controls.Label { text: qsTr("File type"); opacity: 0.7; visible: root.deviceType === "Catalog" }
        Controls.ComboBox {
            id: edit_comboBox_FileType
            Layout.fillWidth: true
            visible: root.deviceType === "Catalog"
            textRole: "text"
            valueRole: "value"
            displayText: ""
            model: [
                { text: qsTr("All"),   value: "All",   iconName: "folder"                 },
                { text: qsTr("Audio"), value: "Audio", iconName: "audio-x-mpeg"            },
                { text: qsTr("Image"), value: "Image", iconName: "image-jpeg"              },
                { text: qsTr("Text"),  value: "Text",  iconName: "view-list-text"          },
                { text: qsTr("Video"), value: "Video", iconName: "video-mp4"               },
                { text: qsTr("Other"), value: "Other", iconName: "document-open"           },
                { text: qsTr("None"),  value: "None",  iconName: "application-x-zerosize" }
            ]
            currentIndex: 0
            delegate: Controls.ItemDelegate {
                width: edit_comboBox_FileType.width
                text: modelData.text
                icon.name: modelData.iconName
                highlighted: edit_comboBox_FileType.highlightedIndex === index
            }
            contentItem: RowLayout {
                function positionToRectangle(pos) { return Qt.rect(0, 0, 0, 0) }
                property int selectionStart: 0
                spacing: Kirigami.Units.smallSpacing
                Item { width: Kirigami.Units.smallSpacing }
                Kirigami.Icon {
                    source: edit_comboBox_FileType.currentIndex >= 0
                            ? edit_comboBox_FileType.model[edit_comboBox_FileType.currentIndex].iconName : ""
                    implicitWidth:  Kirigami.Units.iconSizes.small
                    implicitHeight: Kirigami.Units.iconSizes.small
                }
                Controls.Label {
                    text: edit_comboBox_FileType.currentIndex >= 0
                          ? edit_comboBox_FileType.model[edit_comboBox_FileType.currentIndex].text : ""
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        Controls.Label { text: qsTr("Include subdirectories"); opacity: 0.7; visible: root.deviceType === "Catalog" }
        Controls.CheckBox {
            id: edit_checkBox_IncludeSubDir
            checked: true
            visible: root.deviceType === "Catalog"
        }

        Controls.Label { text: qsTr("Include hidden files"); opacity: 0.7; visible: root.deviceType === "Catalog" }
        Controls.ComboBox {
            id: edit_comboBox_IncludeHidden
            Layout.fillWidth: true
            visible: root.deviceType === "Catalog"
            model: [ qsTr("None"), qsTr("All") ]
            currentIndex: 0
        }

        Controls.Label { text: qsTr("Include metadata"); opacity: 0.7; visible: root.deviceType === "Catalog" }
        Controls.ComboBox {
            id: edit_comboBox_Metadata
            Layout.fillWidth: true
            visible: root.deviceType === "Catalog"
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

        Controls.Label { text: qsTr("Include checksum"); opacity: 0.7; visible: root.deviceType === "Catalog" }
        Controls.ComboBox {
            id: edit_comboBox_Checksum
            Layout.fillWidth: true
            visible: root.deviceType === "Catalog"
            textRole: "text"
            valueRole: "value"
            model: [
                { text: qsTr("None"),  value: "None"   },
                { text: "SHA-256",     value: "SHA256" }
            ]
            currentIndex: 0
        }

        // ── Exclude folders or files (Catalog) — sub-title within Content options ─
        Controls.Label {
            text: qsTr("Exclude folders or files")
            font.bold: true
            Layout.columnSpan: 2
            Layout.topMargin: Kirigami.Units.smallSpacing
            visible: root.deviceType === "Catalog"
        }

        Controls.Label { text: qsTr("Path or text to exclude"); opacity: 0.7; visible: root.deviceType === "Catalog" }
        RowLayout {
            Layout.fillWidth: true
            visible: root.deviceType === "Catalog"
            Controls.TextField {
                id: edit_lineEdit_NewExcludeFolder
                Layout.fillWidth: true
                placeholderText: qsTr("Path or text to exclude")
            }
            Controls.Button {
                icon.name: "folder-open"
                onClicked: {
                    var p = edit_lineEdit_Path.text.trim()
                    if (p !== "") editExcludeDialog.currentFolder = appManager1.pathToFileUrl(p)
                    editExcludeDialog.open()
                }
            }
            Controls.Button {
                icon.name: "list-add"
                text: qsTr("Add")
                onClicked: {
                    var p = edit_lineEdit_NewExcludeFolder.text.trim()
                    if (p === "") return
                    if (appManager1.addDeviceExcludeFolder(deviceId, p)) {
                        for (var i = 0; i < excludeFoldersModel.count; i++)
                            if (excludeFoldersModel.get(i).folderPath === p) return
                        excludeFoldersModel.append({ folderPath: p })
                        edit_lineEdit_NewExcludeFolder.text = ""
                    }
                }
            }
        }

        Item { Layout.preferredWidth: 1; visible: root.deviceType === "Catalog" }
        ColumnLayout {
            Layout.fillWidth: true
            visible: root.deviceType === "Catalog"
            spacing: 2
            Repeater {
                model: excludeFoldersModel
                delegate: RowLayout {
                    Layout.fillWidth: true
                    Controls.Label { text: folderPath; elide: Text.ElideMiddle; Layout.fillWidth: true }
                    Controls.Button {
                        icon.name: "edit-delete"; flat: true
                        onClicked: {
                            appManager1.removeDeviceExcludeFolder(deviceId, folderPath)
                            excludeFoldersModel.remove(index)
                        }
                    }
                }
            }
        }

        // ── Section: Storage details ───────────────────────────────────────────
        Kirigami.Separator {
            Layout.fillWidth: true; Layout.columnSpan: 2
            Layout.topMargin: Kirigami.Units.largeSpacing
            visible: root.deviceType === "Storage"
        }
        Kirigami.Heading {
            level: 3
            text: qsTr("Storage details")
            color: Kirigami.Theme.linkColor
            Layout.columnSpan: 2
            Layout.topMargin: Kirigami.Units.smallSpacing
            visible: root.deviceType === "Storage"
        }

        Controls.Label { text: qsTr("Storage ID"); opacity: 0.7; visible: root.deviceType === "Storage" }
        Controls.TextField { id: edit_lineEdit_StorageExtId; Layout.fillWidth: true; visible: root.deviceType === "Storage" }

        Controls.Label { text: qsTr("Type"); opacity: 0.7; visible: root.deviceType === "Storage" }
        Controls.TextField { id: edit_lineEdit_StorageType; Layout.fillWidth: true; visible: root.deviceType === "Storage" }

        Controls.Label { text: qsTr("Label"); opacity: 0.7; visible: root.deviceType === "Storage" }
        Controls.TextField { id: edit_lineEdit_StorageLabel; Layout.fillWidth: true; visible: root.deviceType === "Storage" }

        Controls.Label { text: qsTr("File system"); opacity: 0.7; visible: root.deviceType === "Storage" }
        Controls.TextField { id: edit_lineEdit_StorageFS; Layout.fillWidth: true; visible: root.deviceType === "Storage" }

        Controls.Label { text: qsTr("Total space (bytes)"); opacity: 0.7; visible: root.deviceType === "Storage" }
        RowLayout {
            Layout.fillWidth: true
            visible: root.deviceType === "Storage"
            Controls.Label {
                text: appManager1.formatDataSize(parseInt(edit_lineEdit_TotalSpace.text) || 0)
                opacity: 0.7
                visible: text.length > 0
                Layout.preferredWidth: 75
                horizontalAlignment: Text.AlignRight

            }
            Controls.TextField {
                id: edit_lineEdit_TotalSpace
                Layout.fillWidth: true
                inputMethodHints: Qt.ImhDigitsOnly
                horizontalAlignment: Text.AlignLeft
            }
        }

        Controls.Label { text: qsTr("Free space (bytes)"); opacity: 0.7; visible: root.deviceType === "Storage" }
        RowLayout {
            Layout.fillWidth: true
            visible: root.deviceType === "Storage"
            Controls.Label {
                text: appManager1.formatDataSize(parseInt(edit_lineEdit_FreeSpace.text) || 0)
                opacity: 0.7
                visible: text.length > 0
                Layout.preferredWidth: 75
                horizontalAlignment: Text.AlignRight
            }
            Controls.TextField {
                id: edit_lineEdit_FreeSpace
                Layout.fillWidth: true
                inputMethodHints: Qt.ImhDigitsOnly
            }
       }

        Item { visible: root.deviceType === "Storage"; Layout.preferredWidth: 1 }
        Controls.Button {
            visible: root.deviceType === "Storage"
            text: qsTr("Refresh from disk")
            icon.name: "view-refresh"
            onClicked: {
                var updated = appManager1.refreshStorageFromDisk(root.deviceId)
                if (updated.error === "") {
                    edit_lineEdit_TotalSpace.text   = String(updated.totalSpace)
                    edit_lineEdit_FreeSpace.text    = String(updated.freeSpace)
                    edit_lineEdit_StorageType.text  = updated.storageType
                    edit_lineEdit_StorageLabel.text = updated.storageLabel
                    edit_lineEdit_StorageFS.text    = updated.fileSystem
                }
            }
        }

        Controls.Label { text: qsTr("Brand"); opacity: 0.7; visible: root.deviceType === "Storage" }
        Controls.TextField { id: edit_lineEdit_Brand; Layout.fillWidth: true; visible: root.deviceType === "Storage" }

        Controls.Label { text: qsTr("Model"); opacity: 0.7; visible: root.deviceType === "Storage" }
        Controls.TextField { id: edit_lineEdit_Model; Layout.fillWidth: true; visible: root.deviceType === "Storage" }

        Controls.Label { text: qsTr("Serial number"); opacity: 0.7; visible: root.deviceType === "Storage" }
        Controls.TextField { id: edit_lineEdit_SerialNumber; Layout.fillWidth: true; visible: root.deviceType === "Storage" }

        Controls.Label { text: qsTr("Build date"); opacity: 0.7; visible: root.deviceType === "Storage" }
        Controls.TextField { id: edit_lineEdit_BuildDate; Layout.fillWidth: true; visible: root.deviceType === "Storage" }

        Controls.Label { text: qsTr("Comment 1"); opacity: 0.7; visible: root.deviceType === "Storage" }
        Controls.TextField { id: edit_lineEdit_Comment1; Layout.fillWidth: true; visible: root.deviceType === "Storage" }

        Controls.Label { text: qsTr("Comment 2"); opacity: 0.7; visible: root.deviceType === "Storage" }
        Controls.TextField { id: edit_lineEdit_Comment2; Layout.fillWidth: true; visible: root.deviceType === "Storage" }

        Controls.Label { text: qsTr("Comment 3"); opacity: 0.7; visible: root.deviceType === "Storage" }
        Controls.TextField { id: edit_lineEdit_Comment3; Layout.fillWidth: true; visible: root.deviceType === "Storage" }

        // ── Storage picture ────────────────────────────────────────────────────
        Kirigami.Separator {
            Layout.fillWidth: true; Layout.columnSpan: 2
            Layout.topMargin: Kirigami.Units.largeSpacing
            visible: root.deviceType === "Storage"
        }

        Controls.Label { text: qsTr("Picture"); opacity: 0.7; visible: root.deviceType === "Storage" }
        RowLayout {
            Layout.fillWidth: true
            visible: root.deviceType === "Storage"
            Controls.ComboBox {
                id: edit_comboBox_StoragePicture
                Layout.fillWidth: true
                model: root.storagePictureList
            }
            Controls.Button {
                icon.name: "view-refresh"
                Controls.ToolTip.text: qsTr("Reload pictures")
                Controls.ToolTip.visible: hovered
                onClicked: {
                    var prev = edit_comboBox_StoragePicture.currentIndex >= 0
                               ? edit_comboBox_StoragePicture.model[edit_comboBox_StoragePicture.currentIndex] : ""
                    root.storagePictureList = appManager1.getStoragePictureList()
                    var idx = root.storagePictureList.indexOf(prev)
                    edit_comboBox_StoragePicture.currentIndex = idx >= 0 ? idx : 0
                }
            }
        }

        Item { visible: root.deviceType === "Storage"; Layout.preferredWidth: 1 }
        Image {
            visible: root.deviceType === "Storage"
            Layout.preferredWidth:  300
            Layout.preferredHeight: 250
            fillMode: Image.PreserveAspectFit
            source: {
                if (root.deviceType !== "Storage") return ""
                var folder = appManager1.getStorageImageFolderPath()
                if (folder === "") return ""
                var sel = (edit_comboBox_StoragePicture.currentIndex >= 0 && root.storagePictureList.length > 0)
                          ? root.storagePictureList[edit_comboBox_StoragePicture.currentIndex] : ""
                if (sel !== "")
                    return appManager1.pathToFileUrl(folder + "/" + sel)
                return appManager1.pathToFileUrl(folder + "/" + root.deviceId + ".jpg")
            }
        }

        Item { Layout.columnSpan: 2; Layout.preferredHeight: Kirigami.Units.gridUnit }
    }
}
