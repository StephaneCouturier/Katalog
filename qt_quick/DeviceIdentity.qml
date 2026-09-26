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

    // Overridden when the name sits on a filled highlight, where the ordinary
    // text colour would have too little contrast.
    property color nameColor: Kirigami.Theme.textColor

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

    // The Devices card draws its own, larger icon in a separate column, so it
    // takes the name rules from here without a second icon beside them
    // (DVP-C14). Default true, so every other caller is unaffected.
    property bool showIcon: true

    // A card never hides text, it wraps it (SpecCardsAndTables.md CDT-F1). The
    // caller decides, because this component also serves the selected-device
    // reminder above the Selection list, which is a single line that must not
    // grow and push the list down (CDT-C4 / DVP-C16). Default off: only the
    // Devices card turns it on so far.
    property bool nameWraps: false

    // The icon's size, so a caller can follow the "Use bigger icon size"
    // setting (THM-F7). Default unchanged, so the Selection cards and the
    // reminder above them keep the size they have.
    property real iconSize: Kirigami.Units.iconSizes.small

    spacing: Kirigami.Units.smallSpacing

    Kirigami.Icon {
        visible: identity.showIcon
        // "All" has no device of its own, so it borrows the generic folder icon
        // K3 already uses for the all-devices entry elsewhere (SEL-F5).
        source: identity.deviceType === "All"     ? "folder"
              : identity.deviceType === "Virtual" ? "drive-multidisk"
              : identity.deviceType === "Storage" ? "drive-harddisk"
              : identity.deviceIsActive ? "media-optical-blu-ray" : "media-optical"
        implicitWidth:  identity.iconSize
        implicitHeight: identity.iconSize
    }

    Kirigami.Heading {
        Layout.fillWidth: true
        level: 2
        text: identity.deviceName
        wrapMode:  identity.nameWraps ? Text.Wrap : Text.NoWrap
        elide:     identity.nameWraps ? Text.ElideNone : Text.ElideRight
        maximumLineCount: identity.nameWraps ? Number.MAX_VALUE : 1
        font.pointSize: appManager1.textPointSize

        // font.weight, not font.bold: Kirigami.Heading binds font.weight from its
        // own level, and that binding overwrites whatever font.bold sets.
        font.weight: identity.nameWeight
        font.italic: identity.nameItalic
        opacity:     identity.nameOpacity
        color:       identity.nameColor
    }
}
