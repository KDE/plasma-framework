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

#ifndef TASKACTIONS_H
#define TASKACTIONS_H

// Qt
#include <QtGui/QMenu>
#include <QtGui/QAction>

// Own
#include <taskmanager/groupmanager.h>
#include <taskmanager/task.h>
#include <taskmanager/taskgroup.h>
#include <taskmanager/taskitem.h>
#include <taskmanager/taskmanager_export.h>

namespace TaskManager
{

enum GroupableAction { MaximizeAction = 0,
                       MinimizeAction,
                       ToCurrentDesktopAction,
                       ToDesktopAction,
                       ShadeAction,
                       CloseAction,
                       ViewFullscreenAction,
                       KeepBelowAction
                    };

enum TaskAction { ResizeAction = 0,
                  MoveAction
                 };

enum GroupingAction { LeaveGroupAction = 0
                      };

/** 
 * Factory method to create standard actions for groupable items.
 * 
 * @param action the action to create
 * @param item the groupable item to associate it with
 * @param parent the parent for the action
 * @param desktop the desktop to associate the action with, only used for ToDesktopAction
 */
QAction *standardGroupableAction(GroupableAction action, AbstractItemPtr item, QObject *parent = 0, int desktop = 0);

/**
 * Factory method to create standard actions for groupable items.
 *
 * @param action the action to create
 * @param task the task to associate it with
 * @param parent the parent for the action
 */
QAction *standardTaskAction(TaskAction action, TaskItem *task, QObject *parent = 0);

/**
 * Factory method to create standard actions for groupable items.
 *
 * @param action the action to create
 * @param item the groupable item to associate it with
 * @param strategy the GroupManager used to coorinate the grouping
 * @param parent the parent for the action
 */
QAction* standardGroupingAction(GroupingAction action, AbstractItemPtr item,
                                GroupManager *strategy, QObject *parent = 0);

/** The ToDesktop menu */
class TASKMANAGER_EXPORT DesktopsMenu : public QMenu
{
    Q_OBJECT
public:
    DesktopsMenu(QWidget *parent, AbstractItemPtr task);
};

/** Menu with the actions that the groupingStrategy provides*/
class TASKMANAGER_EXPORT GroupingStrategyMenu : public QMenu
{
    Q_OBJECT
public:
    GroupingStrategyMenu(QWidget *parent, AbstractGroupableItem *task, GroupManager *strategy);
};

/** The Advanced menu */
class TASKMANAGER_EXPORT AdvancedMenu : public QMenu
{
    Q_OBJECT
public:
    AdvancedMenu(QWidget *parent, AbstractItemPtr task);
};

/** The standard menu*/
class TASKMANAGER_EXPORT BasicMenu : public QMenu
{
    Q_OBJECT
public:
    BasicMenu(QWidget *parent, GroupPtr task, GroupManager *strategy, QList <QAction*> visualizationActions = QList <QAction*>());
    BasicMenu(QWidget *parent, TaskItem* task, GroupManager *strategy, QList <QAction*> visualizationActions = QList <QAction*>());
};

/** A Menu that shows  a list of all tasks of the group, and shows a BasicMenu on right click on an item*/
class TASKMANAGER_EXPORT GroupPopupMenu : public QMenu
{
    Q_OBJECT
public:
    GroupPopupMenu(QWidget *parent, GroupPtr task, GroupManager *strategy);
};


} // TaskManager namespace


#endif
