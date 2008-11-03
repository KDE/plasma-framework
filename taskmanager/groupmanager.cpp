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

#include "groupmanager.h"

#include <QList>
#include <KDebug>
#include <QTimer>


#include "abstractsortingstrategy.h"
#include "startup.h"
#include "task.h"
#include "taskitem.h"
#include "taskgroup.h"
#include "taskmanager.h"
#include "strategies/alphasortingstrategy.h"
#include "strategies/programgroupingstrategy.h"
#include "strategies/manualgroupingstrategy.h"
#include "strategies/manualsortingstrategy.h"

namespace TaskManager
{

class GroupManagerPrivate
{
public:
    GroupManagerPrivate(GroupManager *manager)
        : q(manager),
          rootGroup(0),
          sortingStrategy(GroupManager::NoSorting),
          groupingStrategy(GroupManager::NoGrouping),
          abstractGroupingStrategy(0),
          abstractSortingStrategy(0),
          currentScreen(-1),
          showOnlyCurrentDesktop(false),
          showOnlyCurrentScreen(false),
          showOnlyMinimized(false),
          onlyGroupWhenFull(false),
          groupIsFullLimit(0),
          lastGroupingStrategy(GroupManager::NoGrouping)
    {
    }

    /** reload all tasks from TaskManager */
    void reloadTasks();

    /**
    * Keep track of changes in Taskmanager
    */
    void currentDesktopChanged(int);
    void taskChanged(TaskPtr, ::TaskManager::TaskChanges);

    void checkScreenChange();

    void itemDestroyed();
    void checkIfFull();

    GroupManager *q;
    QHash<TaskPtr, TaskItem*> itemList; //holds all tasks of the Taskmanager
    QHash<StartupPtr, TaskItem*> startupList;
    TaskGroup *rootGroup; //the current layout
    GroupManager::TaskSortingStrategy sortingStrategy;
    GroupManager::TaskGroupingStrategy groupingStrategy;
    AbstractGroupingStrategy *abstractGroupingStrategy;
    AbstractSortingStrategy *abstractSortingStrategy;
    int currentScreen;
    QTimer screenTimer;
    QList<TaskPtr> geometryTasks;
    bool showOnlyCurrentDesktop : 1;
    bool showOnlyCurrentScreen : 1;
    bool showOnlyMinimized : 1;
    bool onlyGroupWhenFull : 1;
    int groupIsFullLimit;
    GroupManager::TaskGroupingStrategy lastGroupingStrategy;
};




GroupManager::GroupManager(QObject *parent)
    : QObject(parent),
      d(new GroupManagerPrivate(this))
{
    connect(TaskManager::self(), SIGNAL(taskAdded(TaskPtr)), this, SLOT(add(TaskPtr)));
    connect(TaskManager::self(), SIGNAL(taskRemoved(TaskPtr)), this, SLOT(remove(TaskPtr)));
    connect(TaskManager::self(), SIGNAL(startupAdded(StartupPtr)), this, SLOT(add(StartupPtr)));
    connect(TaskManager::self(), SIGNAL(startupRemoved(StartupPtr)), this, SLOT(remove(StartupPtr)));
    d->rootGroup = new TaskGroup(this, "RootGroup", Qt::transparent);
    //reloadTasks();
    d->screenTimer.setSingleShot(true);
    d->screenTimer.setInterval(100);
    connect(&d->screenTimer, SIGNAL(timeout()), this, SLOT(checkScreenChange()));
}

GroupManager::~GroupManager()
{
    delete d->abstractSortingStrategy;
    delete d->abstractGroupingStrategy;
    delete d->rootGroup;
    delete d;
}

void GroupManagerPrivate::reloadTasks()
{
    //kDebug() << "number of tasks available " << TaskManager::self()->tasks().size();

    QList <TaskPtr> taskList = TaskManager::self()->tasks().values();
    foreach(TaskPtr task, taskList) { //Add all existing tasks
        if (!q->add(task)) {
            q->remove(task); //remove what isn't needed anymore
        }
        taskList.removeAll(task);
    }

    foreach(TaskPtr task, taskList) { //Remove the remaining
        q->remove(task);
    }

    emit q->reload();
}

void GroupManager::add(StartupPtr task)
{
    kDebug();
    TaskItem *item;
    if (!d->startupList.contains(task)) {
        item = new TaskItem(this,task);
        d->startupList.insert(task, item); 
        d->rootGroup->add(item);
    }
}

void GroupManager::remove(StartupPtr task)
{
    kDebug() << "startup";
    AbstractItemPtr item = d->startupList.take(task);
    if (!item) {
        kDebug() << "invalid item";
        return;
    }

    if (item->parentGroup()) {
        item->parentGroup()->remove(item);
    }
    emit itemRemoved(item);

}

bool GroupManager::add(TaskPtr task)
{
    //kDebug();
    /*kDebug() << task->visibleName();
    kDebug() <<  task->visibleNameWithState();
    kDebug() <<  task->name();
    kDebug() <<  task->className();
    kDebug() <<  task->classClass();*/

    // Go through all filters whether the task should be displayed or not
    bool show = true;
    if (!task->showInTaskbar()) {
        kDebug() << "Do not show in taskbar";
        show = false;
    }

    if (showOnlyCurrentDesktop() && !task->isOnCurrentDesktop()) {
        kDebug() << "Not on this desktop and showOnlyCurrentDesktop";
        show = false;
    }

    if (showOnlyCurrentScreen() && !task->isOnScreen(d->currentScreen)) {
        kDebug() << "Not on this screen and showOnlyCurrentScreen";
        show = false;
    }

    if (showOnlyMinimized() && !task->isMinimized()) {
        show = false;
    }

    NET::WindowType type = task->info().windowType(NET::NormalMask | NET::DialogMask |
                                                   NET::OverrideMask | NET::UtilityMask);
    if (type == NET::Utility) {
        kDebug() << "skipping utility window" << task->name();
        show = false;
    }

        //TODO: should we check for transiency? if so the following code can detect it.
    /*
        QHash <TaskPtr, TaskItem*>::iterator it = d->itemList.begin();

        while (it != d->itemList.end()) {
            TaskItem *item = it.value();
            if (item->task()->hasTransient(task->window())) {
                kDebug() << "TRANSIENT TRANSIENT TRANSIENT!";
            }
            ++it;
        }
    */

    if (task->demandsAttention()) { //override all other limitations
        show = true;
    }

    if (!show) {
        return false;
    }
    //Ok the Task should be displayed
    TaskItem *item = 0;
    if (!d->itemList.contains(task)) {
        //Lookout for existing startuptask of this task
        QMutableHashIterator<StartupPtr, TaskItem*> it(d->startupList);
        while (it.hasNext()) {
            it.next();
            if (it.key()->matchesWindow(task->window())) {
                kDebug() << "startup task";
                item = it.value();
                item->setTaskPointer(task);
                it.remove();
                break;
            }
        }

        if (!item) {
            item = new TaskItem(this,task);
        }

        connect(item, SIGNAL(destroyed()), this, SLOT(itemDestroyed()));
        d->itemList.insert(task, item); 
    } else {
        item = d->itemList.value(task); //we add it again so the group is evaluated again
    }

    //Find a fitting group for the task with GroupingStrategies
    if (d->abstractGroupingStrategy && !task->demandsAttention()) { //do not group attetion tasks
        d->abstractGroupingStrategy->handleItem(item);
    } else {
        d->rootGroup->add(item);
    }
    return true;
}


void GroupManager::remove(TaskPtr task)
{
    //kDebug() << "remove: " << task->visibleName();
    TaskItem *item = d->itemList.value(task);
    if (!item) {
        kDebug() << "invalid item";
        return;
    }
    if (item->parentGroup()) {
        item->parentGroup()->remove(item);
    }
    emit itemRemoved(item);
    //the item must exist as long as the TaskPtr does because of activate calls so don't delete the item here, it will delete itself. We keep it in the itemlist because it may return
    
}

void GroupManagerPrivate::itemDestroyed()
{
    TaskItem *taskItem = qobject_cast<TaskItem*>(q->sender());
    TaskItem *item = itemList.take(itemList.key(taskItem));
    if (!item) {
        kDebug() << "invalid item";
        return;
    }
    QObject::disconnect(item, 0, q, 0);
}


bool GroupManager::manualGroupingRequest(AbstractGroupableItem* item, TaskGroup* groupItem)
{
    kDebug();
    if (d->abstractGroupingStrategy) {
        return d->abstractGroupingStrategy->addItemToGroup(item, groupItem);
    //    kDebug() << d->abstractGroupingStrategy->type() << ManualGrouping;
        /*if (d->abstractGroupingStrategy->type() == ManualGrouping) {
   //         kDebug();
            return (qobject_cast<ManualGroupingStrategy*>(d->abstractGroupingStrategy))->addItemToGroup(item,groupItem);
        }*/
    }
    return false;
}

bool GroupManager::manualGroupingRequest(ItemList items)
{
    kDebug();
    if (d->abstractGroupingStrategy) {
     //   kDebug() << d->abstractGroupingStrategy->type() << ManualGrouping;
        if (d->abstractGroupingStrategy->type() == ManualGrouping) {
      //      kDebug();
            return (qobject_cast<ManualGroupingStrategy*>(d->abstractGroupingStrategy))->groupItems(items);
        }
    }
    return false;
}

bool GroupManager::manualSortingRequest(AbstractGroupableItem* taskItem, int newIndex)
{
    kDebug();
    if (d->abstractSortingStrategy) {
        if (d->abstractSortingStrategy->type() == ManualSorting) {
            return (qobject_cast<ManualSortingStrategy*>(d->abstractSortingStrategy))->moveItem(taskItem, newIndex);
        }
    }
    return false;
}


GroupPtr GroupManager::rootGroup() const
{
    return d->rootGroup;
}


void GroupManagerPrivate::currentDesktopChanged(int newDesktop)
{
    kDebug();
    if (!showOnlyCurrentDesktop) {
        return;
    }

    if (abstractSortingStrategy) {
        abstractSortingStrategy->desktopChanged(newDesktop);
    }

    if (abstractGroupingStrategy) {
        abstractGroupingStrategy->desktopChanged(newDesktop);
    }

    reloadTasks();
}


void GroupManagerPrivate::taskChanged(TaskPtr task, ::TaskManager::TaskChanges changes)
{
    kDebug();
    bool takeAction = false;
    bool show = true;

    if (showOnlyCurrentDesktop && changes & ::TaskManager::DesktopChanged) {
        takeAction = true;
        show = task->isOnCurrentDesktop();
        //kDebug() << task->visibleName() << "on" << TaskManager::self()->currentDesktop();
    }

    if (showOnlyMinimized && changes & ::TaskManager::StateChanged) {
        //TODO: wouldn't it be nice to get notification of JUST minimization?
        takeAction = true;
        show = task->isMinimized();
    }

    if (changes & ::TaskManager::GeometryChanged) {
        //kDebug() << "geomtery Changed";
        if (!geometryTasks.contains(task)) {
            geometryTasks.append(task);
        }

        if (!screenTimer.isActive()) {
            screenTimer.start();
        }
    }

    //show tasks anyway if they demand attention
    if (changes & ::TaskManager::StateChanged && task->demandsAttention()) {
        takeAction = true;
        show = true;
    }

    if (!takeAction) {
        return;
    }

    if (show) {
        //kDebug() << "add(task);";
        q->add(task);
    } else {
        //kDebug() << "remove(task);";
        q->remove(task);
    }
}

void GroupManager::setScreen(int screen)
{
    kDebug() << "new Screen: " << screen;
    d->currentScreen = screen;
}


void GroupManagerPrivate::checkScreenChange()
{
    //kDebug();
    foreach (const TaskPtr &task, geometryTasks) {
        if (!task->isOnScreen(currentScreen) && !task->demandsAttention()) {
            q->remove(task);
        } else {
            q->add(task);
        }
    }
    geometryTasks.clear();
}


void GroupManager::reconnect()
{
    kDebug();
    disconnect(TaskManager::self(), SIGNAL(desktopChanged(int)),
               this, SLOT(currentDesktopChanged(int)));
    disconnect(TaskManager::self(), SIGNAL(windowChanged(TaskPtr,::TaskManager::TaskChanges)),
               this, SLOT(taskChanged(TaskPtr,::TaskManager::TaskChanges)));

    if (d->showOnlyCurrentDesktop || d->showOnlyMinimized || d->showOnlyCurrentScreen) {
        // listen to the relevant task manager signals
        if (d->showOnlyCurrentDesktop) {
            connect(TaskManager::TaskManager::self(), SIGNAL(desktopChanged(int)),
                    this, SLOT(currentDesktopChanged(int)));
        }

        connect(TaskManager::self(), SIGNAL(windowChanged(TaskPtr,::TaskManager::TaskChanges)),
                this, SLOT(taskChanged(TaskPtr,::TaskManager::TaskChanges)));
    }

    if (d->showOnlyCurrentScreen) {
        TaskManager::TaskManager::self()->trackGeometry(true);
    } else {
        TaskManager::TaskManager::self()->trackGeometry(false);
    }

    d->reloadTasks();
}


bool GroupManager::onlyGroupWhenFull() const
{
    return d->onlyGroupWhenFull;
}

void GroupManager::setOnlyGroupWhenFull(bool state)
{
    kDebug() << state;
    d->onlyGroupWhenFull = state;

    if (state) {
        connect(d->rootGroup, SIGNAL(itemAdded(AbstractItemPtr)), this, SLOT(checkIfFull()));
        connect(d->rootGroup, SIGNAL(itemRemoved(AbstractItemPtr)), this, SLOT(checkIfFull()));
    } else {
        disconnect(d->rootGroup, SIGNAL(itemAdded(AbstractItemPtr)), this, SLOT(checkIfFull()));
        disconnect(d->rootGroup, SIGNAL(itemRemoved(AbstractItemPtr)), this, SLOT(checkIfFull()));
    }
}

void GroupManager::setFullLimit(int limit)
{
    kDebug() << limit;
    d->groupIsFullLimit = limit;
    if (!onlyGroupWhenFull()) {
        return;
    }
    d->checkIfFull();
}

void GroupManagerPrivate::checkIfFull()
{
    kDebug();
    if (!q->onlyGroupWhenFull()) {
        return;
    }
    if (groupingStrategy != GroupManager::ProgramGrouping) {
        return;
    }
    if (itemList.size() >= groupIsFullLimit) {
        //kDebug() << "group is full, setting program grouping";
        q->setGroupingStrategy(GroupManager::ProgramGrouping);
    } else {
        //kDebug() << "group is not full";
        q->setGroupingStrategy(GroupManager::NoGrouping);
        //let the visualization thing we still use the programGrouping
        groupingStrategy = GroupManager::ProgramGrouping;
    }
}

bool GroupManager::showOnlyCurrentScreen() const
{
    return d->showOnlyCurrentScreen;
}

void GroupManager::setShowOnlyCurrentScreen(bool showOnlyCurrentScreen)
{
    d->showOnlyCurrentScreen = showOnlyCurrentScreen;
}

bool GroupManager::showOnlyCurrentDesktop() const
{
    return d->showOnlyCurrentDesktop;
}

void GroupManager::setShowOnlyCurrentDesktop(bool showOnlyCurrentDesktop)
{
    d->showOnlyCurrentDesktop = showOnlyCurrentDesktop;
}

bool GroupManager::showOnlyMinimized() const
{
    return d->showOnlyMinimized;
}

void GroupManager::setShowOnlyMinimized(bool showOnlyMinimized)
{
    d->showOnlyMinimized = showOnlyMinimized;
}

GroupManager::TaskSortingStrategy GroupManager::sortingStrategy() const
{
    return d->sortingStrategy;
}

AbstractSortingStrategy* GroupManager::taskSorter() const
{
    return d->abstractSortingStrategy;
}

void GroupManager::setSortingStrategy(TaskSortingStrategy sortOrder)
{
    kDebug() << sortOrder;

    if (d->abstractSortingStrategy) {
        if (d->abstractSortingStrategy->type() == sortOrder){
            return;
        } else {
            d->abstractSortingStrategy->deleteLater();
        }
    }

    switch (sortOrder) {
        case NoSorting: //manual and no grouping result both in non automatic grouping
            d->abstractSortingStrategy = 0;
            break;
        case ManualSorting:
            d->abstractSortingStrategy = new ManualSortingStrategy(this);
            d->abstractSortingStrategy->handleGroup(d->rootGroup);
            break;

        case AlphaSorting:
            d->abstractSortingStrategy = new AlphaSortingStrategy(this);
            d->abstractSortingStrategy->handleGroup(d->rootGroup);
            break;

        case DesktopSorting:
            kDebug() << "Strategy not implemented";
            //d->abstractSortingStrategy = new DesktopSortingStrategy(this);
            break;

        default:
            kDebug() << "Invalid Strategy";
            d->abstractSortingStrategy = 0;
    }
    d->sortingStrategy = sortOrder;
    d->reloadTasks();
}

GroupManager::TaskGroupingStrategy GroupManager::groupingStrategy() const
{
    return d->groupingStrategy;
}

AbstractGroupingStrategy* GroupManager::taskGrouper() const
{
    return d->abstractGroupingStrategy;
}

void GroupManager::setGroupingStrategy(TaskGroupingStrategy strategy)
{
    kDebug() << strategy;

    if (d->abstractGroupingStrategy) {
        if (d->abstractGroupingStrategy->type() == strategy){
            return;
        } else {
            d->abstractGroupingStrategy->deleteLater();
        }
    }

    switch (strategy) {
        case NoGrouping:
            d->abstractGroupingStrategy = 0;
            break;
        case ManualGrouping:
            d->abstractGroupingStrategy = new ManualGroupingStrategy(this);
            break;

        case ProgramGrouping:
            d->abstractGroupingStrategy = new ProgramGroupingStrategy(this);
            break;

        default:
            kDebug() << "Strategy not implemented";
            d->abstractGroupingStrategy = 0;
    }
    d->groupingStrategy = strategy;
    d->reloadTasks();
}


} // TaskManager namespace

#include "groupmanager.moc"

