/*
 *   Copyright 2006-2007 Aaron Seigo <aseigo@kde.org>
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

#include <QtCore/QUrl>
#include <QtGui/QGraphicsSceneContextMenuEvent>
#include <QtGui/QGraphicsSceneDragDropEvent>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QGraphicsSceneWheelEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtWebKit/QWebFrame>

#include <KDE/KDebug>

#include "webcontent.h"
#include "webpage.h"

namespace Plasma
{

class WebContent::Private
{
public:
    WebPage *page;
    bool loaded;
};

WebContent::WebContent(QGraphicsItem *parent , QObject *parentObject)
    : Widget(parent, parentObject),
      d(new Private)
{
    d->loaded = false;
    d->page = new WebPage(this);
    connect(d->page, SIGNAL(loadProgress(int)), this, SIGNAL(loadProgress(int)));
    connect(d->page->mainFrame(), SIGNAL(loadDone(bool)), this, SLOT(loadingComplete(bool)));
}

WebContent::~WebContent()
{
}

void WebContent::setUrl(const QUrl &url)
{
    d->loaded = false;
    d->page->mainFrame()->load(url);
}

void WebContent::setHtml(const QByteArray &html, const QUrl &baseUrl)
{
    d->loaded = false;
    d->page->mainFrame()->setHtml(html, baseUrl);
}

QSizeF WebContent::sizeHint() const
{
    if (d->loaded) {
        return d->page->mainFrame()->contentsSize();
    }

    return Widget::sizeHint();
}

QWebPage* WebContent::page() const
{
    return d->page;
}

QWebFrame* WebContent::mainFrame() const
{
    return d->page->mainFrame();
}

void WebContent::paintWidget(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    if (d->loaded) {
        //kDebug() << "painting page";
        d->page->mainFrame()->render(painter, option->rect);
    }
}

void WebContent::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QMouseEvent me(QEvent::MouseMove, event->pos().toPoint(), event->button(),
                   event->buttons(), event->modifiers());
    d->page->event(&me);
    if (me.isAccepted()) {
        event->accept();
    }
}

void WebContent::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QMouseEvent me(QEvent::MouseButtonPress, event->pos().toPoint(), event->button(),
                   event->buttons(), event->modifiers());
    d->page->event(&me);
    if (me.isAccepted()) {
        event->accept();
    }
}

void WebContent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QMouseEvent me(QEvent::MouseButtonDblClick, event->pos().toPoint(), event->button(),
                   event->buttons(), event->modifiers());
    d->page->event(&me);
    if (me.isAccepted()) {
        event->accept();
    }
}

void WebContent::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QMouseEvent me(QEvent::MouseButtonRelease, event->pos().toPoint(), event->button(),
                   event->buttons(), event->modifiers());
    d->page->event(&me);
    if (me.isAccepted()) {
        event->accept();
    }
}

void WebContent::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QContextMenuEvent ce(static_cast<QContextMenuEvent::Reason>(event->reason()),
                         event->pos().toPoint(), event->screenPos());
    d->page->event(&ce);
    if (ce.isAccepted()) {
        event->accept();
    }
}

void WebContent::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    QWheelEvent we(event->pos().toPoint(), event->delta(), event->buttons(),
                   event->modifiers(), event->orientation());

    d->page->event(&we);

    if (we.isAccepted()) {
        event->accept();
    } else {
        Widget::wheelEvent(event);
    }
}

void WebContent::keyPressEvent(QKeyEvent * event)
{
    d->page->event(event);

    if (!event->isAccepted()) {
        Widget::keyPressEvent(event);
    }
}

void WebContent::keyReleaseEvent(QKeyEvent * event)
{
    d->page->event(event);

    if (!event->isAccepted()) {
        Widget::keyPressEvent(event);
    }
}

void WebContent::focusInEvent(QFocusEvent * event)
{
    d->page->event(event);
    Widget::focusInEvent(event);
}

void WebContent::focusOutEvent(QFocusEvent * event)
{
    d->page->event(event);
    Widget::focusOutEvent(event);
}

void WebContent::dragEnterEvent(QGraphicsSceneDragDropEvent * event)
{
    QDragEnterEvent de(event->pos().toPoint(), event->possibleActions(), event->mimeData(),
                       event->buttons(), event->modifiers());
    d->page->event(&de);

    if (de.isAccepted()) {
        event->accept();
    }
}

void WebContent::dragLeaveEvent(QGraphicsSceneDragDropEvent * event)
{
    QDragLeaveEvent de;
    d->page->event(&de);

    if (de.isAccepted()) {
        event->accept();
    }
}

void WebContent::dragMoveEvent(QGraphicsSceneDragDropEvent * event)
{
    // Ok, so the docs say "don't make a QDragMoveEvent yourself" but we're just
    // replicating it here, not really creating a new one. hopefully we get away with it ;)
    QDragMoveEvent de(event->pos().toPoint(), event->possibleActions(), event->mimeData(),
                      event->buttons(), event->modifiers());
    d->page->event(&de);

    if (de.isAccepted()) {
        event->accept();
    }
}

void WebContent::dropEvent(QGraphicsSceneDragDropEvent * event)
{
    QDragMoveEvent de(event->pos().toPoint(), event->possibleActions(), event->mimeData(),
                      event->buttons(), event->modifiers());
    d->page->event(&de);

    if (de.isAccepted()) {
        event->accept();
    }
}

void WebContent::setGeometry(const QRectF &geometry)
{
    Widget::setGeometry(geometry);
    d->page->setViewportSize(geometry.size().toSize());
}

void WebContent::loadingComplete(bool success)
{
    d->loaded = success;
    emit loadDone(success);
}

} // namespace Plasma

#include "webcontent.moc"

