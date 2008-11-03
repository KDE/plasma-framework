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

#include "manualsortingstrategy.h"

#include "taskitem.h"
#include "taskgroup.h"
#include "taskmanager.h"

#include <QMap>
#include <QtAlgorithms>

#include <KDebug>


namespace TaskManager
{

class ManualSortingStrategy::Private
{
public:
    Private()
    {
    }
    GroupManager *groupingStrategy;

    itemHashTable *managedItems;
    desktopHashTable *desktops;
    int oldDesktop;
};


ManualSortingStrategy::ManualSortingStrategy(GroupManager *parent)
    :AbstractSortingStrategy(parent),
    d(new Private)
{
    d->groupingStrategy = parent;
    setType(GroupManager::ManualSorting);

    d->desktops = new desktopHashTable();
    //TODO add a screenHashTable
    d->oldDesktop = TaskManager::TaskManager::self()->currentDesktop();

    if (d->groupingStrategy->showOnlyCurrentDesktop()) {
        d->desktops->insert(TaskManager::TaskManager::self()->currentDesktop(), new itemHashTable());
        d->managedItems = d->desktops->value(TaskManager::TaskManager::self()->currentDesktop());
    } else {
        d->desktops->insert(0,new itemHashTable());
        d->managedItems = d->desktops->value(0);
    }
}

ManualSortingStrategy::~ManualSortingStrategy()
{
    if (d->desktops) {
        foreach(itemHashTable *table, *d->desktops) {
            if (table) {
                delete table;
            }
        }
        delete d->desktops;
    }
    delete d;
}

void ManualSortingStrategy::storePositions(TaskGroup *group)
{
    Q_ASSERT(group);
    for(int i = 0; i < group->members().size(); i++) {
        AbstractGroupableItem *item = group->members().at(i);
        Q_ASSERT(item);
        if (item->isGroupItem()) {
            d->managedItems->insert(item, i);
            storePositions(dynamic_cast<TaskGroup*>(item));
        } else {
            d->managedItems->insert(item, i);
        }
        kDebug() << item << i;
    }
}

//Here we should store all infos about the sorting
void ManualSortingStrategy::desktopChanged(int newDesktop)
{
    kDebug() << "Desktop changed" << d->oldDesktop << newDesktop;
    //store positions of old desktop
    d->managedItems->clear();
    storePositions(d->groupingStrategy->rootGroup());
    d->desktops->insert(d->oldDesktop, d->managedItems);

    //load positions of new desktop
    if (d->desktops->contains(newDesktop)) {
        d->managedItems = d->desktops->value(newDesktop);
    } else {
        d->managedItems = new itemHashTable();
    }

    d->oldDesktop = newDesktop;
}

void ManualSortingStrategy::sortItems(ItemList &items)
{
    kDebug();

    QMap<int, AbstractGroupableItem*> map;
    int i = 1000;
    foreach (AbstractGroupableItem *item, items) {
        if (d->managedItems->contains(item)) {
            map.insertMulti(d->managedItems->value(item), item);
        } else {//make sure unkwown items are appended
            kDebug() << "item not found in managedItems";
            map.insertMulti(i, item);
            i++;
        }
    }
    items.clear();
    items = map.values();
}

//since we have no way of knowing about a desktop change before it happens we have to track every single change....
void ManualSortingStrategy::handleItem(AbstractItemPtr item)
{
    if (d->managedItems->contains(item)) {
        if (item->isGroupItem()) {
            handleGroup(qobject_cast<TaskGroup*>(item));
        }
        check(item);
    } else {
        Q_ASSERT(item->parentGroup());
        d->managedItems->insert(item, item->parentGroup()->members().indexOf(item));
    }
}

} //namespace

#include "manualsortingstrategy.moc"

