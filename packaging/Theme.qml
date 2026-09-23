/*
 * Kirigami colour theme for the Fusion Controls style, bundled with packaged
 * Katalog 3 builds (Linux AppImage, Windows and macOS portable).
 *
 * Packaging copies this file into the bundled Kirigami module as
 *   org/kde/kirigami/styles/Fusion/Theme.qml
 * which is where Kirigami looks for a per-style colour theme.
 *
 * Packaged builds ship no qqc2-desktop-style, so Kirigami falls back to its
 * Basic theme, whose colours are fixed Breeze Light. This file replaces that
 * fallback: it follows the host light/dark preference as reported to Qt
 * (Qt.styleHints.colorScheme) and uses the Breeze Light or Breeze Dark values.
 * Unknown / no preference resolves to light. See SpecK3Deployment.md (DPL-F5).
 *
 * Fusion controls take their colours from the Qt palette, not from Kirigami,
 * so the same colours are also written into the palette of each window that
 * holds a themed item; the controls inherit it, keeping the whole window
 * consistently light or dark.
 */

import QtQuick
import org.kde.kirigami as Kirigami

Kirigami.BasicThemeDefinition {
    id: theme

    readonly property bool dark: Qt.styleHints.colorScheme === Qt.ColorScheme.Dark

    textColor: dark ? "#fcfcfc" : "#232629"
    disabledTextColor: dark ? "#a1a9b1" : "#707d8a"

    highlightColor: "#3daee9"
    highlightedTextColor: dark ? "#fcfcfc" : "#ffffff"
    backgroundColor: dark ? "#202326" : "#eff0f1"
    alternateBackgroundColor: dark ? "#292c30" : "#e3e5e7"

    hoverColor: "#3daee9"
    focusColor: "#3daee9"

    activeTextColor: "#3daee9"
    activeBackgroundColor: "#3daee9"
    linkColor: dark ? "#1d99f3" : "#2980b9"
    linkBackgroundColor: dark ? "#1d99f3" : "#2980b9"
    visitedLinkColor: "#9b59b6"
    visitedLinkBackgroundColor: "#9b59b6"
    negativeTextColor: "#da4453"
    negativeBackgroundColor: "#da4453"
    neutralTextColor: "#f67400"
    neutralBackgroundColor: "#f67400"
    positiveTextColor: "#27ae60"
    positiveBackgroundColor: "#27ae60"

    buttonTextColor: dark ? "#fcfcfc" : "#232629"
    buttonBackgroundColor: dark ? "#292c30" : "#fcfcfc"
    buttonAlternateBackgroundColor: dark ? "#1e5774" : "#a3d4fa"
    buttonHoverColor: "#3daee9"
    buttonFocusColor: "#3daee9"

    viewTextColor: dark ? "#fcfcfc" : "#232629"
    viewBackgroundColor: dark ? "#141618" : "#ffffff"
    viewAlternateBackgroundColor: dark ? "#1d1f22" : "#f7f7f7"
    viewHoverColor: "#3daee9"
    viewFocusColor: "#3daee9"

    selectionTextColor: dark ? "#fcfcfc" : "#ffffff"
    selectionBackgroundColor: "#3daee9"
    selectionAlternateBackgroundColor: dark ? "#1e5774" : "#a3d4fa"
    selectionHoverColor: "#3daee9"
    selectionFocusColor: "#3daee9"

    tooltipTextColor: dark ? "#fcfcfc" : "#232629"
    tooltipBackgroundColor: dark ? "#292c30" : "#f7f7f7"
    tooltipAlternateBackgroundColor: dark ? "#202326" : "#eff0f1"
    tooltipHoverColor: "#3daee9"
    tooltipFocusColor: "#3daee9"

    complementaryTextColor: "#fcfcfc"
    complementaryBackgroundColor: dark ? "#202326" : "#2a2e32"
    complementaryAlternateBackgroundColor: dark ? "#1e5774" : "#1b1e20"
    complementaryHoverColor: "#3daee9"
    complementaryFocusColor: "#3daee9"

    headerTextColor: dark ? "#fcfcfc" : "#232629"
    headerBackgroundColor: dark ? "#292c30" : "#dee0e2"
    headerAlternateBackgroundColor: dark ? "#202326" : "#eff0f1"
    headerHoverColor: "#3daee9"
    headerFocusColor: "#3daee9"

    // Fusion controls inherit the window palette. It is written once per window
    // and again when the light/dark preference changes — never on every sync:
    // onSync runs for every themed item, and each palette write propagates to
    // every item in the window, which made startup take minutes on the real app.
    property var styledWindows: []

    function applyPalette(win) {
        const p = win.palette
        p.window = backgroundColor
        p.windowText = textColor
        p.base = viewBackgroundColor
        p.alternateBase = viewAlternateBackgroundColor
        p.text = viewTextColor
        p.button = buttonBackgroundColor
        p.buttonText = buttonTextColor
        p.brightText = textColor
        p.placeholderText = disabledTextColor
        p.highlight = selectionBackgroundColor
        p.highlightedText = selectionTextColor
        p.toolTipBase = tooltipBackgroundColor
        p.toolTipText = tooltipTextColor
        p.link = linkColor
        p.linkVisited = visitedLinkColor
        p.disabled.windowText = disabledTextColor
        p.disabled.text = disabledTextColor
        p.disabled.buttonText = disabledTextColor
    }

    function applyToAllWindows() {
        styledWindows = styledWindows.filter(win => win)
        styledWindows.forEach(win => applyPalette(win))
    }

    // Deferred so every colour binding has updated before the palette is written.
    onDarkChanged: Qt.callLater(applyToAllWindows)

    onSync: object => {
        const win = object.Window.window
        if (!win || styledWindows.indexOf(win) >= 0)
            return
        styledWindows.push(win)
        applyPalette(win)
    }
}
