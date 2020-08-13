/*
    SPDX-FileCopyrightText: 2011 Nokia Corporation and /or its subsidiary(-ies) <qt-info@nokia.com>

    This file is part of the Qt Components project.

    SPDX-License-Identifier: BSD-3-Clause
*/

var _sectionData = [];
var _sections = [];

function initSectionData(list) {
    if (!list || !list.model) return;
    _sectionData = [];
    _sections = [];
    var current = "";
    var prop = list.section.property;

    for (var i = 0, count = list.model.count; i < count; i++) {
        var item = list.model.get(i);
        if (item[prop] !== current) {
            current = item[prop];
            _sections.push(current);
            _sectionData.push({ index: i, header: current });
        }
    }
}

function closestSection(pos) {
    var tmp = (_sections.length) * pos;
    var val = Math.ceil(tmp) // TODO: better algorithm
    val = val < 2 ? 1 : val;
    return _sections[val-1];
}

function indexOf(sectionName) {
    var val = _sectionData[_sections.indexOf(sectionName)].index;
    return val === 0 || val > 0 ? val : -1;
}
