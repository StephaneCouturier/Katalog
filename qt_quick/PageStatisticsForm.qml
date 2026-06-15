import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import QtCharts

ColumnLayout {
    id: root
    spacing: 0

    // Internal keys: source = "all"|"updates"|"snapshots", dataType = "size"|"count"
    property string selectedSource:   "all"
    property string selectedDataType: "size"
    property bool   displayEachValue: false
    property string startDateText:    ""

    // Unit label map used in axis title and chart title
    function unitLabel(key) {
        if (key === "TiB") return qsTr("TiB")
        if (key === "GiB") return qsTr("GiB")
        if (key === "MiB") return qsTr("MiB")
        if (key === "KiB") return qsTr("KiB")
        return ""
    }

    function loadSettings() {
        var src = appManager1.getStatisticsSetting("SelectedSource")
        if (src !== "") root.selectedSource = src
        var dt  = appManager1.getStatisticsSetting("TypeOfData")
        if (dt  !== "") root.selectedDataType = dt
        root.displayEachValue = appManager1.getStatisticsSetting("DisplayEachValue") === "true"
        root.startDateText    = appManager1.getStatisticsSetting("graphStartDate")
    }

    function refresh() {
        var data = appManager1.getStatisticsData(root.selectedSource, root.selectedDataType, root.startDateText)
        updateChart(data)
    }

    function updateChart(data) {
        series1Line.clear()
        series2Line.clear()
        series3Line.clear()

        if (!data.hasData) {
            chart.title = qsTr("No data")
            return
        }

        var i
        if (data.loadSeries1)
            for (i = 0; i < data.series1.length; i++)
                series1Line.append(data.series1[i].x, data.series1[i].y)
        if (data.loadSeries2)
            for (i = 0; i < data.series2.length; i++)
                series2Line.append(data.series2[i].x, data.series2[i].y)
        if (data.loadSeries3)
            for (i = 0; i < data.series3.length; i++)
                series3Line.append(data.series3[i].x, data.series3[i].y)

        // Series visibility and point markers
        series1Line.visible       = data.loadSeries1
        series2Line.visible       = data.loadSeries2
        series3Line.visible       = data.loadSeries3
        series1Line.pointsVisible = data.loadSeries1 && root.displayEachValue
        series2Line.pointsVisible = data.loadSeries2 && root.displayEachValue
        series3Line.pointsVisible = data.loadSeries3 && root.displayEachValue

        // Y axis: round up to next leading digit (e.g. 848 365 → 900 000)
        var maxVal = data.maxValue
        var yMax = 10
        if (maxVal > 0) {
            var digits     = Math.floor(Math.log10(maxVal)) + 1
            var firstDigit = Math.floor(maxVal / Math.pow(10, digits - 1))
            yMax = (firstDigit + 1) * Math.pow(10, digits - 1)
        }
        var stepSize    = yMax / 10
        axisY.max       = yMax + stepSize
        axisY.min       = 0
        axisY.tickCount = 10
        var ul = data.unitKey !== "" ? " (" + unitLabel(data.unitKey) + ")" : ""
        axisY.titleText = qsTr("Total") + ul

        // X axis: always set range explicitly — DateTimeAxis has no auto-range
        var pts = data.series1.length > 0 ? data.series1 : data.series2.length > 0 ? data.series2 : data.series3
        if (pts.length === 1) {
            axisX.min = new Date(pts[0].x - 86400000)   // ±1 day padding
            axisX.max = new Date(pts[0].x + 86400000)
        } else if (pts.length > 1) {
            var firstX   = pts[0].x
            var lastX    = pts[pts.length - 1].x
            var padding  = Math.max((lastX - firstX) * 0.05, 86400000)
            axisX.min    = new Date(firstX - padding)
            axisX.max    = new Date(lastX  + padding)
        }

        // Y axis label format
        axisY.labelFormat = maxVal < 10.0 ? "%.1f" : "%.0f"

        // Series names (reflect current data type)
        var dtLabel = root.selectedDataType === "size" ? qsTr("Total File Size") : qsTr("Number of Files")
        series1Line.name = qsTr("Catalogs") + " / " + dtLabel

        // Chart title
        chart.title = data.deviceName + " — " + dtLabel + ul
    }

    Component.onCompleted: {
        loadSettings()
        // Sync comboboxes to restored settings
        var si = sourceCombo.indexOfValue(root.selectedSource)
        if (si >= 0) sourceCombo.currentIndex = si
        var di = dataTypeCombo.indexOfValue(root.selectedDataType)
        if (di >= 0) dataTypeCombo.currentIndex = di
        displayEachValueCheck.checked = root.displayEachValue
        startDateField.text = root.startDateText
        refresh()
    }

    Connections {
        target: appManager1
        function onSelectedDeviceChanged() { root.refresh() }
        function onStatisticsRefreshed()   { root.refresh() }
    }

    // ─── Controls bar ─────────────────────────────────────────────────────────
    GridLayout {
        Layout.fillWidth: true
        Layout.topMargin:   Kirigami.Units.largeSpacing
        Layout.leftMargin:  Kirigami.Units.largeSpacing
        Layout.rightMargin: Kirigami.Units.largeSpacing
        columns: 4
        columnSpacing: Kirigami.Units.largeSpacing
        rowSpacing: Kirigami.Units.smallSpacing

        Controls.Label { text: qsTr("Device"); opacity: 0.7 }
        Controls.Label {
            // Guard the selectedDeviceId so the untranslated "No device selected"
            // fallback in getSelectedDeviceName() never reaches the UI.
            text: appManager1.selectedDeviceId > 0 ? appManager1.selectedDeviceName : ""
            font.bold: true
            elide: Text.ElideRight
            Layout.columnSpan: 3
            Layout.fillWidth: true
        }

        Controls.Label { text: qsTr("Source"); opacity: 0.7 }
        Controls.ComboBox {
            id: sourceCombo
            textRole: "text"
            valueRole: "value"
            model: [
                { value: "all",       text: qsTr("all records")    },
                { value: "updates",   text: qsTr("updates only")   },
                { value: "snapshots", text: qsTr("snapshots only") }
            ]
            onActivated: {
                root.selectedSource = currentValue
                appManager1.setStatisticsSetting("SelectedSource", currentValue)
                root.refresh()
            }
        }

        Controls.Label { text: qsTr("Data"); opacity: 0.7 }
        Controls.ComboBox {
            id: dataTypeCombo
            textRole: "text"
            valueRole: "value"
            model: [
                { value: "size",  text: qsTr("Total File Size")   },
                { value: "count", text: qsTr("Number of Files") }
            ]
            onActivated: {
                root.selectedDataType = currentValue
                appManager1.setStatisticsSetting("TypeOfData", currentValue)
                root.refresh()
            }
        }

        Controls.Label { text: qsTr("From date"); opacity: 0.7 }
        RowLayout {
            Layout.columnSpan: 3
            Controls.TextField {
                id: startDateField
                placeholderText: "yyyy-MM-dd"
                readOnly: true
                implicitWidth: 100
            }
            Controls.Button {
                icon.name: "view-calendar"
                onClicked: statsDateDialog.open()
            }
            Controls.Button {
                icon.name: "edit-clear-history"
                onClicked: {
                    startDateField.text = ""
                    root.startDateText  = ""
                    appManager1.setStatisticsSetting("graphStartDate", "")
                    root.refresh()
                }
            }
        }

        Controls.Label { text: qsTr("Display each value"); opacity: 0.7 }
        Controls.CheckBox {
            id: displayEachValueCheck
            Layout.columnSpan: 3
            onCheckedChanged: {
                root.displayEachValue = checked
                appManager1.setStatisticsSetting("DisplayEachValue", checked ? "true" : "false")
                series1Line.pointsVisible = checked && series1Line.visible
                series2Line.pointsVisible = checked && series2Line.visible
                series3Line.pointsVisible = checked && series3Line.visible
            }
        }
    }

    Kirigami.Dialog {
        id: statsDateDialog
        title: qsTr("Select start date")

        function applyDate(d) {
            var mm = d.getMonth() + 1
            var dd = d.getDate()
            var text = d.getFullYear() + '-'
                + (mm < 10 ? '0' : '') + mm + '-'
                + (dd < 10 ? '0' : '') + dd
            startDateField.text = text
            root.startDateText  = text
            appManager1.setStatisticsSetting("graphStartDate", text)
            root.refresh()
            statsDateDialog.close()
        }

        contentItem: RowLayout {
            Controls.Button {
                text: qsTr("1 day ago")
                onClicked: { var d = new Date(); d.setDate(d.getDate() - 1);   statsDateDialog.applyDate(d) }
            }
            Controls.Button {
                text: qsTr("1 week ago")
                onClicked: { var d = new Date(); d.setDate(d.getDate() - 7);   statsDateDialog.applyDate(d) }
            }
            Controls.Button {
                text: qsTr("1 month ago")
                onClicked: { var d = new Date(); d.setMonth(d.getMonth() - 1); statsDateDialog.applyDate(d) }
            }
            Controls.Button {
                text: qsTr("1 year ago")
                onClicked: { var d = new Date(); d.setFullYear(d.getFullYear() - 1); statsDateDialog.applyDate(d) }
            }
        }
    }

    // ─── Chart ────────────────────────────────────────────────────────────────
    ChartView {
        id: chart
        Layout.fillWidth:  true
        Layout.fillHeight: true
        Layout.minimumHeight: Kirigami.Units.gridUnit * 20
        antialiasing: true
        legend.visible:    true
        legend.alignment:  Qt.AlignBottom

        DateTimeAxis {
            id: axisX
            format:    "yyyy-MM-dd"
            titleText: qsTr("Date")
        }
        ValueAxis {
            id: axisY
            titleText: qsTr("Total")
            min: 0
            max: 100
        }

        LineSeries {
            id: series1Line
            name:          qsTr("Catalogs") + " / " + qsTr("Total File Size")
            color:         "#209fdf"
            width:         2
            pointsVisible: false
            axisX: axisX
            axisY: axisY
        }

        LineSeries {
            id: series2Line
            name:          qsTr("Storage") + " / " + qsTr("Used Space")
            color:         "#f6a625"
            width:         2
            pointsVisible: false
            axisX: axisX
            axisY: axisY
        }

        LineSeries {
            id: series3Line
            name:          qsTr("Storage") + " / " + qsTr("Total Space")
            color:         "#99ca53"
            width:         2
            pointsVisible: false
            axisX: axisX
            axisY: axisY
        }

        // Rubber-band zoom: drag a rectangle to zoom in (K2 RectangleRubberBand).
        // Right-click or double-click resets the zoom.
        Rectangle {
            id: zoomSelection
            visible: false
            color:        "#3309c6f4"
            border.color: "#09c6f4"
            border.width: 1
        }

        MouseArea {
            id: zoomArea
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            property real startX: 0
            property real startY: 0

            onPressed: (mouse) => {
                if (mouse.button === Qt.LeftButton) {
                    startX = mouse.x
                    startY = mouse.y
                    zoomSelection.x = mouse.x
                    zoomSelection.y = mouse.y
                    zoomSelection.width = 0
                    zoomSelection.height = 0
                    zoomSelection.visible = true
                }
            }
            onPositionChanged: (mouse) => {
                if (zoomSelection.visible) {
                    zoomSelection.x = Math.min(startX, mouse.x)
                    zoomSelection.y = Math.min(startY, mouse.y)
                    zoomSelection.width  = Math.abs(mouse.x - startX)
                    zoomSelection.height = Math.abs(mouse.y - startY)
                }
            }
            onReleased: (mouse) => {
                if (mouse.button === Qt.RightButton) {
                    chart.zoomReset()
                    return
                }
                zoomSelection.visible = false
                if (zoomSelection.width > 5 && zoomSelection.height > 5)
                    chart.zoomIn(Qt.rect(zoomSelection.x, zoomSelection.y,
                                         zoomSelection.width, zoomSelection.height))
            }
            onDoubleClicked: chart.zoomReset()
        }
    }
}
