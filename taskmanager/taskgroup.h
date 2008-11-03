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

#ifndef TASKGROUP_H
#define TASKGROUP_H

#include <QtGui/QIcon>

#include <taskmanager/abstractgroupableitem.h>
#include <taskmanager/taskmanager_export.h>

namespace TaskManager
{
class GroupManager;

/**
 * TaskGroup, a container for tasks and subgroups
 */
class TASKMANAGER_EXPORT TaskGroup : public AbstractGroupableItem
{
    Q_OBJECT
public:
    TaskGroup(GroupManager *parent, QString name, QColor color);
    TaskGroup(GroupManager *parent);
    ~TaskGroup();

    ItemList members() const;

    QIcon icon() const;
    void setIcon(const QIcon &icon);

    QColor color() const;
    void setColor(const QColor &color); 

    QString name() const;
    void setName(const QString &newName);

    virtual bool isGroupItem() const { return true; }
    bool isRootGroup() const;

    /** only true if item is in this group */
    bool hasDirectMember(AbstractItemPtr item) const;
    /** only true if item is in this or any sub group */
    bool hasMember(AbstractItemPtr item) const;
    /** Returns Direct Member group if the passed item is in a subgroup */
    AbstractItemPtr directMember(AbstractItemPtr) const;

    int desktop() const;
    bool isShaded() const;
    bool isMaximized() const;
    bool isMinimized() const;
    bool isFullScreen() const;
    bool isKeptBelowOthers() const;
    bool isAlwaysOnTop() const;
    bool isActionSupported(NET::Action) const;
    /** returns true if at least one member is active */
    bool isActive() const;
    /** returns true if at least one member is demands attention */
    bool demandsAttention() const;
    bool isOnAllDesktops() const;
    bool isOnCurrentDesktop() const;

    /**
     * Sorting strategies may use this to move items around
     * @param oldIndex the index the item to be moved is currently at
     * @param newIndex the index the item will be moved to
     */
    bool moveItem(int oldIndex, int newIndex);

public Q_SLOTS:
    /** the following are functions which perform the corresponding actions on all member tasks */
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

    /** close all members of this group */
    void close();

    /** add item to group */
    void add(AbstractItemPtr);

    /** remove item from group */
    void remove(AbstractItemPtr);

    /** Removes all tasks and groups from this group */
    void clear();

Q_SIGNALS:
    /** inform visualization about wat is added and removed */
    void itemAdded(const AbstractItemPtr item);
    void itemRemoved(const AbstractItemPtr item);
    void groupEditRequest();
    void itemChanged(AbstractItemPtr item);
    /** The group changed the desktop, is emitted in the toDesktop function */
    void movedToDesktop(int newDesk);

private:
    class Private;
    Private * const d;
};


} // TaskManager namespace

#endif
