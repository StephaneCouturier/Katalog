import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import Qt.labs.platform

Kirigami.AbstractCard {
    contentItem: Item {
        // implicitWidth/Height define the natural width/height
        // of an item if no width or height is specified.
        // The setting below defines a component's preferred size based on its content
        implicitWidth: deviceDelegateLayout.implicitWidth
        implicitHeight: deviceDelegateLayout.implicitHeight

        GridLayout {
            id: deviceDelegateLayout
            anchors {
                left: parent.left
                top: parent.top
                right: parent.right
            }
            rowSpacing: Kirigami.Units.largeSpacing
            columnSpacing: Kirigami.Units.largeSpacing
            columns: root.wideScreen ? 4 : 2

            ColumnLayout {
                RowLayout {
                    Kirigami.Icon {
                        source: "drive-harddisk"
                    }

                    Kirigami.Heading {
                        Layout.fillWidth: true
                        level: 2
                        text: name
                    }
                }
                Kirigami.Separator {
                    Layout.fillWidth: true
                    visible: description.length > 0
                }

                Controls.Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: description
                    visible: description.length > 0
                }
            }

            Controls.Button {
                Layout.alignment: Qt.AlignRight
                Layout.columnSpan: 2
                //text: "Select" //i18n("Select")
                icon.name: "edit-select"
                onClicked: showPassiveNotification("Select clicked, no action")
            }
        }
    }
}


