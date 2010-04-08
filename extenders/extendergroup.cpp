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
#include <QtGui/QGraphicsLinearLayout>

#include "applet.h"
#include "extender.h"
#include "extenderitem.h"
#include "theme.h"
#include "widgets/scrollwidget.h"

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

    QGraphicsLinearLayout *lay = static_cast<QGraphicsLinearLayout *>(layout());
    d->scrollWidget = new ScrollWidget(this);
    d->scrollWidget->show();
    lay->addItem(d->scrollWidget);
    d->scrollWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->childsWidget = new QGraphicsWidget(d->scrollWidget);
    d->childsWidget->installEventFilter(this);
    d->scrollWidget->setWidget(d->childsWidget);
    d->layout = new QGraphicsLinearLayout(Qt::Vertical, d->childsWidget);
    d->scrollWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

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
        extender()->itemRemovedEvent(this);
        hide();
    }

    if (!config().readEntry("groupCollapsed", true)) {
        expandGroup();
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
    if (autoHide && items().isEmpty()) {
        extender()->itemRemovedEvent(this);
        hide();
    } else if (!autoHide && !isVisible()) {
        extender()->itemAddedEvent(this);
        show();
    }
}

bool ExtenderGroup::isAutoCollapse() const
{
    return d->autoCollapse;
}

void ExtenderGroup::setAutoCollapse(bool collapse)
{
    d->autoCollapse = collapse;
    if (collapse) {
        setCollapsed(d->collapsed);
    }
}

bool ExtenderGroup::isGroupCollapsed() const
{
    return d->collapsed;
}

void ExtenderGroup::setGroupCollapsed(bool collapsed)
{
    if (collapsed) {
        collapseGroup();
    } else {
        expandGroup();
    }
}

void ExtenderGroup::expandGroup()
{
    if (d->autoCollapse) {
        setCollapsed(false);
    }
    if (d->collapsed == false) {
        return;
    }

    d->collapsed = false;
    config().writeEntry("groupCollapsed", d->collapsed);
    action("collapse")->setVisible(true);
    action("expand")->setVisible(false);
    foreach (ExtenderItem *item, extender()->attachedItems()) {
        if (item->group() == this) {
            item->show();
            extender()->itemAddedEvent(item);
        }
    }
    d->scrollWidget->show();
    static_cast<QGraphicsLinearLayout *>(layout())->addItem(d->scrollWidget);

    //resize to the bax between our hint and extender one
    //TODO: do this on every childswidget resize?
    extender()->resize(extender()->effectiveSizeHint(Qt::PreferredSize).expandedTo(effectiveSizeHint(Qt::PreferredSize)));
}

void ExtenderGroup::collapseGroup()
{
    if (d->autoCollapse) {
        setCollapsed(true);
    }
    if (d->collapsed == true) {
        return;
    }

    d->collapsed = true;
    config().writeEntry("groupCollapsed", d->collapsed);
    action("collapse")->setVisible(false);
    action("expand")->setVisible(true);
    foreach (ExtenderItem *item, extender()->attachedItems()) {
        if (item != this && item->group() == this) {
            item->hide();
            extender()->itemRemovedEvent(item);
        }
    }
    d->scrollWidget->hide();
    static_cast<QGraphicsLinearLayout *>(layout())->removeItem(d->scrollWidget);
    extender()->resize(extender()->effectiveSizeHint(Qt::MinimumSize));
}

void ExtenderGroup::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    ExtenderItem::resizeEvent(event);
}

bool ExtenderGroup::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d->childsWidget && event->type() == QEvent::GraphicsSceneResize) {
        //resize to the max between the extender size hint and ours (that's because the group can still not be in the extender layout)
        extender()->resize(extender()->effectiveSizeHint(Qt::PreferredSize).expandedTo(effectiveSizeHint(Qt::PreferredSize)).width(), extender()->size().height());
    }

    return ExtenderItem::eventFilter(watched, event);
}

ExtenderGroupPrivate::ExtenderGroupPrivate(ExtenderGroup *group)
    : q(group),
      svg(new Svg(group)),
      collapsed(true),
      autoHide(true),
      autoCollapse(false)
{
}

ExtenderGroupPrivate::~ExtenderGroupPrivate()
{
}

void ExtenderGroupPrivate::addItemToGroup(Plasma::ExtenderItem *item)
{
    if (item->group() == q) {
        item->setParentItem(childsWidget);
        layout->addItem(item);
        layout->activate();

        childsWidget->resize(childsWidget->size().width(),
                             childsWidget->effectiveSizeHint(Qt::PreferredSize).height());

        if (!q->isVisible() && !q->items().isEmpty()) {
            q->extender()->itemAddedEvent(q);
            q->show();
        }
        scrollWidget->setVisible(!q->isCollapsed());
    }
}

void ExtenderGroupPrivate::removeItemFromGroup(Plasma::ExtenderItem *item)
{
    if (item->group() == q) {
        if (q->items().isEmpty() && autoHide && !q->isDetached()) {
            q->extender()->itemRemovedEvent(q);
            q->hide();
        }
        childsWidget->resize(childsWidget->size().width(),
                             childsWidget->effectiveSizeHint(Qt::PreferredSize).height());
        layout->removeItem(item);
    }
}

void ExtenderGroupPrivate::themeChanged()
{
    svg->setImagePath("widgets/configuration-icons");
    svg->resize();

    q->action("expand")->setIcon(QIcon(svg->pixmap("restore")));
    q->action("collapse")->setIcon(QIcon(svg->pixmap("collapse")));
}

} // Plasma namespace

#include "extendergroup.moc"
