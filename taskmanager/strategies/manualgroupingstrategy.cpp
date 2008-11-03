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

#include "manualgroupingstrategy.h"

#include <QAction>

#include <KDebug>
#include <KLocale>

#include "abstractgroupingstrategy.h"
#include "groupmanager.h"
#include "taskmanager.h"


namespace TaskManager
{

class ManualGroupingStrategy::Private
{
public:
    Private()
        : currentTemplate(0),
          editableGroupProperties(AbstractGroupingStrategy::All),
          tempItem(0),
          tempGroup(0),
          oldDesktop(TaskManager::self()->currentDesktop())
    {
    }

    GroupManager *groupManager;
    QHash<int, TaskGroupTemplate*> templateTrees;
    TaskGroupTemplate* currentTemplate;
    QList<TaskGroup*> protectedGroups;
    AbstractGroupingStrategy::EditableGroupProperties editableGroupProperties;
    AbstractGroupableItem *tempItem;
    TaskGroup *tempGroup;
    int oldDesktop;
};



ManualGroupingStrategy::ManualGroupingStrategy(GroupManager *groupManager)
    :AbstractGroupingStrategy(groupManager),
     d(new Private)
{
    d->groupManager = groupManager;
    setType(GroupManager::ManualGrouping);
}

ManualGroupingStrategy::~ManualGroupingStrategy()
{
    delete d;
}

AbstractGroupingStrategy::EditableGroupProperties ManualGroupingStrategy::editableGroupProperties()
{
    return d->editableGroupProperties;
}

QList<QAction*> ManualGroupingStrategy::strategyActions(QObject *parent, AbstractGroupableItem *item)
{
    QList<QAction*> actionList;

    if (item->isGrouped()) {
        QAction *a = new QAction(i18n("Leave Group"), parent);
        connect(a, SIGNAL(triggered()), this, SLOT(leaveGroup()));
        actionList.append(a);
        d->tempItem = item;
    }

    if (item->isGroupItem()) {
        QAction *a = new QAction(i18n("Remove Group"), parent);
        connect(a, SIGNAL(triggered()), this, SLOT(removeGroup()));
        actionList.append(a);
        d->tempGroup = dynamic_cast<TaskGroup*>(item);
    }

    return actionList;
}

void ManualGroupingStrategy::leaveGroup()
{
    Q_ASSERT(d->tempItem);
    if (d->tempItem->isGrouped()) {
        d->tempItem->parentGroup()->parentGroup()->add(d->tempItem);
    }
    d->tempItem = 0;
}

void ManualGroupingStrategy::removeGroup()
{
    Q_ASSERT(d->tempGroup);
    if (d->tempGroup->parentGroup()) {
        foreach (AbstractGroupableItem *item, d->tempGroup->members()) {
            d->tempGroup->parentGroup()->add(item);
        }
        //Group gets automatically closed on empty signal
    }
    d->tempGroup = 0;
}

void ManualGroupingStrategy::unprotectGroup(TaskGroup *group)
{
    kDebug() << group->name() << d->protectedGroups.count(group);
    d->protectedGroups.removeOne(group);
    if (group->members().isEmpty()) {
        closeGroup(group);//check if group is needed anymore
    }
}

void ManualGroupingStrategy::protectGroup(TaskGroup *group)
{
    kDebug() << group->name();
    d->protectedGroups.append(group);
}

//Check if the item was previously manually grouped
void ManualGroupingStrategy::handleItem(AbstractItemPtr item)
{
    kDebug();
    if (d->currentTemplate) { //TODO this won't work over sessions because the task is identified by the pointer (maybe the name without the current status would work), one way would be to store the items per name if the session is closed and load them per name on startup but use the pointer otherwise because of changing names of browsers etc
        TaskGroupTemplate *templateGroup = d->currentTemplate;
        kDebug() << templateGroup->name();
        if (templateGroup->hasMember(item)) { 
            kDebug() << "item found in template tree";
            while(!templateGroup->hasDirectMember(item)) {//Create tree of groups if not already existing
                kDebug() << "Creating group tree";
                TaskGroupTemplate *oldTemplateGroup = templateGroup;
                AbstractGroupableItem *templateItem = templateGroup->directMember(item);
                if (templateItem->isGroupItem()) {
                    templateGroup = dynamic_cast<TaskGroupTemplate*>(templateItem);
                } else {
                    kDebug() << "Error no template Found";
                }
                if (templateGroup->group()) {
                    oldTemplateGroup->group()->add(templateGroup->group()); //add group to parent Group
                } else {
                    //kDebug();
                    d->groupManager->rootGroup()->add(item);
                    return;
                }
            }

            kDebug() << "Item added to group: " << templateGroup->name();
            templateGroup->group()->add(item);
            templateGroup->remove(item);
        } else {
            kDebug() << "Item not in templates";
            d->groupManager->rootGroup()->add(item);
        }
    } else {
        d->groupManager->rootGroup()->add(item);
    }
}

TaskGroupTemplate *ManualGroupingStrategy::createDuplication(TaskGroup *group)
{
    TaskGroupTemplate *templateGroup = new TaskGroupTemplate(this, group);
    return templateGroup;
}


void ManualGroupingStrategy::desktopChanged(int newDesktop)
{
    kDebug() << "old: " << d->oldDesktop << "new: " << newDesktop;
    if (d->oldDesktop == newDesktop) {
        return;
    }

    //Store the group under the current Desktop
    if (d->currentTemplate) {
        d->currentTemplate->clear();
    }
    kDebug();
    TaskGroupTemplate *group = createDuplication(d->groupManager->rootGroup());
    d->templateTrees.insert(d->oldDesktop, group); 
    if (d->templateTrees.contains(newDesktop)) {
        kDebug() << "Template found";
        d->currentTemplate = d->templateTrees.value(newDesktop);
        connect (d->currentTemplate, SIGNAL(destroyed()), this, SLOT(resetCurrentTemplate()));
    } else {
        d->currentTemplate = 0;
    }
    d->oldDesktop = newDesktop;
}


//This function makes sure that if the rootGroup template already got deleted nobody tries to access it again
void ManualGroupingStrategy::resetCurrentTemplate()
{
    kDebug();
    d->currentTemplate = 0;
}

//The group was moved to another desktop, we have to move it to the rootTree of newDesk
void ManualGroupingStrategy::groupChangedDesktop(int newDesktop)
{
    kDebug();
    TaskGroup *group = qobject_cast<TaskGroup*>(sender());
    if (!group) {
        return;
    }
    if (newDesktop && (newDesktop != d->oldDesktop)) {
        if (group->parentGroup()) {
            group->parentGroup()->remove(group);
        }
    }
    TaskGroupTemplate *templateGroup;
if (newDesktop) {
    if (d->templateTrees.contains(newDesktop)) {
        kDebug() << "Template found";
        templateGroup = d->templateTrees.value(newDesktop);
    } else {
        kDebug() << "No Template found";
        templateGroup = new TaskGroupTemplate(this, 0);
        templateGroup->setGroup(d->groupManager->rootGroup());
        d->templateTrees.insert(newDesktop, templateGroup);
    }
    //Add group to all existing desktops
} else {
    for (int i = 1; i <= TaskManager::self()->numberOfDesktops(); i++) {
        if (d->templateTrees.contains(newDesktop)) {
            kDebug() << "Template found";
            templateGroup = d->templateTrees.value(newDesktop);
            if (templateGroup->hasMember(group)) {
                continue;
            }
        } else {
            kDebug() << "No Template found";
            templateGroup = new TaskGroupTemplate(this, 0);
            templateGroup->setGroup(d->groupManager->rootGroup());
            d->templateTrees.insert(newDesktop, templateGroup);
        }
        templateGroup->add(createDuplication(group));
    }
}

}

bool ManualGroupingStrategy::groupItems(ItemList items)
{
    kDebug();
    TaskGroup *group = createGroup(items);
    connect(group, SIGNAL(movedToDesktop(int)), this, SLOT(groupChangedDesktop(int)));
    setName(nameSuggestions(group).first(), group);
    setColor(colorSuggestions(group).first(), group);
    setIcon(iconSuggestions(group).first(), group);
    return true;
}

void ManualGroupingStrategy::closeGroup(TaskGroup *group)
{
    kDebug();
    if (!d->protectedGroups.contains(group)) {
        AbstractGroupingStrategy::closeGroup(group);
    } else if (group->parentGroup()) {
        group->parentGroup()->remove(group);
        kDebug() << "Group protected";
    }
}

class TaskGroupTemplate::Private
{
public:
    Private()
    : group(0),
      parentGroup(0),
      groupingStrategy(0)
    {
    }

    ItemList members;
    QString name;
    QColor color;
    QIcon icon;
    TaskGroup *group;
    TaskGroupTemplate *parentGroup;
    ManualGroupingStrategy *groupingStrategy;
};


TaskGroupTemplate::TaskGroupTemplate(ManualGroupingStrategy *parent, TaskGroup *group)
:   AbstractGroupableItem(parent),
    d(new Private)
{
    connect(this, SIGNAL(unprotectGroup(TaskGroup *)), parent, SLOT(unprotectGroup(TaskGroup *)));
    connect(this, SIGNAL(protectGroup(TaskGroup *)), parent, SLOT(protectGroup(TaskGroup *)));
    if (group) {
        d->name = group->name();
        d->color = group->color();
        d->icon = group->icon();
        setGroup(group);
        foreach (AbstractGroupableItem *item, group->members()) {
            //We don't use TaskGroup::add because this would inform the tasks about the change of the group
            //and we use the taskgroup just as a temporary container
            if (item->isGroupItem()) {
                kDebug() << "GroupItem Duplication";
                TaskGroupTemplate *createdDuplication = new TaskGroupTemplate(parent, dynamic_cast<TaskGroup*>(item));
                add(createdDuplication);
            } else {
                add(item);
            }
        }
    }
    kDebug() << "TemplateGroup Created: Name: " << d->name << "Color: " << d->color;
}

TaskGroupTemplate::~TaskGroupTemplate()
{
    emit unprotectGroup(group());
    emit destroyed(this);
    //clear();
    kDebug() << name();
    delete d;
}

TaskGroup *TaskGroupTemplate::group()
{
    return d->group;
}

void TaskGroupTemplate::setGroup(TaskGroup *group)
{
    if (d->group) {
        emit unprotectGroup(group);
    }
    if (group) {
        emit protectGroup(group);
    }
    d->group = group;
}

ItemList &TaskGroupTemplate::members() const
{
    return d->members;
}

QIcon TaskGroupTemplate::icon() const
{
    return d->icon;
}

QColor TaskGroupTemplate::color() const
{
    return d->color;
}

QString TaskGroupTemplate::name() const
{
    return d->name;
}

/** add item to group */
void TaskGroupTemplate::add(AbstractItemPtr item)
{
    if (d->members.contains(item)) {
        return;
    }
    d->members.append(item);
    if (item->isGroupItem()) { 
        connect(item, SIGNAL(destroyed(AbstractGroupableItem *)), this, SLOT(itemDestroyed(AbstractGroupableItem *)));
        (dynamic_cast<TaskGroupTemplate*>(item))->setParentGroup(this);
    }
}

/** remove item from group */
void TaskGroupTemplate::remove(AbstractItemPtr item)
{
    disconnect(item, 0, this, 0);
    disconnect(this, 0, item, 0);
    d->members.removeAll(item);
    if (item->isGroupItem()) { 
        (dynamic_cast<TaskGroupTemplate*>(item))->setParentGroup(0);
    }
    if (d->members.isEmpty()) {
        closeGroup();
    }
}

/** Removes all tasks and groups from this group */
void TaskGroupTemplate::clear()
{
    foreach(AbstractGroupableItem *item, d->members) {
        Q_ASSERT(item);
        if (item->isGroupItem()) { 
            TaskGroupTemplate* templateGroup = qobject_cast<TaskGroupTemplate*>(item);
            Q_ASSERT(templateGroup);
            templateGroup->clear();
        } else {
            remove(item);
        }
    }
}
/** Reparents all members and closes this group */
void TaskGroupTemplate::closeGroup()
{
    if (parentGroup()) {
        foreach(AbstractGroupableItem *item, d->members) {
            Q_ASSERT(item);
            remove(item);
            parentGroup()->add(item);
        }
    } else {
        foreach(AbstractGroupableItem *item, d->members) {
            Q_ASSERT(item);
            remove(item);
        }
    }
    deleteLater();
}


TaskGroupTemplate *TaskGroupTemplate::parentGroup() const
{
    return d->parentGroup;
}

void TaskGroupTemplate::setParentGroup(TaskGroupTemplate *group)
{
    d->parentGroup = group;
}


/** only true if item is in this group */
bool TaskGroupTemplate::hasDirectMember(AbstractItemPtr item) const
{
    return d->members.contains(item);
}

/** true if item is in this or any sub group */
bool TaskGroupTemplate::hasMember(AbstractItemPtr item) const
{
    kDebug();
    if (members().contains(item)) {
        return true;
    }
    ItemList::const_iterator iterator = members().constBegin();
    while (iterator != members().constEnd()) {
        if ((*iterator)->isGroupItem()) {
            if ((dynamic_cast<TaskGroupTemplate *>(*iterator))->hasMember(item)) { //look into group
                return true;
            }
        }
        ++iterator;
    }
    return false;

}

/** Returns Direct Member group if the passed item is in a subgroup */
AbstractItemPtr TaskGroupTemplate::directMember(AbstractItemPtr item) const
{
    if (members().contains(item)) {
        return item;
    } else {
        ItemList::const_iterator iterator = members().constBegin();
        while (iterator != members().constEnd()) {
            if ((*iterator)->isGroupItem()) {
                if ((dynamic_cast<TaskGroupTemplate*>(*iterator))->hasMember(item)) {
                    kDebug() << "item found";
                    return (*iterator);
                }
            } 
            ++iterator;
        }
    }
    kDebug() << "item not found";
    return AbstractItemPtr();
}

TaskGroupTemplate *TaskGroupTemplate::findParentGroup(AbstractItemPtr item) const
{
    if (members().contains(item)) {
        return const_cast<TaskGroupTemplate*>(this);
    } else {
        ItemList::const_iterator iterator = members().constBegin();
        while (iterator != members().constEnd()) {
            if ((*iterator)->isGroupItem()) {
                TaskGroupTemplate *returnedGroup = (dynamic_cast<TaskGroupTemplate*>(*iterator))->findParentGroup(item);
                if (returnedGroup) {
                    kDebug() << "item found";
                    return returnedGroup;
                }
            }
            ++iterator;
        }
    }
    kDebug() << "item not found";
    return 0;
}

void TaskGroupTemplate::itemDestroyed(AbstractGroupableItem *item)
{
    if (!item) {
        kDebug() << "Error";
        return;
    }
    kDebug() << d->group->name(); 
    /**
    * The following code is needed in case one creates a group on desktop 1 with a task which is on multiple * * desktops. If this task gets closed on another desktop as last task in the group the empty group is still * stored in the template and therefore all attributes (name, color, ..) stay reserved
    */
    d->members.removeAll(item); // we can't use remove because the item was already deleted
    disconnect(item, 0, this, 0);
    disconnect(this, 0, item, 0);
    if (members().isEmpty()) {
        closeGroup();
    }
}

}//namespace

#include "manualgroupingstrategy.moc"

