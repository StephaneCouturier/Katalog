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
 * so onSync also pushes the same colours into the palette of the window of
 * each synced item; the controls inherit it, keeping the whole window
 * consistently light or dark. It runs on every sync, not only once: when the
 * preference changes, the first syncs happen before all colours are updated.
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

    onSync: object => {
        const win = object.Window.window
        const p = win ? win.palette : null
        if (!p)
            return
        p.window = theme.backgroundColor
        p.windowText = theme.textColor
        p.base = theme.viewBackgroundColor
        p.alternateBase = theme.viewAlternateBackgroundColor
        p.text = theme.viewTextColor
        p.button = theme.buttonBackgroundColor
        p.buttonText = theme.buttonTextColor
        p.brightText = theme.textColor
        p.placeholderText = theme.disabledTextColor
        p.highlight = theme.selectionBackgroundColor
        p.highlightedText = theme.selectionTextColor
        p.toolTipBase = theme.tooltipBackgroundColor
        p.toolTipText = theme.tooltipTextColor
        p.link = theme.linkColor
        p.linkVisited = theme.visitedLinkColor
        p.disabled.windowText = theme.disabledTextColor
        p.disabled.text = theme.disabledTextColor
        p.disabled.buttonText = theme.disabledTextColor
    }
}
