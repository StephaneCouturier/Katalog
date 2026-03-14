import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import Qt.labs.platform

Kirigami.FormLayout {
    id: pageDevicesView

    ListModel {
        id: devicesList
        ListElement {
            name: "Local Drive"
            description: "#1 - 1Tb"
        }
        ListElement {
            name: "External Drive 1"
            description: "#2 - 500Gb"
        }
        ListElement {
            name: "External Drive 2"
            description: ""
        }
    }

    Kirigami.CardsListView {
        id: devicesListView
        model: devicesList
        delegate: PageDevicesViewDelegate {}
    }
}
