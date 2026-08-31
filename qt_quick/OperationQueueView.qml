import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

// Global activity panel: what is running now, and what is queued behind it.
// Catalog operations run strictly one at a time (SpecOperationQueue.md), so this
// is application-wide state and lives in the window footer. It reports the
// running job's progress alongside the queue, and hides itself when idle.
//
// Root is a ToolBar so the window footer gives it the full window width; a bare
// layout would size to its contents and sit against the left edge.
Controls.ToolBar {
    id: root

    // Creation, update and search all report here and nowhere else (OPQ-C16).
    readonly property bool busy: appManager1.catalogIsCreating
                                 || appManager1.deviceUpdateIsRunning
                                 || appManager1.searchIsRunning
    readonly property string statusText: appManager1.catalogIsCreating
                                         ? appManager1.catalogStatusText
                                         : appManager1.searchIsRunning
                                           ? appManager1.searchStatusText
                                           : appManager1.deviceUpdateStatusText

    // How long the panel stays up after the work ends. The catalog job runs on
    // the GUI thread and pumps the event loop by hand, so a small catalog can
    // finish before Qt Quick paints a single frame: without a tail the panel is
    // shown for zero frames and the completion message, which is built and
    // emitted, is never seen. 5000 ms is what K2's status bar uses; it is a
    // design choice, not a ratified rule.
    readonly property int completionLinger: 5000

    property bool lingering: false

    Timer {
        id: lingerTimer
        interval: root.completionLinger
        onTriggered: root.lingering = false
    }

    onBusyChanged: {
        if (root.busy) {
            // A new operation supersedes the previous one's tail.
            root.lingering = false
            lingerTimer.stop()
        } else {
            root.lingering = true
            lingerTimer.restart()
        }
    }

    // The tail only holds the panel open while there is something to read: a
    // stopped or failed operation clears the status text, and an empty bar
    // lingering for five seconds would say nothing.
    visible: busy || (lingering && root.statusText.length > 0)
             || appManager1.operationQueue.length > 0
    height: visible ? implicitHeight : 0

    // Positioned by the caller against the page area, so no drawer compensation
    // is needed here.
    leftPadding:  Kirigami.Units.smallSpacing
    rightPadding: Kirigami.Units.smallSpacing

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        // ── Running job ──────────────────────────────────────────────────
        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing
            visible: root.busy || root.statusText.length > 0

            // Deliberately bound to the real running state, not to the linger
            // tail: a turning indicator must never sit beside a "Completed"
            // line. During the tail the row shows the message alone.
            Controls.BusyIndicator {
                // Not simply `busy`: a paused search is still "running", and an
                // indicator that keeps turning would say work is happening when
                // none is (OPQ-C12).
                running: root.busy && !appManager1.searchIsPaused
                visible: root.busy
                implicitWidth:  Kirigami.Units.gridUnit * 1.2
                implicitHeight: Kirigami.Units.gridUnit * 1.2
            }
            Controls.Label {
                Layout.fillWidth: true
                text: root.statusText
                textFormat: Text.StyledText
                elide: Text.ElideRight
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.85
            }
            Controls.Button {
                text: qsTr("Stop")
                icon.name: "process-stop"
                flat: true
                visible: root.busy
                onClicked: {
                    if (appManager1.catalogIsCreating)
                        appManager1.stopCatalogCreation()
                    else if (appManager1.searchIsRunning)
                        appManager1.stopSearch()
                    else
                        appManager1.stopDeviceUpdate()
                }
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
            visible: appManager1.operationQueueWaitingCount > 0
        }

        // ── Queue ────────────────────────────────────────────────────────
        RowLayout {
            Layout.fillWidth: true
            visible: appManager1.operationQueueWaitingCount > 0

            Controls.Label {
                text: qsTr("Queue")
                font.bold: true
                opacity: 0.8
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.85
            }
            Controls.Label {
                text: qsTr("%1 waiting").arg(appManager1.operationQueueWaitingCount)
                opacity: 0.7
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.85
            }
            Item { Layout.fillWidth: true }
            Controls.Button {
                text: qsTr("Clear")
                icon.name: "edit-clear"
                flat: true
                onClicked: appManager1.clearOperationQueue()
            }
        }

        Repeater {
            model: appManager1.operationQueue

            RowLayout {
                id: entryRow
                required property var modelData
                required property int index

                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.largeSpacing
                spacing: Kirigami.Units.smallSpacing
                // The running entry is already described by the status row above.
                visible: !entryRow.modelData.running
                height: visible ? implicitHeight : 0

                Kirigami.Icon {
                    source: entryRow.modelData.isCreate ? "list-add" : "view-refresh"
                    implicitWidth:  Kirigami.Units.iconSizes.small
                    implicitHeight: Kirigami.Units.iconSizes.small
                    opacity: 0.7
                }
                Controls.Label {
                    text: (entryRow.modelData.isCreate ? qsTr("Create") : qsTr("Update"))
                          + " · " + entryRow.modelData.deviceName
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                    opacity: 0.8
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.85
                }
                Controls.Button {
                    icon.name: "list-remove"
                    flat: true
                    // Waiting entries only; the running one is stopped above.
                    onClicked: appManager1.removeQueuedOperation(
                                   entryRow.index - (appManager1.operationQueue.length
                                                     > appManager1.operationQueueWaitingCount ? 1 : 0))
                }
            }
        }
    }
}
