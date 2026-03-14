import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import Qt.labs.platform
import Qt.labs.qmlmodels

Column {
    id: pageSearchResults_column
    anchors.fill: parent

    function formatFileSize(size) {
        if (size < 1024) {
            return size + " B";
        } else if (size < 1024 * 1024) {
            return (size / 1024).toFixed(2) + " KB";
        } else if (size < 1024 * 1024 * 1024) {
            return (size / (1024 * 1024)).toFixed(2) + " MB";
        } else if (size < 1024 * 1024 * 1024 * 1024) {
            return (size / (1024 * 1024 * 1024)).toFixed(2) + " GB";
        } else {
            return (size / (1024 * 1024 * 1024 * 1024)).toFixed(2) + " TB";
        }
    }

    //Results statistics
    RowLayout{
        id: pageSearchResults_rowlayout_ResultStatistics
        Text{
            text: "Results statistics: "
        }
        Text{
            text: "Files found: "
        }
        Text{
            text: newSearch1.properties.filesFoundNumber
            font.bold: true
        }
        Text{
            text: "Total size: "
        }
        Text{
            text: formatFileSize(newSearch1.properties.filesFoundTotalSize)
            font.bold: true
            //format value to be more readable as a file size (KB, MB, GB, etc)

        }
        Text{
            text: "Min size: "
        }
        Text{
            text: formatFileSize(newSearch1.properties.filesFoundMinSize)
            font.bold: true
        }
        Text{
            text: "Max size: "
        }
        Text{
            text: formatFileSize(newSearch1.properties.filesFoundMaxSize)
            font.bold: true
        }
        Text{
            text: "Min Date: "
        }
        Text{
            text: newSearch1.properties.filesFoundMinDate.toString()
            font.bold: true
        }
        Text{
            text: "Max Date: "
        }
        Text{
            text: newSearch1.properties.filesFoundMaxDate.toString()
            font.bold: true
        }
    }

    //Results table
    TableView {
        id: pageSearchResults_tableView_results
        anchors.fill: parent
        anchors.top: pageSearchResults_rowlayout_ResultStatistics.bottom
        anchors.topMargin: 40
        columnSpacing: 1
        rowSpacing: 2
        resizableColumns: true

        model: newSearch1

        delegate: Item {
            width: textElement.implicitWidth
            implicitHeight: 30

            Rectangle {
                anchors.fill: parent
                //border.color: "lightgray"
                //color: column === 0 ? "lightgrey" : "white"

                Text {
                    id: textElement
                    clip: true
                    anchors {
                        verticalCenter: parent.verticalCenter
                        //horizontalCenter: column === 1 || column === 2 ? parent.horizontalCenter : undefined
                        //left: column !== 1 && column !== 2 ? parent.left : undefined
                        //leftMargin: column !== 1 && column !== 2 ? 20 : 0
                    }
                    //color: column === 0 ? "black" : "grey"
                    //text: column === 1 ? formatFileSize(model.display) : model.display
                    text: model.display
                }
            }
        }

        Component.onCompleted: {
            adjustColumnWidths();
        }

        onModelChanged: {
            adjustColumnWidths();
        }

        function adjustColumnWidths() {
            pageSearchResults_tableView_results.setColumnWidth(0, 350); //Name
            pageSearchResults_tableView_results.setColumnWidth(1, 100); //Size
            pageSearchResults_tableView_results.setColumnWidth(2, 150); //Date
            pageSearchResults_tableView_results.setColumnWidth(3, 600); //Path
            pageSearchResults_tableView_results.setColumnWidth(4, 500); //Type
        }

        function calculateColumnWidth(column) {
            var maxWidth = 300;
            for (var i = 0; i < newSearch1.rowCount(); i++) {
                var textWidth = textElementWidth(newSearch1.data(newSearch1.index(i, column), Qt.DisplayRole));
                if (textWidth > maxWidth) {
                    maxWidth = textWidth;
                }
            }
            return maxWidth + 20; // Add some padding
        }

        function textElementWidth(text) {
            var textElement = Qt.createQmlObject('import QtQuick 2.15; Text { text: "' + text + '" }', pageSearchResults_tableView_results);
            return textElement.implicitWidth;
        }
    }

    ListModel {
        id: emptyModel
    }

    Connections {
        target: root
        onSearchTriggered: {
            // Set the TableView to an empty model
            pageSearchResults_tableView_results.model = emptyModel;

            // Set the TableView to the actual model with updated data
            pageSearchResults_tableView_results.model = newSearch1;

            // Force the TableView to refresh
            pageSearchResults_tableView_results.forceLayout();
        }
    }

}
