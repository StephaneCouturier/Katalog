import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

// Left panel of the Explore page: folder tree for the active explore catalog.
// Entry point: openByDeviceId(deviceId) called from context menus.
Item {
    id: root

    signal folderClicked(string folderPath)
    signal catalogOpenRequested(var catalogInfo)

    property string catalogName: ""
    property string catalogPath: ""
    property int    folderCount: 0
    property string selectedFolderPath: ""

    // Open a catalog by device ID (called from Devices/Selection context menus)
    function openByDeviceId(deviceId) {
        _applyOpenResult(appManager1.exploreOpenCatalog(deviceId))
    }

    function _applyOpenResult(info) {
        if (!info.success) {
            root.catalogName = ""
            root.catalogPath = ""
            root.folderCount = 0
            folderListModel.clear()
            return
        }
        root.catalogName        = info.name
        root.catalogPath        = info.path
        root.folderCount        = info.folderCount
        root.selectedFolderPath = info.path

        folderListModel.clear()
        folderListModel.append({ name: info.name, fullPath: info.path, level: 0 })

        var folders = appManager1.getExploreFolders()
        for (var i = 0; i < folders.length; i++) {
            folderListModel.append({
                name:     folders[i].name,
                fullPath: folders[i].fullPath,
                level:    folders[i].level + 1
            })
        }

        folderListView.currentIndex = 0
        root.folderClicked(info.path)
        root.catalogOpenRequested(info)
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Directories count header
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin:  Kirigami.Units.smallSpacing
            Layout.rightMargin: Kirigami.Units.smallSpacing
            Layout.topMargin:   Kirigami.Units.smallSpacing
            Layout.bottomMargin: Kirigami.Units.smallSpacing
            spacing: Kirigami.Units.smallSpacing

            Controls.Label {
                text: qsTr("Directories")
            }
            Controls.Label {
                text: root.folderCount.toLocaleString(Qt.locale(), "f", 0)
                font.bold: true
            }
            Item { Layout.fillWidth: true }
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
}
