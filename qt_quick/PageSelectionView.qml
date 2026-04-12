import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

Kirigami.CardsListView {
    id: selectionListView1
    model: appManager1.deviceListModel
    delegate: PageSelectionDelegate {}
}
