/*
 * Copyright 2008, 2009 by Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
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

#include <QAction>
#include <QApplication>
#include <QBitmap>
#include <QDrag>
#include <QGraphicsSceneResizeEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLinearLayout>
#include <QLayout>
#include <QMimeData>
#include <QPainter>
#include <QTimer>

#include <kdebug.h>
#include <kicon.h>
#include <kiconloader.h>

#include "applet.h"
#include "containment.h"
#include "corona.h"
#include "dialog.h"
#include "extender.h"
#include "extendergroup.h"
#include "framesvg.h"
#include "popupapplet.h"
#include "theme.h"
#include "view.h"

#include "widgets/iconwidget.h"

#include "private/applethandle_p.h"
#include "private/extender_p.h"
#include "private/extenderapplet_p.h"
#include "private/extendergroup_p.h"
#include "private/extenderitem_p.h"
#include "private/extenderitemmimedata_p.h"
#include "widgets/label.h"

namespace Plasma
{

class ExtenderItemToolbox : public QGraphicsWidget
{
public:
    ExtenderItemToolbox(QGraphicsWidget *parent)
        : QGraphicsWidget(parent),
          m_background(new FrameSvg(this))
    {
        m_background->setImagePath("widgets/extender-dragger");
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        updateTheme();
    }

    qreal iconSize()
    {
        return m_iconSize;
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
    {
        m_background->paintFrame(painter, option->exposedRect, option->exposedRect);
    }

    void updateTheme()
    {
        //Read the preferred icon size hint, look at the font size, and calculate the desired title bar
        //icon height.
        m_background->resize();
        QSizeF size = m_background->elementSize("hint-preferred-icon-size");
        size = size.expandedTo(QSizeF(KIconLoader::SizeSmall,KIconLoader::SizeSmall));

        Plasma::Theme *theme = Plasma::Theme::defaultTheme();
        QFont font = theme->font(Plasma::Theme::DefaultFont);
        QFontMetrics fm(font);
        m_iconSize = qMax(size.height(), (qreal) fm.height());
    }

protected:
    void resizeEvent(QGraphicsSceneResizeEvent *)
    {
        m_background->resizeFrame(size());
        qreal left, top, right, bottom;
        m_background->getMargins(left, top, right, bottom);
        setContentsMargins(0, top, 0, bottom);
    }

private:
    FrameSvg *m_background;
    qreal m_iconSize;
};

ExtenderItem::ExtenderItem(Extender *hostExtender, uint extenderItemId)
        : QGraphicsWidget(hostExtender),
          d(new ExtenderItemPrivate(this, hostExtender))
{
    Q_ASSERT(hostExtender);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    //set the extenderId
    if (extenderItemId) {
        d->extenderItemId = extenderItemId;
        ExtenderItemPrivate::s_maxExtenderItemId =
            qMax(ExtenderItemPrivate::s_maxExtenderItemId, extenderItemId);
    } else {
        d->extenderItemId = ++ExtenderItemPrivate::s_maxExtenderItemId;
    }

    //create the toolbox.
    d->toolbox = new ExtenderItemToolbox(this);
    d->toolboxLayout = new QGraphicsLinearLayout(d->toolbox);

    //create items's configgroup
    KConfigGroup cg = hostExtender->d->applet.data()->config("ExtenderItems");
    KConfigGroup dg = KConfigGroup(&cg, QString::number(d->extenderItemId));

    //create own layout
    d->layout = new QGraphicsLinearLayout(Qt::Vertical, this);
    d->layout->addItem(d->toolbox);

    uint sourceAppletId = dg.readEntry("sourceAppletId", 0);

    //check if we're creating a new item or reinstantiating an existing one.
    d->collapseIcon = new IconWidget(d->toolbox);
    d->collapseIcon->setCursor(Qt::ArrowCursor);
    d->titleLabel = new Label(d->toolbox);

    d->toolboxLayout->addItem(d->collapseIcon);
    d->toolboxLayout->addItem(d->titleLabel);
    d->toolboxLayout->setStretchFactor(d->titleLabel, 10);

    if (!sourceAppletId) {
        //The item is new
        dg.writeEntry("sourceAppletPluginName", hostExtender->d->applet.data()->pluginName());
        dg.writeEntry("sourceAppletId", hostExtender->d->applet.data()->id());
        dg.writeEntry("extenderIconName", hostExtender->d->applet.data()->icon());
        d->sourceApplet = hostExtender->d->applet.data();
        d->collapseIcon->setIcon(KIcon(hostExtender->d->applet.data()->icon()));
    } else {
        //The item already exists.
        d->name = dg.readEntry("extenderItemName", "");
        d->titleLabel->setText(dg.readEntry("extenderTitle", ""));
        setCollapsed(dg.readEntry("isCollapsed", false));

        QString iconName = dg.readEntry("extenderIconName", "utilities-desktop-extra");
        if (iconName.isEmpty()) {
            iconName = "utilities-desktop-extra";
        }
        d->collapseIcon->setIcon(iconName);

        //Find the group if it's already there.
        QString groupName = dg.readEntry("group", "");
        d->group = hostExtender->d->findGroup(groupName);

        //Find the sourceapplet.
        Corona *corona = hostExtender->d->applet.data()->containment()->corona();
        foreach (Containment *containment, corona->containments()) {
            foreach (Applet *applet, containment->applets()) {
                if (applet->id() == sourceAppletId &&
                        applet->pluginName() == dg.readEntry("sourceAppletPluginName", "")) {
                    d->sourceApplet = applet;
                }
            }
        }
    }

    //make sure we keep monitoring if the source applet still exists, so the return to source icon
    //can be hidden if it is removed.
    if (d->sourceApplet) {
        connect(d->sourceApplet, SIGNAL(destroyed()), this, SLOT(sourceAppletRemoved()));
    }

    connect(d->collapseIcon, SIGNAL(clicked()), this, SLOT(toggleCollapse()));

    //set the extender we want to move to.
    setExtender(hostExtender);

    //set the image paths, image sizes
    d->themeChanged();

    //show or hide the toolbox interface itmems
    d->updateToolBox();

    setAcceptsHoverEvents(true);

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeChanged()));
    d->setMovable(d->extender->d->applet.data()->immutability() == Plasma::Mutable);
}

ExtenderItem::~ExtenderItem()
{
    emit destroyed(this);
    delete d;
}

KConfigGroup ExtenderItem::config() const
{
    if (!d->extender->d->applet) {
        return KConfigGroup();
    }
    KConfigGroup cg = d->extender->d->applet.data()->config("ExtenderItems");
    return KConfigGroup(&cg, QString::number(d->extenderItemId));
}

void ExtenderItem::setTitle(const QString &title)
{
    if (d->titleLabel->text() != title) {
        d->titleLabel->setText(title);
        config().writeEntry("extenderTitle", title);
        update();
    }
}

QString ExtenderItem::title() const
{
    return d->titleLabel->text();
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
    if (d->widget) {
        d->widget->removeSceneEventFilter(this);
        d->layout->removeItem(d->widget);
    }

    if (!widget || !widget->isWidget()) {
        d->widget = 0;
        return;
    }

    widget->setParentItem(this);
    d->widget = static_cast<QGraphicsWidget *>(widget);
    d->layout->insertItem(1, d->widget);
    d->widget->setVisible(!d->collapsed);
}

QGraphicsItem *ExtenderItem::widget() const
{
    return d->widget;
}

void ExtenderItem::setIcon(const QIcon &icon)
{
    if (d->collapseIcon->icon().isNull() || icon.cacheKey() != d->collapseIcon->icon().cacheKey()) {
        d->iconName.clear();
        d->collapseIcon->setIcon(icon);
    }
}

void ExtenderItem::setIcon(const QString &icon)
{
    if (icon != d->iconName) {
        d->collapseIcon->setIcon(icon);
        d->iconName = icon;
        config().writeEntry("extenderIconName", icon);
    }
}

QIcon ExtenderItem::icon() const
{
    return d->collapseIcon->icon();
}

void ExtenderItem::setExtender(Extender *extender, const QPointF &pos)
{
    Q_ASSERT(extender);

    //themeChanged() has to now that by now, we're no longer dragging, even though the QDrag has not
    //been entirely finished.
    d->dragStarted = false;

    ExtenderGroup *group = qobject_cast<ExtenderGroup*>(this);
    QList<ExtenderItem*> childItems;
    if (group) {
        childItems = group->items();
    }

    if (extender == d->extender) {
        //We're not moving between extenders, so just insert this item back into the layout.
        setParentItem(extender);
        extender->d->addExtenderItem(this, pos);
        return;
    }

    //We are switching extender...
    //first remove this item from the old extender.
    d->extender->d->removeExtenderItem(this);

    //move the configuration.
    if (d->hostApplet() && (extender != d->extender)) {
        KConfigGroup c = extender->d->applet.data()->config("ExtenderItems");
        config().reparent(&c);
    }

    //and notify the applet of the item being detached, after the config has been moved.
    emit d->extender->itemDetached(this);

    setParentItem(extender);
    setParent(extender);
    if (d->extender) {
        disconnect(d->extender->applet(), SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)), this, SLOT(updateToolBox()));
    }
    d->extender = extender;
    connect(d->extender->applet(), SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)), this, SLOT(updateToolBox()));

    //change parent.
    extender->d->addExtenderItem(this, pos);

    //cancel the timer.
    if (d->expirationTimer && isDetached()) {
        d->expirationTimer->stop();
        delete d->expirationTimer;
        d->expirationTimer = 0;
    }

    Corona *corona = qobject_cast<Corona*>(scene());
    KConfigGroup extenderItemGroup(corona->config(), "DetachedExtenderItems");

    if (isDetached()) {
        kDebug() << "detached, adding entry to the global group";
        KConfigGroup itemConfig = extenderItemGroup.group(QString::number(d->extenderItemId));
        itemConfig.writeEntry("sourceAppletPluginName",
                config().readEntry("sourceAppletPluginName", ""));
        itemConfig.writeEntry("sourceAppletId",
                config().readEntry("sourceAppletId", 0));
        itemConfig.writeEntry("extenderItemName",
                config().readEntry("extenderItemName", ""));
    } else if (extenderItemGroup.hasGroup(QString::number(d->extenderItemId))) {
        kDebug() << "no longer detached, removing entry from the global group";
        extenderItemGroup.deleteGroup(QString::number(d->extenderItemId));
    }

    //we might have to enable or disable the returnToSource button.
    d->updateToolBox();

    //invoke setGroup on all items belonging to this group, to make sure all children move to the
    //new extender together with the group.
    if (group) {
        foreach (ExtenderItem *item, childItems) {
            item->setGroup(group);
        }
    }
}

Extender *ExtenderItem::extender() const
{
    return d->extender;
}

//TODO KDE5: only one setGroup()
void ExtenderItem::setGroup(ExtenderGroup *group)
{
    setGroup(group, QPointF(-1, -1));
}

void ExtenderItem::setGroup(ExtenderGroup *group, const QPointF &pos)
{
    if (isGroup()) {
        //nesting extender groups is just insane. I don't think we'd even want to support that.
        kWarning() << "Nesting ExtenderGroups is not supported";
        return;
    }

    ExtenderGroup *oldGroup = d->group;
    d->group = group;

    if (group) {
        config().writeEntry("group", group->name());
        //TODO: move to another extender if the group we set is actually detached.
        if (group->extender() != extender()) {
            kDebug() << "moving to another extender because we're joining a detached group.";
            setExtender(group->extender());
        }
        group->d->addItemToGroup(this, pos);
    } else {
        if (oldGroup) {
            oldGroup->d->removeItemFromGroup(this);
        }
        config().deleteEntry("group");
    }
}

ExtenderGroup *ExtenderItem::group() const
{
    return d->group;
}

bool ExtenderItem::isGroup() const
{
    return (config().readEntry("isGroup", false) && qobject_cast<const Plasma::ExtenderGroup *>(this));
}

bool ExtenderItem::isCollapsed() const
{
    return d->collapsed;
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

uint ExtenderItem::autoExpireDelay() const
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
        return (d->sourceApplet != d->hostApplet());
    } else {
        return false;
    }
}

void ExtenderItem::addAction(const QString &name, QAction *action)
{
    Q_ASSERT(action);
    if (d->actionsInOrder.contains(action)) {
        return;
    }

    d->actions.insert(name, action);
    d->actionsInOrder.append(action);
    connect(action, SIGNAL(changed()), this, SLOT(updateToolBox()));
    connect(action, SIGNAL(destroyed(QObject*)), this, SLOT(actionDestroyed(QObject*)));
    d->updateToolBox();
}

QAction *ExtenderItem::action(const QString &name) const
{
    return d->actions.value(name, 0);
}

void ExtenderItem::showCloseButton()
{
    if (d->destroyActionVisibility) {
        return;
    }

    d->destroyActionVisibility = true;
    d->updateToolBox();
}

void ExtenderItem::hideCloseButton()
{
    if (!d->destroyActionVisibility) {
        return;
    }

    d->destroyActionVisibility = false;
    d->updateToolBox();
}

void ExtenderItem::destroy()
{
    if (d->dragStarted) {
        //avoid being destroyed while we're being dragged.
        return;
    }

    //remove global entry if needed.
    Corona *corona = qobject_cast<Corona*>(scene());
    KConfigGroup extenderItemGroup(corona->config(), "DetachedExtenderItems");
    if (extenderItemGroup.hasGroup(QString::number(d->extenderItemId))) {
        extenderItemGroup.deleteGroup(QString::number(d->extenderItemId));
    }

    d->hostApplet()->config("ExtenderItems").deleteGroup(QString::number(d->extenderItemId));
    d->extender->d->removeExtenderItem(this);
    emit d->extender->itemDetached(this);

    deleteLater();
}

void ExtenderItem::setCollapsed(bool collapsed)
{
    config().writeEntry("isCollapsed", collapsed);
    d->collapsed = collapsed;
    d->collapseIcon->setToolTip(collapsed ? i18n("Expand this widget") : i18n("Collapse this widget"));
    if (d->widget) {
        d->widget->setVisible(!collapsed);
        if (collapsed) {
            d->layout->removeItem(d->widget);
        } else {
            d->layout->insertItem(1, d->widget);
        }
        updateGeometry();
        if (extender()) {
            extender()->resize(extender()->effectiveSizeHint(Qt::PreferredSize));
        }
    }
}

void ExtenderItem::returnToSource()
{
    if (!d || !d->sourceApplet) {
        return;
    }

    if (d->sourceApplet->d) {
        setExtender(d->sourceApplet->extender());
    }
}

void ExtenderItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
    if (d->background->enabledBorders() != (FrameSvg::LeftBorder | FrameSvg::RightBorder) &&
        d->background->enabledBorders()) {
        //Don't paint if only the left and right borders are enabled, we only use the left and right
        //border in this situation to set the correct margins on this item.
        d->background->paintFrame(painter, option->exposedRect, option->exposedRect);
    }
}

void ExtenderItem::moveEvent(QGraphicsSceneMoveEvent *event)
{
    Q_UNUSED(event)
    //not needed anymore, but here for binary compatibility
}

void ExtenderItem::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_UNUSED(event)
    //resize the applet background
    d->background->resizeFrame(size());
    //d->resizeContent(size());
}

void ExtenderItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!(d->dragHandleRect().contains(event->pos())) ||
        d->extender->d->applet.data()->immutability() != Plasma::Mutable) {
        event->ignore();
        return;
    }
}

void ExtenderItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPoint mousePressPos = event->buttonDownPos(Qt::LeftButton).toPoint();
    if (!(event->buttons() & Qt::LeftButton) ||
        (event->pos().toPoint() - mousePressPos).manhattanLength()
        < QApplication::startDragDistance()) {
        return;
    }

    if (!d->extender->d->applet) {
        return;
    }

    //Start the drag:
    d->dragStarted = true;
    QPointF curPos = pos();

    //remove item from the layout, and add it somewhere off screen so we can render it to a pixmap,
    //without other widgets interefing.
    d->extender->itemRemovedEvent(this);
    Corona *corona = qobject_cast<Corona*>(scene());
    corona->addOffscreenWidget(this);

    //update the borders, since while dragging, we want all of theme.
    d->themeChanged();

    //create a view to render the ExtenderItem and it's contents to a pixmap and set up a painter on
    //a pixmap.
    QGraphicsView view(scene());
    QSize screenSize(view.mapFromScene(sceneBoundingRect()).boundingRect().size());
    QPixmap pixmap(screenSize);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);

    //the following is necesarry to avoid having an offset when rendering the widget into the
    //pixmap.
    view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view.setFrameShape(QFrame::NoFrame);

    //aim the view and render.
    view.resize(screenSize);
    view.setSceneRect(sceneBoundingRect());
    view.render(&p, QRectF(QPointF(0, 0), pixmap.size()), QRect(QPoint(0, 0), screenSize));

    //create the necesarry mimedata.
    ExtenderItemMimeData *mimeData = new ExtenderItemMimeData();
    mimeData->setExtenderItem(this);
    mimeData->setPointerOffset(mousePressPos);

    //Hide empty internal extender containers when we drag the last item away. Avoids having
    //an ugly empty applet on the desktop temporarily.
    ExtenderApplet *extenderApplet = qobject_cast<ExtenderApplet*>(d->extender->d->applet.data());
    if (extenderApplet && d->extender->attachedItems().count() < 2 &&
        extenderApplet->formFactor() != Plasma::Horizontal &&
        extenderApplet->formFactor() != Plasma::Vertical) {
        kDebug() << "leaving the internal extender container, so hide the applet and it's handle.";
        extenderApplet->hide();
    }

    ExtenderGroup *group = qobject_cast<ExtenderGroup*>(this);
    bool collapsedGroup = false;
    if (isGroup()) {
        collapsedGroup = group->d->collapsed;
        group->collapseGroup();
    }

    if (!isGroup() && this->group()) {
        setGroup(0);
    }

    //and execute the drag.
    QWidget *dragParent = extender()->d->applet.data()->view();
    QDrag *drag = new QDrag(dragParent);
    drag->setPixmap(pixmap);
    drag->setMimeData(mimeData);
    drag->setHotSpot(mousePressPos);

    Qt::DropAction action = drag->exec();

    corona->removeOffscreenWidget(this);
    d->dragStarted = false;

    if (!action || !drag->target()) {
        //we weren't moved, so reinsert the item in our current layout.
        //TODO: make it into a stand-alone window?
        d->themeChanged();
        d->extender->itemAddedEvent(this, curPos);
        if (extenderApplet) {
            extenderApplet->show();
        }
    }

    if (isGroup() && !collapsedGroup) {
        group->expandGroup();
    }
}

void ExtenderItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (d->dragHandleRect().contains(event->pos())) {
        d->toggleCollapse();
    }
}

bool ExtenderItem::sceneEventFilter(QGraphicsItem *, QEvent *)
{
    return false;
}

void ExtenderItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    //not needed anymore, but here for binary compatibility
}

void ExtenderItem::hoverMoveEvent(QGraphicsSceneHoverEvent *)
{
}

void ExtenderItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
}

QSizeF ExtenderItem::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    return QGraphicsWidget::sizeHint(which, constraint);
}

ExtenderItemPrivate::ExtenderItemPrivate(ExtenderItem *extenderItem, Extender *hostExtender)
    : q(extenderItem),
      widget(0),
      toolbox(0),
      extender(hostExtender),
      sourceApplet(0),
      group(0),
      background(new FrameSvg(extenderItem)),
      collapseIcon(0),
      dragStarted(false),
      destroyActionVisibility(false),
      collapsed(false),
      expirationTimer(0)
{
}

ExtenderItemPrivate::~ExtenderItemPrivate()
{
    delete widget;
    widget = 0;
}

//returns a Rect containing the area of the detachable where the draghandle will be drawn.
QRectF ExtenderItemPrivate::dragHandleRect()
{
    return toolbox->boundingRect();
}

void ExtenderItemPrivate::toggleCollapse()
{
    q->setCollapsed(!q->isCollapsed());
}

void ExtenderItemPrivate::updateToolBox()
{
    Q_ASSERT(toolbox);
    Q_ASSERT(toolboxLayout);

    QAction *closeAction = actions.value("close");
    QAction *returnToSourceAction = actions.value("extenderItemReturnToSource");
    bool returnToSourceVisibility = q->isDetached() && sourceApplet && (hostApplet()->immutability() == Plasma::Mutable);
    int closeIndex = -1;
    int returnToSourceIndex = -1;
    const int startingIndex = 2; // collapse item is index 0, title label is 1
    const QSizeF widgetSize = collapseIcon->sizeFromIconSize(toolbox->iconSize());

    QSet<QAction*> shownActions = actionsInOrder.toSet();
    QHash<QAction *, IconWidget *> actionIcons;
    for (int index = startingIndex; index < toolboxLayout->count(); ++index) {
        IconWidget *widget = dynamic_cast<IconWidget*>(toolboxLayout->itemAt(index));

        if (!widget) {
            continue;
        }

        if (closeIndex == -1 && destroyActionVisibility &&
            closeAction && widget->action() == closeAction) {
            closeIndex = index;
            continue;
        }

        if (returnToSourceIndex == -1 && returnToSourceVisibility &&
            returnToSourceAction && widget->action() == returnToSourceAction) {
            returnToSourceIndex = index;
            continue;
        }

        if (shownActions.contains(widget->action())) {
            actionIcons.insert(widget->action(), widget);
            continue;
        }

        toolboxLayout->removeAt(index);
        widget->deleteLater();
    }

    // ensure the collapseIcon is the correct size.
    collapseIcon->setMinimumSize(widgetSize);
    collapseIcon->setMaximumSize(widgetSize);

    //add the actions that are actually set to visible.
    foreach (QAction *action, actionsInOrder) {
        if (action->isVisible() && action != closeAction) {
            IconWidget *icon = actionIcons.value(action);
            if (!icon) {
                icon = new IconWidget(q);
                icon->setAction(action);
            }

            icon->setMinimumSize(widgetSize);
            icon->setMaximumSize(widgetSize);
            icon->setCursor(Qt::ArrowCursor);
            toolboxLayout->insertItem(startingIndex, icon);
        }
    }

    //add the returntosource icon if we are detached, and have a source applet.
    if (returnToSourceVisibility && returnToSourceIndex == -1) {
        IconWidget *returnToSourceIcon = new IconWidget(q);
        if (!returnToSourceAction) {
            returnToSourceAction = new QAction(q);
            returnToSourceAction->setToolTip(i18n("Reattach"));
            actions.insert("extenderItemReturnToSource", returnToSourceAction);
            QObject::connect(returnToSourceAction, SIGNAL(triggered()), q, SLOT(returnToSource()));
        }

        returnToSourceIcon->setAction(returnToSourceAction);
        returnToSourceIcon->setSvg("widgets/configuration-icons", "return-to-source");
        returnToSourceIcon->setMinimumSize(widgetSize);
        returnToSourceIcon->setMaximumSize(widgetSize);
        returnToSourceIcon->setCursor(Qt::ArrowCursor);

        if (closeIndex == -1) {
            toolboxLayout->addItem(returnToSourceIcon);
        } else {
            toolboxLayout->insertItem(closeIndex - 1, returnToSourceIcon);
        }
    }

    //add the close icon if desired.
    if (destroyActionVisibility && closeIndex == -1) {
        IconWidget *destroyButton = new IconWidget(q);
        if (!closeAction) {
            closeAction = new QAction(q);
            actions.insert("close", closeAction);
            if (returnToSourceAction) {
                returnToSourceAction->setToolTip(i18n("Close"));
            }
            QObject::connect(closeAction, SIGNAL(triggered()), q, SLOT(destroy()));
        }

        destroyButton->setAction(closeAction);
        destroyButton->setSvg("widgets/configuration-icons", "close");
        destroyButton->setMinimumSize(widgetSize);
        destroyButton->setMaximumSize(widgetSize);
        destroyButton->setCursor(Qt::ArrowCursor);
        toolboxLayout->addItem(destroyButton);
    }
}

Applet *ExtenderItemPrivate::hostApplet() const
{
    if (extender) {
        return extender->d->applet.data();
    } else {
        return 0;
    }
}

void ExtenderItemPrivate::themeChanged()
{
    kDebug();
    if (dragStarted) {
        background->setImagePath("opaque/widgets/extender-background");
        background->setEnabledBorders(FrameSvg::AllBorders);
    } else {
        background->setImagePath("widgets/extender-background");
        background->setEnabledBorders(extender->enabledBordersForItem(q));
    }

    qreal left, top, right, bottom;
    background->getMargins(left, top, right, bottom);
    layout->setContentsMargins(left, top, right, bottom);

    toolbox->updateTheme();
}

void ExtenderItemPrivate::sourceAppletRemoved()
{
    //the original source applet is removed, set the pointer to 0 and no longer show the return to
    //source icon.
    sourceApplet = 0;
    updateToolBox();
}

void ExtenderItemPrivate::actionDestroyed(QObject *o)
{
    QAction *action = static_cast<QAction *>(o);
    QMutableHashIterator<QString, QAction *> hit(actions);
    while (hit.hasNext()) {
        if (hit.next().value() == action) {
            hit.remove();
            break;
        }
    }

    QMutableListIterator<QAction *> lit(actionsInOrder);
    while (lit.hasNext()) {
        if (lit.next() == action) {
            lit.remove();
            break;
        }
    }
}

void ExtenderItemPrivate::setMovable(bool movable)
{
    if (movable) {
        titleLabel->setCursor(Qt::OpenHandCursor);
        toolbox->setCursor(Qt::OpenHandCursor);
    } else {
        titleLabel->unsetCursor();
        toolbox->unsetCursor();
    }
}

uint ExtenderItemPrivate::s_maxExtenderItemId = 0;

} // namespace Plasma

#include "extenderitem.moc"
