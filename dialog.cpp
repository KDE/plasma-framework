/***************************************************************************
 *   Copyright (C) 2007 by Alexis Ménard <darktears31@gmail.com>           *
 *   Copyright (C) 2007 Sebastian Kuegler <sebas@kde.org>                  *
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>		          	   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "dialog.h"

#include <QPainter>
#include <QSvgRenderer>
#include <QResizeEvent>
#include <QMouseEvent>
#ifdef Q_WS_X11
#include <QX11Info>
#endif
#include <QGraphicsView>
#include <QtGui/QGraphicsSceneEvent>

#include <KDebug>
#include <NETRootInfo>

#include <plasma/svg.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif


namespace Plasma
{


Dialog::Dialog( QWidget * parent, Qt::WindowFlags f )
    : QWidget(parent, f),
      m_cachedBackground(0)
{
    m_background = new Plasma::Svg("dialogs/background", this);

    const int topHeight = m_background->elementSize("top").height();
    const int leftWidth = m_background->elementSize("left").width();
    const int rightWidth = m_background->elementSize("right").width();
    const int bottomHeight = m_background->elementSize("bottom").height();
    setContentsMargins(leftWidth, topHeight, rightWidth, bottomHeight);

    connect(m_background, SIGNAL(repaintNeeded()), this, SLOT(update()));
}

Dialog::~Dialog()
{
}

void Dialog::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setClipRect(e->rect());
    p.setCompositionMode(QPainter::CompositionMode_Source );
    p.fillRect(rect(), Qt::transparent);
    paintBackground(&p, e->rect());
}

void Dialog::paintBackground(QPainter* painter, const QRect &exposedRect)
{
    if (!m_cachedBackground || m_cachedBackground->size() != rect().size()) {
        const int contentWidth = rect().width();
        const int contentHeight = rect().height();
        
        m_background->resize();
        
        const int topHeight = m_background->elementSize("top").height();
        const int topWidth = m_background->elementSize("top").width();
        const int leftWidth = m_background->elementSize("left").width();
        const int leftHeight = m_background->elementSize("left").height();
        const int rightHeight = m_background->elementSize("right").height();
        const int rightWidth = m_background->elementSize("right").width();
        const int bottomHeight = m_background->elementSize("bottom").height();
        const int bottomWidth = m_background->elementSize("bottom").width();

        const int topOffset = 0;
        const int leftOffset = 0;
        const int rightOffset = contentWidth - rightWidth;
        const int bottomOffset = contentHeight - bottomHeight;
        const int contentTop = topHeight;
        const int contentLeft = leftWidth;
        
        delete m_cachedBackground;
        m_cachedBackground = new QPixmap(contentWidth, contentHeight);
        m_cachedBackground->fill(Qt::transparent);
        QPainter p(m_cachedBackground);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        
        //FIXME: This is a hack to fix a drawing problems with svg files where a thin transparent border is drawn around the svg image.
        //       the transparent border around the svg seems to vary in size depending on the size of the svg and as a result increasing the
        //       svn image by 2 all around didn't resolve the issue. For now it resizes based on the border size.
        
        m_background->resize(contentWidth, contentHeight);
        m_background->paint(&p, QRect(contentLeft, contentTop, contentWidth, contentHeight), "center");
        m_background->resize();

        m_background->paint(&p, QRect(leftOffset, topOffset, leftWidth, topHeight), "topleft");
        m_background->paint(&p, QRect(rightOffset, topOffset, rightWidth, topHeight), "topright");
        m_background->paint(&p, QRect(leftOffset, bottomOffset, leftWidth, bottomHeight), "bottomleft");
        m_background->paint(&p, QRect(rightOffset, bottomOffset, rightWidth, bottomHeight), "bottomright");
        
        if (m_background->elementExists("hint-stretch-borders")) {
            m_background->paint(&p, QRect(leftOffset, contentTop, leftWidth, contentHeight), "left");
            m_background->paint(&p, QRect(rightOffset, contentTop, rightWidth, contentHeight), "right");
            m_background->paint(&p, QRect(contentLeft, topOffset, contentWidth, topHeight), "top");
            m_background->paint(&p, QRect(contentLeft, bottomOffset, contentWidth, bottomHeight), "bottom");
        } else {
            QPixmap left(leftWidth, leftHeight);
            left.fill(Qt::transparent);
            {
                QPainter sidePainter(&left);
                sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                m_background->paint(&sidePainter, QPoint(0, 0), "left");
            }
            p.drawTiledPixmap(QRect(leftOffset, contentTop, leftWidth, contentHeight - topHeight - bottomHeight), left);
            
            QPixmap right(rightWidth, rightHeight);
            right.fill(Qt::transparent);
            {
                QPainter sidePainter(&right);
                sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                m_background->paint(&sidePainter, QPoint(0, 0), "right");
            }
            p.drawTiledPixmap(QRect(rightOffset, contentTop, rightWidth, contentHeight - topHeight - bottomHeight), right);
            
            QPixmap top(topWidth, topHeight);
            top.fill(Qt::transparent);
            {
                QPainter sidePainter(&top);
                sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                m_background->paint(&sidePainter, QPoint(0, 0), "top");
            }
            p.drawTiledPixmap(QRect(contentLeft, topOffset, contentWidth - rightWidth - leftWidth, topHeight), top);
            
            QPixmap bottom(bottomWidth, bottomHeight);
            bottom.fill(Qt::transparent);
            {
                QPainter sidePainter(&bottom);
                sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                m_background->paint(&sidePainter, QPoint(0, 0), "bottom");
            }
            p.drawTiledPixmap(QRect(contentLeft, bottomOffset, contentWidth - rightWidth - leftWidth, bottomHeight), bottom);
        }
        
        // re-enable this once Qt's svg rendering is un-buggered
        //background->resize(contentWidth, contentHeight);
        //background->paint(&p, QRect(contentLeft, contentTop, contentWidth, contentHeight), "center");
    }
    
    painter->drawPixmap(exposedRect, *m_cachedBackground, exposedRect);
}

void Dialog::position(QGraphicsSceneEvent *event, const QRectF boundingRect, QPointF scenePos)
{
    QWidget *viewWidget = event->widget() ? event->widget()->parentWidget() : 0;
    //QPointF scenePos = mapToScene(boundingRect.topLeft());
    QGraphicsView *view = qobject_cast<QGraphicsView*>(viewWidget);
    position(view,boundingRect,scenePos);
}

void Dialog::position(QGraphicsView * view,const QRectF boundingRect,QPointF scenePos)
{
    if (view) {
	QPoint viewPos = view->mapFromScene(scenePos);
	QPoint globalPos = view->mapToGlobal(viewPos);
	if ((globalPos.ry()-height())< 0) {
	scenePos = QPointF(scenePos.x() + boundingRect.width(), scenePos.y() + boundingRect.height());
	viewPos = view->mapFromScene(scenePos);
	globalPos = view->mapToGlobal(viewPos)+QPoint(0,10);
	}
	else {
	    globalPos.ry() -= (height()+10);
	}
	if ((globalPos.rx() + width()) > view->width()) {
	    globalPos.rx()-=((globalPos.rx() + width())-view->width());
	}
	move(globalPos);
	kDebug() << globalPos;
    }
}

}
#include "dialog.moc"
