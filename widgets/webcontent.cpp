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
#include <QtWebKit/QWebPage>

#include <KDE/KDebug>

#include "plasma/widgets/webcontent.h"

namespace Plasma
{

class WebContentPrivate
{
public:
    WebContentPrivate(WebContent *parent)
        : q(parent)
    {
    }

    void loadingFinished(bool success);

    WebContent *q;
    QWebPage *page;
    bool loaded;
};

WebContent::WebContent(QGraphicsItem *parent)
    : QGraphicsWidget(parent),
      d(new WebContentPrivate(this))
{
    d->page = 0;
    d->loaded = false;
    setPage(new QWebPage(this));
}

WebContent::~WebContent()
{
   delete d;
}

void WebContent::setUrl(const QUrl &url)
{
    d->loaded = false;
    if (d->page) {
        d->page->mainFrame()->load(url);
    }
}

void WebContent::setHtml(const QByteArray &html, const QUrl &baseUrl)
{
    d->loaded = false;
    if (d->page) {
        d->page->mainFrame()->setContent(html, QString(), baseUrl);
    }
}

void WebContent::setHtml(const QString &html, const QUrl &baseUrl)
{
    d->loaded = false;
    if (d->page) {
        d->page->mainFrame()->setHtml(html, baseUrl);
    }
}

QRectF WebContent::geometry() const
{
    if (d->loaded && d->page) {
        return d->page->mainFrame()->geometry();
    }

    return QGraphicsWidget::geometry();
}

void WebContent::setPage(QWebPage *page)
{
    if (page == d->page) {
        return;
    }

    if (d->page && d->page->parent() == this) {
        delete d->page;
    }

    d->page = page;

    if (d->page) {
        connect(d->page, SIGNAL(loadProgress(int)), this, SIGNAL(loadProgress(int)));
        connect(d->page, SIGNAL(loadFinished(bool)), this, SLOT(loadingFinished(bool)));
    }
}

QWebPage* WebContent::page() const
{
    return d->page;
}

QWebFrame* WebContent::mainFrame() const
{
    return d->page ? d->page->mainFrame() : 0;
}

void WebContent::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    if (d->loaded && d->page) {
        //kDebug() << "painting page";
        d->page->mainFrame()->render(painter, option->rect);
    }
}

void WebContent::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!d->page) {
        QGraphicsWidget::mouseMoveEvent(event);
        return;
    }

    QMouseEvent me(QEvent::MouseMove, event->pos().toPoint(), event->button(),
                   event->buttons(), event->modifiers());
    d->page->event(&me);
    if (me.isAccepted()) {
        event->accept();
    }
}

void WebContent::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!d->page) {
        QGraphicsWidget::mousePressEvent(event);
        return;
    }

    QMouseEvent me(QEvent::MouseButtonPress, event->pos().toPoint(), event->button(),
                   event->buttons(), event->modifiers());
    d->page->event(&me);
    if (me.isAccepted()) {
        event->accept();
    }
}

void WebContent::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (!d->page) {
        QGraphicsWidget::mouseDoubleClickEvent(event);
        return;
    }

    QMouseEvent me(QEvent::MouseButtonDblClick, event->pos().toPoint(), event->button(),
                   event->buttons(), event->modifiers());
    d->page->event(&me);
    if (me.isAccepted()) {
        event->accept();
    }
}

void WebContent::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!d->page) {
        QGraphicsWidget::mouseReleaseEvent(event);
        return;
    }

    QMouseEvent me(QEvent::MouseButtonRelease, event->pos().toPoint(), event->button(),
                   event->buttons(), event->modifiers());
    d->page->event(&me);
    if (me.isAccepted()) {
        event->accept();
    }
}

void WebContent::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (!d->page) {
        QGraphicsWidget::contextMenuEvent(event);
        return;
    }

    QContextMenuEvent ce(static_cast<QContextMenuEvent::Reason>(event->reason()),
                         event->pos().toPoint(), event->screenPos());
    d->page->event(&ce);
    if (ce.isAccepted()) {
        event->accept();
    }
}

void WebContent::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (!d->page) {
        QGraphicsWidget::wheelEvent(event);
        return;
    }

    QWheelEvent we(event->pos().toPoint(), event->delta(), event->buttons(),
                   event->modifiers(), event->orientation());

    d->page->event(&we);

    if (we.isAccepted()) {
        event->accept();
    } else {
        QGraphicsWidget::wheelEvent(event);
    }
}

void WebContent::keyPressEvent(QKeyEvent * event)
{
    if (!d->page) {
        QGraphicsWidget::keyPressEvent(event);
        return;
    }

    d->page->event(event);

    if (!event->isAccepted()) {
        QGraphicsWidget::keyPressEvent(event);
    }
}

void WebContent::keyReleaseEvent(QKeyEvent * event)
{
    if (!d->page) {
        QGraphicsWidget::keyReleaseEvent(event);
        return;
    }

    d->page->event(event);

    if (!event->isAccepted()) {
        QGraphicsWidget::keyPressEvent(event);
    }
}

void WebContent::focusInEvent(QFocusEvent * event)
{
    if (d->page) {
        d->page->event(event);
    }

    QGraphicsWidget::focusInEvent(event);
}

void WebContent::focusOutEvent(QFocusEvent * event)
{
    if (d->page) {
        d->page->event(event);
    }

    QGraphicsWidget::focusOutEvent(event);
}

void WebContent::dragEnterEvent(QGraphicsSceneDragDropEvent * event)
{
    if (!d->page) {
        QGraphicsWidget::dragEnterEvent(event);
        return;
    }

    QDragEnterEvent de(event->pos().toPoint(), event->possibleActions(), event->mimeData(),
                       event->buttons(), event->modifiers());
    d->page->event(&de);

    if (de.isAccepted()) {
        event->accept();
    }
}

void WebContent::dragLeaveEvent(QGraphicsSceneDragDropEvent * event)
{
    if (!d->page) {
        QGraphicsWidget::dragLeaveEvent(event);
        return;
    }

    QDragLeaveEvent de;
    d->page->event(&de);

    if (de.isAccepted()) {
        event->accept();
    }
}

void WebContent::dragMoveEvent(QGraphicsSceneDragDropEvent * event)
{
    if (!d->page) {
        QGraphicsWidget::dragMoveEvent(event);
        return;
    }

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
    if (!d->page) {
        QGraphicsWidget::dropEvent(event);
        return;
    }

    QDragMoveEvent de(event->pos().toPoint(), event->possibleActions(), event->mimeData(),
                      event->buttons(), event->modifiers());
    d->page->event(&de);

    if (de.isAccepted()) {
        event->accept();
    }
}

void WebContent::setGeometry(const QRectF &geometry)
{
    QGraphicsWidget::setGeometry(geometry);
    d->page->setViewportSize(geometry.size().toSize());
}

void WebContentPrivate::loadingFinished(bool success)
{
    loaded = success;
    emit q->loadFinished(success);
}

} // namespace Plasma

#include "webcontent.moc"

