/*
 *   Copyright 2006-2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2010 Davide Bettio <davide.bettio@kdemail.net>
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

#include <QtGui/QApplication>
#include <QtGui/QStyleOptionGraphicsItem>

#include <fixx11h.h>
#include <QtWebKit/QWebFrame>
#include <QtWebKit/QWebPage>

#include <QtCore/QTimer>

#include <kdebug.h>

#include "animator.h"
#include "plasma.h"
#include "widgets/webview.h"
#include "widgets/scrollwidget.h"
#include "private/animablegraphicswebview_p.h"

namespace Plasma
{

class WebViewPrivate
{
public:
    WebViewPrivate(WebView *parent)
        : q(parent)
    {
    }

    void loadingFinished(bool success);
    void dragTimeoutExpired();

    WebView *q;
    AnimableGraphicsWebView *webView;
    ScrollWidget *scrollWidget;
    bool loaded;
};

WebView::WebView(QGraphicsItem *parent)

    : QGraphicsWidget(parent),
      d(new WebViewPrivate(this))
{
    d->loaded = false;
    setAcceptTouchEvents(true);
    setAcceptsHoverEvents(true);
    setFlags(QGraphicsItem::ItemIsFocusable);

    d->scrollWidget = new Plasma::ScrollWidget(this);
    d->webView = new AnimableGraphicsWebView(d->scrollWidget);
    d->scrollWidget->setWidget(d->webView);
    d->scrollWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->scrollWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragToScroll(false);
    QPalette palette = qApp->palette();
    palette.setBrush(QPalette::Base, Qt::transparent);
    d->webView->page()->setPalette(palette);

    connect(d->webView, SIGNAL(loadProgress(int)),
            this, SIGNAL(loadProgress(int)));
    connect(d->webView, SIGNAL(loadFinished(bool)),
            this, SLOT(loadingFinished(bool)));
}

WebView::~WebView()
{
   delete d;
}

void WebView::setUrl(const KUrl &url)
{
    d->loaded = false;
    d->webView->load(url);
}

KUrl WebView::url() const
{
    return d->webView->url();
}

void WebView::setHtml(const QByteArray &html, const KUrl &baseUrl)
{
    d->loaded = false;
    d->webView->setContent(html, QString(), baseUrl);
}

void WebView::setHtml(const QString &html, const KUrl &baseUrl)
{
    d->loaded = false;
    d->webView->setHtml(html, baseUrl);
}

QString WebView::html() const
{
    return d->webView->page()->mainFrame()->toHtml();
}

QRectF WebView::geometry() const
{
    if (d->loaded) {
        return QRectF(pos(), d->webView->page()->mainFrame()->geometry().size());
    } else {
        return QGraphicsWidget::geometry();
    }
}

QSizeF WebView::contentsSize() const
{
    return d->webView->page()->mainFrame()->contentsSize();
}

void WebView::setScrollPosition(const QPointF &position)
{
    d->webView->setScrollPosition(position);
}

QPointF WebView::scrollPosition() const
{
    return d->webView->scrollPosition();
}

QRectF WebView::viewportGeometry() const
{
    return d->webView->page()->mainFrame()->geometry();
}

qreal WebView::zoomFactor() const
{
    return d->webView->zoomFactor();
}

void WebView::setZoomFactor(const qreal zoom)
{
    d->webView->setZoomFactor(zoom);
}

void WebView::setPage(QWebPage *page)
{
    d->webView->setPage(page);
}

QWebPage *WebView::page() const
{
    return d->webView->page();
}

QWebFrame *WebView::mainFrame() const
{
    return d->webView->page()->mainFrame();
}

void WebView::setDragToScroll(bool drag)
{
    d->webView->setDragToScroll(drag);
    d->scrollWidget->setFiltersChildEvents(drag);
}

bool WebView::dragToScroll()
{
    return d->webView->dragToScroll();
}

void WebView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsWidget::paint(painter, option, widget);
}

void WebView::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsWidget::mouseMoveEvent(event);
}

void WebView::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsWidget::hoverMoveEvent(event);
}

void WebView::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsWidget::mousePressEvent(event);
}

void WebView::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsWidget::mouseDoubleClickEvent(event);
}

void WebView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsWidget::mouseReleaseEvent(event);
}

void WebView::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QGraphicsWidget::contextMenuEvent(event);
}

void WebView::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    QGraphicsWidget::wheelEvent(event);
}

void WebView::keyPressEvent(QKeyEvent * event)
{
    QGraphicsWidget::keyPressEvent(event);
}

void WebView::keyReleaseEvent(QKeyEvent * event)
{
    QGraphicsWidget::keyReleaseEvent(event);
}

void WebView::focusInEvent(QFocusEvent * event)
{
    QGraphicsWidget::focusInEvent(event);
}

void WebView::focusOutEvent(QFocusEvent * event)
{
    QGraphicsWidget::focusOutEvent(event);
}

void WebView::dragEnterEvent(QGraphicsSceneDragDropEvent * event)
{
    QGraphicsWidget::dragEnterEvent(event);
}

void WebView::dragLeaveEvent(QGraphicsSceneDragDropEvent * event)
{
    QGraphicsWidget::dragLeaveEvent(event);
}

void WebView::dragMoveEvent(QGraphicsSceneDragDropEvent * event)
{
    QGraphicsWidget::dragMoveEvent(event);
}

void WebView::dropEvent(QGraphicsSceneDragDropEvent * event)
{
    QGraphicsWidget::dropEvent(event);
}

QVariant WebView::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSceneHasChanged) {
        //FIXME: QWebPage _requires_ a QWidget view to not crash in places such as 
        // WebCore::PopupMenu::show() due to hostWindow()->platformPageClient() == NULL
        // because QWebPage::d->client is NULL
        //d->webView->page()->setView(viewFor(this));
    }
    return QGraphicsWidget::itemChange(change, value);
}

void WebView::setGeometry(const QRectF &geometry)
{
    QGraphicsWidget::setGeometry(geometry);
    d->scrollWidget->setGeometry(QRectF(0, 0, geometry.width(), geometry.height()));
    d->webView->setGeometry(d->scrollWidget->viewportGeometry());
}

QSizeF WebView::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    if (which == Qt::PreferredSize) {
        return d->webView->page()->mainFrame()->contentsSize();
    } else {
        return QGraphicsWidget::sizeHint(which, constraint);
    }
}

void WebViewPrivate::loadingFinished(bool success)
{
    loaded = success;
    q->updateGeometry();
    emit q->loadFinished(success);
    q->update();
}

} // namespace Plasma

#include "webview.moc"

