import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import QtQuick.Dialogs as Dialogs
import Qt.labs.platform
import Katalog 3.0

// Provides basic features needed for all kirigami applications
Kirigami.ApplicationWindow {
    // Unique identifier to reference this object
    id: root

    width: 1080
    height: 720

    signal searchTriggered()
    property real cardScale: 1.0
    // Window title
    // i18nc() makes a string translatable
    // and provides additional context for the translators
    //title: "Katalog"

    // Global Drawer
    globalDrawer: Kirigami.GlobalDrawer {
/*
        header: Kirigami.SearchField {
            id: searchField
        }
*/
        // Global Drawer

                    // Add the slider at the top
                header: Column {
                    spacing: Kirigami.Units.smallSpacing
                    padding: Kirigami.Units.largeSpacing

                    Controls.Label {
                        text: "Card text size"
                        font.bold: true
                        width: parent.width
                    }

                    Row {
                        width: parent.width
                        spacing: Kirigami.Units.smallSpacing

                        Kirigami.Icon {
                            source: "zoom-out"
                            width: Kirigami.Units.iconSizes.small
                            height: Kirigami.Units.iconSizes.small
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        Controls.Slider {
                            id: cardSizeSlider
                            from: 0.7
                            to: 1.5
                            value: 1.0
                            stepSize: 0.1
                            Layout.fillWidth: true
                            width: parent.width - 60

                            // Store the scale factor globally
                            onValueChanged: {
                                root.cardScale = value
                            }
                        }

                        Kirigami.Icon {
                            source: "zoom-in"
                            width: Kirigami.Units.iconSizes.small
                            height: Kirigami.Units.iconSizes.small
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Kirigami.Separator {
                        width: parent.width
                    }
                }

        width: 200
        actions: [
            Kirigami.Action {
                text: "Open Collection..."
                icon.name: "document-open-data"
                Kirigami.Action {
                    icon.name: "document-open-data"
                    text: "Local Files"
                    onTriggered: {
                        var p = appManager1.getCollectionFolder()
                        if (p.length > 0)
                            memoryFolderDialog.currentFolder = "file://" + p
                        memoryFolderDialog.open()
                    }
                }
                Kirigami.Action {
                    icon.name: "document-open-data"
                    text: "SQLite Db"
                    onTriggered: {
                        var p = appManager1.getDatabaseFilePath()
                        if (p.length > 0) {
                            var slash = p.lastIndexOf("/")
                            databaseFileDialog.currentFolder = "file://" + (slash >= 0 ? p.substring(0, slash) : p)
                        }
                        databaseFileDialog.open()
                    }
                }
                Kirigami.Action {
                    icon.name: "document-open-data"
                    text: "Hosted Db"
                    onTriggered: {
                        pageSettings.showHostedForm = true
                        pageStack.push(pageSettings)
                    }
                }
            },
            Kirigami.Action {
                separator: true
            },
            Kirigami.Action {
                icon.name: "select"
                text: "Selection"
                onTriggered: pageStack.insertPage(0, pageSelection)
            },
            Kirigami.Action {
                icon.name: "search"
                text: "Search"
                onTriggered: pageStack.push(pageSearch)//{pageSearchForm.executeSearch();}
            },
            // Kirigami.Action {
            //     icon.name: "drive-multidisk"
            //     text: "Devices"
            //     onTriggered: pageStack.push(pageDevices)
            // },
            // Kirigami.Action {
            //     icon.name: "view-list-tree"
            //     text: "Explore"
            //     onTriggered: pageStack.insertPage(4, pageExplore)
            // },
            // Kirigami.Action {
            //     icon.name: "journal-new"
            //     text: "Create"
            //     onTriggered: pageStack.insertPage(5, pageCreate)
            // },
            // Kirigami.Action {
            //     icon.name: "view-statistics"
            //     text: "Statistics"
            //     onTriggered: pageStack.insertPage(6, pageStatistics)
            // },
            // Kirigami.Action {
            //     icon.name: "lastfm-tag"
            //     text: "Tags"
            //     onTriggered: pageStack.insertPage(7, pageTags)
            // },
            Kirigami.Action {
                icon.name: "settings-configure"
                text: "Settings"
                //onTriggered: pageStack.insertPage(8, pageSettings)
                onTriggered: pageStack.push(pageSettings)
            },
            Kirigami.Action {
                separator: true
            },
            Kirigami.Action {
                icon.name: "help-about"
                text: "Documentation"
                onTriggered: Qt.openUrlExternally("https://stephanecouturier.github.io/Katalog/");
            },
            Kirigami.Action {
                text: "About" //i18n("About")
                icon.name: "help-about"
                onTriggered: pageStack.layers.push(aboutPage)
            },
            Kirigami.Action {
                text: "Quit"
                icon.name: "application-exit-symbolic"
                shortcut: StandardKey.Quit
                onTriggered: Qt.quit()
            }
        ]
    }

        // Database status notification
        Connections {
            target: appManager1
            function onDatabaseConnectionChanged(success, message) {
                if (success) {
                    showPassiveNotification("✓ " + message, "positive")
                    pageStack.currentIndex = 0
                } else {
                    showPassiveNotification("✗ " + message, "warning")
                }
            }
        }

    //Pages ---------------------------------------------------------------------
    pageStack.initialPage: [ pageSelection, pageSearch ]

    Component.onCompleted: {
        if (appManager1.shouldShowAlphaWarning())
            alphaWarningDialog.open()
    }

    Controls.Dialog {
        id: alphaWarningDialog
        title: "Katalog 3 - Alpha1 Version"
        modal: true
        anchors.centerIn: parent
        width: Math.min(520, parent.width - Kirigami.Units.largeSpacing * 4)

        Controls.Label {
            width: parent.width
            wrapMode: Text.WordWrap
            text: "This is an early alpha version of Katalog intended to support development and gather feedback.\n\n" +
                  "Features available (read-only):\n" +
                  "  • Open a collection\n" +
                  "  • Device selection\n" +
                  "  • Search\n\n" +
                  "All other features are not yet available in this version."
        }

        footer: Controls.DialogButtonBox {
            Controls.CheckBox {
                id: doNotShowAgain
                text: "Do not show again"
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.ResetRole
            }
            Controls.Button {
                text: "OK"
                Controls.DialogButtonBox.buttonRole: Controls.DialogButtonBox.AcceptRole
            }
            onAccepted: {
                if (doNotShowAgain.checked)
                    appManager1.setAlphaWarningShown()
                alphaWarningDialog.close()
            }
        }
    }

    Component {
        id: aboutPage
        Kirigami.AboutPage {
            aboutData: About
        }
    }

    //Pages - Selection
    Kirigami.ScrollablePage {
        id: pageSelection
        title: "Selection"

        property string deviceType: "Storage"

        Connections {
            target: appManager1
            function onSelectedDeviceChanged(deviceId) {
                console.log("QML received selectedDeviceChanged signal for device ID:", deviceId)

                // Force refresh of the selection list view to update visual states
                if (selectionListView1 && selectionListView1.model) {
                    // Trigger a visual update of all delegates
                    selectionListView1.model.dataChanged(
                        selectionListView1.model.index(0, 0),
                        selectionListView1.model.index(selectionListView1.model.rowCount() - 1, 0)
                    )
                }
            }
        }

        actions: [
            Kirigami.Action {
                //text: "Reset"
                icon.name: "edit-clear-all"
                onTriggered: showPassiveNotification("Reset clicked, no action")
            },
            Kirigami.Action {
                //text: "Refresh"
                icon.name: "view-refresh"
                onTriggered: {
                    appManager1.refreshDeviceList();
                    showPassiveNotification("Device list refreshed");
                }
            },
            Kirigami.Action {
                //text: "Close"
                icon.name: "view-close"
                onTriggered: pageStack.removePage(pageSelection)
            }
        ]

        header: Item {
            width: parent.width
            height: deviceSearchField.implicitHeight + Kirigami.Units.smallSpacing * 2
            Kirigami.SearchField {
                id: deviceSearchField
                anchors {
                    left: parent.left
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                    leftMargin: Kirigami.Units.gridUnit
                    rightMargin: Kirigami.Units.gridUnit
                }
                onTextChanged: appManager1.setDeviceFilter(text)
            }
        }

        Kirigami.CardsListView {
            id: selectionListView1
            model: appManager1.deviceFilterModel
            delegate: PageSelectionDelegate {}
        }
    }

    //Pages - Search
    Kirigami.ScrollablePage {
        id: pageSearch
        title: "Search"
        property int testValue: 333

        Kirigami.Dialog {
                id: search_Dialog_Submit
                title: "Search criteria"
                standardButtons: Kirigami.Dialog.Ok
                padding: Kirigami.Units.largeSpacing
                preferredWidth: Kirigami.Units.gridUnit * 20
                Controls.Label {
                    id: nameField
                    //Kirigami.FormData.label: i18nc("@label:textbox", "Name*:")
                    Kirigami.FormData.label: "Search criteria"
                    text: "<br/>searchOnFileName:  " + newSearch1.properties.searchOnFileName
                          + "<br/>searchText:  " + newSearch1.properties.searchText
                          + "<br/>selectedSearchWith:  " + newSearch1.properties.selectedSearchWith
                          + "<br/>selectedSearchIn:  " + newSearch1.properties.selectedSearchIn
                          + "<br/>caseSensitive:  " + newSearch1.properties.caseSensitive
                          + "<br/>selectedFileType:  " + newSearch1.properties.selectedFileType
                          + "<br/>searchInCatalogsChecked:  " + newSearch1.properties.searchInCatalogsChecked
                          + "<br/>searchOnDuplicates:  " + newSearch1.properties.searchOnDuplicates
                          + "<br/>searchOnDifferences:  " + newSearch1.properties.searchOnDifferences
                }
        }

        Kirigami.Dialog {
            id: search_Dialog_Results
            title: "Search criteria"
            standardButtons: Kirigami.Dialog.Ok
            padding: Kirigami.Units.largeSpacing
            preferredWidth: Kirigami.Units.gridUnit * 20

            Controls.Label {
                id: nameField1
                Kirigami.FormData.label: "Search criteria"
                text: "<br/>filesFoundNumber:  " + newSearch1.properties.filesFoundNumber
            }
        }
        ListModel {
            id: emptyModel
        }

        actions: [
            Kirigami.Action {
                text: "Search"
                icon.name: "search"
                onTriggered: {
                    root.searchTriggered();
                    pageSearchForm.getCriteria();
                    //search_Dialog_Submit.open()
                    pageSearchForm.executeSearch();

                    // Insert the new page into the PageStack
                    pageStack.removePage(pageSearchResults);
                    pageStack.insertPage(3, pageSearchResults);
                    pageSearchForm.pageSearchResultsForm.pageSearchResults_tableView_results.forceLayout();

                    //pageSearchForm.pageSearchResults_tableView_results.model = emptyModel;
                    //pageSearchForm.pageSearchResults_column.pageSearchResults_tableView_results.model = emptyModel;
                    //pageSearchForm.pageSearchResults_tableView_results.model = newSearch1;
                    //pageSearchResults_tableView_results.model = newSearch1;
                    //pageSearchForm.pageSearchResults_tableView_results.adjustColumnWidths();
                    //search_Dialog_Results.open();

                    showPassiveNotification( "newSearch1.properties.filesFoundNumber:   " + newSearch1.properties.filesFoundNumber );
                }
            },
            Kirigami.Action {
                text: "Reset"
                icon.name: "edit-clear-all"
                onTriggered: pageSearchForm.resetSearch()
            },
            Kirigami.Action {
                text: "Close"
                icon.name: "view-close"
                onTriggered: pageStack.removePage(pageSearch)
            }
        ]

        PageSearchForm {
            id: pageSearchForm
        }
    }

    //Pages - SearchResults
    Kirigami.ScrollablePage {
        title: "Search Results"
        id: pageSearchResults
        visible: false

        actions: [
            /*Kirigami.Action {
                text: "Batch process"
                icon.name: "document-export"
                onTriggered: showPassiveNotification("Batch process clicked, no action")
            },*/
            Kirigami.Action {
                text: "Close"
                icon.name: "view-close"
                onTriggered: pageStack.removePage(pageSearchResults)
            }
        ]

        PageSearchResultsForm {
            id: pageSearchResultsForm
        }
    }

    //Pages - Devices
    Kirigami.ScrollablePage {
        id: pageDevices
        title: "Devices"
        visible: false

        actions: [
            /*Kirigami.Action {
                text: "Batch process"
                icon.name: "document-export"
                onTriggered: showPassiveNotification("Batch process clicked, no action")
            },*/
            Kirigami.Action {
                text: "Close"
                icon.name: "view-close"
                onTriggered: pageStack.pop()
            }
        ]

        PageDevicesView {
            id: pageDevicesView
        }
    }

    //Pages - Explore
    Kirigami.ScrollablePage {
        id: pageExplore
        title: "Explore"
        visible: false

        actions: [
            /*Kirigami.Action {
                text: "Batch process"
                icon.name: "document-export"
                onTriggered: showPassiveNotification("Batch process clicked, no action")
            },*/
            Kirigami.Action {
                text: "Close"
                icon.name: "view-close"
                onTriggered: pageStack.removePage(pageExplore)
            }
        ]

        ListModel {
        id: exploreList
        // Each ListElement is an element on the list, containing information
            ListElement {
                name: "Local Drive"
                description: "1Tb"
            }
            ListElement {
                name: "External Drive 1"
                description: "500Gb"
            }
            ListElement {
                name: "External Drive 2"
                description: ""
            }
        }

        Kirigami.CardsListView {
            id: exploreListView
            model: exploreList
            delegate: PageSelectionDelegate {}
        }
    }

    //Pages - Create
    Kirigami.ScrollablePage {
        id: pageCreate
        title: "Create"
        visible: false

        actions: [
            Kirigami.Action {
                text: "Close"
                icon.name: "view-close"
                onTriggered: pageStack.removePage(pageCreate)
            }
        ]

        PageCreateForm {
            id: pageCreate_formLayout_Create
        }
    }


    //Pages - Statistics
    Kirigami.ScrollablePage {
        id: pageStatistics
        title: "Statistics"
        visible: false

        actions: [
            /*Kirigami.Action {
                text: "Batch process"
                icon.name: "document-export"
                onTriggered: showPassiveNotification("Batch process clicked, no action")
            },*/
            Kirigami.Action {
                text: "Close"
                icon.name: "view-close"
                onTriggered: pageStack.removePage(pageStatistics)
            }
        ]

        ListModel {
        id: statisticsList
        // Each ListElement is an element on the list, containing information
            ListElement {
                name: "Stat1"
                description: "1Tb"
            }
            ListElement {
                name: "Stat2"
                description: "500Gb"
            }
            ListElement {
                name: "Stat3"
                description: ""
            }
        }

        Kirigami.CardsListView {
            id: statisticsListView
            model: statisticsList
            delegate: PageSelectionDelegate {}
        }
    }

    //Pages - Tags
    Kirigami.ScrollablePage {
        id: pageTags
        title: "Tags"
        visible: false

        actions: [
            /*Kirigami.Action {
                text: "Batch process"
                icon.name: "document-export"
                onTriggered: showPassiveNotification("Batch process clicked, no action")
            },*/
            Kirigami.Action {
                text: "Close"
                icon.name: "view-close"
                onTriggered: pageStack.removePage(pageTags)
            }
        ]

        ListModel {
        id: tagsList
        // Each ListElement is an element on the list, containing information
            ListElement {
                name: "Local Drive"
                description: "1Tb"
            }
            ListElement {
                name: "External Drive 1"
                description: "500Gb"
            }
            ListElement {
                name: "External Drive 2"
                description: ""
            }
        }

        Kirigami.CardsListView {
            id: tagsListView
            model: tagsList
            delegate: PageSelectionDelegate {}
        }
    }

    //Pages - Settings
    PageSettings {
        id: pageSettings
        visible: false
    }

    // Dialogs - triggered from Open Collection menu
    Dialogs.FolderDialog {
        id: memoryFolderDialog
        title: "Select Collection Folder"
        onAccepted: {
            var path = selectedFolder.toString().replace("file://", "")
            appManager1.openCollectionMemory(path)
        }
    }

    Dialogs.FileDialog {
        id: databaseFileDialog
        title: "Open SQLite Database"
        nameFilters: ["SQLite databases (*.db *.sqlite *.sqlite3)", "All files (*)"]
        onAccepted: {
            var path = selectedFile.toString().replace("file://", "")
            appManager1.setDatabaseFilePath(path)
        }
    }
}
