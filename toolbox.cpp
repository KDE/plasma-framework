
/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Marco Martin <notmart@gmail.com>
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

#include "toolbox_p.h"

#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QRadialGradient>

#include <plasma/theme.h>
#include <KColorScheme>

#include <KDebug>

namespace Plasma
{

// used with QGrahphicsItem::setData
static const int ToolName = 7001;

class Toolbox::Private
{
public:
    Private()
      : size(50),
      iconSize(32, 32),
      hidden(false),
      showing(false)
    {}

    int size;
    QSize iconSize;
    bool hidden;
    bool showing;
};

Toolbox::Toolbox(QGraphicsItem *parent)
    : QGraphicsItem(parent),
      d(new Private)
{
    setAcceptsHoverEvents(true);
    setZValue(10000000);
    setFlag(ItemClipsToShape, true);
    setFlag(ItemClipsChildrenToShape, false);
    setFlag(ItemIgnoresTransformations, true);
}

QRectF Toolbox::boundingRect() const
{
    return QRectF(0, 0, -size()*2, size()*2);
}

void Toolbox::addTool(QGraphicsItem *tool, const QString &name)
{
    if (!tool) {
        return;
    }

    tool->hide();
    const int height = static_cast<int>(tool->boundingRect().height());
    tool->setPos(QPoint( d->size*2,-height));
    tool->setZValue(zValue() + 1);
    tool->setParentItem(this);
    tool->setData(ToolName, name);
}

void Toolbox::enableTool(const QString &toolName, bool visible)
{
    //kDebug() << (visible? "enabling" : "disabling") << "tool" << toolName;
    QGraphicsItem *t = tool(toolName);

    if (t && t->isEnabled() != visible) {
        t->setEnabled(visible);
        // adjust the visible items
        if ( d->showing) {
             d->showing = false;
            showToolbox();
        }
    }
}

bool Toolbox::isToolEnabled(const QString &toolName) const
{
    QGraphicsItem *t = tool(toolName);

    if (t) {
        return t->isEnabled();
    }

    return false;
}

QGraphicsItem* Toolbox::tool(const QString &toolName) const
{
    foreach (QGraphicsItem *child, QGraphicsItem::children()) {
        //kDebug() << "checking tool" << child << child->data(ToolName);
        if (child->data(ToolName).toString() == toolName) {
            //kDebug() << "tool found!";
            return child;
        }
    }

    return 0;
}

int Toolbox::size() const
{
    return  d->size;
}

void Toolbox::setSize(const int newSize)
{
    d->size = newSize;
}

QSize Toolbox::iconSize() const
{
    return d->iconSize;
}

void Toolbox::setIconSize(const QSize newSize)
{
    d->iconSize = newSize;
}

bool Toolbox::showing() const
{
    return  d->showing;
}

void Toolbox::setShowing(const bool show)
{
     d->showing = show;
}

} // plasma namespace

#include "toolbox_p.moc"

