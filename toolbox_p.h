/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.

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

#ifndef PLASMA_TOOLBOX_P_H
#define PLASMA_TOOLBOX_P_H

#include <QGraphicsItem>
#include <QObject>

#include "animator.h"

class QAction;

namespace Plasma
{

//class Widget;
//class EmptyGraphicsItem;

class Toolbox : public QObject, public QGraphicsItem
{
    Q_OBJECT

public:
    explicit Toolbox(QGraphicsItem *parent = 0);
    ~Toolbox();

    /**
     * create a toolbox tool from the given action
     * @p action the action to associate hte tool with
     */
    void addTool(QAction *action);
    /**
     * remove the tool associated with this action
     */
    void removeTool(QAction *action);
    int size() const;
    void setSize(const int newSize);
    QSize iconSize() const;
    void  setIconSize(const QSize newSize);
    bool showing() const;
    void setShowing(const bool show);
    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation orient);

    virtual void showToolbox() = 0;
    virtual void hideToolbox() = 0;
public Q_SLOTS:
    /**
     * re-show the toolbox, in case any tools have changed
     */
    void updateToolbox();
Q_SIGNALS:
    void toggled();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    class Private;
    Private *d;
};

} // Plasma namespace
#endif // multiple inclusion guard

