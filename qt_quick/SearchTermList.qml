import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami

// An editable list of search terms, one term per row.
//
// Read and write the whole list through `text`: rows are joined with '\n', which
// is the wire format Search and the search history already use — see
// SpecSearchList.md (SRL-C1). Assigning `text` rebuilds the rows; editing a row
// updates `text` with the trimmed, blank-stripped join.
//
// Set showInlineClear: true to draw the per-row clear icon inside each field
// (the exclude list uses it; the text list uses the list-level Clear button).
// The accepted() signal fires when the user presses Enter in any row.
ColumnLayout {
    id: root
    spacing: Kirigami.Units.smallSpacing

    property string text: ""
    property bool showInlineClear: false

    // SpecSearchList.md P4 / P7
    readonly property int maxVisibleRows: 6
    readonly property int pasteRowCap: 100

    signal accepted()

    // True while we write `text` ourselves, so onTextChanged does not rebuild the
    // rows under the user's cursor and destroy the selection and focus.
    property bool _updating: false

    ListModel { id: termsModel }

    Component.onCompleted: _rebuildFromText()

    onTextChanged: {
        if (!_updating)
            _rebuildFromText()
    }

    // ── Public API ────────────────────────────────────────────────────────────

    // Prune blank rows from the UI, always keeping at least one row.
    // Called at search launch (SRL-F9); not called while typing, which would
    // delete a row the instant the user emptied it.
    function normalize() {
        var terms = _currentTerms()
        var kept = []
        for (var i = 0; i < terms.length; ++i) {
            if (terms[i].trim().length > 0)
                kept.push(terms[i].trim())
        }
        _setTerms(kept, -1)
    }

    // Reset to a single empty row.
    function clearAll() {
        _setTerms([], -1)
    }

    // Replace the whole list with the clipboard, one row per non-empty line.
    function pasteReplaceAll() {
        _setTerms(_splitLines(pageSearch1.returnClipboard()), -1)
    }

    // Apply the punctuation cleanup to every row.
    function cleanAll() {
        var terms = _currentTerms()
        for (var i = 0; i < terms.length; ++i)
            terms[i] = pageSearch1.returnCleanedText(terms[i])
        _setTerms(terms, -1)
    }

    // ── Internals ─────────────────────────────────────────────────────────────

    function _currentTerms() {
        var terms = []
        for (var i = 0; i < termsModel.count; ++i)
            terms.push(termsModel.get(i).term)
        return terms
    }

    // Split on any newline flavour, trim, drop blanks, and cap the row count.
    function _splitLines(raw) {
        var out = []
        var lines = raw.split(/\r\n|\r|\n/)
        for (var i = 0; i < lines.length; ++i) {
            var t = lines[i].trim()
            if (t.length > 0)
                out.push(t)
        }
        // SpecSearchList.md P7: terms are OR'd into one alternation regex that is
        // evaluated per file, so an unbounded paste would make search crawl.
        if (out.length > root.pasteRowCap)
            out = out.slice(0, root.pasteRowCap)
        return out
    }

    function _rebuildFromText() {
        var lines = root.text.split("\n")
        var terms = []
        for (var i = 0; i < lines.length; ++i) {
            if (lines[i].trim().length > 0)
                terms.push(lines[i].trim())
        }
        _fillModel(terms)
    }

    function _fillModel(terms) {
        termsModel.clear()
        for (var i = 0; i < terms.length; ++i)
            termsModel.append({ term: terms[i] })
        if (termsModel.count === 0)
            termsModel.append({ term: "" })
    }

    function _setTerms(terms, focusIndex) {
        _fillModel(terms)
        _commit()
        if (focusIndex >= 0)
            Qt.callLater(_focusRow, focusIndex)
    }

    // Push the normalized join back into `text` without triggering a rebuild.
    function _commit() {
        var parts = []
        for (var i = 0; i < termsModel.count; ++i) {
            var t = termsModel.get(i).term.trim()
            if (t.length > 0)
                parts.push(t)
        }
        _updating = true
        root.text = parts.join("\n")
        _updating = false
    }

    function _focusRow(index) {
        if (index < 0 || index >= rowsRepeater.count)
            return
        var item = rowsRepeater.itemAt(index)
        if (item)
            item.focusField()
    }

    // Add and remove go through the term array and rebuild the model, rather
    // than insert()/remove(). Typing into a TextField breaks its `text` binding,
    // so mutating the model in place can leave a reused delegate showing stale
    // text; a rebuild gives every row a fresh binding.
    function _addRowAfter(index) {
        var terms = _currentTerms()
        terms.splice(index + 1, 0, "")
        _setTerms(terms, index + 1)
    }

    function _removeRow(index) {
        if (termsModel.count <= 1)
            return
        var terms = _currentTerms()
        terms.splice(index, 1)
        _setTerms(terms, Math.max(0, index - 1))
    }

    // Splice a multi-line clipboard in at the cursor. Returns true when it
    // handled the paste, false to let the field paste normally.
    function _pasteAt(rowIndex, cursorPos) {
        var lines = _splitLines(pageSearch1.returnClipboard())
        if (lines.length <= 1)
            return false

        var terms = _currentTerms()
        var current = terms[rowIndex]
        var before = current.substring(0, cursorPos)
        var after = current.substring(cursorPos)

        var spliced = []
        spliced.push(before + lines[0])
        for (var i = 1; i < lines.length - 1; ++i)
            spliced.push(lines[i])
        spliced.push(lines[lines.length - 1] + after)

        var result = terms.slice(0, rowIndex).concat(spliced, terms.slice(rowIndex + 1))
        _setTerms(result, rowIndex + spliced.length - 1)
        return true
    }

    // ── Rows ──────────────────────────────────────────────────────────────────

    Controls.ScrollView {
        id: rowsScroll
        Layout.fillWidth: true
        // Grow with the rows, then cap and scroll. Per-row height is derived from
        // the column itself so it follows the platform font and style.
        Layout.preferredHeight: termsModel.count <= root.maxVisibleRows
                                ? rowsColumn.implicitHeight
                                : (rowsColumn.implicitHeight / termsModel.count) * root.maxVisibleRows
        clip: true
        Controls.ScrollBar.horizontal.policy: Controls.ScrollBar.AlwaysOff

        Column {
            id: rowsColumn
            width: rowsScroll.availableWidth
            spacing: Kirigami.Units.smallSpacing

            Repeater {
                id: rowsRepeater
                model: termsModel

                RowLayout {
                    id: termRow
                    width: rowsColumn.width
                    spacing: Kirigami.Units.smallSpacing

                    required property int index
                    required property string term

                    function focusField() {
                        termField.forceActiveFocus()
                        termField.cursorPosition = termField.text.length
                    }

                    Controls.TextField {
                        id: termField
                        Layout.fillWidth: true
                        text: termRow.term
                        leftPadding: root.showInlineClear ? Kirigami.Units.largeSpacing : undefined
                        rightPadding: root.showInlineClear && text.length > 0
                                      ? Kirigami.Units.iconSizes.small + Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
                                      : undefined

                        onTextEdited: {
                            termsModel.setProperty(termRow.index, "term", text)
                            root._commit()
                        }

                        Keys.onPressed: function(event) {
                            // Enter runs the search; Shift+Enter adds a row — the
                            // gesture that used to insert a newline now creates the
                            // row that newline stood for (SpecSearchList.md P1).
                            if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                                if (event.modifiers & Qt.ShiftModifier)
                                    root._addRowAfter(termRow.index)
                                else
                                    root.accepted()
                                event.accepted = true
                                return
                            }
                            // A single-line TextField silently drops newlines, so a
                            // multi-line paste has to be intercepted (SRL-F7).
                            if (event.key === Qt.Key_V && (event.modifiers & Qt.ControlModifier)) {
                                if (root._pasteAt(termRow.index, cursorPosition))
                                    event.accepted = true
                                return
                            }
                            if (event.key === Qt.Key_Backspace
                                    && cursorPosition === 0 && text.length === 0
                                    && termRow.index > 0) {
                                root._removeRow(termRow.index)
                                event.accepted = true
                                return
                            }
                            if (event.key === Qt.Key_Down) {
                                root._focusRow(termRow.index + 1)
                                event.accepted = true
                                return
                            }
                            if (event.key === Qt.Key_Up) {
                                root._focusRow(termRow.index - 1)
                                event.accepted = true
                                return
                            }
                        }

                        // Per-row inline clear, matching the affordance the exclude
                        // field has today.
                        Kirigami.Icon {
                            anchors { right: parent.right; rightMargin: Kirigami.Units.smallSpacing * 2; verticalCenter: parent.verticalCenter }
                            source: parent.LayoutMirroring.enabled ? "edit-clear-locationbar-ltr" : "edit-clear-locationbar-rtl"
                            implicitWidth: Kirigami.Units.iconSizes.small
                            implicitHeight: Kirigami.Units.iconSizes.small
                            visible: root.showInlineClear && parent.text.length > 0
                            opacity: rowClearTap.pressed ? 0.5 : 1.0
                            Behavior on opacity { NumberAnimation { duration: Kirigami.Units.shortDuration } }
                            HoverHandler { cursorShape: Qt.ArrowCursor }
                            TapHandler {
                                id: rowClearTap
                                onTapped: {
                                    termField.clear()
                                    termsModel.setProperty(termRow.index, "term", "")
                                    root._commit()
                                }
                            }
                        }
                    }

                    IconButton {
                        icon.name: "list-remove"
                        flat: true
                        // Hidden on a single-row list, so the last row can never be
                        // removed and a one-term search looks unchanged.
                        visible: termsModel.count > 1
                        onClicked: root._removeRow(termRow.index)
                    }
                }
            }
        }
    }

    IconButton {
        icon.name: "list-add"
        flat: true
        Layout.alignment: Qt.AlignLeft
        onClicked: root._addRowAfter(termsModel.count - 1)
    }
}
