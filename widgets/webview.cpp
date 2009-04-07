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

#include <QtGui/QGraphicsSceneContextMenuEvent>
#include <QtGui/QGraphicsSceneDragDropEvent>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QGraphicsSceneWheelEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QStyleOptionGraphicsItem>
#include <fixx11h.h>
#include <QtWebKit/QWebFrame>
#include <QtWebKit/QWebPage>
#include <QtCore/QTimer>

#include <kdebug.h>

#include "plasma/widgets/webview.h"

namespace Plasma
{

class WebViewPrivate
{
public:
    WebViewPrivate(WebView *parent)
        : q(parent),
          dragToScroll(false),
          dragging(false),
          dragTimeout(false),
          dragTimeoutTimer(0)
    {
    }

    void loadingFinished(bool success);
    void updateRequested(const QRect &dirtyRect);
    void scrollRequested(int dx, int dy, const QRect &scrollRect);
    void dragTimeoutExpired();

    WebView *q;
    QWebPage *page;
    bool loaded;
    bool dragToScroll;
    bool dragging;
    bool dragTimeout;
    QTimer *dragTimeoutTimer;
};

WebView::WebView(QGraphicsItem *parent)
    : QGraphicsWidget(parent),
      d(new WebViewPrivate(this))
{
    d->page = 0;
    d->loaded = false;
    setPage(new QWebPage(this));
    setAcceptsHoverEvents(true);
    setFlags(QGraphicsItem::ItemIsFocusable);
}

WebView::~WebView()
{
   delete d;
}

void WebView::setUrl(const KUrl &url)
{
    d->loaded = false;
    if (d->page) {
        d->page->mainFrame()->load(url);
    }
}

KUrl WebView::url() const
{
    return d->page ? d->page->mainFrame()->url() : KUrl();
}

void WebView::setHtml(const QByteArray &html, const KUrl &baseUrl)
{
    d->loaded = false;
    if (d->page) {
        d->page->mainFrame()->setContent(html, QString(), baseUrl);
    }
}

void WebView::setHtml(const QString &html, const KUrl &baseUrl)
{
    d->loaded = false;
    if (d->page) {
        d->page->mainFrame()->setHtml(html, baseUrl);
    }
}

QString WebView::html() const
{
    return d->page ? d->page->mainFrame()->toHtml() : QByteArray();
}

QRectF WebView::geometry() const
{
    if (d->loaded && d->page) {
        return d->page->mainFrame()->geometry();
    }

    return QGraphicsWidget::geometry();
}

void WebView::setPage(QWebPage *page)
{
    if (page == d->page) {
        return;
    }

    if (d->page) {
        if (d->page->parent() == this) {
            delete d->page;
        } else {
            disconnect(d->page, 0, this, 0);
        }
    }

    d->page = page;

    if (d->page) {
        connect(d->page, SIGNAL(loadProgress(int)),
                this, SIGNAL(loadProgress(int)));
        connect(d->page, SIGNAL(loadFinished(bool)),
                this, SLOT(loadingFinished(bool)));
        connect(d->page, SIGNAL(repaintRequested(const QRect&)),
                this, SLOT(updateRequested(const QRect&)));
        connect(d->page, SIGNAL(scrollRequested(int, int, const QRect &)),
                this, SLOT(scrollRequested(int, int, const QRect &)));
    }
}

QWebPage *WebView::page() const
{
    return d->page;
}

QWebFrame *WebView::mainFrame() const
{
    return d->page ? d->page->mainFrame() : 0;
}

void WebView::setDragToScroll(bool drag)
{
    d->dragToScroll = drag;
}

bool WebView::dragToScroll()
{
    return d->dragToScroll;
}

void WebView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)

    if (d->loaded && d->page) {
        //kDebug() << "painting page";
        d->page->mainFrame()->render(painter, option->rect);
    }
}

void WebView::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!d->page) {
        QGraphicsWidget::mouseMoveEvent(event);
        return;
    }

    if (d->dragToScroll && d->dragTimeout) {
        QPointF deltaPos = event->pos() - event->lastPos();
        d->page->mainFrame()->setScrollBarValue(Qt::Horizontal, d->page->mainFrame()->scrollBarValue(Qt::Horizontal) - deltaPos.x());
        d->page->mainFrame()->setScrollBarValue(Qt::Vertical, d->page->mainFrame()->scrollBarValue(Qt::Vertical) - deltaPos.y());
        d->dragging = true;
    } else {
        if (d->dragTimeoutTimer) {
            d->dragTimeoutTimer->stop();
        }
        QMouseEvent me(QEvent::MouseMove, event->pos().toPoint(), event->button(),
                      event->buttons(), event->modifiers());
        d->page->event(&me);
        if (me.isAccepted()) {
            event->accept();
        }
    }
}

void WebView::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!d->page) {
        QGraphicsWidget::hoverMoveEvent(event);
        return;
    }

    QMouseEvent me(QEvent::MouseMove, event->pos().toPoint(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    d->page->event(&me);
    if (me.isAccepted()) {
        event->accept();
    }
}

void WebView::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!d->page) {
        QGraphicsWidget::mousePressEvent(event);
        return;
    }

    d->dragTimeout = false;
    if (!d->dragTimeoutTimer) {
        d->dragTimeoutTimer = new QTimer(this);
        d->dragTimeoutTimer->setSingleShot(true);
        connect(d->dragTimeoutTimer, SIGNAL(timeout()), this, SLOT(dragTimeoutExpired()));
    }
    d->dragTimeoutTimer->start(250);

    setFocus();

    QMouseEvent me(QEvent::MouseButtonPress, event->pos().toPoint(), event->button(),
                   event->buttons(), event->modifiers());
    d->page->event(&me);
    if (me.isAccepted()) {
        event->accept();
    }
}

void WebView::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
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

void WebView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!d->page || d->dragging) {
        d->dragging = false;
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

void WebView::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (!d->page) {
        QGraphicsWidget::contextMenuEvent(event);
        return;
    }

    QContextMenuEvent ce(static_cast<QContextMenuEvent::Reason>(event->reason()),
                         event->pos().toPoint(), event->screenPos());

    if (d->page->swallowContextMenuEvent(&ce)) {
        event->accept();
    } else {
        d->page->updatePositionDependentActions(event->pos().toPoint());

        d->page->event(&ce);
        if (ce.isAccepted()) {
            event->accept();
        }
    }
}

void WebView::wheelEvent(QGraphicsSceneWheelEvent *event)
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

void WebView::keyPressEvent(QKeyEvent * event)
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

void WebView::keyReleaseEvent(QKeyEvent * event)
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

void WebView::focusInEvent(QFocusEvent * event)
{
    if (d->page) {
        d->page->event(event);
    }

    QGraphicsWidget::focusInEvent(event);
}

void WebView::focusOutEvent(QFocusEvent * event)
{
    if (d->page) {
        d->page->event(event);
    }

    QGraphicsWidget::focusOutEvent(event);
}

void WebView::dragEnterEvent(QGraphicsSceneDragDropEvent * event)
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

void WebView::dragLeaveEvent(QGraphicsSceneDragDropEvent * event)
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

void WebView::dragMoveEvent(QGraphicsSceneDragDropEvent * event)
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

void WebView::dropEvent(QGraphicsSceneDragDropEvent * event)
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

void WebView::setGeometry(const QRectF &geometry)
{
    QGraphicsWidget::setGeometry(geometry);
    d->page->setViewportSize(geometry.size().toSize());
}

void WebViewPrivate::loadingFinished(bool success)
{
    loaded = success;
    emit q->loadFinished(success);
    q->update();
}

void WebViewPrivate::updateRequested(const QRect &dirtyRect)
{
    if (loaded && page) {
       q->update(QRectF(dirtyRect.topLeft().x(), dirtyRect.topLeft().y(),
                        dirtyRect.width(), dirtyRect.height()));
    }
}

void WebViewPrivate::scrollRequested(int dx, int dy, const QRect &scrollRect)
{
    Q_UNUSED(dx)
    Q_UNUSED(dy)
    updateRequested(scrollRect);
}

void WebViewPrivate::dragTimeoutExpired()
{
    dragTimeout = true;
}

} // namespace Plasma

#include "webview.moc"

