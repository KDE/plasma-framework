/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef TASKITEM_H
#define TASKITEM_H

#include <taskmanager/abstractgroupableitem.h>
#include <taskmanager/startup.h>
#include <taskmanager/task.h>
#include <taskmanager/taskmanager_export.h>

#include <QtGui/QIcon>

namespace TaskManager
{


/**
 * Wrapper class so we do not have to use the Task class directly and the TaskPtr remains guarded
 */
class TASKMANAGER_EXPORT TaskItem : public AbstractGroupableItem
{
    Q_OBJECT
public:
    /** Creates a taskitem for a task*/
    TaskItem(QObject *parent, TaskPtr item);
    /** Creates a taskitem for a startuptask*/
    TaskItem(QObject *parent, StartupPtr item);
    ~TaskItem();
    /** Sets the taskpointer after the startup pointer */
    void setTaskPointer(TaskPtr);
    /** Returns the shared pointer to the  Task */
    TaskPtr task() const;

    StartupPtr startup() const;
    bool isGroupItem() const { return false; }

    QIcon icon() const;
    QString name() const;

    bool isOnCurrentDesktop() const;
    bool isOnAllDesktops() const;
    int desktop() const;
    bool isShaded() const;
    bool isMaximized() const;
    bool isMinimized() const;
    bool isFullScreen() const;
    bool isKeptBelowOthers() const;
    bool isAlwaysOnTop() const;
    bool isActive() const;
    bool demandsAttention() const;
    bool isActionSupported(NET::Action) const;

public Q_SLOTS:
    void toDesktop(int);

    void setShaded(bool);
    void toggleShaded();

    void setMaximized(bool);
    void toggleMaximized();

    void setMinimized(bool);
    void toggleMinimized();

    void setFullScreen(bool);
    void toggleFullScreen();

    void setKeptBelowOthers(bool);
    void toggleKeptBelowOthers();

    void setAlwaysOnTop(bool);
    void toggleAlwaysOnTop();

    void close();

Q_SIGNALS:
    /** Indicates that the startup task now is a normal task */
    void gotTaskPointer();

private:
    class Private;
    Private * const d;
};

} // TaskManager namespace

#endif
