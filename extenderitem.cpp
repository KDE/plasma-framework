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

#include "applet.h"
#include "containment.h"
#include "corona.h"
#include "dialog.h"
#include "extender.h"
#include "framesvg.h"
#include "popupapplet.h"
#include "theme.h"
#include "view.h"

#include "widgets/iconwidget.h"

#include "private/applethandle_p.h"
#include "private/extender_p.h"
#include "private/extenderapplet_p.h"
#include "private/extenderitem_p.h"
#include "private/extenderitemmimedata_p.h"

namespace Plasma
{

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

    //create the toolbox.
    d->toolbox = new QGraphicsWidget(this);
    d->toolboxLayout = new QGraphicsLinearLayout(d->toolbox);
    d->toolbox->setLayout(d->toolboxLayout);


    //create items's configgroup
    KConfigGroup cg = hostExtender->d->applet->config("ExtenderItems");
    KConfigGroup dg = KConfigGroup(&cg, QString::number(d->extenderItemId));

    uint sourceAppletId = dg.readEntry("sourceAppletId", 0);

    //check if we're creating a new item or reinstantiating an existing one.
    if (!sourceAppletId) {
        //The item is new
        dg.writeEntry("sourceAppletPluginName", hostExtender->d->applet->pluginName());
        dg.writeEntry("sourceAppletId", hostExtender->d->applet->id());
        dg.writeEntry("extenderIconName", hostExtender->d->applet->icon());
        d->sourceApplet = hostExtender->d->applet;
        d->collapseIcon = new IconWidget(KIcon(hostExtender->d->applet->icon()), "", this);
    } else {
        //The item already exists.
        d->name = dg.readEntry("extenderItemName", "");
        d->title = dg.readEntry("extenderTitle", "");
        setCollapsed(dg.readEntry("isCollapsed", false));

        QString iconName = dg.readEntry("extenderIconName", "utilities-desktop-extra");
        if (iconName.isEmpty()) {
            iconName = "utilities-desktop-extra";
        }
        d->collapseIcon = new IconWidget(KIcon(iconName), "", this);

        //Find the sourceapplet.
        Corona *corona = hostExtender->d->applet->containment()->corona();
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
    connect(d->sourceApplet, SIGNAL(destroyed()), this, SLOT(sourceAppletRemoved()));
    connect(d->collapseIcon, SIGNAL(clicked()), this, SLOT(toggleCollapse()));

    //set the extender we want to move to.
    setExtender(hostExtender);

    //show or hide the return to source icon.
    d->updateToolBox();

    //set the image paths, image sizes and collapseIcon position.
    d->themeChanged();

    setAcceptsHoverEvents(true);

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeChanged()));
}

ExtenderItem::~ExtenderItem()
{
    //make sure the original mousepointer always get's restored.
    if (d->mouseOver) {
        QApplication::restoreOverrideCursor();
    }
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
    config().writeEntry("extenderTitle", title);
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
    widget->installSceneEventFilter(this);

    QSizeF panelSize(QSizeF(size().width() - d->bgLeft - d->bgRight,
                     d->iconSize + d->dragTop + d->dragBottom));
    widget->setPos(QPointF(d->bgLeft + d->dragLeft, panelSize.height() +
                                                    d->bgTop + d->dragTop));
    d->widget = widget;
    d->updateSizeHints();
}

QGraphicsItem *ExtenderItem::widget() const
{
    return d->widget;
}

void ExtenderItem::setIcon(const QIcon &icon)
{
    d->iconName.clear();
    d->collapseIcon->setIcon(icon);
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
        KConfigGroup c = extender->d->applet->config("ExtenderItems");
        config().reparent(&c);
    }

    //and notify the applet of the item being detached, after the config has been moved.
    emit d->extender->itemDetached(this);

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

    //we might have to enable or disable the returnToSource button.
    d->updateToolBox();
}

Extender *ExtenderItem::extender() const
{
    return d->extender;
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

    d->hostApplet()->config("ExtenderItems").deleteGroup(QString::number(d->extenderItemId));
    d->extender->d->removeExtenderItem(this);
    deleteLater();
}

void ExtenderItem::setCollapsed(bool collapsed)
{
    config().writeEntry("isCollapsed", collapsed);
    d->collapsed = collapsed;
    if (d->widget) {
        d->widget->setVisible(!collapsed);
        d->updateSizeHints();
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

void ExtenderItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                         QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::TextAntialiasing, true);
    painter->setRenderHint(QPainter::Antialiasing, true);

    if (d->background->enabledBorders() != (FrameSvg::LeftBorder | FrameSvg::RightBorder) &&
        d->background->enabledBorders()) {
        //Don't paint if only the left and right borders are enabled, we only use the left and right
        //border in this situation to set the correct margins on this item.
        d->background->paintFrame(painter, QPointF(0, 0));
    }

    d->dragger->paintFrame(painter, QPointF(d->bgLeft, d->bgTop));

    //We don't need to paint a title if there is no title.
    if (d->title.isEmpty()) {
        return;
    }

    //set the font for the title.
    Plasma::Theme *theme = Plasma::Theme::defaultTheme();
    QFont font = theme->font(Plasma::Theme::DefaultFont);
    font.setWeight(QFont::Bold);

    //create a pixmap with the title that is faded out at the right side of the titleRect.
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

void ExtenderItem::moveEvent(QGraphicsSceneMoveEvent *event)
{
    Q_UNUSED(event)
    //not needed anymore, but here for binary compatibility
}

void ExtenderItem::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_UNUSED(event)
    d->themeChanged();
}

void ExtenderItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!(d->dragHandleRect().contains(event->pos())) ||
        d->extender->d->applet->immutability() != Plasma::Mutable) {
        event->ignore();
        return;
    }

    d->mousePos = event->pos().toPoint();
}

void ExtenderItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->pos().toPoint() - d->mousePos).manhattanLength()
        < QApplication::startDragDistance()) {
        return;
    }

    //Start the drag:
    d->dragStarted = true;

    //first update the borders.
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

    hide();

    //create the necesarry mimedata.
    ExtenderItemMimeData *mimeData = new ExtenderItemMimeData();
    mimeData->setExtenderItem(this);

    //Hide empty internal extender containers when we drag the last item away. Avoids having
    //an ugly empty applet on the desktop temporarily.
    ExtenderApplet *extenderApplet = qobject_cast<ExtenderApplet*>(d->extender->d->applet);
    if (extenderApplet && d->extender->attachedItems().count() < 2) {
        kDebug() << "leaving the internal extender container, so hide the applet and it's handle.";
        extenderApplet->hide();
        AppletHandle *handle = qgraphicsitem_cast<AppletHandle*>(extenderApplet->parentItem());
        if (handle) {
            handle->hide();
        }
    }

    //and execute the drag.
    QWidget *dragParent = extender()->d->applet->view();
    QDrag *drag = new QDrag(dragParent);
    drag->setPixmap(pixmap);
    drag->setMimeData(mimeData);
    drag->setHotSpot(d->mousePos);
    drag->exec();

    show();
    d->widget->show();

    d->dragStarted = false;
}

void ExtenderItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (d->titleRect().contains(event->pos())) {
        d->toggleCollapse();
    }
}

bool ExtenderItem::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    if (watched == d->widget && event->type() == QEvent::GraphicsSceneResize) {
        d->updateSizeHints();
    }
    return false;
}

void ExtenderItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    //not needed anymore, but here for binary compatibility
}

void ExtenderItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (d->titleRect().contains(event->pos()) &&
        d->extender->d->applet->immutability() == Plasma::Mutable) {
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

ExtenderItemPrivate::ExtenderItemPrivate(ExtenderItem *extenderItem, Extender *hostExtender)
    : q(extenderItem),
      widget(0),
      toolbox(0),
      extender(hostExtender),
      sourceApplet(0),
      dragger(new FrameSvg(extenderItem)),
      background(new FrameSvg(extenderItem)),
      collapseIcon(0),
      title(QString()),
      mouseOver(false),
      dragStarted(false),
      destroyActionVisibility(false),
      collapsed(false),
      expirationTimer(0),
      iconSize(qreal(0))
{
    dragLeft = dragTop = dragRight = dragBottom = 0;
    bgLeft = bgTop = bgRight = bgBottom = 0;
}

ExtenderItemPrivate::~ExtenderItemPrivate()
{
    delete widget;
    widget = 0;
}

//returns a Rect containing the area of the detachable where the draghandle will be drawn.
QRectF ExtenderItemPrivate::dragHandleRect()
{
    QSizeF panelSize(QSizeF(q->size().width() - bgLeft - bgRight,
                     iconSize + dragTop + dragBottom));
    return QRectF(QPointF(bgLeft, bgTop), panelSize);
}

QRectF ExtenderItemPrivate::titleRect()
{
    return dragHandleRect().adjusted(dragLeft + collapseIcon->size().width() + 1, dragTop,
                                     -toolbox->size().width() - dragRight, -dragBottom);
}

void ExtenderItemPrivate::toggleCollapse()
{
    q->setCollapsed(!q->isCollapsed());
}

void ExtenderItemPrivate::updateToolBox()
{
    Q_ASSERT(toolbox);
    Q_ASSERT(dragger);
    Q_ASSERT(toolboxLayout);

    //TODO: only delete items that actually have to be deleted, current performance is horrible.
    while (toolboxLayout->count()) {
        QGraphicsLayoutItem *icon = toolboxLayout->itemAt(0);
        QGraphicsWidget *widget = dynamic_cast<QGraphicsWidget*>(icon);
        widget->deleteLater();
        toolboxLayout->removeAt(0);
    }

    //add the actions that are actually set to visible.
    foreach (QAction *action, actionsInOrder) {
        if (action->isVisible()) {
            IconWidget *icon = new IconWidget(q);
            icon->setAction(action);
            QSizeF size = icon->sizeFromIconSize(iconSize);
            icon->setMinimumSize(size);
            icon->setMaximumSize(size);
            toolboxLayout->addItem(icon);
        }
    }

    //add the returntosource icon if we are detached, and have a source applet.
    if (q->isDetached() && sourceApplet) {
        IconWidget *returnToSource = new IconWidget(q);
        returnToSource->setSvg("widgets/configuration-icons", "return-to-source");
        QSizeF size = returnToSource->sizeFromIconSize(iconSize);
        returnToSource->setMinimumSize(size);
        returnToSource->setMaximumSize(size);

        toolboxLayout->addItem(returnToSource);
        QObject::connect(returnToSource, SIGNAL(clicked()), q, SLOT(returnToSource()));
    }

    //add the close icon if desired.
    if (destroyActionVisibility) {
        IconWidget *destroyAction = new IconWidget(q);
        destroyAction->setSvg("widgets/configuration-icons", "close");
        QSizeF size = destroyAction->sizeFromIconSize(iconSize);
        destroyAction->setMinimumSize(size);
        destroyAction->setMaximumSize(size);

        toolboxLayout->addItem(destroyAction);
        QObject::connect(destroyAction, SIGNAL(clicked()), q, SLOT(destroy()));
    }

    toolboxLayout->updateGeometry();

    //position the toolbox correctly.
    QSizeF minimum = toolboxLayout->minimumSize();
    toolbox->resize(minimum);
    repositionToolbox();
}

void ExtenderItemPrivate::repositionToolbox()
{
    QSizeF minimum = toolboxLayout->minimumSize();
    toolbox->setPos(q->size().width() - minimum.width() - bgRight,
                    (dragHandleRect().height()/2) -
                    (minimum.height()/2) + bgTop);
}

Applet *ExtenderItemPrivate::hostApplet() const
{
    if (extender) {
        return extender->d->applet;
    } else {
        return 0;
    }
}

void ExtenderItemPrivate::themeChanged()
{
    background->setImagePath("widgets/extender-background");
    if (dragStarted) {
        background->setEnabledBorders(FrameSvg::AllBorders);
    } else {
        background->setEnabledBorders(extender->enabledBordersForItem(q));
    }
    background->getMargins(bgLeft, bgTop, bgRight, bgBottom);

    dragger->setImagePath("widgets/extender-dragger");

    //Read the preferred icon size hint, look at the font size, and calculate the desired title bar
    //icon height.
    dragger->resize();
    QSizeF size = dragger->elementSize("hint-preferred-icon-size");
    size = size.expandedTo(QSizeF(16,16));

    Plasma::Theme *theme = Plasma::Theme::defaultTheme();
    QFont font = theme->font(Plasma::Theme::DefaultFont);
    QFontMetrics fm(font);

    iconSize = qMax(size.height(), (qreal) fm.height());


    dragger->getMargins(dragLeft, dragTop, dragRight, dragBottom);

    QSizeF panelSize(QSizeF(q->size().width() - bgLeft - bgRight,
                     iconSize + dragTop + dragBottom));

    //resize the collapse icon.
    collapseIcon->resize(collapseIcon->sizeFromIconSize(iconSize));

    //reposition the collapse icon based on the new margins and size.
    collapseIcon->setPos(bgLeft + dragLeft,
                         panelSize.height()/2 -
                         collapseIcon->size().height()/2 + bgTop);

    //reposition the widget based on the new margins.
    if (widget) {
        widget->setPos(QPointF(bgLeft + dragLeft, panelSize.height() +
                                                  bgTop + dragTop));
    }

    //reposition the toolbox.
    repositionToolbox();

    updateSizeHints();

    if (!q->size().isEmpty())
        resizeContent(q->size());
}

void ExtenderItemPrivate::sourceAppletRemoved()
{
    //the original source applet is removed, set the pointer to 0 and no longer show the return to
    //source icon.
    sourceApplet = 0;
    updateToolBox();
}

void ExtenderItemPrivate::resizeContent(const QSizeF &newSize)
{
    qreal width = newSize.width();
    qreal height = newSize.height();

    //resize the dragger
    dragger->resizeFrame(QSizeF(width - bgLeft - bgRight,
                         iconSize + dragTop + dragBottom));

    //resize the applet background
    background->resizeFrame(newSize);

    //resize the widget
    if (widget && widget->isWidget()) {
        QSizeF newWidgetSize(width - bgLeft - bgRight - dragLeft - dragRight,
                             height - dragHandleRect().height() - bgTop - bgBottom -
                             2 * dragTop - 2 * dragBottom);

        QGraphicsWidget *graphicsWidget = static_cast<QGraphicsWidget*>(widget);
        graphicsWidget->resize(newWidgetSize);
    }

    //reposition the toolbox.
    repositionToolbox();

    q->update();
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

void ExtenderItemPrivate::updateSizeHints()
{
    if (!widget) {
        return;
    }

    qreal marginWidth = bgLeft + bgRight + dragLeft + dragRight;
    qreal marginHeight = bgTop + bgBottom + 2 * dragTop + 2 * dragBottom;

    QSizeF min;
    QSizeF pref;
    QSizeF max;

    if (widget->isWidget()) {
        QGraphicsWidget *graphicsWidget = static_cast<QGraphicsWidget*>(widget);
        min = graphicsWidget->minimumSize();
        pref = graphicsWidget->preferredSize();
        max = graphicsWidget->maximumSize();
    } else {
        min = widget->boundingRect().size();
        pref = widget->boundingRect().size();
        max = widget->boundingRect().size();
    }

    if (collapsed) {
        q->setPreferredSize(QSizeF(pref.width() + marginWidth,
                                   dragHandleRect().height() + marginHeight));
        q->setMinimumSize(QSizeF(min.width() + marginWidth,
                                 dragHandleRect().height() + marginHeight));
        q->setMaximumSize(QSizeF(max.width() + marginWidth,
                                 dragHandleRect().height() + marginHeight));
        q->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

        if (collapseIcon) {
            collapseIcon->setToolTip(i18n("Expand this widget"));
        }
    } else {
        q->setPreferredSize(QSizeF(pref.width() + marginWidth,
                            pref.height() + dragHandleRect().height() + marginHeight));
        q->setMinimumSize(QSizeF(min.width() + marginWidth,
                          min.height() + dragHandleRect().height() + marginHeight));
        q->setMaximumSize(QSizeF(max.width() + marginWidth,
                          max.height() + dragHandleRect().height() + marginHeight));

        //set sane size policies depending on the appearence.
        if (extender->d->appearance == Extender::TopDownStacked ||
            extender->d->appearance == Extender::BottomUpStacked) {
            //used in popups, so fixed make sense.
            kDebug() << "updating size hints for stacked look, vertical policy is fixed";
            q->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        } else if (extender->d->appearance == Extender::NoBorders) {
            //on the desktop or panel so take all the space we want.
            kDebug() << "updating size hints for no borders look, both policies are expanding";
            q->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        }

        if (collapseIcon) {
            collapseIcon->setToolTip(i18n("Collapse this widget"));
        }
    }

    q->updateGeometry();
}

uint ExtenderItemPrivate::s_maxExtenderItemId = 0;

} // namespace Plasma

#include "extenderitem.moc"
