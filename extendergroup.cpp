/*
 * Copyright 2009 by Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
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

#include <QAction>
#include <QString>
#include <QList>
#include <QtGui/QGraphicsWidget>

#include "applet.h"
#include "extender.h"
#include "extendergroup.h"
#include "extenderitem.h"

#include "private/extendergroup_p.h"

namespace Plasma
{

ExtenderGroup::ExtenderGroup(Extender *parent, uint groupId)
             : ExtenderItem(parent, groupId),
               d(new ExtenderGroupPrivate(this))
{
    connect(extender(), SIGNAL(itemAttached(Plasma::ExtenderItem*)),
            this, SLOT(addItemToGroup(Plasma::ExtenderItem*)));
    connect(extender(), SIGNAL(itemDetached(Plasma::ExtenderItem*)),
            this, SLOT(removeItemFromGroup(Plasma::ExtenderItem*)));

    config().writeEntry("isGroup", true);

    //TODO: monitor for change in theme
    Plasma::Svg *svg = new Plasma::Svg(this);
    svg->setImagePath("widgets/configuration-icons");
    svg->resize();

    QAction *expand = new QAction(this);
    expand->setIcon(QIcon(svg->pixmap("restore")));
    expand->setVisible(true);
    expand->setToolTip(i18n("Show this group."));
    connect(expand, SIGNAL(triggered()), this, SLOT(expandGroup()));
    addAction("expand", expand);

    QAction *collapse = new QAction(this);
    collapse->setIcon(QIcon(svg->pixmap("collapse")));
    collapse->setVisible(false);
    collapse->setToolTip(i18n("Hide this group."));
    connect(collapse, SIGNAL(triggered()), this, SLOT(collapseGroup()));
    addAction("collapse", collapse);

    QString groupName;
    foreach (ExtenderItem *item, extender()->attachedItems()) {
        groupName = item->config().readEntry("group", "");
        if (groupName != "" && groupName == name()) {
            item->setGroup(this);
        }
    }

    if (items().isEmpty()) {
        hide();
        extender()->itemRemovedEvent(this);
    }
}

ExtenderGroup::~ExtenderGroup()
{
    delete d;
}

QList<ExtenderItem*> ExtenderGroup::items() const
{
    QList<ExtenderItem*> result;
    foreach (ExtenderItem *item, extender()->attachedItems()) {
        if (item->group() == this) {
            result.append(item);
        }
    }

    return result;
}

bool ExtenderGroup::hideIfEmpty() const
{
    return d->hideIfEmpty;
}

void ExtenderGroup::setHideIfEmpty(bool hideIfEmpty)
{
    d->hideIfEmpty = hideIfEmpty;
    if (hideIfEmpty && items().isEmpty()) {
        hide();
        extender()->itemRemovedEvent(this);
    }
}

void ExtenderGroup::expandGroup()
{
    d->collapsed = false;
    action("collapse")->setVisible(true);
    action("expand")->setVisible(false);
    foreach (ExtenderItem *item, extender()->attachedItems()) {
        if (item->group() == this) {
            item->show();
            extender()->itemAddedEvent(item);
        }
    }
}

void ExtenderGroup::collapseGroup()
{
    d->collapsed = true;
    action("collapse")->setVisible(false);
    action("expand")->setVisible(true);
    foreach (ExtenderItem *item, extender()->attachedItems()) {
        if (item != this && item->group() == this) {
            item->hide();
            extender()->itemRemovedEvent(item);
        }
    }
}


ExtenderGroupPrivate::ExtenderGroupPrivate(ExtenderGroup *group)
    : collapsed(true),
      hideIfEmpty(true)
{
    q = group;
}

ExtenderGroupPrivate::~ExtenderGroupPrivate()
{
}

void ExtenderGroupPrivate::addItemToGroup(Plasma::ExtenderItem *item)
{
    if (item->group() == q) {
        if (collapsed) {
            q->extender()->itemRemovedEvent(item);
            item->hide();
        }
        if (!q->isVisible()) {
            q->extender()->itemAddedEvent(q);
            q->show();
        }
    }
}

void ExtenderGroupPrivate::removeItemFromGroup(Plasma::ExtenderItem *item)
{
    if (item->group() == q) {
        if (q->items().isEmpty()) {
            q->extender()->itemRemovedEvent(q);
            q->hide();
        }
    }
}

} // Plasma namespace

#include "extendergroup.moc"
