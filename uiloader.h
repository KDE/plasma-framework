/*
 *   Copyright 2007 Richard J. Moore <rich@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PLASMA_UILOADER_H
#define PLASMA_UILOADER_H

#include <QtCore/QObject>

#include <plasma/plasma_export.h>

class QGraphicsItem;

namespace Plasma
{

class Widget;
class Layout;
class LayoutItem;

/**
 * Dynamically create plasma Widgets and Layouts.
 *
 * @author Richard J. Moore, <rich@kde.org>
 */
class PLASMA_EXPORT UiLoader : public QObject
{
    Q_OBJECT

public:
    UiLoader(QObject *parent = 0);
    virtual ~UiLoader();

    QStringList availableWidgets() const;
    Widget *createWidget(const QString &className, Widget *parent = 0);

    QStringList availableLayouts() const;
    Layout *createLayout(const QString &className, LayoutItem *parent);

private:
    class Private;
    Private *const d;
};

}

#endif // PLASMA_UILOADER_H
