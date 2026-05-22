import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

ColumnLayout {
    id: root
    spacing: 0

    // Internal state
    property string filterType:        "None"   // "None"|"Source"|"Target"
    property string mappingTypeFilter: "All"    // "All"|"Backup"|"Archive"
    property var    mappings:          []
    property var    totals:            ({})
    property int    runningMappingId:  -1
    property bool   isPaused:          false
    property int    progressFilesDone: 0
    property int    progressTotalFiles:0
    property real   progressFraction:  0.0
    property string progressFile:      ""
    property string lastReportSummary: ""
    property bool   lastReportIsError: false

    // Signals for sub-page navigation (connected in Main.qml)
    signal requestAddMapping()
    signal requestPreviewMapping(int mappingId)

    function refresh() {
        var deviceId = appManager1.selectedDeviceId
        root.mappings = appManager1.getBackupMappings(root.filterType, deviceId, root.mappingTypeFilter)
        root.totals   = appManager1.getBackupTotals(root.filterType, deviceId, root.mappingTypeFilter)
    }

    function formatDiff(diff, unit) {
        if (diff === 0) return qsTr("in sync")
        var sign = diff > 0 ? "+" : ""
        return sign + diff + " " + unit
    }

    Component.onCompleted: {
        var mt = appManager1.getBackupSetting("MappingTypeFilter")
        if (mt !== "") {
            root.mappingTypeFilter = mt
            var mi = typeFilterCombo.indexOfValue(mt)
            if (mi >= 0) typeFilterCombo.currentIndex = mi
        }
        var ft = appManager1.getBackupSetting("FilterType")
        if (ft !== "") root.filterType = ft
        refresh()
    }

    Connections {
        target: appManager1
        function onSelectedDeviceChanged()   { root.refresh() }
        function onBackupMappingsChanged()   { root.refresh() }
        function onBackupProgress(filesDone, totalFiles, bytesCopied, totalBytes, currentFile) {
            root.progressFilesDone  = filesDone
            root.progressTotalFiles = totalFiles
            root.progressFraction   = (totalBytes > 0) ? bytesCopied / totalBytes : 0
            root.progressFile       = currentFile
        }
        function onBackupFinished(copiedCount, movedCount, renamedCount, conflictCount, errorCount, totalBytesCopied, wasCancelled) {
            root.runningMappingId  = -1
            root.isPaused          = false
            root.progressFraction  = 1.0
            root.progressFile      = ""
            root.lastReportIsError = false
            var status = wasCancelled ? qsTr("Cancelled") : qsTr("Completed")
            root.lastReportSummary = status + " - "
                + qsTr("Copied: %1").arg(copiedCount + movedCount)
                + (renamedCount > 0 ? " · " + qsTr("Archived & copied: %1").arg(renamedCount) : "")
                + (conflictCount > 0 ? " · " + qsTr("Conflicts: %1").arg(conflictCount) : "")
                + (errorCount    > 0 ? " · " + qsTr("Errors: %1").arg(errorCount)        : "")
            root.refresh()
        }
        function onBackupNotification(message, isError) {
            root.lastReportIsError = isError
            root.lastReportSummary = message
            if (isError && root.runningMappingId !== -1)
                root.runningMappingId = -1
        }
    }

    // ─── Filter bar ───────────────────────────────────────────────────────────
    RowLayout {
        Layout.fillWidth:   true
        Layout.topMargin:   Kirigami.Units.largeSpacing
        Layout.leftMargin:  Kirigami.Units.largeSpacing
        Layout.rightMargin: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        Controls.Label { text: qsTr("Type"); opacity: 0.7 }
        Controls.ComboBox {
            id: typeFilterCombo
            textRole:  "text"
            valueRole: "value"
            model: [
                { value: "All",     text: qsTr("All")     },
                { value: "Backup",  text: qsTr("Backup")  },
                { value: "Archive", text: qsTr("Archive") }
            ]
            onActivated: {
                root.mappingTypeFilter = currentValue
                appManager1.setBackupSetting("MappingTypeFilter", currentValue)
                root.refresh()
            }
        }

        Controls.Label { text: qsTr("Filter"); opacity: 0.7 }
        RowLayout {
            spacing: 2
            Repeater {
                model: [
                    { value: "None",   label: qsTr("All")    },
                    { value: "Source", label: qsTr("Source") },
                    { value: "Target", label: qsTr("Target") }
                ]
                delegate: Controls.Button {
                    required property var modelData
                    text:      modelData.label
                    flat:      root.filterType !== modelData.value
                    checked:   root.filterType === modelData.value
                    checkable: true
                    onClicked: {
                        root.filterType = modelData.value
                        appManager1.setBackupSetting("FilterType", modelData.value)
                        root.refresh()
                    }
                }
            }
        }

        Item { Layout.fillWidth: true }

        Controls.CheckBox {
            text:    qsTr("Update catalogs before backup")
            checked: appManager1.updateBeforeBackup
            onToggled: appManager1.updateBeforeBackup = checked
        }
    }

    // ─── Coverage summary ─────────────────────────────────────────────────────
    Kirigami.InlineMessage {
        id: summaryBar
        Layout.fillWidth:    true
        Layout.topMargin:    Kirigami.Units.smallSpacing
        Layout.leftMargin:   Kirigami.Units.largeSpacing
        Layout.rightMargin:  Kirigami.Units.largeSpacing
        visible:             root.totals.totalMappings !== undefined
        type:                Kirigami.MessageType.Information
        showCloseButton:     false
        text: {
            if (!root.totals.deviceName) return ""
            return root.totals.deviceName
                + " - " + (root.totals.totalMappings || 0) + " " + qsTr("link(s)")
                + " · " + (root.totals.totalSourceSizeStr || "0") + " " + qsTr("source")
                + " · " + (root.totals.coveragePct || "0") + "% " + qsTr("covered")
        }
    }

    // ─── Report summary (backup finished / profile generated / error) ─────────
    Kirigami.InlineMessage {
        Layout.fillWidth:    true
        Layout.topMargin:    Kirigami.Units.smallSpacing
        Layout.leftMargin:   Kirigami.Units.largeSpacing
        Layout.rightMargin:  Kirigami.Units.largeSpacing
        visible:             root.lastReportSummary !== ""
        type:                root.lastReportIsError ? Kirigami.MessageType.Error
                                                    : Kirigami.MessageType.Positive
        showCloseButton:     true
        text:                root.lastReportSummary
        onVisibleChanged:    if (!visible) { root.lastReportSummary = ""; root.lastReportIsError = false }
    }

    // ─── No mappings placeholder ──────────────────────────────────────────────
    Kirigami.PlaceholderMessage {
        Layout.fillWidth:  true
        Layout.fillHeight: true
        Layout.margins:    Kirigami.Units.largeSpacing * 2
        visible:           root.mappings.length === 0
        text:              qsTr("No backup links")
        explanation:       qsTr("Create a link to define a source and target for backup or archive operations.")
        icon.name:         "backup"
    }

    // ─── Mapping cards ────────────────────────────────────────────────────────
    //
    // Rectangle + anchored ColumnLayout instead of Kirigami.AbstractCard:
    // AbstractCard is a QQC2 ItemDelegate whose internal layout chain goes stale
    // after any page visibility cycle (layer push/pop, close/reopen).
    // Rectangle.implicitHeight is a direct binding to the inner ColumnLayout's
    // implicitHeight - no QQC2 internals, always correct by construction.
    Repeater {
        id: mappingRepeater
        model: root.mappings
        delegate: Rectangle {
            id: mappingCard
            required property var modelData
            required property int index

            Layout.fillWidth:    true
            Layout.topMargin:    Kirigami.Units.smallSpacing
            Layout.leftMargin:   Kirigami.Units.largeSpacing
            Layout.rightMargin:  Kirigami.Units.largeSpacing
            Layout.bottomMargin: index === root.mappings.length - 1 ? Kirigami.Units.largeSpacing : 0

            // Height is a direct binding - never stale after visibility cycles
            implicitHeight: cardContent.implicitHeight + Kirigami.Units.largeSpacing * 2

            Kirigami.Theme.inherit:   false
            Kirigami.Theme.colorSet:  Kirigami.Theme.View

            color:        Kirigami.Theme.backgroundColor
            border.width: 1
            border.color: Kirigami.ColorUtils.linearInterpolation(
                              Kirigami.Theme.backgroundColor,
                              Kirigami.Theme.textColor,
                              Kirigami.Theme.frameContrast)
            radius:       Kirigami.Units.cornerRadius

            // Drop shadow matching Kirigami DefaultCardBackground
            Rectangle {
                anchors { left: parent.left; right: parent.right; top: parent.top }
                anchors.topMargin: Math.round(Kirigami.Units.smallSpacing / 4)
                radius: parent.radius
                height: parent.height
                color:  Qt.darker(Qt.rgba(Kirigami.Theme.backgroundColor.r,
                                          Kirigami.Theme.backgroundColor.g,
                                          Kirigami.Theme.backgroundColor.b, 0.6), 1.1)
                z: -1
            }

            property bool isRunning: root.runningMappingId === modelData.mappingId

            ColumnLayout {
                id: cardContent
                anchors {
                    top:   parent.top
                    left:  parent.left
                    right: parent.right
                    margins: Kirigami.Units.largeSpacing
                }
                spacing: Kirigami.Units.smallSpacing

                // ── Header: icon + name + type badge ──────────────────────────
                RowLayout {
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing

                    Kirigami.Icon {
                        source: modelData.mappingType === "Archive" ? "archive-extract" : "backup"
                        implicitWidth:  Kirigami.Units.iconSizes.small
                        implicitHeight: Kirigami.Units.iconSizes.small
                    }
                    Controls.Label {
                        text: modelData.mappingName
                        font.bold: true
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                    Controls.Label {
                        text:  modelData.mappingType
                        color: modelData.mappingType === "Archive"
                               ? Kirigami.Theme.neutralTextColor
                               : Kirigami.Theme.positiveTextColor
                        font.pixelSize: Kirigami.Units.gridUnit * 0.75
                        padding: 3
                        background: Rectangle {
                            color:   parent.color
                            opacity: 0.15
                            radius:  3
                        }
                    }
                }

                Kirigami.Separator { Layout.fillWidth: true; opacity: 0.5 }

                // ── Source ────────────────────────────────────────────────────
                RowLayout {
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing
                    Controls.Label {
                        text:    qsTr("Source")
                        opacity: 0.7
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                    }
                    Rectangle {
                        implicitWidth:  Kirigami.Units.iconSizes.small * 0.55
                        implicitHeight: implicitWidth
                        radius: width / 2
                        color: modelData.sourceActive ? Kirigami.Theme.positiveTextColor
                                                      : Kirigami.Theme.disabledTextColor
                    }
                    Controls.Label {
                        text:  modelData.sourceName
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                    Controls.Label {
                        text:    modelData.sourceSizeStr + " · " + modelData.sourceFileCount + " " + qsTr("files")
                        opacity: 0.7
                        elide:   Text.ElideRight
                    }
                }

                // ── Target ────────────────────────────────────────────────────
                RowLayout {
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing
                    Controls.Label {
                        text:    qsTr("Target")
                        opacity: 0.7
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                    }
                    Rectangle {
                        implicitWidth:  Kirigami.Units.iconSizes.small * 0.55
                        implicitHeight: implicitWidth
                        radius: width / 2
                        color: modelData.targetActive ? Kirigami.Theme.positiveTextColor
                                                      : Kirigami.Theme.disabledTextColor
                    }
                    Controls.Label {
                        text:  modelData.targetName
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                    Controls.Label {
                        text:    modelData.targetSizeStr + " · " + modelData.targetFileCount + " " + qsTr("files")
                        opacity: 0.7
                        elide:   Text.ElideRight
                    }
                }

                // ── Diff ──────────────────────────────────────────────────────
                RowLayout {
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing
                    Controls.Label {
                        text:    qsTr("Diff")
                        opacity: 0.7
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 4
                    }
                    Controls.Label {
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        text: {
                            var sizeSign  = modelData.sizeDiff       > 0 ? "+" : (modelData.sizeDiff       < 0 ? "−" : "")
                            var filesSign = modelData.fileCountDiff   > 0 ? "+" : (modelData.fileCountDiff   < 0 ? "−" : "")
                            if (modelData.sizeDiff === 0 && modelData.fileCountDiff === 0)
                                return qsTr("in sync")
                            return sizeSign + modelData.sizeDiffStr
                                + " · " + filesSign + Math.abs(modelData.fileCountDiff) + " " + qsTr("files")
                                + (modelData.sourceDateUpdated ? " · " + modelData.sourceDateUpdated : "")
                        }
                        color: modelData.sizeDiff > 0 ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.textColor
                    }
                }

                // ── Progress (only while running) ─────────────────────────────
                Loader {
                    id: progressLoader
                    Layout.fillWidth: true
                    active:  mappingCard.isRunning
                    visible: mappingCard.isRunning

                    sourceComponent: ColumnLayout {
                        width: progressLoader.width
                        spacing: Kirigami.Units.smallSpacing

                        Controls.ProgressBar {
                            Layout.fillWidth: true
                            from: 0; to: 1
                            value: root.progressFraction
                        }
                        Controls.Label {
                            Layout.fillWidth: true
                            text: Math.round(root.progressFraction * 100) + "% · "
                                + root.progressFilesDone + "/" + root.progressTotalFiles
                                + (root.progressFile ? " · " + root.progressFile : "")
                            elide: Text.ElideRight
                            font.pixelSize: Kirigami.Units.gridUnit * 0.85
                            opacity: 0.8
                        }
                    }
                }

                // ── Action buttons - Flow wraps on narrow windows ──────────────
                Flow {
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing

                    Controls.Button {
                        visible:   !mappingCard.isRunning
                        text:      modelData.mappingType === "Archive" ? qsTr("Run Archive") : qsTr("Run Backup")
                        icon.name: "media-playback-start"
                        enabled:   modelData.sourceActive && modelData.targetActive
                                   && root.runningMappingId === -1
                                   && !appManager1.catalogUpdateForBackupRunning
                        onClicked: {
                            root.runningMappingId   = modelData.mappingId
                            root.isPaused           = false
                            root.progressFraction   = 0
                            root.progressFilesDone  = 0
                            root.progressTotalFiles = 0
                            root.progressFile       = ""
                            root.lastReportSummary  = ""
                            appManager1.runBackup(modelData.mappingId)
                        }
                    }
                    Controls.Button {
                        visible:   mappingCard.isRunning && !root.isPaused
                        text:      qsTr("Pause")
                        icon.name: "media-playback-pause"
                        onClicked: { root.isPaused = true; appManager1.pauseBackup() }
                    }
                    Controls.Button {
                        visible:   mappingCard.isRunning && root.isPaused
                        text:      qsTr("Resume")
                        icon.name: "media-playback-start"
                        onClicked: { root.isPaused = false; appManager1.resumeBackup() }
                    }
                    Controls.Button {
                        visible:   mappingCard.isRunning
                        text:      qsTr("Cancel")
                        icon.name: "process-stop"
                        onClicked: appManager1.stopBackup()
                    }
                    Controls.Button {
                        visible:   !mappingCard.isRunning
                        text:      modelData.mappingType === "Archive" ? qsTr("Preview Archive") : qsTr("Preview Backup")
                        icon.name: "view-preview"
                        onClicked: root.requestPreviewMapping(modelData.mappingId)
                    }
                    Controls.Button {
                        visible:   !mappingCard.isRunning
                        icon.name: "overflow-menu"
                        display:   Controls.AbstractButton.IconOnly
                        Controls.ToolTip.text:    qsTr("More actions")
                        Controls.ToolTip.visible: hovered
                        onClicked: moreMenu.open()

                        Controls.Menu {
                            id: moreMenu
                            Controls.MenuItem {
                                text: qsTr("Replicate directories")
                                icon.name: "folder-sync"
                                onTriggered: {
                                    var r = appManager1.replicateDirectories(modelData.mappingId)
                                    root.lastReportSummary = r.error || (qsTr("Replicate") + " - "
                                        + qsTr("Created: %1").arg(r.created || 0)
                                        + " · " + qsTr("Already existing: %1").arg(r.skipped || 0)
                                        + (r.errors > 0 ? " · " + qsTr("Errors: %1").arg(r.errors) : ""))
                                }
                            }
                            Controls.MenuItem {
                                text: qsTr("Invert (swap source and target)")
                                icon.name: "object-flip-horizontal"
                                onTriggered: {
                                    if (appManager1.invertBackupMapping(modelData.mappingId))
                                        root.refresh()
                                }
                            }
                            Controls.MenuItem {
                                text: qsTr("Export last preview to CSV")
                                icon.name: "document-export"
                                onTriggered: {
                                    var path = appManager1.exportLastBackupPreviewToCsv()
                                    root.lastReportSummary = path
                                }
                            }
                            Controls.MenuSeparator {}
                            Controls.MenuItem {
                                text: qsTr("Delete")
                                icon.name: "edit-delete"
                                onTriggered: deleteDialog.open()
                            }
                        }

                        Kirigami.PromptDialog {
                            id: deleteDialog
                            title: qsTr("Delete link")
                            subtitle: qsTr("Delete link \"%1\"? This cannot be undone.").arg(modelData.mappingName)
                            standardButtons: Kirigami.Dialog.Yes | Kirigami.Dialog.No
                            onAccepted: {
                                if (appManager1.deleteBackupMapping(modelData.mappingId))
                                    root.refresh()
                            }
                        }
                    }
                }
            }
        }
    }

    // Bottom spacer
    Item { implicitHeight: Kirigami.Units.largeSpacing }
}
