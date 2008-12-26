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
#include <QPainter>

#include "applet.h"
#include "containment.h"
#include "corona.h"
#include "extenderitem.h"
#include "framesvg.h"
#include "paintutils.h"
#include "popupapplet.h"
#include "svg.h"
#include "theme.h"
#include "widgets/label.h"

#include "private/applet_p.h"
#include "private/extender_p.h"
#include "private/extenderitem_p.h"

namespace Plasma
{

//TODO: copied from panel containment. We'll probably want a spacer widget in libplasma for 4.3.
class Spacer : public QGraphicsWidget
{
    public:
        Spacer(QGraphicsWidget *parent)
                 : QGraphicsWidget(parent)
       {
       }

       ~Spacer()
       {
       }

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget * widget = 0)
        {
            Q_UNUSED(option)
            Q_UNUSED(widget)

            painter->setRenderHint(QPainter::Antialiasing);
            QPainterPath p = Plasma::PaintUtils::roundedRectangle(contentsRect().adjusted(4, 4, -4, -4), 4);

            QColor c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
            c.setAlphaF(0.3);
            painter->fillPath(p, c);
        }
};

Extender::Extender(Applet *applet)
        : QGraphicsWidget(applet),
          d(new ExtenderPrivate(applet, this))
{
    //At multiple places in the extender code, we make the assumption that an applet doesn't have
    //more then one extender. If a second extender is created, destroy the first one to avoid leaks.
    if (applet->d->extender) {
        kWarning() << "Applet already has an extender, and can have only one extender."
                   << "The previous extender will be destroyed.";
        delete applet->d->extender;
    }
    applet->d->extender = this;

    setContentsMargins(0, 0, 0, 0);
    d->layout = new QGraphicsLinearLayout(this);
    d->layout->setOrientation(Qt::Vertical);
    d->layout->setContentsMargins(0, 0, 0, 0);
    d->layout->setSpacing(0);
    setLayout(d->layout);

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

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
                    if (item->d->sourceApplet == d->applet) {
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

void Extender::setAppearance(Appearance appearance)
{
    if (d->appearance == appearance) {
        return;
    }

    d->appearance = appearance;
    d->updateBorders();
}

Extender::Appearance Extender::appearance() const
{
    return d->appearance;
}

void Extender::saveState()
{
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

void Extender::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    PopupApplet *popupApplet = qobject_cast<PopupApplet*>(d->applet);
    if (attachedItems().isEmpty() && popupApplet) {
        popupApplet->hidePopup();
    }
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
    d->updateEmptyExtenderLabel();

    //if the item doesn't got a widget one will be set real soon, causing it's sizehints to
    //change. Don't adjust the size hints in that case, since that spares us a call to
    //adjustSizeHints()
    if (item->widget()) {
        d->adjustSizeHints();
    }
}

void Extender::itemRemovedEvent(ExtenderItem *item)
{
    d->layout->removeItem(item);

    if (d->spacerWidget) {
        d->layout->removeItem(d->spacerWidget);
        delete d->spacerWidget;
        d->spacerWidget = 0;
    }

    //add the empty extender message if needed.
    d->updateEmptyExtenderLabel();
    d->adjustSizeHints();
}

void Extender::itemHoverEnterEvent(ExtenderItem *item)
{
    itemHoverMoveEvent(item, QPointF(0, 0));
}

void Extender::itemHoverMoveEvent(ExtenderItem *item, const QPointF &pos)
{
    if (d->spacerWidget && d->spacerWidget->geometry().contains(pos)) {
        return;
    }

    //Make sure we remove any spacer that might already be in the layout.
    if (d->spacerWidget) {
        d->layout->removeItem(d->spacerWidget);
    }

    int insertIndex = d->insertIndexFromPos(pos);

    //Create a widget that functions as spacer, and add that to the layout.
    if (!d->spacerWidget) {
        Spacer *widget = new Spacer(this);
        widget->setMinimumSize(item->minimumSize());
        widget->setPreferredSize(item->preferredSize());
        widget->setMaximumSize(item->maximumSize());
        widget->setSizePolicy(item->sizePolicy());
        d->spacerWidget = widget;
    }
    d->layout->insertItem(insertIndex, d->spacerWidget);

    //Make sure we remove any 'no detachables' label that might be there, and update the layout.
    d->updateEmptyExtenderLabel();
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

        d->updateEmptyExtenderLabel();
        d->adjustSizeHints();
    }
}

FrameSvg::EnabledBorders Extender::enabledBordersForItem(ExtenderItem *item) const
{
    if (d->layout->count() < 1) {
        return 0;
    }

    ExtenderItem *topItem = dynamic_cast<ExtenderItem*>(d->layout->itemAt(0));
    ExtenderItem *bottomItem = dynamic_cast<ExtenderItem*>(d->layout->itemAt(d->layout->count() - 1));
    if (d->appearance == TopDownStacked && bottomItem != item) {
        return FrameSvg::LeftBorder | FrameSvg::BottomBorder | FrameSvg::RightBorder;
    } else if (d->appearance == BottomUpStacked && topItem != item) {
        return FrameSvg::LeftBorder | FrameSvg::TopBorder | FrameSvg::RightBorder;
    } else if (d->appearance != NoBorders) {
        return FrameSvg::LeftBorder | FrameSvg::RightBorder;
    } else {
        return 0;
    }
}

ExtenderPrivate::ExtenderPrivate(Applet *applet, Extender *extender) :
    q(extender),
    applet(applet),
    currentSpacerIndex(-1),
    spacerWidget(0),
    emptyExtenderMessage(QString()),
    emptyExtenderLabel(0),
    appearance(Extender::NoBorders)
{
}

ExtenderPrivate::~ExtenderPrivate()
{
}

void ExtenderPrivate::addExtenderItem(ExtenderItem *item, const QPointF &pos)
{
    attachedExtenderItems.append(item);
    q->itemHoverLeaveEvent(item);
    q->itemAddedEvent(item, pos);
    updateBorders();
    emit q->itemAttached(item);
}

void ExtenderPrivate::removeExtenderItem(ExtenderItem *item)
{
    attachedExtenderItems.removeOne(item);

    //collapse the popupapplet if the last item is removed.
    if (!q->attachedItems().count()) {
        PopupApplet *popupApplet = qobject_cast<PopupApplet*>(applet);
        if (popupApplet) {
            popupApplet->hidePopup();
        }
    }

    q->itemRemovedEvent(item);
    updateBorders();
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

        kDebug() << "applet id = " << applet->id();
        kDebug() << "sourceappletid = " << sourceAppletId;

        if (sourceAppletId != applet->id()) {
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
                sourceApplet->initExtenderItem(item);

                if (temporarySourceApplet) {
                    delete sourceApplet;
                }
            }
        } else {
            //this entry is still here, probably because plasma crashed, but it isn't detached and
            //should be reinstantiated. Just delete the entry.
            cg.deleteGroup(pair.second);
        }
    }
}

void ExtenderPrivate::updateBorders()
{
    foreach (ExtenderItem *item, q->attachedItems()) {
        //kDebug() << "checking" << (QObject*)item << item->d->background->enabledBorders()
        //         << q->enabledBordersForItem(item);
        if (item && (item->d->background->enabledBorders() != q->enabledBordersForItem(item))) {
            //call themeChanged to change the backgrounds enabled borders, and move all contained
            //widgets according to it's changed margins.
            item->d->themeChanged();
        }
    }
}

void ExtenderPrivate::adjustSizeHints()
{
    //FIXME: what happens in this function are some nasty workarounds for a bug in qt4.4's QGL.
    //Alexis has told me they are working on a fix for qt4.5, so this can be removed once the bug
    //has been fixed in Qt.
    if (layout) {
        layout->updateGeometry();
        q->setMinimumSize(layout->preferredSize());
    }

    if (applet->layout()) {
        applet->layout()->updateGeometry();

        qreal left, top, right, bottom;
        applet->getContentsMargins(&left, &top, &right, &bottom);
        QSizeF margins(left + right, top + bottom);

        applet->setMinimumSize(applet->layout()->minimumSize() + margins);
        applet->adjustSize();
    }

    q->adjustSize();

    emit q->geometryChanged();
}

void ExtenderPrivate::updateEmptyExtenderLabel()
{
    if (q->attachedItems().isEmpty() && !emptyExtenderLabel && !emptyExtenderMessage.isEmpty()
                                     && !spacerWidget ) {
        //add the empty extender label.
        emptyExtenderLabel = new Label(q);
        emptyExtenderLabel->setAlignment(Qt::AlignCenter);
        emptyExtenderLabel->setText(emptyExtenderMessage);
        emptyExtenderLabel->setMinimumSize(QSizeF(150, 64));
        emptyExtenderLabel->setPreferredSize(QSizeF(200, 64));
        emptyExtenderLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        layout->addItem(emptyExtenderLabel);
    } else if (!q->attachedItems().isEmpty() && emptyExtenderLabel) {
        //remove the empty extender label.
        layout->removeItem(emptyExtenderLabel);
        delete emptyExtenderLabel;
        emptyExtenderLabel = 0;
    }
}

} // Plasma namespace

#include "extender.moc"
