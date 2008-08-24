/*
 *   Copyright © 2008 Fredrik Höglund <fredrik@kde.org>
 *   Copyright © 2008 Marco Martin <notmart@gmail.com> 
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

#ifndef PLASMA_SCROLLBAR_H
#define PLASMA_SCROLLBAR_H

#include <QtGui/QScrollBar>
#include <QtGui/QGraphicsProxyWidget>

#include <plasma/plasma_export.h>

namespace Plasma
{

// Wraps a QScrollBar in a QGraphicsProxyWidget
class PLASMA_EXPORT ScrollBar : public QGraphicsProxyWidget
{
public:
    explicit ScrollBar(Qt::Orientation orientation, QGraphicsWidget *parent);
    ~ScrollBar();

    void setRange(int min, int max);
    void setSingleStep(int val);
    void setPageStep(int val);
    void setValue(int val);
    int value() const;
    int minimum() const;
    int maximum() const;
    void setStyleSheet(const QString &stylesheet);
    QString styleSheet();
    QScrollBar *nativeWidget() const;
};

}

#endif
