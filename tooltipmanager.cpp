/*
 * Copyright 2007 by Dan Meltzer <hydrogen@notyetimplemented.com>
 * Copyright 2008 by Aaron Seigo <aseigo@kde.org>
 * Copyright 2008 by Alexis Ménard <darktears31@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include "tooltipmanager.h"

//Qt
#include <QCoreApplication>
#include <QLabel>
#include <QTimer>
#include <QGridLayout>
#include <QGraphicsView>
#include <QGraphicsSceneHoverEvent>

//KDE
#include <kwindowsystem.h>

//X11
#ifdef Q_WS_X11
#include <QtGui/QX11Info>
#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

//Plasma
#include "plasma/applet.h"
#include "plasma/containment.h"
#include "plasma/corona.h"
#include "plasma/framesvg.h"
#include "plasma/popupapplet.h"
#include "plasma/theme.h"
#include "plasma/view.h"
#include "plasma/private/tooltip_p.h"

namespace Plasma
{

class ToolTipManagerPrivate
{
public :
    ToolTipManagerPrivate(ToolTipManager *manager)
        : q(manager),
          currentWidget(0),
          showTimer(new QTimer(manager)),
          hideTimer(new QTimer(manager)),
          tipWidget(new ToolTip(0)),
          state(ToolTipManager::Activated),
          isShown(false),
          delayedHide(false),
          clickable(false)
    {
    }

    ~ToolTipManagerPrivate()
    {
        if (!QCoreApplication::closingDown()) {
            delete tipWidget;
        }
    }

    void showToolTip();
    void resetShownState();

    /**
      * called when a widget inside the tooltip manager is deleted
      */
    void onWidgetDestroyed(QObject * object);
    void removeWidget(QGraphicsWidget *w, bool canSafelyAccess = true);
    void clearTips();
    void doDelayedHide();
    void toolTipHovered(bool);

    ToolTipManager *q;
    QGraphicsWidget *currentWidget;
    QTimer *showTimer;
    QTimer *hideTimer;
    QHash<QGraphicsWidget *, ToolTipContent> tooltips;
    ToolTip *tipWidget;
    ToolTipManager::State state;
    bool isShown : 1;
    bool delayedHide : 1;
    bool clickable : 1;
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
K_GLOBAL_STATIC(ToolTipManagerSingleton, privateInstance)

ToolTipManager *ToolTipManager::self()
{
    return &privateInstance->self;
}

ToolTipManager::ToolTipManager(QObject *parent)
  : QObject(parent),
    d(new ToolTipManagerPrivate(this)),
    m_corona(0)
{
    d->showTimer->setSingleShot(true);
    connect(d->showTimer, SIGNAL(timeout()), SLOT(showToolTip()));

    d->hideTimer->setSingleShot(true);
    connect(d->hideTimer, SIGNAL(timeout()), SLOT(resetShownState()));

    connect(d->tipWidget, SIGNAL(activateWindowByWId(WId,Qt::MouseButtons,Qt::KeyboardModifiers,QPoint)),
            this, SIGNAL(windowPreviewActivated(WId,Qt::MouseButtons,Qt::KeyboardModifiers,QPoint)));
    connect(d->tipWidget, SIGNAL(linkActivated(QString,Qt::MouseButtons,Qt::KeyboardModifiers,QPoint)),
            this, SIGNAL(linkActivated(QString,Qt::MouseButtons,Qt::KeyboardModifiers,QPoint)));
    connect(d->tipWidget, SIGNAL(hovered(bool)), this, SLOT(toolTipHovered(bool)));
}

ToolTipManager::~ToolTipManager()
{
    delete d;
}

void ToolTipManager::show(QGraphicsWidget *widget)
{
    if (!d->tooltips.contains(widget)) {
        return;
    }

    KConfig config("plasmarc");
    KConfigGroup cg(&config, "PlasmaToolTips");
    qreal delay = cg.readEntry("Delay", qreal(0.7));
    if (delay < 0) {
        return;
    }

    if (d->currentWidget) {
        disconnect(this, 0, d->currentWidget, 0);
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
        d->showTimer->start(delay * 1000);
    }
}

bool ToolTipManager::isVisible(QGraphicsWidget *widget) const
{
    return d->currentWidget == widget && d->tipWidget->isVisible();
}

void ToolTipManagerPrivate::doDelayedHide()
{
    showTimer->stop();  // stop the timer to show the tooltip
    delayedHide = true;

    if (isShown && clickable) {
        // leave enough time for user to choose
        hideTimer->start(1000);
    } else {
        hideTimer->start(250);
    }
}

void ToolTipManager::hide(QGraphicsWidget *widget)
{
    if (d->currentWidget != widget) {
        return;
    }

    if (d->currentWidget) {
        disconnect(this, 0, d->currentWidget, 0);
    }

    d->currentWidget = 0;
    d->showTimer->stop();  // stop the timer to show the tooltip
    d->delayedHide = false;
    d->tipWidget->hide();
}

void ToolTipManager::registerWidget(QGraphicsWidget *widget)
{
    if (d->state == Deactivated || d->tooltips.contains(widget)) {
        return;
    }

    //the tooltip is not registered we add it in our map of tooltips
    d->tooltips.insert(widget, ToolTipContent());
    widget->installEventFilter(this);
    connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(onWidgetDestroyed(QObject*)));
}

void ToolTipManager::unregisterWidget(QGraphicsWidget *widget)
{
    if (!d->tooltips.contains(widget)) {
        return;
    }

    widget->removeEventFilter(this);
    d->removeWidget(widget);
}

void ToolTipManager::setContent(QGraphicsWidget *widget, const ToolTipContent &data)
{
    if (d->state == Deactivated || !widget) {
        return;
    }

    registerWidget(widget);
    d->tooltips.insert(widget, data);

    if (d->currentWidget == widget) {
        if (data.isEmpty()) {
            hide(widget);
        } else {
            d->delayedHide = data.autohide();
            d->clickable = data.isClickable();
            if (d->delayedHide) {
                //kDebug() << "starting authoide";
                d->hideTimer->start(3000);
            } else {
                d->hideTimer->stop();
            }
        }

        d->tipWidget->setContent(widget, data);
        d->tipWidget->prepareShowing();
        if (m_corona) {
            d->tipWidget->moveTo(m_corona->popupPosition(widget, d->tipWidget->size(), Qt::AlignCenter));
        }
    }
}

void ToolTipManager::clearContent(QGraphicsWidget *widget)
{
    setContent(widget, ToolTipContent());
}

void ToolTipManager::setState(ToolTipManager::State state)
{
    d->state = state;

    switch (state) {
        case Activated:
            break;
        case Deactivated:
            d->clearTips();
            //fallthrough
        case Inhibited:
            d->resetShownState();
            break;
    }
}

ToolTipManager::State ToolTipManager::state() const
{
    return d->state;
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
    removeWidget(w, false);
}

void ToolTipManagerPrivate::removeWidget(QGraphicsWidget *w, bool canSafelyAccess)
{
    if (currentWidget == w && currentWidget) {
        if (canSafelyAccess) {
            QObject::disconnect(q, 0, currentWidget, 0);
        }

        currentWidget = 0;
        showTimer->stop();  // stop the timer to show the tooltip
        tipWidget->setContent(0, ToolTipContent());
        tipWidget->hide();
        delayedHide = false;
    }

    tooltips.remove(w);
}

void ToolTipManagerPrivate::clearTips()
{
    tooltips.clear();
}

void ToolTipManagerPrivate::resetShownState()
{
    if (currentWidget) {
        if (!tipWidget->isVisible() || delayedHide) {
            //One might have moused out and back in again
            delayedHide = false;
            isShown = false;
            QObject::disconnect(q, 0, currentWidget, 0);
            currentWidget = 0;
            tipWidget->hide();
        }
    }
}

void ToolTipManagerPrivate::showToolTip()
{
    if (state != ToolTipManager::Activated ||
        !currentWidget ||
        QApplication::activePopupWidget() ||
        QApplication::activeModalWidget()) {
        return;
    }

    PopupApplet *popup = qobject_cast<PopupApplet*>(currentWidget);
    if (popup && popup->isPopupShowing()) {
        return;
    }

    // toolTipAboutToShow may call into methods such as setContent which play
    // with the current widget; so let's just pretend for a moment that we don't have
    // a current widget
    QGraphicsWidget *temp = currentWidget;
    currentWidget = 0;
    QMetaObject::invokeMethod(temp, "toolTipAboutToShow");
    currentWidget = temp;

    QHash<QGraphicsWidget *, ToolTipContent>::const_iterator tooltip = tooltips.constFind(currentWidget);

    if (tooltip == tooltips.constEnd() || tooltip.value().isEmpty()) {
        if (isShown) {
            delayedHide = true;
            hideTimer->start(250);
        }

        return;
    }

    Containment *c = dynamic_cast<Containment *>(currentWidget->topLevelItem());
    //kDebug() << "about to show" << (QObject*)c;
    if (c) {
        tipWidget->setDirection(Plasma::locationToDirection(c->location()));
    }

    clickable = tooltip.value().isClickable();
    tipWidget->setContent(currentWidget, tooltip.value());
    tipWidget->prepareShowing();
    if (q->m_corona) {
        tipWidget->moveTo(q->m_corona->popupPosition(currentWidget, tipWidget->size(), Qt::AlignCenter));
    }
    tipWidget->show();
    isShown = true;  //ToolTip is visible

    delayedHide = tooltip.value().autohide();
    if (delayedHide) {
        //kDebug() << "starting authoide";
        hideTimer->start(3000);
    } else {
        hideTimer->stop();
    }
}

void ToolTipManagerPrivate::toolTipHovered(bool hovered)
{
    if (!clickable) {
        return;
    }

    if (hovered) {
        hideTimer->stop();
    } else {
        hideTimer->start(500);
    }
}

bool ToolTipManager::eventFilter(QObject *watched, QEvent *event)
{
    QGraphicsWidget * widget = dynamic_cast<QGraphicsWidget *>(watched);
    if (d->state != Activated || !widget) {
        return QObject::eventFilter(watched, event);
    }

    switch (event->type()) {
        case QEvent::GraphicsSceneHoverMove:
            // If the tooltip isn't visible, run through showing the tooltip again
            // so that it only becomes visible after a stationary hover
            if (Plasma::ToolTipManager::self()->isVisible(widget)) {
                break;
            }

            // Don't restart the show timer on a mouse move event if there hasn't
            // been an enter event or the current widget has been cleared by a click
            // or wheel event.
            {
                QGraphicsSceneHoverEvent *me = static_cast<QGraphicsSceneHoverEvent *>(event);
                //FIXME: seems that wheel events generate hovermoves as well, with 0 delta
                if (!d->currentWidget || (me->pos() == me->lastPos())) {
                    break;
            }
            }

        case QEvent::GraphicsSceneHoverEnter:
        {
            // Check that there is a tooltip to show
            if (!d->tooltips.contains(widget)) {
                break;
            }

            show(widget);
            break;
        }

        case QEvent::GraphicsSceneHoverLeave:
            if (d->currentWidget == widget) {
                d->doDelayedHide();
            }
            break;

        case QEvent::GraphicsSceneMousePress:
            if (d->currentWidget == widget) {
                hide(widget);
            }
            break;

        case QEvent::GraphicsSceneWheel:
        default:
            break;
    }

    return QObject::eventFilter(watched, event);
}

} // Plasma namespace

#include "tooltipmanager.moc"

