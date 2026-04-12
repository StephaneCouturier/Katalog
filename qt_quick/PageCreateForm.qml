import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import Qt.labs.platform

Item {
    id: pageCreate_formLayout_Create
    anchors.fill: parent

    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent
        text: "Create — coming soon"
        icon.name: "journal-new"
    }

    /*
    Kirigami.FormLayout {

        //Section1: File name criteria
        Kirigami.FormLayout {
            id: pageCreate_formLayout_Source
            anchors.left: parent.left
            anchors.leftMargin: 25

            Controls.Label{
                id: pageCreate_Label_Section1Title
                text:"1- Select the source path"
                font.bold: true
                anchors.left: parent.left
            }
            RowLayout {
                id: pageCreate_fileNameTextRow
                //anchors.top: pageCreate_Label_Section1Title.bottom
                //anchors.topMargin: 25
                Controls.TextField {
                    id: pageCreate_pageCreate_TextField_SourcePath
                    Kirigami.FormData.label: "File name"
                    text: "/"
                }
                Controls.Button {
                    id: pageCreate_pasteClipboard
                    icon.name: "edit-select"
                    //onClicked: fileNameText.text = pageSearch1.returnClipboard()
                }
            }
            /*
            Controls.TextField {
                id: fileNameExclude
                text: "DEV: replace by file system tree"
            }
            *\/
            Text {
                id: pageCreate_separate0
            }
        }

        //Section2: File attributes criteria
        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            //Kirigami.FormData.label: "New Section!"
            anchors.left: parent.left
        }
        Kirigami.FormLayout {
            id: pageCreate_fileAtrributeCriteria
            anchors.left: parent.left
            anchors.leftMargin: 25

            Controls.Label{
                id: pageCreate_Label_Section2Title
                text:"2- Select content options"
                font.bold: true
                anchors.left: parent.left
            }
            Text {
                id: pageCreate_separate1
            }
            Controls.Label{
                id: pageCreate_Label_FileType
                text:"File Type"
            }
            ColumnLayout {
                Controls.RadioButton {
                    checked: true
                    text: qsTr("All")
                }
                Controls.RadioButton {
                    text: qsTr("Image")
                    //icon: "image-jpeg"
                }
                Controls.RadioButton {
                    text: qsTr("Audio")
                    //icon: "audio-x-mpeg"
                }
                Controls.RadioButton {
                    text: qsTr("Text")
                    //icon: "folder-text"
                }
                Controls.RadioButton {
                    text: qsTr("Video")
                    //icon: "video-mp4"
                }

            }
            Text {
                id: pageCreate_separate2
            }
            Controls.Label{
                id: pageCreate_Label_OtherOptions
                text:"Other Options"
            }
            Controls.CheckBox{
                id: pageCreate_CheckBox_IncludeHiddenFile
                text:"IncludeHiddenFile"
            }
            /*
            Controls.CheckBox{
                id: pageCreate_CheckBox_IncludeMediaMetaData
                text:"IncludeMediaMetaData"
            }
            Controls.CheckBox{
                id: pageCreate_CheckBox_FollowSymbolicLinks
                text:"Follow Symbolic Links"
            }
            *\/
            Text {
                id: pageCreate_separate3
            }
        }

        //Section3: Folder criteria
        Kirigami.Separator {
            //Kirigami.FormData.isSection: true
            //anchors.left: parent.left
            //Kirigami.FormData.label: "New Section!"
        }
        Kirigami.FormLayout {
            //anchors.fill: parent
            id: pageCreate_folderCriteria
            anchors.left: parent.left
            anchors.leftMargin: 25

            Controls.Label{
                id: pageCreate_Label_Section3Title
                text:"3- Define and Create catalog"
                font.bold: true
                anchors.left: parent.left
            }
            Text {
                id: pageCreate_separate4
            }
            Controls.Label{
                id: pageCreate_Label_SelectStorage
                text:"Select storage"
            }
            RowLayout {
                id:pageCreate_RowLayout_StorageSelection
                //Kirigami.FormData.label: "Search text"
                Controls.ComboBox {
                    id: pageCreate_ComboBox_DeviceList
                    model: ["Device1","Device2","Device3"]
                }
                Controls.Button {
                    id: pageCreate_Button_AddStorage
                    icon.name: "drive-harddisk"
                    text: "Add Storage"
                    //onClicked: fileNameText.text = pageSearch1.returnClipboard()
                }
            }

            Controls.Label{
                id: pageCreate_Label_EnterName
                text:"Enter the name of the new catalog"
                //anchors.top: pageCreate_RowLayout_StorageSelection.anchors.bottom
                //anchors.topMargin: 50
            }
            RowLayout {
                id:pageCreate_RowLayout_CatalogName
                //Kirigami.FormData.label: "Search text"
                Controls.TextField {
                    id: pageCreate_TextField_CatalogName
                    text: "NewCatalog"
                }
                Controls.Button {
                    id: pageCreate_Button_Generate
                    icon.name: "tools-wizard"
                    text: "Generate"
                    //onClicked: fileNameText.text = pageSearch1.returnClipboard()
                }
            }
            Text {
                id: pageCreate_separate5
            }
            Controls.Button {
                id: pageCreate_Button_CreateCatalog
                icon.name: "document-save"
                text: "Create Catalog"
                //onClicked: fileNameText.text = pageSearch1.returnClipboard()
            }
        }
    }
    */
}
