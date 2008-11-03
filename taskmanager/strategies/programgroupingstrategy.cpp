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

#include "programgroupingstrategy.h"

#include <QAction>

#include <KDebug>
#include <KLocale>

#include "abstractgroupingstrategy.h"
#include "groupmanager.h"

namespace TaskManager
{

class ProgramGroupingStrategy::Private
{
public:
    Private()
        :editableGroupProperties(AbstractGroupingStrategy::None)
    {
    }
    GroupManager *groupManager;
    AbstractGroupingStrategy::EditableGroupProperties editableGroupProperties;
    AbstractGroupableItem *tempItem;
    QStringList blackList; //Programs in this list should not be grouped
};


ProgramGroupingStrategy::ProgramGroupingStrategy(GroupManager *groupManager)
    :AbstractGroupingStrategy(groupManager),
     d(new Private)
{
    d->groupManager = groupManager;
    setType(GroupManager::ProgramGrouping);
}

ProgramGroupingStrategy::~ProgramGroupingStrategy()
{
    delete d;
}

QList<QAction*> ProgramGroupingStrategy::strategyActions(QObject *parent, AbstractGroupableItem *item)
{
    QAction *a = new QAction(parent);
    QString name = className(item);
    if (d->blackList.contains(name)) {
        a->setText(i18n("Allow This Program to Be Grouped"));
    } else {
        a->setText(i18n("Do Not Allow This Program to Be Grouped"));
    }
    connect(a, SIGNAL(triggered()), this, SLOT(toggleGrouping()));

    QList<QAction*> actionList;
    actionList.append(a);
    d->tempItem = item;
    return actionList;
}

QString ProgramGroupingStrategy::className(AbstractGroupableItem *item)
{
    QString name;
    if (item->isGroupItem()) { //maybe add the condition that the subgroup was created by programGrouping
        TaskGroup *group = qobject_cast<TaskGroup*>(item);
        TaskItem *task = qobject_cast<TaskItem*>(group->members().first()); //There are only TaskItems in programGrouping groups
        return task->task()->classClass();
    }

    return (qobject_cast<TaskItem*>(item))->task()->classClass();
}

void ProgramGroupingStrategy::toggleGrouping()
{
    QString name = className(d->tempItem);

    if (d->blackList.contains(name)) {
        d->blackList.removeAll(name);
        if (d->tempItem->isGroupItem()) {
            foreach (AbstractGroupableItem *item, (qobject_cast<TaskGroup*>(d->tempItem))->members()) {
                handleItem(item);
            }
        } else {
            handleItem(d->tempItem);
        }
    } else {
        d->blackList.append(name);
        if (d->tempItem->isGroupItem()) {
            closeGroup(qobject_cast<TaskGroup*>(d->tempItem));
        } else {
            d->groupManager->rootGroup()->add(d->tempItem);
        }
    }
    d->tempItem = 0;
}

void ProgramGroupingStrategy::handleItem(AbstractItemPtr item)
{
    if (item->isGroupItem()) {
        d->groupManager->rootGroup()->add(item);
        return;
    } else if (d->blackList.contains((qobject_cast<TaskItem*>(item))->task()->classClass())) {
        d->groupManager->rootGroup()->add(item);
        return;
    }

    TaskItem *task = dynamic_cast<TaskItem*>(item);
    if (task && !programGrouping(task, d->groupManager->rootGroup())) {
        kDebug() << "joined rootGroup ";
        d->groupManager->rootGroup()->add(item);
    }
}

bool ProgramGroupingStrategy::programGrouping(TaskItem* taskItem, TaskGroup* groupItem)
{
    kDebug();
    QHash <QString,AbstractItemPtr> itemMap;

    foreach (AbstractItemPtr item, groupItem->members()) { //search for an existing group
        if (item->isGroupItem()) { //maybe add the condition that the subgroup was created by programGrouping
            if (programGrouping(taskItem, static_cast<TaskGroup*>(item))) {
                kDebug() << "joined subGroup";
                return true;
            }
        } else {
            TaskItem *task = static_cast<TaskItem*>(item);
            if (task->task()) { //omit startup tasks
                QString name = task->task()->classClass();
                itemMap.insertMulti(name,item);
            }
        }
    }

    if (!itemMap.values().contains(taskItem)) {
        itemMap.insertMulti(taskItem->task()->classClass(), taskItem);
    }

    QString name = taskItem->task()->classClass();
    if (itemMap.count(name) >= groupItem->members().count()) { //join this group
        kDebug() << "joined this Group";
        groupItem->add(taskItem);
        return true;
    } else if (itemMap.count(name) >= 2) { //create new subgroup with at least 2 other task
        kDebug() << "create Group";
        QIcon icon = taskItem->task()->icon();
        QList <AbstractItemPtr> list(itemMap.values(name));
        TaskGroup* group = createGroup(list);
        group->setName(name);
        group->setColor(Qt::red);
        group->setIcon(icon);
        return true;
    }
    return false;
}

void ProgramGroupingStrategy::checkGroup()
{
    TaskGroup *group = qobject_cast<TaskGroup*>(sender()); 
    if (!group) {
        return;
    }
    if (group->members().size() <= 1) {
        closeGroup(group);
    }
}

}//namespace

#include "programgroupingstrategy.moc"

