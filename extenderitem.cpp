/*
 * Copyright 2008 by Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
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

#include "extenderitem.h"

#include <QApplication>
#include <QAction>
#include <QBitmap>
#include <QGraphicsSceneResizeEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLinearLayout>
#include <QLayout>
#include <QPainter>
#include <QTimer>

#include <KDebug>
#include <KIcon>
#include <KWindowSystem>
#include <KGlobalSettings>

#include "applet.h"
#include "containment.h"
#include "corona.h"
#include "dialog.h"
#include "extender.h"
#include "panelsvg.h"
#include "popupapplet.h"
#include "theme.h"
#include "view.h"

#include "private/applet_p.h"
#include "private/extender_p.h"

namespace Plasma
{

class ExtenderItemPrivate
{
    public:
        ExtenderItemPrivate(ExtenderItem *extenderItem, Extender *hostExtender)
            : q(extenderItem),
              widget(0),
              toplevel(0),
              previousTargetExtender(0),
              extender(hostExtender),
              title(QString()),
              sourceAppletId(hostExtender->d->applet->id()),
              mousePressed(false),
              expirationTimer(0)
        {
        }

        ~ExtenderItemPrivate()
        {
            delete toplevel;
        }

        /**
         * @return a Rect containing the area of the detachable where the draghandle can be drawn.
         */
        QRectF dragHandleRect()
        {
            QRectF rect(bgLeft, bgTop, q->size().width() - bgLeft - bgRight,
                        dragger->elementSize("hint-preferred-icon-size").height() +
                        dragTop + dragBottom);

            return rect;
        }

        QRectF titleRect()
        {
            return dragHandleRect().adjusted(dragLeft + collapseIcon->size().width() + 1, dragTop,
                                             -toolbox->size().width() - dragRight, -dragBottom);
        }

        //XXX kinda duplicated from applethandle.
        //returns true if the applet overlaps with a different view then the current one.
        bool leaveCurrentView(const QRect &rect)
        {
            if ((q->sceneBoundingRect().left() < 0)) {
                //always leaveCurrentView when the item is in a Plasma::Dialog which can easy be
                //seen by checking if it is in topleft quadrant and it's not just there because it's
                //being viewed by the toplevel view.
                return true;
            }

            foreach (QWidget *widget, QApplication::topLevelWidgets()) {
                if (widget->geometry().intersects(rect) && widget->isVisible() && widget != toplevel) {
                    //is this widget a plasma view, a different view then our current one,
                    //AND not a dashboardview?
                    QGraphicsView *v = qobject_cast<QGraphicsView*>(widget);
                    QGraphicsView *currentV = 0;

                    if (hostApplet()) {
                        currentV = qobject_cast<QGraphicsView*>(hostApplet()->containment()->view());
                    }

                    if (v && v != currentV) {
                        return true;
                    }
                }
            }
            return false;
        }

        QRect screenRect() {
            return hostApplet()->containment()->view()
                               ->mapFromScene(q->sceneBoundingRect()).boundingRect();
        }

        void toggleCollapse() {
            q->setCollapsed(!q->isCollapsed());
        }

        void updateToolBox() {
            if (toolbox && dragger && toolboxLayout) {
                //clean the layout.
                uint iconHeight = dragger->elementSize("hint-preferred-icon-size").height();

                while (toolboxLayout->count()) {
                    QGraphicsLayoutItem *icon = toolboxLayout->itemAt(0);
                    QGraphicsWidget *widget = dynamic_cast<QGraphicsWidget*>(icon);
                    widget->deleteLater();
                    toolboxLayout->removeAt(0);
                }

                //add the actions that are actually set to visible.
                foreach (QAction *action, actions) {
                    if (action->isVisible()) {
                        Icon *icon = new Icon(q);
                        icon->setAction(action);
                        QSizeF iconSize = icon->sizeFromIconSize(iconHeight);
                        icon->setMinimumSize(iconSize);
                        icon->setMaximumSize(iconSize);
                        toolboxLayout->addItem(icon);
                    }
                }

                toolboxLayout->updateGeometry();

                //position the toolbox correctly.
                QSizeF minimum = toolboxLayout->minimumSize();
                toolbox->resize(minimum);
                toolbox->setPos(q->size().width() - minimum.width() - bgRight,
                                ((dragger->size().height() + dragTop + dragBottom)/2) -
                                (minimum.height()/2) + bgTop);
                toolbox->update();
            }
        }

        //TODO: something like this as static function in corona might be a good idea.
        QPointF scenePosFromScreenPos(const QPoint &pos) const
        {
            //get the stacking order of the toplevel windows and remove the toplevel view that's
            //only here while dragging, since we're not interested in finding that.
            QList<WId> order = KWindowSystem::stackingOrder();
            if (toplevel) {
                order.removeOne(toplevel->winId());
            }

            QGraphicsView *found = 0;
            foreach (QWidget *w, QApplication::topLevelWidgets()) {
                QGraphicsView *v = 0;

                //first check if we're over a Dialog.
                Dialog *dialog = qobject_cast<Dialog*>(w);
                if (dialog) {
                    if (dialog->isVisible() && dialog->geometry().contains(pos)) {
                        v = qobject_cast<QGraphicsView*>(dialog->layout()->itemAt(0)->widget());
                        if (v) {
                            return v->mapToScene(v->mapFromGlobal(pos));
                        }
                    }
                } else {
                    v = qobject_cast<QGraphicsView *>(w);
                }

                //else check if it is a QGV:
                if (v && w->isVisible() && w->geometry().contains(pos)) {
                    if (found && order.contains(found->winId())) {
                        if (order.indexOf(found->winId()) < order.indexOf(v->winId())) {
                            found = v;
                        }
                    } else {
                        found = v;
                    }
                }
            }

            if (!found) {
                return QPointF();
            }

            return found->mapToScene(found->mapFromGlobal(pos));
        }

        Applet *hostApplet() const
        {
            if (extender) {
                return extender->d->applet;
            } else {
                return 0;
            }
        }

        void themeChanged()
        {
            dragger->setImagePath("widgets/extender-dragger");
            background->setImagePath("widgets/extender-background");

            dragger->getMargins(dragLeft, dragTop, dragRight, dragBottom);
            background->getMargins(bgLeft, bgTop, bgRight, bgBottom);

            //setCollapsed recalculates size hints.
            q->setCollapsed(q->isCollapsed());
            q->update();
        }

        ExtenderItem *q;

        QGraphicsItem *widget;
        QGraphicsWidget *toolbox;
        QGraphicsLinearLayout *toolboxLayout;
        QGraphicsView *toplevel;

        Extender *previousTargetExtender;
        Extender *extender;
        Applet *sourceApplet;

        KConfigGroup config;

        PanelSvg *dragger;
        PanelSvg *background;

        Icon *collapseIcon;
        QAction *returnAction;
        QMap<QString, QAction*> actions;

        QString title;
        QString name;

        uint sourceAppletId;
        uint extenderItemId;

        qreal dragLeft, dragTop, dragRight, dragBottom;
        qreal bgLeft, bgTop, bgRight, bgBottom;

        QPointF deltaScene;
        QPoint mousePos;

        bool mousePressed;
        bool mouseOver;

        QTimer *expirationTimer;

        static uint s_maxExtenderItemId;
};

uint ExtenderItemPrivate::s_maxExtenderItemId = 0;

ExtenderItem::ExtenderItem(Extender *hostExtender, uint extenderItemId)
        : QGraphicsWidget(hostExtender),
          d(new ExtenderItemPrivate(this, hostExtender))
{
    Q_ASSERT(hostExtender);

    //set the extenderId
    if (extenderItemId) {
        d->extenderItemId = extenderItemId;
        ExtenderItemPrivate::s_maxExtenderItemId =
            qMax(ExtenderItemPrivate::s_maxExtenderItemId, extenderItemId);
    } else {
        d->extenderItemId = ++ExtenderItemPrivate::s_maxExtenderItemId;
    }

    d->sourceApplet = hostExtender->d->applet;

    //create items's configgroup
    KConfigGroup cg = hostExtender->d->applet->config("ExtenderItems");
    KConfigGroup dg = KConfigGroup(&cg, QString::number(d->extenderItemId));

    if (!dg.readEntry("sourceAppletId", 0)) {
        //The item is new
        dg.writeEntry("sourceAppletPluginName", hostExtender->d->applet->pluginName());
        dg.writeEntry("sourceAppletId", hostExtender->d->applet->id());
        d->sourceAppletId = hostExtender->d->applet->id();
        d->sourceApplet = hostExtender->d->applet;
    } else {
        //The item allready exists.
        d->name = dg.readEntry("extenderItemName", "");
        d->sourceAppletId = dg.readEntry("sourceAppletId", 0);
        //Set the sourceapplet.
        Corona *corona = hostExtender->d->applet->containment()->corona();
        foreach (Containment *containment, corona->containments()) {
            foreach (Applet *applet, containment->applets()) {
                if (applet->id() == d->sourceAppletId &&
                        applet->pluginName() == dg.readEntry("sourceAppletPluginName", "")) {
                    d->sourceApplet = applet;
                }
            }
        }
    }

    //create the dragger and standard applet background.
    d->dragger = new PanelSvg(this);
    d->background = new PanelSvg(this);
    d->themeChanged();

    //create the toolbox.
    d->toolbox = new QGraphicsWidget(this);
    d->toolboxLayout = new QGraphicsLinearLayout(d->toolbox);
    d->toolbox->setLayout(d->toolboxLayout);

    //allow the theme to set the size of the icon.
    //TODO: discuss with others to determine details of the theming implementation. I don't really
    //like this approach, but it works...
    QSizeF iconSize = d->dragger->elementSize("hint-preferred-icon-size");

    //create the collapse/applet icon.
    d->collapseIcon = new Icon(KIcon(hostExtender->d->applet->icon()), "", this);
    d->collapseIcon->resize(d->collapseIcon->sizeFromIconSize(iconSize.height()));
    d->collapseIcon->setPos(d->bgLeft + d->dragLeft,
                            (d->dragger->size().height() + d->dragTop + d->dragBottom)/2 -
                            d->collapseIcon->size().height()/2 + d->bgTop);
    connect(d->collapseIcon, SIGNAL(clicked()), this, SLOT(toggleCollapse()));

    //Add the return to source action.
    d->returnAction = new QAction(this);
    d->returnAction->setIcon(KIcon("returntosource"));
    d->returnAction->setEnabled(true);
    d->returnAction->setVisible(true);
    connect(d->returnAction, SIGNAL(triggered()), this, SLOT(moveBackToSource()));
    addAction("returntosource", d->returnAction);

    //set the extender we want to move to.
    setExtender(hostExtender);

    setAcceptHoverEvents(true);

    d->updateToolBox();
    updateGeometry();

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeChanged()));
}

ExtenderItem::~ExtenderItem()
{
    delete d;
}

KConfigGroup ExtenderItem::config() const
{
    KConfigGroup cg = d->extender->d->applet->config("ExtenderItems");
    return KConfigGroup(&cg, QString::number(d->extenderItemId));
}

void ExtenderItem::setTitle(const QString &title)
{
    d->title = title;
    update();
}

QString ExtenderItem::title() const
{
    return d->title;
}

void ExtenderItem::setName(const QString &name)
{
    d->name = name;
    config().writeEntry("extenderItemName", name);
}

QString ExtenderItem::name() const
{
    return d->name;
}

void ExtenderItem::setWidget(QGraphicsItem *widget)
{
    widget->setParentItem(this);
    widget->setPos(QPointF(d->bgLeft + d->dragLeft, d->dragHandleRect().height() +
                                                    d->bgTop + d->dragTop));
    d->widget = widget;
    setCollapsed(isCollapsed()); //updates the size hints.
}

QGraphicsItem *ExtenderItem::widget() const
{
    return d->widget;
}

void ExtenderItem::setIcon(const QIcon &icon)
{
    d->collapseIcon->setIcon(icon);
}

void ExtenderItem::setIcon(const QString &icon)
{
    d->collapseIcon->setIcon(icon);
}

QIcon ExtenderItem::icon() const
{
    return d->collapseIcon->icon();
}

void ExtenderItem::setExtender(Extender *extender, const QPointF &pos)
{
    Q_ASSERT(extender);

    if (extender == d->extender) {
        //We're not moving between extenders, so just insert this item back into the layout.
        setParentItem(extender);
        extender->d->addExtenderItem(this, pos);
        return;
    }

    //We are switching extender...
    //first remove this item from the old extender.
    d->extender->d->removeExtenderItem(this);
    emit d->extender->itemDetached(this);

    //collapse the popupapplet if the last item is removed.
    if (!d->extender->attachedItems().count()) {
        PopupApplet *applet = qobject_cast<PopupApplet*>(d->extender->d->applet);
        if (applet) {
            applet->hidePopup();
        }
    }

    //move the configuration.
    if (d->hostApplet() && (extender != d->extender)) {
        KConfigGroup c = extender->d->applet->config("ExtenderItems");
        config().reparent(&c);
    }

    d->extender = extender;

    //change parent.
    setParentItem(extender);
    extender->d->addExtenderItem(this, pos);

    //cancel the timer.
    if (d->expirationTimer && isDetached()) {
        d->expirationTimer->stop();
        delete d->expirationTimer;
        d->expirationTimer = 0;
    }
}

Extender *ExtenderItem::extender() const
{
    return d->extender;
}

bool ExtenderItem::isCollapsed() const
{
    if (!d->widget) {
        return true;
    } else {
        return !d->widget->isVisible();
    }
}

void ExtenderItem::setAutoExpireDelay(uint time)
{
    if (!time) {
        if (d->expirationTimer) {
            d->expirationTimer->stop();
            delete d->expirationTimer;
            d->expirationTimer = 0;
        }
        return;
    }

    if (!isDetached()) {
        if (!d->expirationTimer) {
            d->expirationTimer = new QTimer(this);
            connect(d->expirationTimer, SIGNAL(timeout()), this, SLOT(destroy()));
        }

        d->expirationTimer->stop();
        d->expirationTimer->setSingleShot(true);
        d->expirationTimer->setInterval(time);
        d->expirationTimer->start();
    }
}

bool ExtenderItem::autoExpireDelay() const
{
    if (d->expirationTimer) {
        return d->expirationTimer->interval();
    } else {
        return 0;
    }
}

bool ExtenderItem::isDetached() const
{
    if (d->hostApplet()) {
        return (sourceAppletId() != d->hostApplet()->id());
    } else {
        return false;
    }
}

void ExtenderItem::addAction(const QString &name, QAction *action)
{
    Q_ASSERT(action);

    d->actions[name] = action;
    connect(action, SIGNAL(changed()), this, SLOT(updateToolBox()));
    d->updateToolBox();
}

QAction *ExtenderItem::action(const QString &name) const
{
    if (d->actions.contains(name)) {
        return d->actions[name];
    } else {
        return 0;
    }
}

uint ExtenderItem::sourceAppletId() const
{
    return d->sourceAppletId;
}

void ExtenderItem::destroy()
{
    if (d->mousePressed) {
        //avoid being destroyed while we're being dragged.
        return;
    }

    d->hostApplet()->config("ExtenderItems").deleteGroup(QString::number(d->extenderItemId));
    d->extender->d->removeExtenderItem(this);
    deleteLater();
}

void ExtenderItem::setCollapsed(bool collapsed)
{
    qreal marginWidth = d->bgLeft + d->bgRight + d->dragLeft + d->dragRight;
    qreal marginHeight = d->bgTop + d->bgBottom + 2*d->dragTop + 2*d->dragBottom;

    if (!d->widget) {
        setPreferredSize(QSizeF(200, d->dragHandleRect().height()));
        setMinimumSize(QSizeF(0, d->dragHandleRect().height()));
        //FIXME: wasn't there some sort of QWIDGETMAXSIZE thingy?
        setMaximumSize(QSizeF(1000, d->dragHandleRect().height()));
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        updateGeometry();
        return;
    }

    d->widget->setVisible(!collapsed);

    QSizeF minimumSize;
    QSizeF preferredSize;
    QSizeF maximumSize;

    if (d->widget->isWidget()) {
        QGraphicsWidget *graphicsWidget = static_cast<QGraphicsWidget*>(d->widget);
        minimumSize = graphicsWidget->minimumSize();
        preferredSize = graphicsWidget->preferredSize();
        maximumSize = graphicsWidget->maximumSize();
    } else {
        minimumSize = d->widget->boundingRect().size();
        preferredSize = d->widget->boundingRect().size();
        maximumSize = d->widget->boundingRect().size();
    }

    if (collapsed) {
        setPreferredSize(QSizeF(preferredSize.width() + marginWidth,
                                d->dragHandleRect().height() + marginHeight));
        setMinimumSize(QSizeF(minimumSize.width() + marginWidth,
                              d->dragHandleRect().height() + marginHeight));
        setMaximumSize(QSizeF(maximumSize.width() + marginWidth,
                              d->dragHandleRect().height() + marginHeight));
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        if (d->collapseIcon) {
            d->collapseIcon->setToolTip(i18n("Expand this widget"));
        }
    } else {
        setPreferredSize(QSizeF(preferredSize.width() + marginWidth,
                         preferredSize.height() + d->dragHandleRect().height() + marginHeight));
        setMinimumSize(  QSizeF(minimumSize.width() + marginWidth,
                         minimumSize.height() + d->dragHandleRect().height() + marginHeight));
        setMaximumSize(  QSizeF(maximumSize.width() + marginWidth,
                         maximumSize.height() + d->dragHandleRect().height() + marginHeight));

        if (d->widget->isWidget()) {
            QGraphicsWidget *graphicsWidget = static_cast<QGraphicsWidget*>(d->widget);
            setSizePolicy(graphicsWidget->sizePolicy());
        } else {
            setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        }

        if (d->collapseIcon) {
            d->collapseIcon->setToolTip(i18n("Collapse this widget"));
        }
    }

    updateGeometry();

    d->extender->d->adjustSizeHints();
}

void ExtenderItem::moveBackToSource()
{
    if (!d->sourceApplet) {
        return;
    }
    setExtender(d->sourceApplet->d->extender);
}

void ExtenderItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                         QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::TextAntialiasing, true);
    painter->setRenderHint(QPainter::Antialiasing, true);

    d->background->paintPanel(painter);

    d->dragger->paintPanel(painter, QPointF(d->bgLeft, d->bgTop));

    //draw the title.
    Plasma::Theme *theme = Plasma::Theme::defaultTheme();
    QFont font = theme->font(Plasma::Theme::DefaultFont);
    font.setPointSize(KGlobalSettings::smallestReadableFont().pointSize());
    font.setWeight(QFont::Bold);

    //XXX: duplicated from windowtaskitem.
    //TODO: hmm, create something generic for this... there's probably more stuff that wants to have
    //this functionality
    QRectF rect = QRectF(d->titleRect().width() - 30, 0, 30, d->titleRect().height());

    QPixmap pixmap(d->titleRect().size().toSize());
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.setPen(theme->color(Plasma::Theme::TextColor));
    p.setFont(font);
    p.drawText(QRectF(QPointF(0, 0), d->titleRect().size()),
               Qt::TextSingleLine | Qt::AlignVCenter | Qt::AlignLeft,
               d->title);

    // Create the alpha gradient for the fade out effect
    QLinearGradient alphaGradient(0, 0, 1, 0);
    alphaGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    //TODO: correct handling of right to left text.
    alphaGradient.setColorAt(0, QColor(0, 0, 0, 255));
    alphaGradient.setColorAt(1, QColor(0, 0, 0, 0));

    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.fillRect(rect, alphaGradient);

    p.end();

    painter->drawPixmap(d->titleRect().topLeft(), pixmap);
}

void ExtenderItem::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    qreal width = event->newSize().width();
    qreal height = event->newSize().height();

    //resize the dragger
    QSizeF newDraggerSize(width - d->bgLeft - d->bgRight,
                          d->dragger->elementSize("hint-preferred-icon-size").height() +
                          d->dragTop + d->dragBottom);
    d->dragger->resizePanel(newDraggerSize);

    //resize the applet background
    d->background->resizePanel(event->newSize());

    //resize the widget
    if (d->widget && d->widget->isWidget()) {
        QSizeF newWidgetSize(width - d->bgLeft - d->bgRight - d->dragLeft - d->dragRight,
                             height - d->dragger->size().height() - d->bgTop - d->bgBottom
                                    - 2*d->dragTop - 2*d->dragBottom);

        QGraphicsWidget *graphicsWidget = static_cast<QGraphicsWidget*>(d->widget);
        graphicsWidget->resize(newWidgetSize);
    }

    d->updateToolBox();
}

void ExtenderItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!(d->dragHandleRect().contains(event->pos()))) {
        event->ignore();
        return;
    }

    d->mousePressed = true;
    d->deltaScene = pos();

    Applet *parentApplet = d->hostApplet();

    d->mousePos = event->pos().toPoint();

    parentApplet->raise();
    setZValue(parentApplet->zValue());

    QPointF mousePos = d->scenePosFromScreenPos(event->screenPos());

    if (!mousePos.isNull()) {
        d->extender->itemHoverMoveEvent(this, d->extender->mapFromScene(mousePos));
    }

    d->extender->d->removeExtenderItem(this);
    QApplication::setOverrideCursor(Qt::ClosedHandCursor);
}

void ExtenderItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!d->mousePressed) {
        return;
    }

    //keep track of the movement in scene coordinates. we use this to set the position of the
    //applet when remaining in the current view.
    d->deltaScene += event->scenePos() - event->lastScenePos();

    //set a rect in screencoordinates so we can check when we need to move to a toplevel
    //view.
    QRect screenRect = QRect();
    screenRect.setTopLeft(event->screenPos() - d->mousePos);
    screenRect.setSize(d->screenRect().size());

    Corona *corona = d->hostApplet()->containment()->corona();

    if (d->leaveCurrentView(screenRect)) {
        //we're moving the applet to a toplevel view, so place it somewhere out of sight
        //first: in the topleft quadrant.

        if (!d->toplevel) {
            //FIXME: duplication from applethandle
            //create a toplevel view and aim it at the applet.
            d->toplevel = new QGraphicsView(corona, 0);

            corona->addOffscreenWidget(this);

            d->toplevel->setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint
                                                    | Qt::WindowStaysOnTopHint);
            d->toplevel->setFrameShape(QFrame::NoFrame);
            d->toplevel->resize(screenRect.size());
            d->toplevel->setSceneRect(sceneBoundingRect());
            d->toplevel->centerOn(this);

            //We might have to scale the view, because we might be zoomed out.
            qreal scale = screenRect.width() / size().width();
            d->toplevel->scale(scale, scale);

            d->toplevel->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            d->toplevel->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

            d->toplevel->setMask(d->background->mask());

            d->toplevel->update();
            d->toplevel->show();
        }

        //move the toplevel view.
        d->toplevel->setSceneRect(sceneBoundingRect());
        d->toplevel->setGeometry(screenRect);
        update();
    } else {
        corona->removeOffscreenWidget(this);
        setParentItem(d->hostApplet());
        setPos(d->deltaScene);

       //remove the toplevel view.
       if (d->toplevel) {
            delete d->toplevel;
            d->toplevel = 0;
        }
    }

    //let's insert spacers in applets we're hovering over for some usefull visual feedback.
    //check which view we're hovering over and use that information to get the mouse
    //position in scene coordinates (event->scenePos won't work, since it doesn't take in
    //consideration that you're leaving the current view).
    QPointF mousePos = d->scenePosFromScreenPos(event->screenPos());

    //find the extender we're hovering over.
    Extender *targetExtender = 0;

    if (!mousePos.isNull()) {
        foreach (Containment *containment, corona->containments()) {
            foreach (Applet *applet, containment->applets()) {
                if (applet->d->extender &&
                        (applet->sceneBoundingRect().contains(mousePos) ||
                         applet->d->extender->sceneBoundingRect().contains(mousePos))) {
                    targetExtender = applet->d->extender;

                    //check if we're hovering over an popupapplet, and open it up in case it does.
                    PopupApplet *popupApplet = qobject_cast<PopupApplet*>(applet);
                    if (popupApplet && (applet->formFactor() == Plasma::Horizontal ||
                                applet->formFactor() == Plasma::Vertical)) {
                        popupApplet->showPopup();
                    }
                }
            }
        }
    }

    //remove any previous spacers.
    if (targetExtender != d->previousTargetExtender) {
        if (d->previousTargetExtender) {
            d->previousTargetExtender->itemHoverLeaveEvent(this);
        }
        d->previousTargetExtender = targetExtender;
        if (targetExtender) {
            targetExtender->itemHoverEnterEvent(this);
        }
    }

    //insert a spacer if the applet accepts detachables.
    if (targetExtender) {
        if (targetExtender->sceneBoundingRect().contains(mousePos)) {
            targetExtender->itemHoverMoveEvent(this, targetExtender->mapFromScene(mousePos));
        }
    }
}

void ExtenderItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (d->titleRect().contains(event->pos())) {
        if (!d->mouseOver) {
            QApplication::setOverrideCursor(Qt::OpenHandCursor);
            d->mouseOver = true;
        }
    } else {
        if (d->mouseOver) {
            QApplication::restoreOverrideCursor();
            d->mouseOver = false;
        }
    }
}

void ExtenderItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);

    if (d->mouseOver) {
        QApplication::restoreOverrideCursor();
        d->mouseOver = false;
    }
}

void ExtenderItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (d->mousePressed) {
        d->mousePressed = false;

        //remove the toplevel view
        if (d->toplevel) {
            delete d->toplevel;
            d->toplevel = 0;
        }

        //let's insert spacers in applets we're hovering over for some usefull visual feedback.
        //check which view we're hovering over and use that information to get the mouse
        //position in scene coordinates (event->scenePos won't work, since it doesn't take in
        //consideration that you're leaving the current view).
        QPointF mousePos = d->scenePosFromScreenPos(event->screenPos());

        //find the extender we're hovering over.
        Extender *targetExtender = 0;
        Corona *corona = qobject_cast<Corona*>(scene());

        corona->removeOffscreenWidget(this);

        if (!mousePos.isNull()) {
            foreach (Containment *containment, corona->containments()) {
                foreach (Applet *applet, containment->applets()) {
                    if (applet->d->extender &&
                        (applet->sceneBoundingRect().contains(mousePos) ||
                         applet->d->extender->sceneBoundingRect().contains(mousePos))) {
                        targetExtender = applet->d->extender;
                    }
                }
            }
        }

        //are we hovering over an applet that accepts extender items?
        if (targetExtender) {
            if (targetExtender->sceneBoundingRect().contains(mousePos)) {
                setExtender(targetExtender, targetExtender->mapFromScene(mousePos));
            } else {
                setExtender(targetExtender);
            }
        } else {
            //apparently, it is not, so instantiate a new ExtenderApplet.
            //TODO: maybe we alow the user to choose a default extenderapplet.
            kDebug() << "Instantiate a new ExtenderApplet";
            mousePos = d->scenePosFromScreenPos(event->screenPos() - d->mousePos);
            if (!mousePos.isNull()) {
                foreach (Containment *containment, corona->containments()) {
                    if (containment->sceneBoundingRect().contains(mousePos)) {
                        Applet *applet = containment->addApplet("internal:extender",
                                QVariantList(),
                                QRectF(mousePos, size()));
                        setExtender(applet->d->extender);
                    }
                }
            }
        }

        QApplication::restoreOverrideCursor();
    }
}

} // namespace Plasma

#include "extenderitem.moc"
