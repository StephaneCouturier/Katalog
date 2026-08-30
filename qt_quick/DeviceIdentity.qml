import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

// A device's icon and name, styled by type.
//
// The single definition of those rules (SpecSelection.md SEL-C1). Both the
// Selection cards and the selected-device reminder above them use this, so the
// reminder can never end up looking like a device card of a different vintage:
// a change here reaches both at once. Duplicating the rules into a second file
// is what this component exists to prevent.
RowLayout {
    id: identity

    // "Virtual" | "Storage" | "Catalog" | "All"
    required property string deviceType
    required property string deviceName
    property bool deviceIsActive: false

    // Scales with the Selection page's card-size setting.
    property real fontScale: 1.0

    // Font rules ported from K2's device tree (qt_widgets/devicetreeview.cpp,
    // FontRole :133-150 and ForegroundRole :199-218): Virtual is bold italic and
    // the most dimmed, Storage is bold and dimmed a little, a Catalog is left
    // plain so it reads as the leaf.
    readonly property int  nameWeight:  (deviceType === "Virtual" || deviceType === "Storage")
                                        ? Font.Bold : Font.Normal
    readonly property bool nameItalic:  deviceType === "Virtual"
    readonly property real nameOpacity: deviceType === "Virtual" ? 0.60
                                      : deviceType === "Storage" ? 0.78
                                      : 1.0

    spacing: Kirigami.Units.smallSpacing

    Kirigami.Icon {
        // "All" has no device of its own, so it borrows the generic folder icon
        // K3 already uses for the all-devices entry elsewhere (SEL-F5).
        source: identity.deviceType === "All"     ? "folder"
              : identity.deviceType === "Virtual" ? "drive-multidisk"
              : identity.deviceType === "Storage" ? "drive-harddisk"
              : identity.deviceIsActive ? "media-optical-blu-ray" : "media-optical"
        implicitWidth:  Kirigami.Units.iconSizes.small
        implicitHeight: Kirigami.Units.iconSizes.small
    }

    Kirigami.Heading {
        Layout.fillWidth: true
        level: 2
        text: identity.deviceName
        elide: Text.ElideRight
        maximumLineCount: 1
        font.pointSize: Kirigami.Theme.defaultFont.pointSize * identity.fontScale

        // font.weight, not font.bold: Kirigami.Heading binds font.weight from its
        // own level, and that binding overwrites whatever font.bold sets.
        font.weight: identity.nameWeight
        font.italic: identity.nameItalic
        opacity:     identity.nameOpacity
    }
}
