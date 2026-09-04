import QtQuick
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

// A button that carries an icon and no text.
//
// Fusion — the style forced on Windows and macOS (main.cpp) — sizes every button
// to a minimum meant for a text label, so an icon-only button comes out as wide
// as one saying "Cancel". Breeze on Linux does not, which is why this only shows
// on the other two platforms. Sizing to the icon plus its padding gives the same
// compact button everywhere, and keeps one definition rather than thirty-odd.
//
// Based on Button rather than ToolButton because almost every call site is a
// framed Button beside a text field; a ToolButton base would have quietly
// flattened them all. The few genuine tool buttons pass flat: true.
Controls.Button {
    id: iconButton

    // The same metric as the Selection panel's filter buttons, which sit well
    // beside a text field. Fusion pads far more generously than Breeze, so
    // without this the glyph looked right on Linux and oversized on Windows.
    readonly property real side: Kirigami.Units.iconSizes.small
                                 + Kirigami.Units.smallSpacing * 2

    // Square, and as tall as a text field at the same font so a button beside one
    // lines up rather than standing taller — which is what showed on Windows.
    // Width follows the same number: setting only the height left them oblong.
    readonly property real box: Math.max(side, iconButton.textFieldHeight)

    display: Controls.AbstractButton.IconOnly
    implicitWidth:  box
    implicitHeight: box
    padding: 0

    // Measured from a real TextField rather than assumed: the two styles pad
    // differently, and hardcoding a number would only be right on one of them.
    readonly property real textFieldHeight: heightProbe.implicitHeight
    Controls.TextField {
        id: heightProbe
        visible: false
        enabled: false
    }

    // One step down from the button's own metric: at smallMedium the glyph filled
    // the button edge to edge and read as heavy next to a text field.
    icon.width:  Kirigami.Units.iconSizes.small
    icon.height: Kirigami.Units.iconSizes.small

    // A tooltip is what tells the user what the icon does, so it is part of the
    // component rather than repeated at every call site. Set `text` for it.
    Controls.ToolTip.text: text
    Controls.ToolTip.visible: hovered && text.length > 0
}
