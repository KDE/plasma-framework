/*****************************************************************

Copyright 2008 Christian Mollekopf <chrigi_1@hotmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

// Own
#include "taskgroup.h"
#include "taskmanager.h"
#include "taskitem.h"
#include "groupmanager.h"

// Qt
#include <QColor>

// KDE
#include <KDebug>
#include <KIcon>
#include <ksharedptr.h>



namespace TaskManager
{


class TaskGroup::Private
{
public:
    Private()
    {
    }

    ItemList members;
    QString groupName;
    QColor groupColor;
    QIcon groupIcon;
    bool aboutToDie;
    GroupManager *groupingStrategy;
};

TaskGroup::TaskGroup(GroupManager *parent,QString name, QColor color)
:   AbstractGroupableItem(parent),
    d(new Private)
{
    d->groupingStrategy = parent;
    d->groupName = name;
    d->groupColor = color;
    d->groupIcon = KIcon("xorg");
    connect(this, SLOT(editRequest()), this, SIGNAL(groupEditRequest()));
    kDebug() << "Group Created: Name: " << d->groupName << "Color: " << d->groupColor;
}

TaskGroup::TaskGroup(GroupManager *parent)
:   AbstractGroupableItem(parent),
    d(new Private)
{
    d->groupingStrategy = parent;
    d->groupName = "default";
    d->groupColor = Qt::red;
    d->groupIcon = KIcon("xorg");
    connect(this, SLOT(editRequest()), this, SIGNAL(groupEditRequest()));
    kDebug() << "Group Created: Name: " << d->groupName << "Color: " << d->groupColor;
}


TaskGroup::~TaskGroup()
{
    kDebug() << name();
    delete d;
}


void TaskGroup::add(AbstractItemPtr item)
{
/*    if (!item->isGroupItem()) {
        if ((dynamic_cast<TaskItem*>(item))->task()) {
            kDebug() << "Add item" << (dynamic_cast<TaskItem*>(item))->task()->visibleName();
        }
        kDebug() << " to Group " << name();
    }
*/

    if (d->members.contains(item)) {
        kDebug() << "already in this group";
        return;
    }

    if (item->parentGroup()) {
        item->parentGroup()->remove(item);
    }

    d->members.append(item);
    item->setParentGroup(this);

    connect(item, SIGNAL(changed(::TaskManager::TaskChanges)),
            this, SIGNAL(changed(::TaskManager::TaskChanges)));
    //For debug
   /* foreach (AbstractGroupableItem *item, d->members) {
        if (item->isGroupItem()) {
            kDebug() << (dynamic_cast<TaskGroup*>(item))->name();
        } else {
            kDebug() << (dynamic_cast<TaskItem*>(item))->task()->visibleName();
        }
    }*/
    emit itemAdded(item);
}



void TaskGroup::remove(AbstractItemPtr item)
{
    Q_ASSERT(item);

    if (item->isGroupItem()) {
        kDebug() << "Remove group" << (dynamic_cast<TaskGroup*>(item))->name();
    } else if ((dynamic_cast<TaskItem*>(item))->task()) {
        kDebug() << "Remove item" << (dynamic_cast<TaskItem*>(item))->task()->visibleName();
    }
    kDebug() << "from Group: " << name();

   /* kDebug() << "GroupMembers: ";
    foreach (AbstractGroupableItem *item, d->members) {
        if (item->isGroupItem()) {
            kDebug() << (dynamic_cast<TaskGroup*>(item))->name();
        } else {
            kDebug() << (dynamic_cast<TaskItem*>(item))->task()->visibleName();
        }
    }*/

    if (!d->members.contains(item)) {
        kDebug() << "couldn't find item";
        return;
    }
    disconnect(item, 0, this, 0);

    d->members.removeAll(item);
    item->setParentGroup(0);
    /*if(d->members.isEmpty()){
        kDebug() << "empty";
        emit empty(this);
    }*/
    emit itemRemoved(item);
}

void TaskGroup::clear()
{
   // kDebug() << "size " << d->members.size();
    foreach(AbstractGroupableItem *item, d->members) {
    //    kDebug();
        Q_ASSERT(item);
        if (item->isGroupItem()) { 
            (dynamic_cast<GroupPtr>(item))->clear();
        }
        remove(item);
    }
    if (!d->members.isEmpty()) {
        kDebug() << "clear doesn't work";
    }
}

ItemList TaskGroup::members() const
{
    return d->members;
}

void TaskGroup::setColor(const QColor &color)
{
    d->groupColor = color;
    emit changed(ColorChanged);
}

QColor TaskGroup::color() const
{
    return d->groupColor;
}

QString TaskGroup::name() const
{
    return d->groupName;
}

void TaskGroup::setName(const QString &newName)
{
    d->groupName = newName;
    emit changed(NameChanged);
}

QIcon TaskGroup::icon() const
{
    return d->groupIcon;
}

void TaskGroup::setIcon(const QIcon &newIcon)
{
    d->groupIcon = newIcon;
    emit changed(IconChanged);
}

bool TaskGroup::isRootGroup() const
{
    return !parentGroup();
}

/** only true if item is in this group */
bool TaskGroup::hasDirectMember(AbstractItemPtr item) const
{
    return d->members.contains(item);
}

/** true if item is in this or any sub group */
bool TaskGroup::hasMember(AbstractItemPtr item) const
{
    kDebug();
    TaskGroup *group = item->parentGroup();
    while (group) {
        if (group == this) {
            return true;
        }
        group = group->parentGroup();
    }
    return false;
}

/** Returns Direct Member group if the passed item is in a subgroup */
AbstractItemPtr TaskGroup::directMember(AbstractItemPtr item) const
{
    AbstractItemPtr tempItem = item;
    while (tempItem) {
        if (d->members.contains(item)) {
            return item;
        }
        tempItem = tempItem->parentGroup();
    }
    kDebug() << "item not found";
    return AbstractItemPtr();
}

void TaskGroup::setShaded(bool state)
{
    foreach (AbstractItemPtr item, d->members) {
        item->setShaded(state);
    }
}

void TaskGroup::toggleShaded()
{
    setShaded(!isShaded());
}

bool TaskGroup::isShaded() const
{
    foreach (AbstractItemPtr item, d->members) {
        if (!item->isShaded()) {
            return false;
        }
    }
    return true;
}

void TaskGroup::toDesktop(int desk)
{
    foreach (AbstractItemPtr item, d->members) {
        item->toDesktop(desk);
    }
    emit movedToDesktop(desk);
}

bool TaskGroup::isOnCurrentDesktop() const
{
    foreach (AbstractItemPtr item, d->members) {
        if (!item->isOnCurrentDesktop()) {
            return false;
        }
    }
    return true;
}

bool TaskGroup::isOnAllDesktops() const
{
    foreach (AbstractItemPtr item, d->members) {
        if (!item->isOnAllDesktops()) {
            return false;
        }
    }
    return true;
}

//return 0 if tasks are on different desktops or on all dektops
int TaskGroup::desktop() const
{
    if (d->members.isEmpty()) {
        return 0;
    }

    int desk = d->members.first()->desktop();
    foreach (AbstractItemPtr item, d->members) {
        if (item->desktop() != desk) {
            return 0;
        }
        desk = item->desktop();
    }
    return desk;
}

void TaskGroup::setMaximized(bool state)
{
    foreach (AbstractItemPtr item, d->members) {
        item->setMaximized(state);
    }
}

void TaskGroup::toggleMaximized()
{
    setMaximized(!isMaximized());
}

bool TaskGroup::isMaximized() const
{
    foreach (AbstractItemPtr item, d->members) {
        if (!item->isMaximized()) {
            return false;
        }
    }
    return true;
}

void TaskGroup::setMinimized(bool state)
{
    foreach (AbstractItemPtr item, d->members) {
        item->setMinimized(state);
    }
}

void TaskGroup::toggleMinimized()
{
    setMinimized(!isMinimized());
}

bool TaskGroup::isMinimized() const
{
    foreach (AbstractItemPtr item, d->members) {
        if (!item->isMinimized()) {
            return false;
        }
    }
    return true;
}

void TaskGroup::setFullScreen(bool state)
{
    foreach (AbstractItemPtr item, d->members) {
        item->setFullScreen(state);
    }
}

void TaskGroup::toggleFullScreen()
{
    setFullScreen(!isFullScreen());
}

bool TaskGroup::isFullScreen() const
{
    foreach (AbstractItemPtr item, d->members) {
        if (!item->isFullScreen()) {
            return false;
        }
    }
    return true;
}

void TaskGroup::setKeptBelowOthers(bool state)
{
    foreach (AbstractItemPtr item, d->members) {
        item->setKeptBelowOthers(state);
    }
}

void TaskGroup::toggleKeptBelowOthers()
{
    setKeptBelowOthers(!isKeptBelowOthers());
}

bool TaskGroup::isKeptBelowOthers() const
{
    foreach (AbstractItemPtr item, d->members) {
        if (!item->isKeptBelowOthers()) {
            return false;
        }
    }
    return true;
}

void TaskGroup::setAlwaysOnTop(bool state)
{
    foreach (AbstractItemPtr item, d->members) {
        item->setAlwaysOnTop(state);
    }
}

void TaskGroup::toggleAlwaysOnTop()
{
    setAlwaysOnTop(!isAlwaysOnTop());
}

bool TaskGroup::isAlwaysOnTop() const
{
    foreach (AbstractItemPtr item, d->members) {
        if (!item->isAlwaysOnTop()) {
            return false;
        }
    }
    return true;
}


bool TaskGroup::isActionSupported(NET::Action action) const
{
    if (KWindowSystem::allowedActionsSupported()) {
        foreach (AbstractItemPtr item, d->members) {
            if (!item->isActionSupported(action)) {
                return false;
            }
        }
        return true;
    }
    return false;
}

void TaskGroup::close()
{
    foreach (AbstractItemPtr item, d->members) {
        item->close();
    }
}

bool TaskGroup::isActive() const
{
    foreach (AbstractItemPtr item, d->members) {
        if (item->isActive()) {
            return true;
        }
    }

    return false;
}

bool TaskGroup::demandsAttention() const
{
    foreach (AbstractItemPtr item, d->members) {
        if (item->demandsAttention()) {
            return true;
        }
    }

    return false;
}

bool TaskGroup::moveItem(int oldIndex, int newIndex)
{
    if ((d->members.count() <= newIndex) || (newIndex < 0) ||
        (d->members.count() <= oldIndex || oldIndex < 0)) {
        kDebug() << "index out of bounds";
        return false;
    }

    AbstractItemPtr item = d->members.at(oldIndex);
    d->members.move(oldIndex, newIndex);
    kDebug() << "new index " << d->members.indexOf(item); 
    emit itemChanged(item);
    return true;
}

} // TaskManager namespace

#include "taskgroup.moc"
