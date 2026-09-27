import QtQuick
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

// The scrollable page every K3 page uses (SpecPageLayout.md, PGL-C5).
//
// A style may draw the vertical scrollbar over the page instead of beside it.
// Breeze on a Plasma desktop does: a thin bar inside the page's right margin,
// widening when hovered, which then lay on the content — the Search column
// beside the Results showed it. When the scrollbar overlaps the content area,
// the right margin grows to clear it at its widest, plus a small gap
// (PGL-F1). A style that already puts it beside the content keeps the normal
// margin, as does a page with nothing to scroll (PGL-F2).
Kirigami.ScrollablePage {
    id: page

    // The flickable sits in the page's ScrollView, which carries the scrollbar.
    readonly property var _verticalBar: flickable && flickable.parent
                                        ? flickable.parent.Controls.ScrollBar.vertical : null

    // How far the scrollbar, at its widest, reaches into the content area.
    // Read from the running style, never assumed (PGL-C1). Measured on change
    // and stored, not bound: the margin moves the scrollbar, so a binding would
    // chase its own result.
    property real _barOverlap: 0

    function _measureBar() {
        var bar = _verticalBar
        if (!bar || !bar.visible || !flickable) {
            _barOverlap = 0
            return
        }
        var barRight = bar.x + bar.width
        var barLeft  = barRight - Math.max(bar.width, bar.implicitWidth)
        _barOverlap = Math.max(0, flickable.x + flickable.width - barLeft)
    }

    Connections {
        target: page._verticalBar
        function onVisibleChanged()       { Qt.callLater(page._measureBar) }
        function onWidthChanged()         { Qt.callLater(page._measureBar) }
        function onImplicitWidthChanged() { Qt.callLater(page._measureBar) }
    }
    onWidthChanged: Qt.callLater(_measureBar)
    Component.onCompleted: Qt.callLater(_measureBar)

    rightPadding: _barOverlap > 0 ? Math.max(padding, _barOverlap + Kirigami.Units.smallSpacing)
                                  : padding
}
