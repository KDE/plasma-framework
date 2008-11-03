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

#ifndef GROUPMANAGER_H
#define GROUPMANAGER_H

#include <QtCore/QObject>

#include <taskmanager/abstractgroupableitem.h>
#include <taskmanager/task.h>
#include <taskmanager/taskitem.h>
#include <taskmanager/taskmanager_export.h>

namespace TaskManager
{

class AbstractSortingStrategy;
class AbstractGroupingStrategy;
class GroupManagerPrivate;

/**
 * Manages the grouping stuff. It doesn't know anything about grouping and sorting itself, this is done in the grouping and sorting strategies.
 */
class TASKMANAGER_EXPORT GroupManager: public QObject
{

Q_OBJECT
public:
    GroupManager(QObject *parent);
    ~GroupManager();

    /**
    * Returns a group which contains all items and subgroups.
    * Visualizations should be based on this.
    */
    GroupPtr rootGroup() const;

    /**
    * Strategy used to Group new items
    */
    enum TaskGroupingStrategy
    {
        NoGrouping = 0,
        ManualGrouping = 1, //Allow manual grouping
        ProgramGrouping = 2 //Group automatically  same programs
    };

    TaskGroupingStrategy groupingStrategy() const;
    void setGroupingStrategy(TaskGroupingStrategy);
    AbstractGroupingStrategy* taskGrouper() const;


    /**
    * How the task are ordered
    */
    enum TaskSortingStrategy
    {
        NoSorting = 0,
        ManualSorting = 1,
        AlphaSorting = 2,
        DesktopSorting = 3
    };

    TaskSortingStrategy sortingStrategy() const;
    void setSortingStrategy(TaskSortingStrategy);
    AbstractSortingStrategy* taskSorter() const;

    bool showOnlyCurrentScreen() const;
    void setShowOnlyCurrentScreen(bool);

    bool showOnlyCurrentDesktop() const;
    void setShowOnlyCurrentDesktop(bool);

    bool showOnlyMinimized() const;
    void setShowOnlyMinimized(bool);

    bool onlyGroupWhenFull() const;
    /**
    * Only apply the grouping startegy when the taskbar is full according to
    * setFullLimit(int). This is currently limited to ProgramGrouping.
    */
    void setOnlyGroupWhenFull(bool state);
    /**
    * Set the limit when the taskbar is considered as full
    */
    void setFullLimit(int limit);

    /**
     * Functions to call if the user wants to do something manually, the strategy allows or refuses the request
     */
    bool manualGroupingRequest(AbstractGroupableItem* taskItem, TaskGroup* groupItem);
    bool manualGroupingRequest(ItemList items);

    bool manualSortingRequest(AbstractGroupableItem* taskItem, int newIndex);

    /**
     * The Visualization is responsible to update the screen number the visualization is currently on.
     */
    void setScreen(int screen);

Q_SIGNALS:
    /** Signal that the rootGroup has to be reloaded in the visualization */
    void reload();
    /** Signal that the item is no longer available */
    void itemRemoved(AbstractGroupableItem*);

public Q_SLOTS:
    /**
    * Slots for newly added tasks from TaskManager
    */
    bool add(TaskPtr);
    void remove(TaskPtr);

    void add(StartupPtr);
    void remove(StartupPtr);

    /**
    *   listen to the relevant signals of taskmanager
    */
    void reconnect();

private:
    Q_PRIVATE_SLOT(d, void currentDesktopChanged(int))
    Q_PRIVATE_SLOT(d, void taskChanged(TaskPtr, ::TaskManager::TaskChanges))
    Q_PRIVATE_SLOT(d, void checkScreenChange())
    Q_PRIVATE_SLOT(d, void itemDestroyed())
    Q_PRIVATE_SLOT(d, void checkIfFull())

    friend class GroupManagerPrivate;
    GroupManagerPrivate * const d;
};
}
#endif
