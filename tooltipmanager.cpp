/**************************************************************************
*   Copyright 2007 by Dan Meltzer <hydrogen@notyetimplemented.com>        *
*   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>           *
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
#include "tooltipmanager.h"

//Qt
#include <QLabel>
#include <QTimer>
#include <QGridLayout>
#include <QGraphicsView>
#include <QDesktopWidget>

//KDE
#include <KWindowSystem>

//Plasma
#include <plasma/theme.h>
#include <plasma/panelsvg.h>
#include <plasma/tooltip_p.h>
#include <plasma/applet.h>
#include <plasma/view.h>
#include <plasma/containment.h>
namespace Plasma
{
class ToolTipManagerPrivate
{
public :
    ToolTipManagerPrivate()
        : currentWidget(0)
        , showTimer(0)
        , hideTimer(0)
    {

    }
    ~ToolTipManagerPrivate()
    {

    }

    void showToolTip();
    void resetShownState();

    /**
      * called when the theme of plasma has change
      */
    void themeUpdated();
    /**
      * called when a widget inside the tooltip manager is deleted
      */
    void onWidgetDestroyed(QObject * object);


    QGraphicsWidget *currentWidget;
    bool isShown;
    bool delayedHide;
    QTimer *showTimer;
    QTimer *hideTimer;
    QMap<QGraphicsWidget *, ToolTip *> tooltips;
};

//TOOLTIP IMPLEMENTATION
class ToolTipManagerSingleton
{
    public:
    ToolTipManagerSingleton()
    {
    }
    ToolTipManager self;
};
K_GLOBAL_STATIC( ToolTipManagerSingleton, privateInstance )

ToolTipManager *ToolTipManager::self()
{
    return &privateInstance->self;
}

ToolTipManager::ToolTipManager(QObject* parent)
  : QObject(parent),
    d(new ToolTipManagerPrivate)
{
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeUpdated()));
    d->themeUpdated();

    d->showTimer = new QTimer(this);
    d->showTimer->setSingleShot(true);
    d->hideTimer = new QTimer(this);
    d->hideTimer->setSingleShot(true);

    connect(d->showTimer, SIGNAL(timeout()), SLOT(showToolTip()));
    connect(d->hideTimer, SIGNAL(timeout()), SLOT(resetShownState()));
}

ToolTipManager::~ToolTipManager()
{
    if (d) delete d;
}

void ToolTipManager::showToolTip(QGraphicsWidget *widget)
{
   ToolTip * tooltip = d->tooltips.value(widget);
   if (tooltip) {
      if (d->currentWidget) {
          hideToolTip(d->currentWidget);
      }
      d->hideTimer->stop();
      d->delayedHide = false;
      d->showTimer->stop();
      d->currentWidget = widget;
      if (d->isShown) {
          // small delay to prevent unnecessary showing when the mouse is moving quickly across items
          // which can be too much for less powerful CPUs to keep up with
          d->showTimer->start(200);
      } else {
          d->showTimer->start(500);
      }
   }
}

bool ToolTipManager::isWidgetToolTipDisplayed(QGraphicsWidget *widget)
{
   ToolTip * tooltip = d->tooltips.value(widget);
   if (tooltip) {
      return tooltip->isVisible();
   }
   else {
      return false;
   }
}

void ToolTipManager::delayedHideToolTip()
{
    d->showTimer->stop();  // stop the timer to show the tooltip
    d->delayedHide = true;
    d->hideTimer->start(250);
}

void ToolTipManager::hideToolTip(QGraphicsWidget *widget)
{
   ToolTip * tooltip = d->tooltips.value(widget);
   if (tooltip) {
      d->showTimer->stop();  // stop the timer to show the tooltip
      d->delayedHide = false;
      d->currentWidget = 0;
      tooltip->hide();
  }
}

void ToolTipManager::registerWidget(QGraphicsWidget *widget)
{
    if (!d->tooltips.contains(widget)) {
        //the tooltip is not registered we add it in our map of tooltips
        d->tooltips.insert(widget,new ToolTip());
        //connect to object destruction
        connect(widget,SIGNAL(destroyed(QObject *)),this,SLOT(onWidgetDestroyed(QObject *)));
    }
}

void ToolTipManager::unregisterWidget(QGraphicsWidget *widget)
{
    if (!d->tooltips.contains(widget)) {
        return;
    }

    ToolTip * tooltip = d->tooltips.take(widget);
    if (tooltip) {
        delete tooltip;
    }
}

void ToolTipManager::setWidgetToolTipContent(QGraphicsWidget *widget,const ToolTipContent &data)
{
    if (!d->tooltips.contains(widget)) {
        return;
    }

    ToolTip * tooltip = d->tooltips.value(widget);
    tooltip->setContent(data);
    tooltip->updateTheme();
}

bool ToolTipManager::widgetHasToolTip(QGraphicsWidget *widget) const
{
    return d->tooltips.contains(widget);
}

void ToolTipManagerPrivate::themeUpdated()
{
    QMapIterator<QGraphicsWidget*, ToolTip *> iterator(tooltips);
    while (iterator.hasNext()) {
        iterator.next();
        iterator.value()->updateTheme();
    }
}

void ToolTipManagerPrivate::onWidgetDestroyed(QObject *object)
{
    if (!object) {
        return;
    }

    // we do a static_cast here since it really isn't a QGraphicsWidget by this
    // point anymore since we are in the QObject dtor. we don't actually
    // try and do anything with it, we just need the value of the pointer
    // so this unsafe looking code is actually just fine.
    //
    // NOTE: DO NOT USE THE w VARIABLE FOR ANYTHING OTHER THAN COMPARING
    //       THE ADDRESS! ACTUALLY USING THE OBJECT WILL RESULT IN A CRASH!!!
    QGraphicsWidget *w = static_cast<QGraphicsWidget*>(object);

    if (currentWidget == w) {
        currentWidget = 0;
        showTimer->stop();  // stop the timer to show the tooltip
        delayedHide = false;
    }

    QMapIterator<QGraphicsWidget*, ToolTip *> iterator(tooltips);
    while (iterator.hasNext()) {
        iterator.next();
        if (iterator.key() == w) {
            ToolTip * tooltip = iterator.value();
            tooltips.remove(iterator.key());
            tooltip->hide();
            delete tooltip;
        }
    }
}

void ToolTipManagerPrivate::resetShownState()
{
    if (currentWidget) {
        ToolTip * tooltip = tooltips.value(currentWidget);
        if (tooltip && (!tooltip->isVisible() || delayedHide)) {
            //One might have moused out and back in again
            delayedHide = false;
            isShown = false;
            tooltip->hide();
            currentWidget = 0;
      }
    }
}

void ToolTipManagerPrivate::showToolTip()
{
    if (!currentWidget) {
        return;
    }

    ToolTip * tooltip = tooltips.value(currentWidget);
    if (tooltip) {
        tooltip->prepareShowing();
        tooltip->move(ToolTipManager::popupPosition(currentWidget,tooltip->size()));
        isShown = true;  //ToolTip is visible
    }
}

QPoint ToolTipManager::popupPosition(const QGraphicsItem * item, const QSize &s)
{
    const Plasma::Applet * applet = dynamic_cast<const Applet *>(item);
    if (!applet) {
        const QGraphicsItem * currentItem = item->parentItem();
        while(currentItem->parentItem() && !dynamic_cast<const Applet *>(currentItem))
        {
            currentItem=currentItem->parentItem();
        }
        applet = dynamic_cast<const Applet *>(currentItem);
        if (!applet) return QPoint(0,0);
    }
    QGraphicsView *v = applet->view();
    Q_ASSERT(v);

    QPoint pos = v->mapFromScene(item->scenePos());
    pos = v->mapToGlobal(pos);
    //kDebug() << "==> position is" << scenePos() << v->mapFromScene(scenePos()) << pos;
    Plasma::View *pv = dynamic_cast<Plasma::View *>(v);

    Plasma::Location loc = Floating;
    if (pv) {
        loc = pv->containment()->location();
    }

    switch (loc) {
    case BottomEdge:
        pos = QPoint(pos.x(), pos.y() - s.height());
        break;
    case TopEdge:
        pos = QPoint(pos.x(), pos.y() + (int)item->boundingRect().size().height());
        break;
    case LeftEdge:
        pos = QPoint(pos.x() + (int)item->boundingRect().size().width(), pos.y());
        break;
    case RightEdge:
        pos = QPoint(pos.x() - s.width(), pos.y());
        break;
    default:
        if (pos.y() - s.height() > 0) {
             pos = QPoint(pos.x(), pos.y() - s.height());
        } else {
             pos = QPoint(pos.x(), pos.y() + (int)item->boundingRect().size().height());
        }
    }

    //are we out of screen?

    QRect screenRect = QApplication::desktop()->screenGeometry(pv ? pv->containment()->screen() : -1);
    //kDebug() << "==> rect for" << (pv ? pv->containment()->screen() : -1) << "is" << screenRect;

    if (pos.rx() + s.width() > screenRect.right()) {
        pos.rx() -= ((pos.rx() + s.width()) - screenRect.right());
    }

    if (pos.ry() + s.height() > screenRect.bottom()) {
        pos.ry() -= ((pos.ry() + s.height()) - screenRect.bottom());
    }
    pos.rx() = qMax(0, pos.rx());

    return pos;
}

}

#include "tooltipmanager.moc"

