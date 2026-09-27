import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls

// The combo box every K3 page uses (SpecComboBoxes.md, CBX-C8).
//
// Left to themselves the two styles size a combo box in opposite ways. Breeze
// makes the box as wide as its widest entry, so a long French entry pushed the
// Search column past the page edge; its list follows the widest entry too, so it
// came out narrower than a stretched box. Fusion — forced on Windows and macOS
// (main.cpp) — keeps the box small and makes the list exactly as wide as the
// box, so long entries were cut in both. One definition gives the same result
// on both:
//
//  - the box takes its widest entry as natural width (CBX-F1), and may only
//    shrink, eliding, to fit the room it is given (CBX-F2). A call site that
//    stretches or aligns its fields sets its own Layout width (CBX-C7);
//  - the list is never narrower than the box, is wide enough for its widest
//    entry, and stays inside the window (CBX-F3, F4, F6).
//
// Widths are measured, never fixed, so they follow the text-size setting and
// the language (CBX-C2).
Controls.ComboBox {
    id: combo

    // Room a custom delegate or contentItem puts before the text (an icon, a
    // flag), with its spacing. The measured text does not include it.
    property real rowLeadingWidth: 0

    // The entry that needs the most room, measured in the combo's own font.
    readonly property string widestEntry: {
        var best = "", bestWidth = -1
        for (var i = 0; i < count; ++i) {
            var w = entryMetrics.advanceWidth(textAt(i))
            if (w > bestWidth) { bestWidth = w; best = textAt(i) }
        }
        return best
    }

    FontMetrics { id: entryMetrics; font: combo.font }

    // A list row holding the widest entry, built by the current style, gives the
    // list width without assuming either style's padding.
    Controls.ItemDelegate {
        id: rowProbe
        visible: false
        enabled: false
        font: combo.font
        text: combo.widestEntry
    }

    implicitContentWidthPolicy: Controls.ComboBox.WidestText
    Layout.fillWidth: true
    Layout.preferredWidth: implicitWidth + rowLeadingWidth
    Layout.maximumWidth: implicitWidth + rowLeadingWidth

    popup.width: Math.min(Window.window ? Window.window.width : width,
                          Math.max(width, rowProbe.implicitWidth + rowLeadingWidth
                                          + popup.leftPadding + popup.rightPadding))
    // Fusion leaves these at -1 (no limit), so a list wider than the room on its
    // right ran off the window; at 0 Qt shifts it back inside.
    popup.leftMargin: 0
    popup.rightMargin: 0
}
