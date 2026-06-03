import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import Qt.labs.platform

ColumnLayout {
    id: pageSearchForm
    spacing: 0

    // Width of the left label column inside sub-sections
    readonly property int labelW: Kirigami.Units.gridUnit * 4

    property var tagNames: []

    Connections {
        target: appManager1
        function onTagsChanged() { pageSearchForm.tagNames = appManager1.getTagNames() }
    }

    Component.onCompleted: { pageSearchForm.tagNames = appManager1.getTagNames() }

    function restoreLastSearch() {
        var hist = appManager1.getSearchHistory()
        if (hist.length === 0) return
        var map = appManager1.restoreSearchHistory(hist[0].dateTime)
        applyHistoryCriteria(map)
    }

    function getCriteria() {
        //Global type of search
        newSearch1.properties = {"searchInCatalogsChecked":  search_radioButton_SearchInCatalogs.checked};
        newSearch1.properties = {"searchInConnectedChecked": search_radioButton_SearchInConnected.checked};
        newSearch1.properties = {"connectedDirectory":       search_lineEdit_ConnectedDirectory.text};

        //File name
        newSearch1.properties = {"searchOnFileName":        search_checkBox_FileNameCriteria.checked};
        newSearch1.properties = {"searchText":              search_TextField_FileNameText.text};
        newSearch1.properties = {"selectedTextCriteria":    search_ComboBox_TextCriteriaWith.currentText};
        newSearch1.properties = {"selectedSearchIn":        search_ComboBox_TextCriteriaIn.currentText};
        newSearch1.properties = {"caseSensitive":           search_CheckBox_FileNameCaseSensitive.checked};
        newSearch1.properties = {"selectedSearchExclude":   search_TextField_FileNameExclude.text};

        //File Attributes
        newSearch1.properties = {"searchOnFileCriteria":    checkBoxFileAttributesCriteria.checked};
        //Type
        newSearch1.properties = {"searchOnType":            search_checkBox_Type.checked};
        newSearch1.properties = {"selectedFileType":        search_comboBox_FileType.currentValue};
        //Size
        newSearch1.properties = {"searchOnSize":            checkBoxFileAttributesCriteria.checked && search_checkBox_Size.checked};
        newSearch1.properties = {"selectedMinimumSize":     search_spinBox_MinimumSize.value};
        newSearch1.properties = {"selectedMaximumSize":     search_spinBox_MaximumSize.value};
        newSearch1.properties = {"selectedMinSizeUnit":     search_comboBox_MinSizeUnit.currentText};
        newSearch1.properties = {"selectedMaxSizeUnit":     search_comboBox_MaxSizeUnit.currentText};
        //Date
        newSearch1.properties = {"searchOnDate":            checkBoxFileAttributesCriteria.checked && search_checkBox_Date.checked};
        newSearch1.properties = {"selectedDateMin":         search_dateTimeEdit_Min.text};
        newSearch1.properties = {"selectedDateMax":         search_dateTimeEdit_Max.text};

        //File metadata
        newSearch1.properties = {"searchOnFileMetadata":    search_checkBox_FileMetadata.checked};
        newSearch1.properties = {"searchOnMetadataText":    search_checkBox_FileMetadata.checked && search_checkBox_MetadataText.checked};
        newSearch1.properties = {"metadataTextSearch":      search_lineEdit_MetadataText.text};
        newSearch1.properties = {"searchOnMetadataSize":    search_checkBox_FileMetadata.checked && search_checkBox_MetadataSize.checked};
        newSearch1.properties = {"metadataMinimumHeight":   search_spinBox_MetadataMinimumHeight.value};
        newSearch1.properties = {"metadataMaximumHeight":   search_spinBox_MetadataMaximumHeight.value};
        newSearch1.properties = {"metadataMinimumWidth":    search_spinBox_MetadataMinimumWidth.value};
        newSearch1.properties = {"metadataMaximumWidth":    search_spinBox_MetadataMaximumWidth.value};
        newSearch1.properties = {"searchOnMetadataDuration":search_checkBox_FileMetadata.checked && search_checkBox_MetadataDuration.checked};
        newSearch1.properties = {"metadataDurationMin":     search_dateTimeEdit_MetadataDurationMin.text};
        newSearch1.properties = {"metadataDurationMax":     search_dateTimeEdit_MetadataDurationMax.text};

        //Folder Attributes
        newSearch1.properties = {"searchOnFolderCriteria":  search_checkBox_FolderCriteria.checked};
        newSearch1.properties = {"showFoldersOnly":         search_checkBox_ShowFoldersOnly.checked};
        newSearch1.properties = {"searchOnTags":            search_checkBox_SearchOnTags.checked};
        newSearch1.properties = {"selectedTagName":         search_comboBox_FolderTag.currentIndex === 0 ? "" : search_comboBox_FolderTag.currentText};

        //Duplicates
        newSearch1.properties = {"searchOnDuplicates":             search_checkBox_Duplicates.checked};
        newSearch1.properties = {"searchDuplicatesOnName":         search_checkBox_DuplicatesOnName.checked};
        newSearch1.properties = {"searchDuplicatesOnSize":         search_checkBox_DuplicatesOnSize.checked};
        newSearch1.properties = {"searchDuplicatesOnDate":         search_checkBox_DuplicatesOnDate.checked};
        newSearch1.properties = {"searchDuplicatesOnChecksum":     search_checkBox_DuplicatesOnChecksum.checked};
        newSearch1.properties = {"searchDuplicatesChecksumEqual":  search_comboBox_DuplicateChecksumSign.currentIndex === 0};
        newSearch1.properties = {"duplicatesCompareDevices":       search_radioButton_DuplicatesCompareTwoDevices.checked};
        newSearch1.properties = {"duplicatesDeviceID1":            search_comboBox_DuplicatesDevice1.selectedDeviceId};
        newSearch1.properties = {"duplicatesDeviceID2":            search_comboBox_DuplicatesDevice2.selectedDeviceId};

        //Differences
        newSearch1.properties = {"searchOnDifferences":            search_checkBox_Differences.checked};
        newSearch1.properties = {"differencesOnName":              search_checkBox_DifferencesOnName.checked};
        newSearch1.properties = {"differencesOnSize":              search_checkBox_DifferencesOnSize.checked};
        newSearch1.properties = {"differencesOnDate":              search_checkBox_DifferencesOnDate.checked};
        newSearch1.properties = {"differencesOnChecksum":          search_checkBox_DifferencesOnChecksum.checked};
        newSearch1.properties = {"differencesChecksumEqual":       search_comboBox_DifferenceChecksumSign.currentIndex === 0};
        newSearch1.properties = {"differencesDeviceID1":           search_comboBox_DifferencesDevice1.selectedDeviceId};
        newSearch1.properties = {"differencesDeviceID2":           search_comboBox_DifferencesDevice2.selectedDeviceId};
    }

    function resetSearch() {
        // Reset search scope
        search_radioButton_SearchInCatalogs.checked = true
        search_lineEdit_ConnectedDirectory.text = ""

        // Reset fileNameCriteria
        search_checkBox_FileNameCriteria.checked = true
        search_TextField_FileNameText.text = ""
        search_ComboBox_TextCriteriaWith.currentIndex = 0
        search_ComboBox_TextCriteriaIn.currentIndex = 0
        search_CheckBox_FileNameCaseSensitive.checked = false
        search_TextField_FileNameExclude.text = ""

        // Reset fileAtrributes
            checkBoxFileAttributesCriteria.checked = false
            //Size
            search_checkBox_Size.checked = false
            search_spinBox_MinimumSize.value = 0
            search_comboBox_MinSizeUnit.currentIndex = 0 //"Bytes"
            search_spinBox_MaximumSize.value = 1000
            search_comboBox_MaxSizeUnit.currentIndex = 3 //"GiB"
            //Date
            search_checkBox_Date.checked = false
            search_dateTimeEdit_Min.text = "1970/01/01 00:00:00"
            search_dateTimeEdit_Max.text = "2030/01/01 00:00:00"
            //Type
            search_checkBox_Type.checked = false
            search_comboBox_FileType.currentIndex = 0

        //File metadata
        search_checkBox_FileMetadata.checked = false
        search_checkBox_MetadataText.checked = false
        search_lineEdit_MetadataText.text = ""
        search_checkBox_MetadataSize.checked = false
        search_spinBox_MetadataMinimumHeight.value = 0
        search_spinBox_MetadataMaximumHeight.value = 30000
        search_spinBox_MetadataMinimumWidth.value = 0
        search_spinBox_MetadataMaximumWidth.value = 30000
        search_checkBox_MetadataDuration.checked = false
        search_dateTimeEdit_MetadataDurationMin.text = "00:00:00"
        search_dateTimeEdit_MetadataDurationMax.text = "23:59:59"

        //Duplicates
        search_checkBox_Duplicates.checked = false
        search_checkBox_DuplicatesOnName.checked = true
        search_checkBox_DuplicatesOnSize.checked = false
        search_checkBox_DuplicatesOnDate.checked = false
        search_checkBox_DuplicatesOnChecksum.checked = false
        search_comboBox_DuplicateChecksumSign.currentIndex = 0
        search_radioButton_DuplicatesWithinSelectedDevice.checked = true
        search_comboBox_DuplicatesDevice1.resetSelection()
        search_comboBox_DuplicatesDevice2.resetSelection()

        //Differences
        search_checkBox_Differences.checked = false
        search_checkBox_DifferencesOnName.checked = true
        search_checkBox_DifferencesOnSize.checked = false
        search_checkBox_DifferencesOnDate.checked = false
        search_checkBox_DifferencesOnChecksum.checked = false
        search_comboBox_DifferenceChecksumSign.currentIndex = 0
        search_comboBox_DifferencesDevice1.resetSelection()
        search_comboBox_DifferencesDevice2.resetSelection()

        // Reset folderAttributes
        search_checkBox_FolderCriteria.checked = false
        search_checkBox_ShowFoldersOnly.checked = false
        search_checkBox_SearchOnTags.checked = false
        search_comboBox_FolderTag.currentIndex = 0
    }

    function returnCleanedDate(date) {
        var dateArray = date.split(" ");
        var datePart = dateArray[0].split("/");
        var timePart = dateArray[1].split(":");

        var yyyy = datePart[0];
        var mm = datePart[1];
        var dd = datePart[2];
        var hh = timePart[0];
        var min = timePart[1];
        var ss = timePart[2];

        if(dd<10) { dd = '0'+dd }
        if(mm<10) { mm = '0'+mm }
        if(hh<10) { hh = '0'+hh }
        if(min<10) { min = '0'+min }
        if(ss<10) { ss = '0'+ss }

        return yyyy + '/' + mm + '/' + dd + ' ' + hh + ':' + min + ':' + ss;
    }

    function executeSearch() {
        getCriteria()
        appManager1.executeSearch()
    }

    function openHistorySheet() {
        _refreshHistoryModel()
        searchHistoryDialog.open()
    }

    function _refreshHistoryModel() {
        historyListModel.clear()
        var hist = appManager1.getSearchHistory()
        for (var i = 0; i < hist.length; ++i)
            historyListModel.append(hist[i])
    }

    function applyHistoryCriteria(map) {
        // Search scope
        if (map.searchInConnectedChecked ?? false) {
            search_radioButton_SearchInConnected.checked = true
        } else {
            search_radioButton_SearchInCatalogs.checked = true
        }
        search_lineEdit_ConnectedDirectory.text           = map.connectedDirectory ?? ""

        // File name
        search_checkBox_FileNameCriteria.checked          = map.searchOnFileName ?? false
        search_TextField_FileNameText.text                = map.searchText ?? ""
        var idx = search_ComboBox_TextCriteriaWith.find(map.selectedTextCriteria ?? "")
        if (idx >= 0) search_ComboBox_TextCriteriaWith.currentIndex = idx
        idx = search_ComboBox_TextCriteriaIn.find(map.selectedSearchIn ?? "")
        if (idx >= 0) search_ComboBox_TextCriteriaIn.currentIndex = idx
        search_CheckBox_FileNameCaseSensitive.checked     = map.caseSensitive ?? false
        search_TextField_FileNameExclude.text             = map.selectedSearchExclude ?? ""

        // File attributes
        checkBoxFileAttributesCriteria.checked            = map.searchOnFileCriteria ?? false
        search_checkBox_Type.checked                      = map.searchOnType ?? false
        for (var i = 0; i < search_comboBox_FileType.count; ++i) {
            if (search_comboBox_FileType.model[i].value === (map.selectedFileType ?? "All")) {
                search_comboBox_FileType.currentIndex = i
                break
            }
        }
        search_checkBox_Size.checked                      = map.searchOnSize ?? false
        search_spinBox_MinimumSize.value                  = map.selectedMinimumSize ?? 0
        idx = search_comboBox_MinSizeUnit.find(map.selectedMinSizeUnit ?? "Bytes")
        if (idx >= 0) search_comboBox_MinSizeUnit.currentIndex = idx
        search_spinBox_MaximumSize.value                  = map.selectedMaximumSize ?? 1000
        idx = search_comboBox_MaxSizeUnit.find(map.selectedMaxSizeUnit ?? "GiB")
        if (idx >= 0) search_comboBox_MaxSizeUnit.currentIndex = idx
        search_checkBox_Date.checked                      = map.searchOnDate ?? false
        search_dateTimeEdit_Min.text                      = map.selectedDateMin ?? "1970/01/01 00:00:00"
        search_dateTimeEdit_Max.text                      = map.selectedDateMax ?? "2030/01/01 00:00:00"

        // File metadata
        search_checkBox_FileMetadata.checked              = map.searchOnFileMetadata ?? false
        search_checkBox_MetadataText.checked              = map.searchOnMetadataText ?? false
        search_lineEdit_MetadataText.text                 = map.metadataTextSearch ?? ""
        search_checkBox_MetadataSize.checked              = map.searchOnMetadataSize ?? false
        search_spinBox_MetadataMinimumHeight.value        = map.metadataMinimumHeight ?? 0
        search_spinBox_MetadataMaximumHeight.value        = map.metadataMaximumHeight ?? 30000
        search_spinBox_MetadataMinimumWidth.value         = map.metadataMinimumWidth ?? 0
        search_spinBox_MetadataMaximumWidth.value         = map.metadataMaximumWidth ?? 30000
        search_checkBox_MetadataDuration.checked          = map.searchOnMetadataDuration ?? false
        search_dateTimeEdit_MetadataDurationMin.text      = map.metadataDurationMin ?? "00:00:00"
        search_dateTimeEdit_MetadataDurationMax.text      = map.metadataDurationMax ?? "23:59:59"

        // Folder attributes
        search_checkBox_FolderCriteria.checked            = map.searchOnFolderCriteria ?? false
        search_checkBox_ShowFoldersOnly.checked           = map.showFoldersOnly ?? false
        search_checkBox_SearchOnTags.checked              = map.searchOnTags ?? false
        if (map.selectedTagName) {
            idx = search_comboBox_FolderTag.find(map.selectedTagName)
            if (idx >= 0) search_comboBox_FolderTag.currentIndex = idx
        } else {
            search_comboBox_FolderTag.currentIndex = 0
        }

        // Duplicates — restore Size/Date/Checksum before Name so the "at least one" guard doesn't fire
        search_checkBox_Duplicates.checked                = map.searchOnDuplicates ?? false
        search_checkBox_DuplicatesOnSize.checked          = map.searchDuplicatesOnSize ?? false
        search_checkBox_DuplicatesOnDate.checked          = map.searchDuplicatesOnDate ?? false
        search_checkBox_DuplicatesOnChecksum.checked      = map.searchDuplicatesOnChecksum ?? false
        search_checkBox_DuplicatesOnName.checked          = map.searchDuplicatesOnName ?? true
        search_comboBox_DuplicateChecksumSign.currentIndex = (map.searchDuplicatesChecksumEqual ?? true) ? 0 : 1
        if (map.duplicatesCompareDevices ?? false)
            search_radioButton_DuplicatesCompareTwoDevices.checked = true
        else
            search_radioButton_DuplicatesWithinSelectedDevice.checked = true
        if ((map.duplicatesDeviceID1 ?? 0) > 0)
            search_comboBox_DuplicatesDevice1._applyDevice(map.duplicatesDeviceID1)
        else
            search_comboBox_DuplicatesDevice1.resetSelection()
        if ((map.duplicatesDeviceID2 ?? 0) > 0)
            search_comboBox_DuplicatesDevice2._applyDevice(map.duplicatesDeviceID2)
        else
            search_comboBox_DuplicatesDevice2.resetSelection()

        // Differences — restore Size/Date/Checksum before Name so the "at least one" guard doesn't fire
        search_checkBox_Differences.checked               = map.searchOnDifferences ?? false
        search_checkBox_DifferencesOnSize.checked         = map.differencesOnSize ?? false
        search_checkBox_DifferencesOnDate.checked         = map.differencesOnDate ?? false
        search_checkBox_DifferencesOnChecksum.checked     = map.differencesOnChecksum ?? false
        search_checkBox_DifferencesOnName.checked         = map.differencesOnName ?? true
        search_comboBox_DifferenceChecksumSign.currentIndex = (map.differencesChecksumEqual ?? true) ? 0 : 1
        if ((map.differencesDeviceID1 ?? 0) > 0)
            search_comboBox_DifferencesDevice1._applyDevice(map.differencesDeviceID1)
        else
            search_comboBox_DifferencesDevice1.resetSelection()
        if ((map.differencesDeviceID2 ?? 0) > 0)
            search_comboBox_DifferencesDevice2._applyDevice(map.differencesDeviceID2)
        else
            search_comboBox_DifferencesDevice2.resetSelection()
    }

    Kirigami.Dialog {
        id: dateDialog
        property string selectedDateField
        title: "Select a date for " + selectedDateField + " date"

        contentItem: ColumnLayout {
            RowLayout {
                Controls.Button {
                    text: "Now"
                    onClicked: {
                        var today = new Date();
                        var dd = today.getDate();
                        var mm = today.getMonth()+1;
                        var yyyy = today.getFullYear();
                        var hh = today.getHours();
                        var min = today.getMinutes();
                        var ss = today.getSeconds();
                        var todayDate = yyyy + '/' + mm + '/' + dd + ' ' + hh + ':' + min + ':' + ss;
                        if (dateDialog.selectedDateField === "Min")
                            search_dateTimeEdit_Min.text = returnCleanedDate(todayDate)
                        else
                            search_dateTimeEdit_Max.text = returnCleanedDate(todayDate)
                        dateDialog.close()
                    }
                }
                Controls.Button {
                    text: "1 day ago"
                    onClicked: {
                        var today = new Date();
                        var yesterday = new Date(today);
                        yesterday.setDate(today.getDate() - 1);
                        var fullDate = yesterday.getFullYear() + '/' + (yesterday.getMonth()+1) + '/' + yesterday.getDate() + ' ' + yesterday.getHours() + ':' + yesterday.getMinutes() + ':' + yesterday.getSeconds();
                        if (dateDialog.selectedDateField === "Min")
                            search_dateTimeEdit_Min.text = returnCleanedDate(fullDate)
                        else
                            search_dateTimeEdit_Max.text = returnCleanedDate(fullDate)
                        dateDialog.close()
                    }
                }
                Controls.Button {
                    text: "1 week ago"
                    onClicked: {
                        var today = new Date();
                        var lastWeek = new Date(today);
                        lastWeek.setDate(today.getDate() - 7);
                        var fullDate = lastWeek.getFullYear() + '/' + (lastWeek.getMonth()+1) + '/' + lastWeek.getDate() + ' ' + lastWeek.getHours() + ':' + lastWeek.getMinutes() + ':' + lastWeek.getSeconds();
                        if (dateDialog.selectedDateField === "Min")
                            search_dateTimeEdit_Min.text = returnCleanedDate(fullDate)
                        else
                            search_dateTimeEdit_Max.text = returnCleanedDate(fullDate)
                        dateDialog.close()
                    }
                }
                Controls.Button {
                    text: "1 month ago"
                    onClicked: {
                        var today = new Date();
                        var lastMonth = new Date(today);
                        lastMonth.setMonth(today.getMonth() - 1);
                        var fullDate = lastMonth.getFullYear() + '/' + (lastMonth.getMonth()+1) + '/' + lastMonth.getDate() + ' ' + lastMonth.getHours() + ':' + lastMonth.getMinutes() + ':' + lastMonth.getSeconds();
                        if (dateDialog.selectedDateField === "Min")
                            search_dateTimeEdit_Min.text = returnCleanedDate(fullDate)
                        else
                            search_dateTimeEdit_Max.text = returnCleanedDate(fullDate)
                        dateDialog.close()
                    }
                }
                Controls.Button {
                    text: "1 year ago"
                    onClicked: {
                        var today = new Date();
                        var lastYear = new Date(today);
                        lastYear.setFullYear(today.getFullYear() - 1);
                        var fullDate = lastYear.getFullYear() + '/' + (lastYear.getMonth()+1) + '/' + lastYear.getDate() + ' ' + lastYear.getHours() + ':' + lastYear.getMinutes() + ':' + lastYear.getSeconds();
                        if (dateDialog.selectedDateField === "Min")
                            search_dateTimeEdit_Min.text = returnCleanedDate(fullDate)
                        else
                            search_dateTimeEdit_Max.text = returnCleanedDate(fullDate)
                        dateDialog.close()
                    }
                }
                Controls.Button {
                    text: "Reset"
                    onClicked: {
                        if (dateDialog.selectedDateField === "Min")
                            search_dateTimeEdit_Min.text = "1970/01/01 00:00:00"
                        else
                            search_dateTimeEdit_Max.text = "2030/01/01 00:00:00"
                        dateDialog.close()
                    }
                }
            }
        }
    }

    Kirigami.PromptDialog {
        id: clearHistoryConfirmDialog
        title: qsTr("Delete search history")
        subtitle: qsTr("This will delete all search history entries. Continue?")
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        onAccepted: {
            appManager1.clearSearchHistory()
            historyListModel.clear()
        }
    }

    Kirigami.PromptDialog {
        id: keepLastHistoryConfirmDialog
        title: qsTr("Keep last 10")
        subtitle: qsTr("This will delete all but the last 10 search history entries. Continue?")
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        onAccepted: {
            appManager1.keepLastSearchHistory(10)
            pageSearchForm._refreshHistoryModel()
        }
    }

    Kirigami.Dialog {
        id: searchHistoryDialog
        title: qsTr("Search History")

        customFooterActions: [
            Kirigami.Action {
                text: qsTr("Keep Last 10")
                icon.name: "edit-delete"
                onTriggered: keepLastHistoryConfirmDialog.open()
            },
            Kirigami.Action {
                text: qsTr("Clear")
                icon.name: "edit-delete"
                onTriggered: clearHistoryConfirmDialog.open()
            },
            Kirigami.Action {
                text: qsTr("Cancel")
                icon.name: "dialog-cancel"
                onTriggered: searchHistoryDialog.close()
            }
        ]

        contentItem: ListView {
            id: historyListView
            implicitWidth: Kirigami.Units.gridUnit * 30
            implicitHeight: Math.min(contentHeight, Kirigami.Units.gridUnit * 20)
            clip: true
            model: ListModel { id: historyListModel }

            delegate: Controls.ItemDelegate {
                width: historyListView.width
                contentItem: ColumnLayout {
                    spacing: Kirigami.Units.smallSpacing / 2
                    Controls.Label {
                        text: model.dateTime
                        font.bold: true
                        Layout.fillWidth: true
                    }
                    Controls.Label {
                        text: model.summary.length > 0 ? model.summary : qsTr("(no text filter)")
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        color: Kirigami.Theme.disabledTextColor
                        font.italic: model.summary.length === 0
                    }
                }
                onClicked: {
                    var map = appManager1.restoreSearchHistory(model.dateTime)
                    pageSearchForm.applyHistoryCriteria(map)
                    searchHistoryDialog.close()
                }
            }
        }
    }

    // ── Section 0: Search scope ───────────────────────────────────────────────
    FolderDialog {
        id: connectedDirectoryDialog
        onAccepted: search_lineEdit_ConnectedDirectory.text = folder.toString().replace("file://", "")
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.leftMargin: Kirigami.Units.smallSpacing

        Controls.RadioButton {
            id: search_radioButton_SearchInCatalogs
            text: qsTr("Catalogs")
            checked: true
            onCheckedChanged: {
                if (checked) search_lineEdit_ConnectedDirectory.text = ""
            }
        }
        Controls.RadioButton {
            id: search_radioButton_SearchInConnected
            text: qsTr("Connected drive")
        }
    }

    RowLayout {
        id: search_RowLayout_ConnectedDirectory
        Layout.fillWidth: true
        Layout.leftMargin: Kirigami.Units.smallSpacing
        Layout.topMargin: Kirigami.Units.smallSpacing * 2
        visible: search_radioButton_SearchInConnected.checked

        Controls.TextField {
            id: search_lineEdit_ConnectedDirectory
            Layout.fillWidth: true
            placeholderText: qsTr("Path to connected drive or folder")
        }
        Controls.Button {
            icon.name: "folder-open"
            onClicked: connectedDirectoryDialog.open()
        }
    }

    Controls.Label { text: ""}

    Kirigami.Separator { Layout.fillWidth: true; Layout.topMargin: Kirigami.Units.smallSpacing; Layout.bottomMargin: Kirigami.Units.smallSpacing }

    // ── Section 1: File name ──────────────────────────────────────────────────
    Controls.CheckBox {
        id: search_checkBox_FileNameCriteria
        checked: true
        text: qsTr("File name")
        Layout.leftMargin: Kirigami.Units.smallSpacing
        font.bold: checked ? true : false
        onCheckedChanged: search_FormLayout_FileNameCriteria.visible = checked
    }
    ColumnLayout {
        id: search_FormLayout_FileNameCriteria
        Layout.leftMargin: Kirigami.Units.smallSpacing
        Layout.fillWidth: true
        spacing: Kirigami.Units.smallSpacing

        RowLayout {
            Layout.fillWidth: true
            Controls.Label { text: qsTr("text"); Layout.preferredWidth: pageSearchForm.labelW; Layout.alignment: Qt.AlignTop }
            Controls.ScrollView {
                Layout.fillWidth: true
                implicitHeight: Kirigami.Units.gridUnit * 2
                background: Rectangle {
                    color: palette.base
                }
                Controls.TextArea {
                    id: search_TextField_FileNameText
                    width: parent.availableWidth
                    wrapMode: Text.WordWrap
                    background: Item {}
                    Keys.onPressed: function(event) {
                        if ((event.key === Qt.Key_Return || event.key === Qt.Key_Enter)
                                && (event.modifiers & Qt.ControlModifier)) {
                            pageSearchForm.executeSearch()
                            event.accepted = true
                        }
                    }
                }
            }
            RowLayout {
                Layout.alignment: Qt.AlignTop
                spacing: Kirigami.Units.smallSpacing
                Controls.Button {
                    id: search_Button_ClearSearchText
                    icon.name: "edit-clear"
                    onClicked: search_TextField_FileNameText.clear()
                }
                Controls.Button {
                    id: search_Button_PasteClipboard
                    icon.name: "edit-paste"
                    onClicked: search_TextField_FileNameText.text = pageSearch1.returnClipboard()
                }
                Controls.Button {
                    id: search_Button_CleanText
                    icon.name: "edit-clear-history"
                    onClicked: search_TextField_FileNameText.text = pageSearch1.returnCleanedText(search_TextField_FileNameText.text)
                }
            }
        }
        RowLayout {
            Controls.Label { text: qsTr("with"); Layout.preferredWidth: pageSearchForm.labelW }
            Controls.ComboBox {
                id: search_ComboBox_TextCriteriaWith
                model: ["All Words", "Exact Phrase", "Begins With", "Any Word", "Regex"]
                onCurrentIndexChanged: {
                    if (currentIndex === 2) {
                        search_ComboBox_TextCriteriaIn.model = ["File names only"];
                    } else {
                        search_ComboBox_TextCriteriaIn.model = ["File names only", "File names or Folder paths", "Folder path only"];
                    }
                }
            }
        }
        RowLayout {
            Controls.Label { text: qsTr("in"); Layout.preferredWidth: pageSearchForm.labelW }
            Controls.ComboBox {
                id: search_ComboBox_TextCriteriaIn
                model: ["File names only", "File names or Folder paths", "Folder path only"]
            }
        }
        RowLayout {
            Controls.Label { text: ""; Layout.preferredWidth: pageSearchForm.labelW }
            Controls.CheckBox {
                id: search_CheckBox_FileNameCaseSensitive
                checked: false
                text: qsTr("case sensitive")
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Controls.Label { text: qsTr("exclude"); Layout.preferredWidth: pageSearchForm.labelW }
            Controls.TextField {
                id: search_TextField_FileNameExclude
                Layout.fillWidth: true
                leftPadding: Kirigami.Units.largeSpacing
                rightPadding: text.length > 0
                              ? Kirigami.Units.iconSizes.small + Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
                              : Kirigami.Units.smallSpacing
                Kirigami.Icon {
                    anchors { right: parent.right; rightMargin: Kirigami.Units.smallSpacing * 2; verticalCenter: parent.verticalCenter }
                    source: parent.LayoutMirroring.enabled ? "edit-clear-locationbar-ltr" : "edit-clear-locationbar-rtl"
                    implicitWidth: Kirigami.Units.iconSizes.small
                    implicitHeight: Kirigami.Units.iconSizes.small
                    visible: parent.text.length > 0
                    opacity: excludeClearTap.pressed ? 0.5 : 1.0
                    Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration } }
                    HoverHandler { cursorShape: Qt.ArrowCursor }
                    TapHandler { id: excludeClearTap; onTapped: search_TextField_FileNameExclude.clear() }
                }
            }
            Controls.Button {
                id: search_Button_ExcludePasteClipboard
                icon.name: "edit-paste"
                onClicked: search_TextField_FileNameExclude.text = pageSearch1.returnClipboard()
            }
            Controls.Button {
                id: search_Button_ExcludeCleanText
                icon.name: "edit-clear-history"
                onClicked: search_TextField_FileNameExclude.text = pageSearch1.returnCleanedText(search_TextField_FileNameExclude.text)
            }
        }
    }
    Controls.Label { text: ""}

    // ── Section 2: File attributes ────────────────────────────────────────────
    Kirigami.Separator { Layout.fillWidth: true; Layout.topMargin: Kirigami.Units.smallSpacing; Layout.bottomMargin: Kirigami.Units.smallSpacing }
    Controls.CheckBox {
        id: checkBoxFileAttributesCriteria
        checked: false
        text: qsTr("File attributes")
        Layout.leftMargin: Kirigami.Units.smallSpacing
        font.bold: checked ? true : false
        onCheckedChanged: fileAtrributeCriteria.visible = checked
    }
    ColumnLayout {
        id: fileAtrributeCriteria
        visible: false
        Layout.leftMargin: Kirigami.Units.largeSpacing
        Layout.fillWidth: true
        spacing: Kirigami.Units.smallSpacing

        RowLayout {
            Controls.CheckBox {
                id: search_checkBox_Type
                checked: false
                text: qsTr("Type")
                Layout.preferredWidth: pageSearchForm.labelW
                onCheckedChanged: search_comboBox_FileType.enabled = checked
            }
            Controls.ComboBox {
                id: search_comboBox_FileType
                enabled: false
                textRole: "text"
                valueRole: "value"
                displayText: ""
                model: [
                    {text: qsTr("All"),   value: "All",   iconName: "folder"},
                    {text: qsTr("Audio"), value: "Audio", iconName: "audio-x-mpeg"},
                    {text: qsTr("Image"), value: "Image", iconName: "image-jpeg"},
                    {text: qsTr("Text"),  value: "Text",  iconName: "view-list-text"},
                    {text: qsTr("Video"), value: "Video", iconName: "video-mp4"},
                    {text: qsTr("Other"), value: "Other", iconName: "document-open"},
                    {text: qsTr("None"),  value: "None",  iconName: "application-x-zerosize"}
                ]
                delegate: Controls.ItemDelegate {
                    width: search_comboBox_FileType.width
                    text: modelData.text
                    icon.name: modelData.iconName
                    highlighted: search_comboBox_FileType.highlightedIndex === index
                }
                contentItem: RowLayout {
                    function positionToRectangle(pos) { return Qt.rect(0, 0, 0, 0) }
                    property int selectionStart: 0
                    spacing: Kirigami.Units.smallSpacing
                    Item { width: Kirigami.Units.smallSpacing }
                    Kirigami.Icon {
                        source: search_comboBox_FileType.currentIndex >= 0
                                ? search_comboBox_FileType.model[search_comboBox_FileType.currentIndex].iconName : ""
                        implicitWidth:  Kirigami.Units.iconSizes.small
                        implicitHeight: Kirigami.Units.iconSizes.small
                    }
                    Controls.Label {
                        text: search_comboBox_FileType.currentIndex >= 0
                              ? search_comboBox_FileType.model[search_comboBox_FileType.currentIndex].text : ""
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Controls.CheckBox {
                id: search_checkBox_Size
                checked: false
                text: qsTr("Size")
                Layout.preferredWidth: pageSearchForm.labelW
                Layout.alignment: Qt.AlignTop
                onCheckedChanged: {
                    search_spinBox_MinimumSize.enabled = checked
                    search_comboBox_MinSizeUnit.enabled = checked
                    search_spinBox_MaximumSize.enabled = checked
                    search_comboBox_MaxSizeUnit.enabled = checked
                }
            }
            Flow {
                Layout.fillWidth: true
                spacing: Kirigami.Units.largeSpacing
                RowLayout {
                    Controls.Label { text: ">" }
                    Controls.SpinBox {
                        id: search_spinBox_MinimumSize
                        enabled: false
                        from: 0; value: 0; to: 1000
                        implicitWidth: 110
                    }
                    Controls.ComboBox {
                        id: search_comboBox_MinSizeUnit
                        enabled: false
                        model: ["Bytes", "KiB", "MiB", "GiB", "TiB"]
                        implicitWidth: 75
                    }
                }
                RowLayout {
                    Controls.Label { text: "<" }
                    Controls.SpinBox {
                        id: search_spinBox_MaximumSize
                        enabled: false
                        from: 0; value: 1000; to: 1000
                        implicitWidth: 110
                    }
                    Controls.ComboBox {
                        id: search_comboBox_MaxSizeUnit
                        enabled: false
                        model: ["Bytes", "KiB", "MiB", "GiB", "TiB"]
                        currentIndex: 3
                        implicitWidth: 75
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Controls.CheckBox {
                id: search_checkBox_Date
                checked: false
                text: qsTr("Date")
                Layout.preferredWidth: pageSearchForm.labelW
                Layout.alignment: Qt.AlignTop
                onCheckedChanged: {
                    search_dateTimeEdit_Min.enabled = checked
                    search_button_ShowMinDateCalendar.enabled = checked
                    search_dateTimeEdit_Max.enabled = checked
                    search_button_ShowMaxDateCalendar.enabled = checked
                }
            }
            Flow {
                Layout.fillWidth: true
                spacing: Kirigami.Units.largeSpacing
                RowLayout {
                    Controls.Label { text: ">" }
                    Controls.TextField {
                        id: search_dateTimeEdit_Min
                        enabled: false
                        inputMask: "9999/99/99 99:99:99"
                        inputMethodHints: Qt.ImhDigitsOnly
                        text: "1970/01/01 00:00:00"
                        implicitWidth: 150
                    }
                    Controls.Button {
                        id: search_button_ShowMinDateCalendar
                        enabled: false
                        icon.name: "view-calendar"
                        onClicked: { dateDialog.selectedDateField = "Min"; dateDialog.open() }
                    }
                }
                RowLayout {
                    Controls.Label { text: "<" }
                    Controls.TextField {
                        id: search_dateTimeEdit_Max
                        enabled: false
                        inputMask: "9999/99/99 99:99:99"
                        inputMethodHints: Qt.ImhDigitsOnly
                        text: "2030/01/01 00:00:00"
                        implicitWidth: 150
                    }
                    Controls.Button {
                        id: search_button_ShowMaxDateCalendar
                        enabled: false
                        icon.name: "view-calendar"
                        onClicked: { dateDialog.selectedDateField = "Max"; dateDialog.open() }
                    }
                }
            }
        }
    }
    Controls.Label { text: ""}

    // ── Section 3: File metadata ──────────────────────────────────────────────
    Kirigami.Separator { Layout.fillWidth: true; Layout.topMargin: Kirigami.Units.smallSpacing; Layout.bottomMargin: Kirigami.Units.smallSpacing }
    Controls.CheckBox {
        id: search_checkBox_FileMetadata
        checked: false
        text: qsTr("File metadata")
        Layout.leftMargin: Kirigami.Units.smallSpacing
        font.bold: checked ? true : false
        onCheckedChanged: search_FormLayout_FileMetadata.visible = checked
    }
    ColumnLayout {
        id: search_FormLayout_FileMetadata
        visible: false
        Layout.leftMargin: Kirigami.Units.largeSpacing
        Layout.fillWidth: true
        spacing: Kirigami.Units.smallSpacing

        RowLayout {
            Layout.fillWidth: true
            Controls.CheckBox {
                id: search_checkBox_MetadataText
                checked: false
                text: qsTr("Text")
                Layout.preferredWidth: pageSearchForm.labelW
                onCheckedChanged: search_lineEdit_MetadataText.enabled = checked
            }
            Controls.TextField {
                id: search_lineEdit_MetadataText
                enabled: false
                Layout.fillWidth: true
                leftPadding: Kirigami.Units.largeSpacing
                rightPadding: text.length > 0
                              ? Kirigami.Units.iconSizes.small + Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
                              : Kirigami.Units.smallSpacing
                Kirigami.Icon {
                    anchors { right: parent.right; rightMargin: Kirigami.Units.smallSpacing * 2; verticalCenter: parent.verticalCenter }
                    source: parent.LayoutMirroring.enabled ? "edit-clear-locationbar-ltr" : "edit-clear-locationbar-rtl"
                    implicitWidth: Kirigami.Units.iconSizes.small
                    implicitHeight: Kirigami.Units.iconSizes.small
                    visible: parent.text.length > 0
                    opacity: metaClearTap.pressed ? 0.5 : 1.0
                    Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration } }
                    HoverHandler { cursorShape: Qt.ArrowCursor }
                    TapHandler { id: metaClearTap; onTapped: search_lineEdit_MetadataText.clear() }
                }
            }
            Controls.Button {
                icon.name: "edit-paste"
                enabled: search_checkBox_MetadataText.checked
                onClicked: search_lineEdit_MetadataText.text = pageSearch1.returnClipboard()
            }
            Controls.Button {
                icon.name: "edit-clear-history"
                enabled: search_checkBox_MetadataText.checked
                onClicked: search_lineEdit_MetadataText.text = pageSearch1.returnCleanedText(search_lineEdit_MetadataText.text)
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Controls.CheckBox {
                id: search_checkBox_MetadataSize
                checked: false
                text: qsTr("Size")
                Layout.preferredWidth: pageSearchForm.labelW
                Layout.alignment: Qt.AlignTop
                onCheckedChanged: {
                    search_spinBox_MetadataMinimumHeight.enabled = checked
                    search_spinBox_MetadataMaximumHeight.enabled = checked
                    search_spinBox_MetadataMinimumWidth.enabled  = checked
                    search_spinBox_MetadataMaximumWidth.enabled  = checked
                }
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing
                Flow {
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.largeSpacing
                    RowLayout {
                        Controls.Label { text: qsTr("Width"); Layout.preferredWidth: 50 }
                        Controls.Label { text: ">" }
                        Controls.SpinBox {
                            id: search_spinBox_MetadataMinimumWidth
                            enabled: false
                            from: 0; value: 0; to: 30000
                            implicitWidth: 110
                        }
                    }
                    RowLayout {
                        Controls.Label { text: "<" }
                        Controls.SpinBox {
                            id: search_spinBox_MetadataMaximumWidth
                            enabled: false
                            from: 0; value: 30000; to: 30000
                            implicitWidth: 110
                        }
                    }
                }
                Flow {
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.largeSpacing
                    RowLayout {
                        Controls.Label { text: qsTr("Height"); Layout.preferredWidth: 50 }
                        Controls.Label { text: ">" }
                        Controls.SpinBox {
                            id: search_spinBox_MetadataMinimumHeight
                            enabled: false
                            from: 0; value: 0; to: 30000
                            implicitWidth: 110
                        }
                    }
                    RowLayout {
                        Controls.Label { text: "<" }
                        Controls.SpinBox {
                            id: search_spinBox_MetadataMaximumHeight
                            enabled: false
                            from: 0; value: 30000; to: 30000
                            implicitWidth: 110
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Controls.CheckBox {
                id: search_checkBox_MetadataDuration
                checked: false
                text: qsTr("Duration")
                Layout.preferredWidth: pageSearchForm.labelW
                Layout.alignment: Qt.AlignTop
                onCheckedChanged: {
                    search_dateTimeEdit_MetadataDurationMin.enabled = checked
                    search_dateTimeEdit_MetadataDurationMax.enabled = checked
                }
            }
            Flow {
                Layout.fillWidth: true
                spacing: Kirigami.Units.largeSpacing
                RowLayout {
                    Controls.Label { text: ">" }
                    Controls.TextField {
                        id: search_dateTimeEdit_MetadataDurationMin
                        enabled: false
                        inputMask: "99:99:99"
                        text: "00:00:00"
                        implicitWidth: 80
                    }
                }
                RowLayout {
                    Controls.Label { text: "<" }
                    Controls.TextField {
                        id: search_dateTimeEdit_MetadataDurationMax
                        enabled: false
                        inputMask: "99:99:99"
                        text: "23:59:59"
                        implicitWidth: 80
                    }
                }
            }
        }
    }
    Controls.Label { text: ""}

    // ── Section 4: Folder criteria ────────────────────────────────────────────
    Kirigami.Separator { Layout.fillWidth: true; Layout.topMargin: Kirigami.Units.smallSpacing; Layout.bottomMargin: Kirigami.Units.smallSpacing }
    Controls.CheckBox {
        id: search_checkBox_FolderCriteria
        checked: false
        text: qsTr("Folder criteria")
        Layout.leftMargin: Kirigami.Units.smallSpacing
        font.bold: checked ? true : false
        onCheckedChanged: search_FormLayout_folderCriteria.visible = checked
    }
    ColumnLayout {
        id: search_FormLayout_folderCriteria
        visible: false
        Layout.leftMargin: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.smallSpacing

        Controls.CheckBox {
            id: search_checkBox_ShowFoldersOnly
            checked: false
            text: qsTr("only list folders in results")
        }
        RowLayout {
            Controls.CheckBox {
                id: search_checkBox_SearchOnTags
                checked: false
                text: qsTr("Tag")
                Layout.preferredWidth: pageSearchForm.labelW
                onCheckedChanged: search_comboBox_FolderTag.enabled = checked
            }
            Controls.ComboBox {
                id: search_comboBox_FolderTag
                enabled: false
                model: pageSearchForm.tagNames
            }
        }
    }
    Controls.Label { text: ""}

    // ── Section 5: Duplicates ─────────────────────────────────────────────────
    Kirigami.Separator { Layout.fillWidth: true; Layout.topMargin: Kirigami.Units.smallSpacing; Layout.bottomMargin: Kirigami.Units.smallSpacing }
    Controls.CheckBox {
        id: search_checkBox_Duplicates
        checked: false
        text: qsTr("Duplicates")
        Layout.leftMargin: Kirigami.Units.smallSpacing
        font.bold: checked ? true : false
        onCheckedChanged: {
            search_FormLayout_Duplicates.visible = checked
            if (checked && search_checkBox_Differences.checked) {
                showPassiveNotification(qsTr("Duplicates and Differences cannot be used at the same time"))
                search_checkBox_Differences.checked = false
            }
        }
    }
    ColumnLayout {
        id: search_FormLayout_Duplicates
        visible: false
        Layout.leftMargin: Kirigami.Units.largeSpacing
        Layout.fillWidth: true
        spacing: Kirigami.Units.smallSpacing

        RowLayout {
            Layout.fillWidth: true
            Controls.Label { text: qsTr("On"); Layout.preferredWidth: pageSearchForm.labelW; Layout.alignment: Qt.AlignTop }
            Flow {
                Layout.fillWidth: true
                spacing: Kirigami.Units.largeSpacing
                RowLayout {
                    Controls.CheckBox {
                        id: search_checkBox_DuplicatesOnName
                        checked: true
                        text: qsTr("Name")
                        onCheckedChanged: {
                            if (!search_checkBox_DuplicatesOnSize.checked && !search_checkBox_DuplicatesOnDate.checked
                                    && !search_checkBox_DuplicatesOnChecksum.checked)
                                search_checkBox_DuplicatesOnName.checked = true
                        }
                    }
                    Controls.CheckBox {
                        id: search_checkBox_DuplicatesOnSize
                        checked: false
                        text: qsTr("Size")
                        onCheckedChanged: {
                            if (!search_checkBox_DuplicatesOnName.checked && !search_checkBox_DuplicatesOnDate.checked
                                    && !search_checkBox_DuplicatesOnChecksum.checked)
                                search_checkBox_DuplicatesOnSize.checked = true
                        }
                    }
                    Controls.CheckBox {
                        id: search_checkBox_DuplicatesOnDate
                        checked: false
                        text: qsTr("Date")
                        onCheckedChanged: {
                            if (!search_checkBox_DuplicatesOnSize.checked && !search_checkBox_DuplicatesOnName.checked
                                    && !search_checkBox_DuplicatesOnChecksum.checked)
                                search_checkBox_DuplicatesOnDate.checked = true
                        }
                    }
                }
                RowLayout {
                    Controls.ComboBox {
                        id: search_comboBox_DuplicateChecksumSign
                        model: ["=", "≠"]
                        implicitWidth: 60
                        enabled: search_checkBox_DuplicatesOnChecksum.checked
                    }
                    Controls.CheckBox {
                        id: search_checkBox_DuplicatesOnChecksum
                        checked: false
                        text: qsTr("Checksum")
                        onCheckedChanged: {
                            search_comboBox_DuplicateChecksumSign.enabled = checked
                            if (!search_checkBox_DuplicatesOnName.checked && !search_checkBox_DuplicatesOnSize.checked
                                    && !search_checkBox_DuplicatesOnDate.checked)
                                search_checkBox_DuplicatesOnChecksum.checked = true
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Controls.Label { text: qsTr("Scope"); Layout.preferredWidth: pageSearchForm.labelW; Layout.alignment: Qt.AlignTop }
            Flow {
                Layout.fillWidth: true
                spacing: Kirigami.Units.largeSpacing
                Controls.RadioButton {
                    id: search_radioButton_DuplicatesWithinSelectedDevice
                    checked: true
                    text: qsTr("Within selected device")
                }
                Controls.RadioButton {
                    id: search_radioButton_DuplicatesCompareTwoDevices
                    text: qsTr("Compare two devices")
                    onCheckedChanged: search_FormLayout_DuplicatesDevices.visible = checked
                }
            }
        }

        Flow {
            id: search_FormLayout_DuplicatesDevices
            visible: false
            Layout.leftMargin: pageSearchForm.labelW + Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            spacing: Kirigami.Units.largeSpacing

            RowLayout {
                Controls.Label { text: qsTr("Device 1") }
                DeviceTreeComboBox { id: search_comboBox_DuplicatesDevice1 }
            }
            RowLayout {
                Controls.Label { text: qsTr("Device 2") }
                DeviceTreeComboBox { id: search_comboBox_DuplicatesDevice2 }
            }
        }
    }
    Controls.Label { text: ""}

    // ── Section 6: Differences ────────────────────────────────────────────────
    Kirigami.Separator { Layout.fillWidth: true; Layout.topMargin: Kirigami.Units.smallSpacing; Layout.bottomMargin: Kirigami.Units.smallSpacing }
    Controls.CheckBox {
        id: search_checkBox_Differences
        checked: false
        enabled: search_radioButton_SearchInCatalogs.checked
        text: qsTr("Differences")
        Layout.leftMargin: Kirigami.Units.smallSpacing
        font.bold: checked ? true : false
        onCheckedChanged: {
            search_FormLayout_Differences.visible = checked
            if (checked && search_checkBox_Duplicates.checked) {
                showPassiveNotification(qsTr("Duplicates and Differences cannot be used at the same time"))
                search_checkBox_Duplicates.checked = false
            }
        }
    }
    ColumnLayout {
        id: search_FormLayout_Differences
        visible: false
        Layout.leftMargin: Kirigami.Units.largeSpacing
        Layout.fillWidth: true
        spacing: Kirigami.Units.smallSpacing

        RowLayout {
            Layout.fillWidth: true
            Controls.Label { text: qsTr("On"); Layout.preferredWidth: pageSearchForm.labelW; Layout.alignment: Qt.AlignTop }
            Flow {
                Layout.fillWidth: true
                spacing: Kirigami.Units.largeSpacing
                RowLayout {
                    Controls.CheckBox {
                        id: search_checkBox_DifferencesOnName
                        checked: true
                        text: qsTr("Name")
                        onCheckedChanged: {
                            if (!search_checkBox_DifferencesOnSize.checked && !search_checkBox_DifferencesOnDate.checked
                                    && !search_checkBox_DifferencesOnChecksum.checked)
                                search_checkBox_DifferencesOnName.checked = true
                        }
                    }
                    Controls.CheckBox {
                        id: search_checkBox_DifferencesOnSize
                        checked: false
                        text: qsTr("Size")
                        onCheckedChanged: {
                            if (!search_checkBox_DifferencesOnName.checked && !search_checkBox_DifferencesOnDate.checked
                                    && !search_checkBox_DifferencesOnChecksum.checked)
                                search_checkBox_DifferencesOnSize.checked = true
                        }
                    }
                    Controls.CheckBox {
                        id: search_checkBox_DifferencesOnDate
                        checked: false
                        text: qsTr("Date")
                        onCheckedChanged: {
                            if (!search_checkBox_DifferencesOnSize.checked && !search_checkBox_DifferencesOnName.checked
                                    && !search_checkBox_DifferencesOnChecksum.checked)
                                search_checkBox_DifferencesOnDate.checked = true
                        }
                    }
                }
                RowLayout {
                    Controls.ComboBox {
                        id: search_comboBox_DifferenceChecksumSign
                        model: ["=", "≠"]
                        implicitWidth: 60
                        enabled: search_checkBox_DifferencesOnChecksum.checked
                    }
                    Controls.CheckBox {
                        id: search_checkBox_DifferencesOnChecksum
                        checked: false
                        text: qsTr("Checksum")
                        onCheckedChanged: {
                            search_comboBox_DifferenceChecksumSign.enabled = checked
                            if (!search_checkBox_DifferencesOnName.checked && !search_checkBox_DifferencesOnSize.checked
                                    && !search_checkBox_DifferencesOnDate.checked)
                                search_checkBox_DifferencesOnChecksum.checked = true
                        }
                    }
                }
            }
        }

        Flow {
            Layout.leftMargin: pageSearchForm.labelW + Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            spacing: Kirigami.Units.largeSpacing
            RowLayout {
                Controls.Label { text: qsTr("Between") }
                DeviceTreeComboBox { id: search_comboBox_DifferencesDevice1 }
            }
            RowLayout {
                Controls.Label { text: qsTr("And") }
                DeviceTreeComboBox { id: search_comboBox_DifferencesDevice2 }
            }
        }
    }

    Item { Layout.fillHeight: true }
}
