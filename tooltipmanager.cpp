/***************************************************************************
 *   Copyright 2007 by Dan Meltzer <hydrogen@notyetimplemented.com>        *
 *   Copyright 2008 by Aaron Seigo <aseigo@kde.org>                        *
 *   Copyright 2008 by Alexis Ménard <darktears31@gmail.com>               *
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

ToolTipManager::ToolTipContent::ToolTipContent()
    : windowToPreview(0)
{
}

bool ToolTipManager::ToolTipContent::isEmpty() const
{
    return mainText.isEmpty() && subText.isEmpty() && image.isNull() && windowToPreview == 0;
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
   ToolTip *tooltip = d->tooltips.value(widget);
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
   ToolTip *tooltip = d->tooltips.value(widget);
   if (tooltip) {
      return tooltip->isVisible();
   } else {
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
    ToolTip *tooltip = d->tooltips.value(widget);
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
        d->tooltips.insert(widget, 0);
        widget->installEventFilter(this);
        //connect to object destruction
        connect(widget, SIGNAL(destroyed(QObject *)),this,SLOT(onWidgetDestroyed(QObject *)));
    }
}

void ToolTipManager::unregisterWidget(QGraphicsWidget *widget)
{
    if (!d->tooltips.contains(widget)) {
        return;
    }
    widget->removeEventFilter(this);
    ToolTip *tooltip = d->tooltips.take(widget);
    if (tooltip) {
        delete tooltip;
    }
}

void ToolTipManager::setToolTipContent(QGraphicsWidget *widget, const ToolTipContent &data)
{
    if (!d->tooltips.contains(widget)) {
        registerWidget(widget);
    }

    ToolTip *tooltip = d->tooltips.value(widget);

    if (data.isEmpty()) {
        delete tooltip;
        d->tooltips.insert(widget, 0);
        return;
    }

    if (!tooltip) {
        d->tooltips.insert(widget, new ToolTip(widget));
    }

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
            ToolTip *tooltip = iterator.value();
            tooltips.remove(iterator.key());
            if (tooltip) {
                tooltip->hide();
                delete tooltip;
            }
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

    ToolTip *tooltip = tooltips.value(currentWidget);
    if (tooltip) {
        tooltip->prepareShowing();
        tooltip->move(popupPosition(currentWidget, tooltip->size()));
        isShown = true;  //ToolTip is visible
    }
}

bool ToolTipManager::eventFilter(QObject *watched, QEvent *event)
{
    QGraphicsWidget * widget = dynamic_cast<QGraphicsWidget *>(watched);
    if (!widget) { 
      return QObject::eventFilter(watched,event);
    }

    switch (event->type()) {
        case QEvent::GraphicsSceneHoverMove:
            // If the tooltip isn't visible, run through showing the tooltip again
            // so that it only becomes visible after a stationary hover
            if (Plasma::ToolTipManager::self()->isWidgetToolTipDisplayed(widget)) {
                break;
            }

        case QEvent::GraphicsSceneHoverEnter:
        {
            // Check that there is a tooltip to show
            if (!widgetHasToolTip(widget)) {
                break;
            }

            // If the mouse is in the widget's area at the time that it is being
            // created the widget can receive a hover event before it is fully
            // initialized, in which case view() will return 0.
            QGraphicsView *parentView = viewFor(widget);
            if (parentView) {
                showToolTip(widget);
            }

            break;
        }

        case QEvent::GraphicsSceneHoverLeave:
            delayedHideToolTip();
            break;

        case QEvent::GraphicsSceneMousePress:
        case QEvent::GraphicsSceneWheel:
            hideToolTip(widget);

        default:
            break;
    }

    return QObject::eventFilter(watched,event);
}

} // Plasma namespace

#include "tooltipmanager.moc"

