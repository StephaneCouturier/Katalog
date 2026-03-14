import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import Qt.labs.platform

Kirigami.FormLayout {
    id: pageSearchForm

    function getCriteria() {
        //showPassiveNotification("debug");

        //Global type of search
        newSearch1.properties = {"searchInCatalogsChecked": true}; //DEV: TEMP

        //File name
        newSearch1.properties = {"searchOnFileName":        search_checkBox_FileNameCriteria.checked};
        newSearch1.properties = {"searchText":              search_TextField_FileNameText.text};
        newSearch1.properties = {"selectedSearchWith":      search_ComboBox_TextCriteriaWith.currentText};
        newSearch1.properties = {"selectedSearchIn":        search_ComboBox_TextCriteriaIn.currentText};
        newSearch1.properties = {"caseSensitive":           search_CheckBox_FileNameCaseSensitive.checked};
        newSearch1.properties = {"selectedSearchExclude":   search_TextField_FileNameExclude.text};

        //File Attributes
        newSearch1.properties = {"searchOnFileCriteria":    checkBoxFileAttributesCriteria.checked};
        //Type
        newSearch1.properties = {"searchOnType":            checkBoxFileAttributesCriteria.checked};
        newSearch1.properties = {"selectedFileType":        search_comboBox_FileType.currentText};
        //Size
        newSearch1.properties = {"searchOnSize":            checkBoxFileAttributesCriteria.checked};
        newSearch1.properties = {"selectedMinimumSize":     search_spinBox_MinimumSize.value};
        newSearch1.properties = {"selectedMaximumSize":     search_spinBox_MaximumSize.value};
        newSearch1.properties = {"selectedMinSizeUnit":     search_comboBox_MinSizeUnit.currentText};
        newSearch1.properties = {"selectedMaxSizeUnit":     search_comboBox_MaxSizeUnit.currentText};       
        //Date
        newSearch1.properties = {"searchOnDate":            checkBoxFileAttributesCriteria.checked};
        newSearch1.properties = {"selectedDateMin":         search_dateTimeEdit_Min.text};
        newSearch1.properties = {"selectedDateMax":         search_dateTimeEdit_Max.text};

        //Folder Attributes
        newSearch1.properties = {"searchOnFolderCriteria":  search_checkBox_FolderCriteria.checked};
        newSearch1.properties = {"showFoldersOnly":         search_checkBox_ShowFoldersOnly.checked};
        newSearch1.properties = {"searchOnTags":            search_checkBox_SearchOnTags.checked};
        newSearch1.properties = {"selectedTagName":         search_comboBox_FolderTag.currentText};

        //Duplicates
        newSearch1.properties = {"searchOnDuplicates":      search_checkBox_Duplicates.checked};
        newSearch1.properties = {"searchDuplicatesOnName":  search_checkBox_DuplicatesOnName.checked};
        newSearch1.properties = {"searchDuplicatesOnSize":  search_checkBox_DuplicatesOnSize.checked};
        newSearch1.properties = {"searchDuplicatesOnDate":  search_checkBox_DuplicatesOnDate.checked};

        //Differences
        newSearch1.properties = {"searchOnDifferences":     search_checkBox_Differences.checked};
        newSearch1.properties = {"searchDifferencesOnName": search_checkBox_DifferencesOnName.checked};
        newSearch1.properties = {"searchDifferencesOnSize": search_checkBox_DifferencesOnSize.checked};
        newSearch1.properties = {"searchDifferencesOnDate": search_checkBox_DifferencesOnDate.checked};
        newSearch1.properties = {"differencesDevice1":      search_comboBox_DifferencesDevice1.selected_device_id};
        newSearch1.properties = {"differencesDevice1":      search_comboBox_DifferencesDevice2.selected_device_id};

        /*


        newSearch1->selectedStorage          = ui->Filters_label_DisplayStorage->text();
        newSearch1->selectedCatalog          = ui->Filters_label_DisplayCatalog->text();
        newSearch1->searchInCatalogsChecked  = ui->Filters_checkBox_SearchInCatalogs->isChecked();
        newSearch1->searchInConnectedChecked = ui->Filters_checkBox_SearchInConnectedDrives->isChecked();
        newSearch1->connectedDirectory       = ui->Filters_lineEdit_SeletedDirectory->text();
        */
    }
    function resetSearch() {
        // Reset fileNameCriteria
        search_checkBox_FileNameCriteria.checked = true
        search_TextField_FileNameText.text = ""
        search_ComboBox_TextCriteriaWith.currentIndex = 0
        search_ComboBox_TextCriteriaIn.currentIndex = 0
        search_CheckBox_FileNameCaseSensitive.checked = false
        search_TextField_FileNameExclude.text = ""

        // Reset fileAtrributes
            //Size, Date, Type
            checkBoxFileAttributesCriteria.checked = false
            search_spinBox_MinimumSize.value = 0
            search_comboBox_MinSizeUnit.currentIndex = 0 //"Bytes"
            search_spinBox_MaximumSize.value = 1000
            search_comboBox_MaxSizeUnit.currentIndex = 3 //"Gb"
            search_dateTimeEdit_Min.text = "1970/01/01 00:00:00"
            search_dateTimeEdit_Max.text = "2030/01/01 00:00:00"
            search_comboBox_FileType.currentIndex = 0

        //Duplicates
        search_checkBox_Duplicates.checked = false
        search_checkBox_DuplicatesOnName.checked = true
        search_checkBox_DuplicatesOnSize.checked = false
        search_checkBox_DuplicatesOnDate.checked = false

        //Differences
        search_checkBox_Differences.checked = false
        search_checkBox_DifferencesOnName.checked = true
        search_checkBox_DifferencesOnSize.checked = false
        search_checkBox_DifferencesOnDate.checked = false
        search_comboBox_DifferencesDevice1.currentIndex = 0
        search_comboBox_DifferencesDevice2.currentIndex = 0

        // Reset folderAttributes
        search_checkBox_FolderCriteria.checked = false
        search_comboBox_FolderTag.currentIndex = 0
    }
    function returnCleanedDate(date) {
        //Function to return a date formatted as yyyy/mm/dd hh:mm:ss
        var dateArray = date.split(" ");
        var datePart = dateArray[0].split("/");
        var timePart = dateArray[1].split(":");

        var yyyy = datePart[0];
        var mm = datePart[1];
        var dd = datePart[2];
        var hh = timePart[0];
        var min = timePart[1];
        var ss = timePart[2];

        if(dd<10) {
            dd = '0'+dd
        }
        if(mm<10) {
            mm = '0'+mm
        }
        if(hh<10) {
            hh = '0'+hh
        }
        if(min<10) {
            min = '0'+min
        }
        if(ss<10) {
            ss = '0'+ss
        }

        return yyyy + '/' + mm + '/' + dd + ' ' + hh + ':' + min + ':' + ss;
    }
    function executeSearch() {
        console.log("=== QML SEARCH START ===");
        // Trigger the search signal on root
        root.searchTriggered();

        // Get search criteria
        getCriteria();

        // Debug: Check what's set on newSearch1
        console.log("QML - searchOnFileName:", newSearch1.properties.searchOnFileName);
        console.log("QML - searchText:", newSearch1.properties.searchText);
        console.log("QML - searchInCatalogsChecked:", newSearch1.properties.searchInCatalogsChecked);

        // Execute search through AppManager
        appManager1.executeSearch();

        // Check results
        console.log("QML - filesFoundNumber:", newSearch1.properties.filesFoundNumber);

        // Navigate to results page
        pageStack.removePage(pageSearchResults);
        pageStack.insertPage(3, pageSearchResults);
        pageSearchResultsForm.pageSearchResults_tableView_results.forceLayout();

        // Show notification with results
        showPassiveNotification("newSearch1.properties.filesFoundNumber: " + newSearch1.properties.filesFoundNumber);
    }

    Kirigami.Dialog {
        id: dateDialog
        property string selectedDateField
        //property alias selectedDate: search_button_ShowMaxDateCalendar.selectedDate
        title: "Select a date for " + selectedDateField + " date"
        //Give the user the choice between: Now, 1 day ago, 1 month ago, 1 year ago.
        //Apply it to the search_dateTimeEdit_Max or search_dateTimeEdit_Min depending on user selection,

        contentItem: ColumnLayout {
            RowLayout {
                Controls.Button {
                    text: "Now"
                    onClicked: {
                        var today = new Date();
                        var dd = today.getDate();
                        var mm = today.getMonth()+1; //January is 0!
                        var yyyy = today.getFullYear();
                        var hh = today.getHours();
                        var min = today.getMinutes();
                        var ss = today.getSeconds();

                        var todayDate = yyyy + '/' + mm + '/' + dd + ' ' + hh + ':' + min + ':' + ss;

                        //update the search_dateTimeEdit_Max or search_dateTimeEdit_Min depending on selectedDateField
                        if (dateDialog.selectedDateField === "Min") {
                            search_dateTimeEdit_Min.text = returnCleanedDate(todayDate)
                        } else {
                            search_dateTimeEdit_Max.text = returnCleanedDate(todayDate)
                        }
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
                        if (dateDialog.selectedDateField === "Min") {
                            search_dateTimeEdit_Min.text = returnCleanedDate(fullDate)
                        } else {
                            search_dateTimeEdit_Max.text = returnCleanedDate(fullDate)
                        }
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
                        if (dateDialog.selectedDateField === "Min") {
                            search_dateTimeEdit_Min.text = returnCleanedDate(fullDate)
                        } else {
                            search_dateTimeEdit_Max.text = returnCleanedDate(fullDate)
                        }
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
                        if (dateDialog.selectedDateField === "Min") {
                            search_dateTimeEdit_Min.text = returnCleanedDate(fullDate)
                        } else {
                            search_dateTimeEdit_Max.text = returnCleanedDate(fullDate)
                        }
                        dateDialog.close()
                    }
                }
                //Add a button "Reset" to reset the date and time to its default
                Controls.Button {
                    text: "Reset"
                    onClicked: {
                        if (dateDialog.selectedDateField === "Min") {
                            search_dateTimeEdit_Min.text = "1970/01/01 00:00:00"
                        } else {
                            search_dateTimeEdit_Max.text = "2030/01/01 00:00:00"
                        }
                        dateDialog.close()
                    }
                }
            }
        }
    }

    //Section1: File name criteria
    Controls.CheckBox {
        id: search_checkBox_FileNameCriteria
        checked: true
        text: qsTr("File name criteria")
        anchors.left: parent.left
        anchors.leftMargin: 10
        font.bold: checked ? true : false
        onCheckedChanged: {
            search_FormLayout_FileNameCriteria.visible = checked
        }
    }
    Kirigami.FormLayout {
        id: search_FormLayout_FileNameCriteria
        anchors.left: parent.left
        anchors.leftMargin: 25

        RowLayout {
            id:search_RowLayout_FileNameTextRow
            Kirigami.FormData.label: "Search text"
            //property int textFieldWidth: search_TextField_FileNameText.width
            Controls.TextField {
                id: search_TextField_FileNameText
                Kirigami.FormData.label: "File name"

                // When Enter is pressed, trigger the search
                onAccepted: pageSearchForm.executeSearch()
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
        Controls.ComboBox {
            id: search_ComboBox_TextCriteriaWith
            Kirigami.FormData.label: "with"
            model: ["All Words", "Exact Phrase", "Begins With", "Any Word"]
            onCurrentIndexChanged: {
                //For Begins with, only "File names only" should be available
                 if (currentIndex === 2) { // "Begins With" is selected
                     search_ComboBox_TextCriteriaIn.model = ["File names only"];
                 } else {
                     search_ComboBox_TextCriteriaIn.model = ["File names only", "File names or Folder paths", "Folder path only"];
                 }
             }
        }
        Controls.ComboBox {
            id: search_ComboBox_TextCriteriaIn
            Kirigami.FormData.label: "in"
            model: ["File names only", "File names or Folder paths", "Folder path only"]
        }
        Controls.CheckBox {
            id: search_CheckBox_FileNameCaseSensitive
            checked: false
            text: qsTr("Case sensitive")
        }

        RowLayout {
            id:search_RowLayout_FileNameExclude
            Kirigami.FormData.label: "Exclude"
            //property int textFieldWidth: search_TextField_FileNameText.width
            Controls.TextField {
                id: search_TextField_FileNameExclude
                //Kirigami.FormData.label: "Exclude"
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
        Text {
            id: separate0
        }
    }

    //Section2: File attributes criteria
    Kirigami.Separator {
        Kirigami.FormData.isSection: true
        anchors.left: parent.left
    }
    Controls.CheckBox {
        id: checkBoxFileAttributesCriteria
        checked: false
        text: qsTr("File attributes")
        anchors.left: parent.left
        anchors.leftMargin: 10
        font.bold: checked ? true : false

        onCheckedChanged: {
            fileAtrributeCriteria.visible = checked
        }
    }
    Kirigami.FormLayout {
        id: fileAtrributeCriteria
        visible: false
        anchors.left: parent.left
        anchors.leftMargin: 10

        //File size
        RowLayout{
            Kirigami.FormData.label: "Size min"
            Controls.Label{
                id: search_label_SizeGreater
                text: ">"
            }
            Controls.SpinBox {
                id: search_spinBox_MinimumSize
                from: 0
                value: 0
                to: 1000
                implicitWidth: 110
            }
            Controls.ComboBox {
                id: search_comboBox_MinSizeUnit
                model: ["Bytes", "KiB", "MiB", "GiB", "TiB"]
                implicitWidth: 75
            }
        }
        RowLayout{
            id: search_RowLayout_SizeLower
            Kirigami.FormData.label: "Size max"
            Controls.Label{
                id: search_label_SizeLower
                text: "<"
                anchors.leftMargin: 100
            }
            Controls.SpinBox {
                id: search_spinBox_MaximumSize
                from: 0
                value: 1000
                to: 1000
                implicitWidth: 110
            }
            Controls.ComboBox {
                id: search_comboBox_MaxSizeUnit
                model: ["Bytes", "KiB", "MiB", "GiB", "TiB"]
                currentIndex: 3
                implicitWidth: 75
            }
        }
        Text {
            id: separate1
        }
        //File date
        RowLayout{
            Kirigami.FormData.label: "Date min"
            Controls.Label{
                id: search_label_DateGreater
                text: ">"
            }
            Controls.TextField {
                id: search_dateTimeEdit_Min
                inputMask: "9999/99/99 99:99:99"
                inputMethodHints: Qt.ImhDigitsOnly
                text: "1970/01/01 00:00:00"
                Layout.preferredWidth: 150
            }
            Controls.Button {
                id: search_button_ShowMinDateCalendar
                icon.name: "view-calendar"
                //onClicked: search_dateTimeEdit_Min.text = returnTodayDate()
                onClicked: {
                    dateDialog.selectedDateField = "Min"
                    dateDialog.open()
                }
            }
        }
        RowLayout{
            Kirigami.FormData.label: "Date max"
            Controls.Label{
                id: search_label_DateLower
                text: "<"
            }
            Controls.TextField {
                id: search_dateTimeEdit_Max
                inputMask: "9999/99/99 99:99:99"
                inputMethodHints: Qt.ImhDigitsOnly
                text: "2030/01/01 00:00:00"
                Layout.preferredWidth: 150
            }
            Controls.Button {
                id: search_button_ShowMaxDateCalendar
                icon.name: "view-calendar"
                property string selectedDate: "1970/01/01"
                onClicked: {
                    dateDialog.selectedDateField = "Max"
                    dateDialog.open()
                }

            }
        }
        Text {
            id: separate2
        }      

        //Type
        RowLayout{
            Kirigami.FormData.label: "File type:"
            Controls.Label{
                id: search_label_Spacer1
                text: ">"
            }
            Controls.ComboBox{
                id: search_comboBox_FileType
                model: ["All", "Audio", "Image", "Text", "Video"]
            }
        }
        Text {
            id: separate3
        }
    }

    //Section3: Folder criteria
    Kirigami.Separator {
        Kirigami.FormData.isSection: true
        anchors.left: parent.left
    }
    Controls.CheckBox {
        id: search_checkBox_FolderCriteria
        checked: false
        text: qsTr("Folder attributes")
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.topMargin: 100
        font.bold: checked ? true : false

        onCheckedChanged: {
            search_FormLayout_folderCriteria.visible = checked
        }
    }
    Kirigami.FormLayout {
        //anchors.fill: parent
        id: search_FormLayout_folderCriteria
        visible: false
        anchors.left: parent.left
        anchors.leftMargin: 25

        Controls.CheckBox {
            id: search_checkBox_ShowFoldersOnly
            checked: false
            text: qsTr("Only show Folders in the results")
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.topMargin: 100
        }
        Text {
            id: separate5
        }
        Controls.CheckBox {
            id: search_checkBox_SearchOnTags
            checked: false

            text: qsTr("Search On Tags")
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.topMargin: 100
            onCheckedChanged: {
                search_comboBox_FolderTag.enabled = checked
            }
        }
        Controls.ComboBox{
            id: search_comboBox_FolderTag
            Kirigami.FormData.label: "Tag"
            enabled: false
            model: ["All", "Tag1", "Tag2"]
        }
        Text {
            id: separate4
        }
    }

    //Section4: Duplicates
    Kirigami.Separator {
        Kirigami.FormData.isSection: true
        anchors.left: parent.left
    }
    Controls.CheckBox {
        id: search_checkBox_Duplicates
        checked: false
        text: qsTr("Duplicates")
        anchors.left: parent.left
        anchors.leftMargin: 10
        font.bold: checked ? true : false

        onCheckedChanged: {
            search_FormLayout_Duplicates.visible = checked

            if (search_checkBox_Duplicates.checked && search_checkBox_Differences.checked) {
                showPassiveNotification("Duplicates and Differences cannot be used at the same time")
                search_checkBox_Differences.checked = false
            }
        }
    }
    Kirigami.FormLayout {
        id: search_FormLayout_Duplicates
        visible: false
        anchors.left: parent.left
        anchors.leftMargin: 10

       RowLayout {
            id:search_RowLayout_Duplicates
            Kirigami.FormData.label: "On"
            Controls.CheckBox {
                id: search_checkBox_DuplicatesOnName
                checked: true
                //enabled: false
                text: qsTr("Name")
                onCheckedChanged: {
                    //leave it checked if other 2 checkboxes are unchecked
                    if (!search_checkBox_DuplicatesOnSize.checked && !search_checkBox_DuplicatesOnDate.checked) {
                        search_checkBox_DuplicatesOnName.checked = true
                    }
                }
            }
            Controls.CheckBox {
                id: search_checkBox_DuplicatesOnSize
                checked: false
                //enabled: false
                text: qsTr("Size")
                onCheckedChanged: {
                    if (!search_checkBox_DuplicatesOnName.checked && !search_checkBox_DuplicatesOnDate.checked) {
                        search_checkBox_DuplicatesOnSize.checked = true
                    }
                }
            }
            Controls.CheckBox {
                id: search_checkBox_DuplicatesOnDate
                checked: false
                //enabled: false
                text: qsTr("Date")
                onCheckedChanged: {
                    if (!search_checkBox_DuplicatesOnSize.checked && !search_checkBox_DuplicatesOnName.checked) {
                        search_checkBox_DuplicatesOnDate.checked = true
                    }
                }
            }
        }
        Text {
            id: separate7
        }
    }

    //Section5: Differences
    Kirigami.Separator {
        Kirigami.FormData.isSection: true
        anchors.left: parent.left
    }
    Controls.CheckBox {
        id: search_checkBox_Differences
        checked: false
        text: qsTr("Differences")
        anchors.left: parent.left
        anchors.leftMargin: 10
        font.bold: checked ? true : false

        onCheckedChanged: {
            search_FormLayout_Differences.visible = checked

            if (search_checkBox_Differences.checked && search_checkBox_Duplicates.checked) {
                showPassiveNotification("Duplicates and Differences cannot be used at the same time")
                search_checkBox_Duplicates.checked = false
            }
        }
    }
    Kirigami.FormLayout {
        id: search_FormLayout_Differences
        visible: false
        anchors.left: parent.left
        anchors.leftMargin: 10

        RowLayout {
            id:search_RowLayout_Differences
            Kirigami.FormData.label: "On"
            Controls.CheckBox {
                id: search_checkBox_DifferencesOnName
                checked: true
                text: qsTr("Name")
                onCheckedChanged: {
                    //leave it checked if other 2 checkboxes are unchecked
                    if (!search_checkBox_DifferencesOnSize.checked && !search_checkBox_DifferencesOnDate.checked) {
                        search_checkBox_DifferencesOnName.checked = true
                    }
                }
            }
            Controls.CheckBox {
                id: search_checkBox_DifferencesOnSize
                checked: false
                text: qsTr("Size")
                onCheckedChanged: {
                    if (!search_checkBox_DifferencesOnName.checked && !search_checkBox_DifferencesOnDate.checked) {
                        search_checkBox_DifferencesOnSize.checked = true
                    }
                }
            }
            Controls.CheckBox {
                id: search_checkBox_DifferencesOnDate
                checked: false
                text: qsTr("Date")
                onCheckedChanged: {
                    if (!search_checkBox_DifferencesOnSize.checked && !search_checkBox_DifferencesOnName.checked) {
                        search_checkBox_DifferencesOnDate.checked = true
                    }
                }
            }
        }
        ListModel {
            id: selectedDevices
            ListElement {
                device_id: 1
                type: "Virtual"
                name: "1- Media"
                description: "5Tb"
                isActive: true
            }
            ListElement {
                device_id: 2
                type: "Virtual"
                name: "2- BackUp"
                description: "2Tb"
                isActive: true
            }
            ListElement {
                device_id: 3
                type: "Storage"
                name: "Drive1"
                description: "1Tb"
                isActive: true
            }
            ListElement {
                device_id: 4
                type: "Catalog"
                name: "Catalog1"
                description: "10 220 files, 200 Mb"
                isActive: true
            }
            ListElement {
                device_id: 5
                type: "Storage"
                name: "Drive2"
                description: "1Tb"
                isActive: true
            }
            ListElement {
                device_id: 6
                type: "Catalog"
                name: "Catalog2"
                description: ""
                isActive: false
            }
            ListElement {
                device_id: 333
                type: "Catalog"
                name: "Catalog3"
                description: ""
                isActive: false
            }
        }
        Controls.ComboBox {
            id: search_comboBox_DifferencesDevice1
            Kirigami.FormData.label: "Between"
            model: selectedDevices
            textRole: "name"
            property int selected_device_id
            onCurrentIndexChanged: {
                selected_device_id = selectedDevices.get(currentIndex).device_id
            }
        }
        Controls.ComboBox {
            id: search_comboBox_DifferencesDevice2
            Kirigami.FormData.label: "And"
            model: selectedDevices
            textRole: "name"
            property int selected_device_id
            onCurrentIndexChanged: {
                selected_device_id = selectedDevices.get(currentIndex).device_id
            }
        }
        Text {
            id: separate8
        }
    }
}
