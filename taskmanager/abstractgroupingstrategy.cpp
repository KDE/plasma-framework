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

#include "abstractgroupingstrategy.h"

#include <KDebug>
#include <KIcon>

#include "task.h"

namespace TaskManager
{

class AbstractGroupingStrategy::Private
{
public:
    Private()
        : type(GroupManager::NoGrouping)
    {
    }

    GroupManager *groupManager;
    QStringList usedNames;
    QList<QColor> usedColors;
    QList<TaskGroup*> createdGroups;
    GroupManager::TaskGroupingStrategy type;
};


AbstractGroupingStrategy::AbstractGroupingStrategy(GroupManager *groupManager)
    : QObject(groupManager),
      d(new Private)
{
    d->groupManager = groupManager;
}

AbstractGroupingStrategy::~AbstractGroupingStrategy()
{
    foreach (TaskGroup *group, d->createdGroups) { //cleanup all created groups
        closeGroup(group);
    }
    delete d;
}

GroupManager::TaskGroupingStrategy AbstractGroupingStrategy::type() const
{
    return d->type;
}

void AbstractGroupingStrategy::setType(GroupManager::TaskGroupingStrategy type)
{
    d->type = type;
}

void AbstractGroupingStrategy::desktopChanged(int newDesktop)
{
    Q_UNUSED(newDesktop)
}

QList<QAction*> AbstractGroupingStrategy::strategyActions(QObject *parent, AbstractGroupableItem *item)
{
    Q_UNUSED(parent)
    Q_UNUSED(item)
    return QList<QAction*>();
}


TaskGroup* AbstractGroupingStrategy::createGroup(ItemList items)
{
    kDebug();
    GroupPtr oldGroup;
    if (items.first()->isGrouped()) {
        oldGroup = items.first()->parentGroup();
    } else {
        oldGroup = d->groupManager->rootGroup();
    }

    TaskGroup *newGroup = new TaskGroup(d->groupManager);
    d->createdGroups.append(newGroup);
    connect(newGroup, SIGNAL(itemRemoved(AbstractItemPtr)), this, SLOT(checkGroup()));
    foreach (AbstractItemPtr item, items) {
        newGroup->add(item);
    }
    oldGroup->add(newGroup);
    return newGroup;
}

void AbstractGroupingStrategy::closeGroup(TaskGroup *group)
{
    Q_ASSERT(group);
    disconnect(group, 0, this, 0);
    kDebug();
    d->createdGroups.removeAll(group);
    d->usedNames.removeAll(group->name());
    d->usedColors.removeAll(group->color());
    //d->usedIcons.removeAll(group->icon());//TODO
    if (group->parentGroup()) {
        foreach (AbstractItemPtr item, group->members()) {
            group->parentGroup()->add(item);
        }
        group->parentGroup()->remove(group);
    } else {
        foreach (AbstractItemPtr item, group->members()) {
            d->groupManager->rootGroup()->add(item);
        }
        //group->clear();
    }
    group->deleteLater();
}

void AbstractGroupingStrategy::checkGroup()
{
    TaskGroup *group = qobject_cast<TaskGroup*>(sender()); 
    if (!group) {
        return;
    }

    if (group->members().size() <= 0) {
        closeGroup(group);
    }
}

bool AbstractGroupingStrategy::addItemToGroup(AbstractGroupableItem *item, TaskGroup *group)
{
    if (editableGroupProperties() & Members) {
        group->add(item);
        return true;
    }

    return false; 
}

bool AbstractGroupingStrategy::setName(const QString &name, TaskGroup *group)
{
    d->usedNames.removeAll(group->name());
    if ((editableGroupProperties() & Name) && (!d->usedNames.contains(name))) {
        //TODO editableGroupProperties shouldn't be tested here i think
        d->usedNames.append(name);
        group->setName(name);
        return true;
    }
    return false; 
}

//Returns 6 free names
QList<QString> AbstractGroupingStrategy::nameSuggestions(TaskGroup *)
{
    QList<QString> nameList;
    int i = 1;

    while (nameList.count() < 6) {
        if (!d->usedNames.contains("Group"+QString::number(i))) {
            nameList.append("Group"+QString::number(i));
        }
        i++;
    }

    if (nameList.isEmpty()) {
        nameList.append("default");
    }

    return nameList;
}

bool AbstractGroupingStrategy::setColor(const QColor &color, TaskGroup *group)
{
    d->usedColors.removeAll(group->color());

    if (editableGroupProperties() && (!d->usedColors.contains(color))) {
        d->usedColors.append(color);
        group->setColor(color);
        return true;
    }

    return false; 
}

QList<QColor> AbstractGroupingStrategy::colorSuggestions(TaskGroup *)
{
    QList<QColor> colorPool;
    //colorPool.append(Qt::red);
    colorPool.append(Qt::blue);
    colorPool.append(Qt::green);
    colorPool.append(Qt::yellow);

    QList<QColor> colorList;
    foreach (QColor color, colorPool) {
        if (!d->usedColors.contains(color)) {
            colorList.append(color);
        }
    }

    if (colorList.isEmpty()) {
        colorList.append(Qt::red);
    }

    return colorList;
}

bool AbstractGroupingStrategy::setIcon(const QIcon &icon, TaskGroup *group)
{ 
    if (editableGroupProperties() & Icon) {
        group->setIcon(icon);
        return true;
    }

    return false;
}

QList <QIcon> AbstractGroupingStrategy::iconSuggestions(TaskGroup *)
{
    QList <QIcon> iconList;
    iconList.append(KIcon("xorg"));
    return iconList;
}

}//namespace

#include "abstractgroupingstrategy.moc"

