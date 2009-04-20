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

#include "extendergroup.h"

#include <QAction>
#include <QString>
#include <QList>
#include <QtGui/QGraphicsWidget>

#include "applet.h"
#include "extender.h"
#include "extenderitem.h"
#include "theme.h"

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

    QAction *expand = new QAction(this);
    expand->setVisible(true);
    expand->setToolTip(i18n("Show this group."));
    connect(expand, SIGNAL(triggered()), this, SLOT(expandGroup()));
    addAction("expand", expand);

    QAction *collapse = new QAction(this);
    collapse->setVisible(false);
    collapse->setToolTip(i18n("Hide this group."));
    connect(collapse, SIGNAL(triggered()), this, SLOT(collapseGroup()));
    addAction("collapse", collapse);

    d->themeChanged();

    QString groupName;
    foreach (ExtenderItem *item, extender()->attachedItems()) {
        groupName = item->config().readEntry("group", "");
        if (!groupName.isEmpty() && groupName == name()) {
            item->setGroup(this);
        }
    }

    if (items().isEmpty() && d->autoHide && !isDetached()) {
        hide();
        extender()->itemRemovedEvent(this);
    }

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(themeChanged()));
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

bool ExtenderGroup::autoHide() const
{
    return d->autoHide;
}

void ExtenderGroup::setAutoHide(bool autoHide)
{
    d->autoHide = autoHide;
    if (autoHide && items().count() < 2) {
        hide();
        extender()->itemRemovedEvent(this);
    } else if (!autoHide && !isVisible()) {
        extender()->itemAddedEvent(this);
        show();
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
      autoHide(true)
{
    q = group;
}

ExtenderGroupPrivate::~ExtenderGroupPrivate()
{
}

void ExtenderGroupPrivate::addItemToGroup(Plasma::ExtenderItem *item)
{
    if (item->group() == q) {
        int itemCount = q->items().count();
        if (collapsed && !(autoHide && itemCount == 1)) {
            //the group is collapsed, so hide the new item unless there's only one item and autohide
            //is true, in which case we hide this group, and not the item in it.
            q->extender()->itemRemovedEvent(item);
            item->hide();
        } else {
            //the group isn't collapsed so show and readd this item to the extender, which takes
            //care of placing the new item directly under the group widget.
            q->extender()->itemAddedEvent(item);
            item->show();
        }
        if (!q->isVisible() && (itemCount > 1 || !autoHide)) {
            //show the group if needed, depending on autoHide policy.
            q->extender()->itemAddedEvent(q);
            q->show();
            if (collapsed) {
                q->extender()->itemRemovedEvent(q->items().first());
                q->items().first()->hide();
            }
        }
    }
}

void ExtenderGroupPrivate::removeItemFromGroup(Plasma::ExtenderItem *item)
{
    if (item->group() == q) {
        if (q->items().count() < 2 && autoHide && !q->isDetached()) {
            q->extender()->itemRemovedEvent(q);
            q->hide();
        }
        if (q->items().count() == 1 && autoHide) {
            q->extender()->itemAddedEvent(q->items().first());
            q->items().first()->show();
        }
    }
}

void ExtenderGroupPrivate::themeChanged()
{
    Plasma::Svg *svg = new Plasma::Svg(q);///Who deletes the svg? (Looks like a memory hog/leak on multiple theme changes)
    svg->setImagePath("widgets/configuration-icons");
    svg->resize();

    q->action("expand")->setIcon(QIcon(svg->pixmap("restore")));
    q->action("collapse")->setIcon(QIcon(svg->pixmap("collapse")));
}

} // Plasma namespace

#include "extendergroup.moc"
