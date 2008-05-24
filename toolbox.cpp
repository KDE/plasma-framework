
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

#include <QAction>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QRadialGradient>

#include <KColorScheme>
#include <KDebug>

#include <plasma/theme.h>
#include "widgets/icon.h"

namespace Plasma
{

class Toolbox::Private
{
public:
    Private()
      : size(50),
      iconSize(32, 32),
      hidden(false),
      showing(false),
      orientation(Qt::Horizontal)
    {}

    int size;
    QSize iconSize;
    bool hidden;
    bool showing;
    Qt::Orientation orientation;
};

Toolbox::Toolbox(QGraphicsItem *parent)
    : QGraphicsItem(parent),
      d(new Private)
{
    setAcceptsHoverEvents(true);
}

Toolbox::~Toolbox()
{
    delete d;
}

void Toolbox::addTool(QAction *action)
{
    if (!action) {
        return;
    }

    Plasma::Icon *tool = new Plasma::Icon(this);

    tool->setAction(action);
    tool->setDrawBackground(true);
    tool->setOrientation(Qt::Horizontal);
    QSizeF iconSize = tool->sizeFromIconSize(22);
    tool->setMinimumSize(iconSize);
    tool->setMaximumSize(iconSize);
    tool->resize(tool->size());

    tool->hide();
    const int height = static_cast<int>(tool->boundingRect().height());
    tool->setPos(QPoint( d->size*2,-height));
    tool->setZValue(zValue() + 1);

    //make enabled/disabled tools appear/disappear instantly
    connect(tool, SIGNAL(changed()), this, SLOT(updateToolbox()));
}

void Toolbox::updateToolbox()
{
    if ( d->showing) {
        d->showing = false;
        showToolbox();
    }
}

void Toolbox::removeTool(QAction *action)
{
    foreach (QGraphicsItem *child, QGraphicsItem::children()) {
        //kDebug() << "checking tool" << child << child->data(ToolName);
        Plasma::Icon *tool = dynamic_cast<Plasma::Icon*>(child);
        if (tool && tool->action() == action) {
            //kDebug() << "tool found!";
            tool->deleteLater();
            break;
        }
    }
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

Qt::Orientation Toolbox::orientation() const
{
    return d->orientation;
}

void Toolbox::setOrientation( Qt::Orientation orient )
{
     d->orientation = orient;
}

void Toolbox::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
}

void Toolbox::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (boundingRect().contains(event->pos())) {
        emit toggled();
    }
}

} // plasma namespace

#include "toolbox_p.moc"

