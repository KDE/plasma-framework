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

#include "extender.h"

#include <QAction>
#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>

#include "applet.h"
#include "containment.h"
#include "corona.h"
#include "extenderitem.h"
#include "widgets/label.h"

#include "private/applet_p.h"
#include "private/extender_p.h"

namespace Plasma
{

Extender::Extender(Applet *applet)
        : QGraphicsWidget(applet),
          d(new ExtenderPrivate(applet, this))
{
    applet->d->extender = this;
    d->layout = new QGraphicsLinearLayout(this);
    d->layout->setOrientation(Qt::Vertical);
    setLayout(d->layout);

    d->emptyExtenderLabel = new Label(this);
    d->emptyExtenderLabel->setText(d->emptyExtenderMessage);
    d->emptyExtenderLabel->setMinimumSize(QSizeF(150, 64));
    d->emptyExtenderLabel->setPreferredSize(QSizeF(200, 64));
    d->emptyExtenderLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    d->layout->addItem(d->emptyExtenderLabel);

    d->adjustSizeHints();

    d->loadExtenderItems();
}

Extender::~Extender()
{
    d->applet->d->extender = 0;
    delete d;
}

void Extender::setEmptyExtenderMessage(const QString &message)
{
    d->emptyExtenderMessage = message;

    if (d->emptyExtenderLabel) {
        d->emptyExtenderLabel->setText(message);
    }
}

QString Extender::emptyExtenderMessage() const
{
    return d->emptyExtenderMessage;
}

QList<ExtenderItem*> Extender::items() const
{
    QList<ExtenderItem*> result;

    //FIXME: a triple nested loop ... ew. there should be a more efficient way to do this
    //iterate through all extenders we can find and check each extenders source applet.
    foreach (Containment *c, d->applet->containment()->corona()->containments()) {
        foreach (Applet *applet, c->applets()) {
            if (applet->d->extender) {
                foreach (ExtenderItem *item, applet->d->extender->attachedItems()) {
                    if (item->sourceAppletId() == d->applet->id()) {
                        result.append(item);
                    }
                }
            }
        }
    }

    return result;
}

QList<ExtenderItem*> Extender::attachedItems() const
{
    return d->attachedExtenderItems;
}

QList<ExtenderItem*> Extender::detachedItems() const
{
    QList<ExtenderItem*> result;

    foreach (ExtenderItem *item, items()) {
        if (item->isDetached()) {
            result.append(item);
        }
    }

    return result;
}

ExtenderItem *Extender::item(const QString &name) const
{
    foreach (ExtenderItem *item, items()) {
        if (item->name() == name) {
            return item;
        }
    }

    return 0;
}

void Extender::saveState()
{
    kDebug() << "saving state";
    foreach (ExtenderItem *item, attachedItems()) {
        item->config().writeEntry("extenderItemPosition", item->geometry().y());
    }
}

QVariant Extender::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionHasChanged) {
        emit geometryChanged();
    }

    return QGraphicsWidget::itemChange(change, value);
}


void Extender::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    QGraphicsWidget::resizeEvent(event);
    emit geometryChanged();
}

void Extender::itemAddedEvent(ExtenderItem *item, const QPointF &pos)
{
    //this is a sane size policy imo.
    item->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    if (pos == QPointF(-1, -1)) {
        d->layout->addItem(item);
    } else {
        d->layout->insertItem(d->insertIndexFromPos(pos), item);
    }

    //remove the empty extender message if needed.
    if (d->emptyExtenderLabel) {
        d->layout->removeItem(d->emptyExtenderLabel);
        d->emptyExtenderLabel->hide();
    }

    d->adjustSizeHints();
}

void Extender::itemRemovedEvent(ExtenderItem *item)
{
    d->layout->removeItem(item);

    //add the empty extender message if needed.
    if (!attachedItems().count() && !d->spacerWidget) {
        d->emptyExtenderLabel->show();
        d->emptyExtenderLabel->setMinimumSize(item->size());
        //just in case:
        d->layout->removeItem(d->emptyExtenderLabel);
        d->layout->addItem(d->emptyExtenderLabel);
    }

    d->adjustSizeHints();
}

void Extender::itemHoverEnterEvent(ExtenderItem *item)
{
    itemHoverMoveEvent(item, QPointF(0, 0));
}

void Extender::itemHoverMoveEvent(ExtenderItem *item, const QPointF &pos)
{
    int insertIndex = d->insertIndexFromPos(pos);

    if (insertIndex == d->currentSpacerIndex) {
        //relayouting is resource intensive, so don't do that when not necesarry
        return;
    }

    //Make sure we remove any spacer that might allready be in the layout.
    itemHoverLeaveEvent(item);

    d->currentSpacerIndex = insertIndex;

    //Create a widget that functions as spacer, and add that to the layout.
    QGraphicsWidget *widget = new QGraphicsWidget(this);
    widget->setPreferredSize(QSizeF(item->size().width(), item->size().height()));
    widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    d->spacerWidget = widget;
    d->layout->insertItem(insertIndex, widget);

    //Make sure we remove any 'no detachables' label that might be there, and update the layout.
    //XXX: duplicated from itemAttachedEvent.
    if (d->emptyExtenderLabel) {
        d->layout->removeItem(d->emptyExtenderLabel);
        d->emptyExtenderLabel->hide();
    }

    d->adjustSizeHints();
}

void Extender::itemHoverLeaveEvent(ExtenderItem *item)
{
    Q_UNUSED(item);

    if (d->spacerWidget) {
        //Remove any trace of the spacer widget.
        d->layout->removeItem(d->spacerWidget);
        delete d->spacerWidget;
        d->spacerWidget = 0;

        d->currentSpacerIndex = -1;

        //Make sure we add a 'no detachables' label when the layout is empty.
        if (!attachedItems().count()) {
            d->emptyExtenderLabel->show();
            d->emptyExtenderLabel->setMinimumSize(item->size());
            d->layout->removeItem(d->emptyExtenderLabel);
            d->layout->addItem(d->emptyExtenderLabel);
        }

        d->adjustSizeHints();
    }
}

ExtenderPrivate::ExtenderPrivate(Applet *applet, Extender *extender) :
    q(extender),
    applet(applet),
    spacerWidget(0),
    emptyExtenderMessage(i18n("no items")),
    emptyExtenderLabel(0),
    popup(false)
{
}

ExtenderPrivate::~ExtenderPrivate()
{
}

void ExtenderPrivate::addExtenderItem(ExtenderItem *item, const QPointF &pos)
{
    attachedExtenderItems.append(item);
    item->action("returntosource")->setVisible(item->isDetached());
    q->itemAddedEvent(item, pos);
    q->itemHoverLeaveEvent(item);
    emit q->itemAttached(item);
}

void ExtenderPrivate::removeExtenderItem(ExtenderItem *item)
{
    attachedExtenderItems.removeOne(item);
    q->itemRemovedEvent(item);
}

void ExtenderPrivate::adjustSizeHints()
{
    if (!q->layout()) {
        return;
    }

    q->layout()->updateGeometry();

    q->setMinimumSize(q->layout()->minimumSize());
    q->setPreferredSize(q->layout()->preferredSize());
    q->setMaximumSize(q->layout()->maximumSize());

    q->updateGeometry();
}

int ExtenderPrivate::insertIndexFromPos(const QPointF &pos) const
{
    int insertIndex = -1;

    //XXX: duplicated from panel
    if (pos != QPointF(-1, -1)) {
        for (int i = 0; i < layout->count(); ++i) {
            QRectF siblingGeometry = layout->itemAt(i)->geometry();
            qreal middle = (siblingGeometry.top() + siblingGeometry.bottom()) / 2.0;
            if (pos.y() < middle) {
                insertIndex = i;
                break;
            } else if (pos.y() <= siblingGeometry.bottom()) {
                insertIndex = i + 1;
                break;
            }
        }
    }

    return insertIndex;
}

void ExtenderPrivate::loadExtenderItems()
{
    KConfigGroup cg = applet->config("ExtenderItems");

    //first create a list of extenderItems, and then sort them on their position, so the items get
    //recreated in the correct order.
    //TODO: this restoring of the correct order should now be done in itemAddedEvent instead of
    //here, to allow easy subclassing of Extender.
    QList<QPair<int, QString> > groupList;
    foreach (const QString &extenderItemId, cg.groupList()) {
        KConfigGroup dg = cg.group(extenderItemId);
        groupList.append(qMakePair(dg.readEntry("extenderItemPosition", 0), extenderItemId));
    }
    qSort(groupList);

    //iterate over the extender items
    for (int i = 0; i < groupList.count(); i++) {
        QPair<int, QString> pair = groupList[i];
        KConfigGroup dg = cg.group(pair.second);

        //load the relevant settings.
        QString extenderItemId = dg.name();
        QString extenderItemName = dg.readEntry("extenderItemName", "");
        QString appletName = dg.readEntry("sourceAppletPluginName", "");
        uint sourceAppletId = dg.readEntry("sourceAppletId", 0);

        bool temporarySourceApplet = false;

        //find the source applet.
        Corona *corona = applet->containment()->corona();
        Applet *sourceApplet = 0;
        foreach (Containment *containment, corona->containments()) {
            foreach (Applet *applet, containment->applets()) {
                if (applet->id() == sourceAppletId) {
                    sourceApplet = applet;
                }
            }
        }

        //There is no source applet. We just instantiate one just for the sake of creating
        //detachables.
        if (!sourceApplet) {
            kDebug() << "creating a temporary applet as factory";
            sourceApplet = Applet::load(appletName);
            temporarySourceApplet = true;
            //TODO: maybe add an option to applet to indicate that it shouldn't be deleted after
            //having used it as factory.
        }

        if (!sourceApplet) {
            kDebug() << "sourceApplet is null? appletName = " << appletName;
            kDebug() << "                      extenderItemId = " << extenderItemId;
        } else {
            ExtenderItem *item = new ExtenderItem(q, extenderItemId.toInt());
            item->setName(extenderItemName);
            sourceApplet->initExtenderItem(item);

            if (temporarySourceApplet) {
                delete sourceApplet;
            }
        }
    }

    adjustSizeHints();
}

} // Plasma namespace

#include "extender.moc"
