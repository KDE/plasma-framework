/*
    SPDX-FileCopyrightText: 2017 Fabian Vogt <fabian@ritter-vogt.de>
    SPDX-FileCopyrightText: 2017 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

.pragma library

/* Like QString::toHtmlEscaped */
function toHtmlEscaped(s) {
    return s.replace(/[&<>]/g, function (tag) {
        return {
            '&': '&amp;',
            '<': '&lt;',
            '>': '&gt;'
        }[tag] || tag
    });
}

function underlineAmpersands(match, p) {
    if(p == "&amp;")
        return p;

    return "<u>" + p + "</u>";
}

/* This function is a replacement for the flawed
 * QtQuickControlsPrivate.StyleHelpers.stylizeMnemonics.
 * It scans the passed text for mnemonics, to put them into HTML <u></u>
 * tags. This means it emits HTML, but accepts only plaintext.
 * Simply passing HTML escaped plaintext won't work, as it would then
 * replace &lt; with <u>l</u>t; so we need to implement it ourselves. */
function stylizeEscapedMnemonics(text) {
	return text.replace(/&amp;(&amp;|.)/g, underlineAmpersands);
}
