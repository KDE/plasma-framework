/*
    Copyright (C) 2010 by BetterInbox <contact@betterinbox.com>
    Original author: Gregory Schlomoff <greg@betterinbox.com>

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#include "DeclarativeDropArea.h"
#include "DeclarativeDragDropEvent.h"

#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>

#include <QDebug>

DeclarativeDropArea::DeclarativeDropArea(QQuickItem *parent)
    : QQuickItem(parent),
    m_enabled(true)
{
    setFlag(ItemAcceptsDrops, m_enabled);
    setFlag(ItemHasContents, m_enabled);
    setAcceptHoverEvents(m_enabled);

}

void DeclarativeDropArea::dragEnterEvent(QDragEnterEvent *event)
{
    DeclarativeDragDropEvent dde(event, this);
    qDebug() << "enter.";
    event->accept();
    emit dragEnter(&dde);
}

void DeclarativeDropArea::dragLeaveEvent(QDragLeaveEvent *event)
{
    DeclarativeDragDropEvent dde(event, this);
    qDebug() << "leave.";
    emit dragLeave(&dde);
}

void DeclarativeDropArea::dragMoveEvent(QDragMoveEvent *event)
{
    DeclarativeDragDropEvent dde(event, this);
    qDebug() << "move.";
    event->accept();
    emit dragMove(&dde);
}

void DeclarativeDropArea::dropEvent(QDropEvent *event)
{
    DeclarativeDragDropEvent dde(event, this);
    qDebug() << "Drop.";
    emit drop(&dde);
}

bool DeclarativeDropArea::isEnabled() const
{
    return m_enabled;
}

void DeclarativeDropArea::setEnabled(bool enabled)
{
    if (enabled == m_enabled) {
        return;
    }

    m_enabled = enabled;
    setAcceptHoverEvents(m_enabled);
    setFlag(ItemAcceptsDrops, m_enabled);
    emit enabledChanged();
}
