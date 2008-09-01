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
#include <QtGui/QVBoxLayout>
#include <QtGui/QGraphicsSceneEvent>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsWidget>

#include <KDebug>
#include <NETRootInfo>

#include <plasma/panelsvg.h>
#include <plasma/theme.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif


namespace Plasma
{

class DialogPrivate
{
public:
    DialogPrivate(Dialog *dialog)
            : q(dialog),
              background(0),
              view(0),
              widget(0)
    {
    }

    ~DialogPrivate()
    {
    }

    void themeUpdated();
    void adjustView();

    Plasma::Dialog *q;
    /**
     * Holds the background SVG, to be re-rendered when the cache is invalidated,
     * for example by resizing the dialogue.
     */
    Plasma::PanelSvg *background;
    QGraphicsView *view;
    QGraphicsWidget *widget;
};

void DialogPrivate::themeUpdated()
{
    const int topHeight = background->marginSize(Plasma::TopMargin);
    const int leftWidth = background->marginSize(Plasma::LeftMargin);
    const int rightWidth = background->marginSize(Plasma::RightMargin);
    const int bottomHeight = background->marginSize(Plasma::BottomMargin);
    q->setContentsMargins(leftWidth, topHeight, rightWidth, bottomHeight);
}

void DialogPrivate::adjustView()
{
    if (view && widget) {
        QSize prevSize = q->size();

        //reposition and resize the view.
        view->setSceneRect(widget->mapToScene(widget->boundingRect()).boundingRect());
        view->resize(widget->preferredSize().toSize());
        view->centerOn(widget);

        //set the sizehints correctly:
        int left, top, right, bottom;
        q->getContentsMargins(&left, &top, &right, &bottom);
        q->setMinimumSize(view->size().width() + left + right,
                          view->size().height() + top + bottom);
        q->setMaximumSize(view->size().width() + left + right,
                          view->size().height() + top + bottom);
        q->updateGeometry();

        if (q->size() != prevSize) {
            //the size of the dialog has changed, emit the signal:
            emit q->dialogResized();
        }
    }
}

Dialog::Dialog( QWidget * parent, Qt::WindowFlags f )
    : QWidget(parent, f),
      d(new DialogPrivate(this))
{
    d->background = new PanelSvg(this);
    d->background->setImagePath("dialogs/background");
    d->background->setEnabledBorders(PanelSvg::AllBorders);
    d->background->resizePanel(size());

    connect(d->background, SIGNAL(repaintNeeded()), this, SLOT(update()));

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeUpdated()));
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
    d->background->paintPanel(&p);
}

void Dialog::resizeEvent(QResizeEvent *e)
{
    d->background->resizePanel(e->size());

    setMask(d->background->mask());
}

void Dialog::setGraphicsWidget(QGraphicsWidget *widget)
{
    if (d->widget) {
        d->widget->removeEventFilter(this);
    }

    d->widget = widget;

    if (widget) {
        if (!layout()) {
            QVBoxLayout *lay = new QVBoxLayout(this);
            lay->setMargin(0);
            lay->setSpacing(0);
        }

        if (!d->view) {
            d->view = new QGraphicsView(this);
            d->view->setFrameShape(QFrame::NoFrame);
            d->view->viewport()->setAutoFillBackground(false);
            layout()->addWidget(d->view);
        }

        d->view->setScene(widget->scene());
        d->adjustView();

        adjustSize();

        widget->installEventFilter(this);
    } else {
        delete d->view;
        d->view = 0;
    }
}

QGraphicsWidget *Dialog::graphicsWidget()
{
    return d->widget;
}

bool Dialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d->widget && (event->type() == QEvent::GraphicsSceneResize ||
                                 event->type() == QEvent::GraphicsSceneMove)) {
        d->adjustView();
    }

    return QWidget::eventFilter(watched, event);
}

void Dialog::hideEvent(QHideEvent * event)
{
    Q_UNUSED(event);
    emit dialogVisible(false);
}

void Dialog::showEvent(QShowEvent * event)
{
    Q_UNUSED(event);
    emit dialogVisible(true);
}

}
#include "dialog.moc"
