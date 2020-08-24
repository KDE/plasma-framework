/*
    SPDX-FileCopyrightText: 2011 Nokia Corporation and /or its subsidiary(-ies) <qt-info@nokia.com>

    This file is part of the Qt Components project.

    SPDX-License-Identifier: BSD-3-Clause
*/

.pragma library

function rootObject(item) {
    var next = item.parent
    while (next && next.parent)
        next = next.parent
    return next
}

function findParent(child, propertyName) {
    if (!child)
        return null
    var next = child.parent
    while (next && !next.hasOwnProperty(propertyName))
        next = next.parent
    return next
}
