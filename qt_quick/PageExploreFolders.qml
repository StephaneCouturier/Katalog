import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

// Left panel of the Explore page: catalog selector and folder tree.
// Signals folderClicked(folderPath) when the user selects a folder.
Item {
    id: root

    signal folderClicked(string folderPath)
    signal catalogOpenRequested(var catalogInfo)

    property string catalogName: ""
    property string catalogPath: ""
    property int    folderCount: 0
    property string selectedFolderPath: ""

    // Refresh the catalog combo with all Catalog-type devices
    function refreshCatalogList() {
        var cats = appManager1.getDeviceList("Catalogs")
        catalogComboModel.clear()
        for (var i = 0; i < cats.length; i++) {
            catalogComboModel.append({
                name:      cats[i].name,
                deviceId:  cats[i].deviceId,
                path:      cats[i].path,
                externalId: cats[i].externalId ?? 0
            })
        }
    }

    // Open the selected catalog and populate the folder tree
    function openSelectedCatalog() {
        if (catalogCombo.currentIndex < 0) return
        var devId = catalogComboModel.get(catalogCombo.currentIndex).deviceId
        var info = appManager1.exploreOpenCatalog(devId)
        if (!info.success) {
            root.catalogName  = ""
            root.catalogPath  = ""
            root.folderCount  = 0
            return
        }
        root.catalogName  = info.name
        root.catalogPath  = info.path
        root.folderCount  = info.folderCount
        root.selectedFolderPath = info.path

        folderListModel.clear()
        // Add the catalog root as the first entry
        folderListModel.append({
            name:     info.name,
            fullPath: info.path,
            level:    0
        })

        var folders = appManager1.getExploreFolders()
        for (var i = 0; i < folders.length; i++) {
            folderListModel.append({
                name:     folders[i].name,
                fullPath: folders[i].fullPath,
                level:    folders[i].level + 1
            })
        }

        // Select the root and notify
        folderListView.currentIndex = 0
        root.folderClicked(info.path)
        root.catalogOpenRequested(info)
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Catalog selector row
        RowLayout {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            spacing: Kirigami.Units.smallSpacing

            Controls.ComboBox {
                id: catalogCombo
                Layout.fillWidth: true
                model: ListModel { id: catalogComboModel }
                textRole: "name"
                displayText: currentIndex >= 0 ? catalogComboModel.get(currentIndex).name : qsTr("Select a catalog…")
            }

            Controls.Button {
                icon.name: "document-open"
                text: qsTr("Open")
                enabled: catalogCombo.currentIndex >= 0
                onClicked: root.openSelectedCatalog()
            }
        }

        // Catalog info bar
        Controls.Label {
            visible: root.catalogName !== ""
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.smallSpacing
            Layout.rightMargin: Kirigami.Units.smallSpacing
            text: qsTr("%1 folder(s)").arg(root.folderCount)
            font.italic: true
            color: Kirigami.Theme.disabledTextColor
            elide: Text.ElideRight
        }

        Kirigami.Separator { Layout.fillWidth: true }

        // Folder tree list
        Controls.ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                id: folderListView
                model: ListModel { id: folderListModel }

                delegate: Controls.ItemDelegate {
                    id: folderDelegate
                    width: folderListView.width
                    height: Kirigami.Units.gridUnit * 1.6
                    highlighted: model.fullPath === root.selectedFolderPath

                    contentItem: RowLayout {
                        spacing: Kirigami.Units.smallSpacing
                        // Indentation
                        Item { width: model.level * Kirigami.Units.gridUnit; height: 1 }
                        Kirigami.Icon {
                            source: "folder"
                            implicitWidth:  Kirigami.Units.iconSizes.small
                            implicitHeight: Kirigami.Units.iconSizes.small
                        }
                        Controls.Label {
                            text: model.name
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                            color: highlighted ? Kirigami.Theme.highlightedTextColor
                                               : Kirigami.Theme.textColor
                        }
                    }

                    onClicked: {
                        root.selectedFolderPath = model.fullPath
                        folderListView.currentIndex = index
                        root.folderClicked(model.fullPath)
                    }
                }
            }
        }
    }

    Component.onCompleted: refreshCatalogList()

    Connections {
        target: appManager1
        function onDeviceListChanged()  { root.refreshCatalogList() }
        function onUiRefreshCompleted() { root.refreshCatalogList() }
    }
}
