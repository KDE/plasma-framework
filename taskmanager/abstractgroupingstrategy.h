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

#ifndef ABSTRACTGROUPINGSTRATEGY_H
#define ABSTRACTGROUPINGSTRATEGY_H

#include <QtCore/QObject>

#include <taskmanager/abstractgroupableitem.h>
#include <taskmanager/groupmanager.h>
#include <taskmanager/taskgroup.h>
#include <taskmanager/taskmanager_export.h>

class QAction;

namespace TaskManager
{

/**
 * Base class for strategies which can be used to
 * automatically group tasks.
 */
class TASKMANAGER_EXPORT AbstractGroupingStrategy : public QObject
{
    Q_OBJECT
public:
    AbstractGroupingStrategy(GroupManager *groupManager);
    virtual ~AbstractGroupingStrategy();

     /** Handles a new item */
    virtual void handleItem(AbstractItemPtr) = 0;

     /** Returns the strategy type */
    GroupManager::TaskGroupingStrategy type() const;

    /** DesktopChanges time to backup any needed data */
    virtual void desktopChanged(int newDesktop);

    /** Returns list of actions that a task can do in this groupingStrategy
    *  If the visualization supports grouping it has to show these actions.
    */
    virtual QList<QAction*> strategyActions(QObject *parent, AbstractGroupableItem *item);

    enum EditableGroupProperties
    {
        None = 0,
        Name = 1,
        Color = 2,
        Icon =  4,
        Members = 8,
        All = 15
    };
    /** Returns which group properties are editable by the user and which are handled solely by the strategy. 
    * The visualization should create a configuration interface based on this.
    */
    virtual EditableGroupProperties editableGroupProperties() = 0;

    /** The following functions check if a property is editable and sets it on group*/

    /**	Adds an item to group if EditableGroupProperties::Members is set*/
    virtual bool addItemToGroup(AbstractGroupableItem *, TaskGroup*);

    virtual bool setName(const QString &, TaskGroup*);
    /** Returns a List of unused Names*/
    virtual QList<QString> nameSuggestions(TaskGroup *);

    virtual bool setColor(const QColor &, TaskGroup*);
    /** Returns a list of unused colors*/
    virtual QList<QColor> colorSuggestions(TaskGroup *);

    virtual bool setIcon(const QIcon &, TaskGroup*);
    /** Returns a list of icons*/
    virtual QList<QIcon> iconSuggestions(TaskGroup *);

protected Q_SLOTS:
    /** Adds all group members to the parentgroup of group and removes the group */
    virtual void closeGroup(TaskGroup *group);

    /** Checks if the group is still necessary, removes group if empty*/
    virtual void checkGroup();

     /** Returns the strategy type */
    void setType(GroupManager::TaskGroupingStrategy type);

protected:
    /** Create a group with items and returns the newly created group */
    TaskGroup* createGroup(ItemList items);

private:
    class Private;
    Private * const d;
};

} // TaskManager namespace
#endif
