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
#include <QBitmap>
#include <QGraphicsView>
#include <QtGui/QGraphicsSceneEvent>

#include <KDebug>
#include <NETRootInfo>

#include <plasma/panelsvg.h>
#include <plasma/theme.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif


namespace Plasma
{

class Dialog::Private
{
public:
    /**
     * Holds the background SVG, to be re-rendered when the cache is invalidated,
     * for example by resizing the dialogue.
     */
    Plasma::PanelSvg *background;
    Plasma::Dialog *q;

    void themeUpdated()
    {
        const int topHeight = background->marginSize(Plasma::TopMargin);
        const int leftWidth = background->marginSize(Plasma::LeftMargin);
        const int rightWidth = background->marginSize(Plasma::RightMargin);
        const int bottomHeight = background->marginSize(Plasma::BottomMargin);
        q->setContentsMargins(leftWidth, topHeight, rightWidth, bottomHeight);
    }; 
};

Dialog::Dialog( QWidget * parent, Qt::WindowFlags f )
    : QWidget(parent, f),
      d(new Private)
{
    d->q = this;
    d->background = new PanelSvg("dialogs/background", this);
    d->background->setEnabledBorders(PanelSvg::AllBorders);
    d->background->resize(size());

    connect(d->background, SIGNAL(repaintNeeded()), this, SLOT(update()));

    connect(Plasma::Theme::defaultTheme(), SIGNAL(changed()), this, SLOT(themeUpdated()));
    d->themeUpdated();
}

Dialog::~Dialog()
{
    delete d;
}

void Dialog::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setClipRect(e->rect());
    p.setCompositionMode(QPainter::CompositionMode_Source );
    p.fillRect(rect(), Qt::transparent);
    d->background->paint(&p, e->rect());
}

void Dialog::resizeEvent(QResizeEvent *e)
{
    d->background->resize(e->size());

    setMask(d->background->mask());
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
