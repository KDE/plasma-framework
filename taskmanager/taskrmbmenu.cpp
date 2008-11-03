/*****************************************************************

Copyright (c) 2001 Matthias Elter <elter@kde.org>
Copyright (c) 2001 John Firebaugh <jfirebaugh@kde.org>

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

// Own
#include "taskmanager.h"

// System
#include <assert.h>

// KDE
#include <kicon.h>
#include <klocale.h>

#include "config-X11.h"

#if defined(HAVE_XCOMPOSITE) && \
    defined(HAVE_XRENDER) && \
    defined(HAVE_XFIXES)
#include <fixx11h.h>
#endif

#include "taskrmbmenu.h"

static const int ALL_DESKTOPS = 0;

namespace TaskManager
{

TaskRMBMenu::TaskRMBMenu(const TaskList& theTasks, bool show, QWidget *parent)
	: QMenu( parent )
	, tasks( theTasks )
	, showAll( show )
{
    assert(tasks.count() > 0);
    if (tasks.count() == 1)
    {
        fillMenu(tasks.first());
    }
    else
    {
        fillMenu();
    }
}

TaskRMBMenu::TaskRMBMenu(TaskPtr task, bool show, QWidget *parent)
	: QMenu( parent )
	, showAll( show )
{
	fillMenu(task);
}

void TaskRMBMenu::fillMenu(TaskPtr t)
{
    QAction *a;

    addMenu(makeAdvancedMenu(t));
    bool checkActions = KWindowSystem::allowedActionsSupported();

    if (TaskManager::self()->numberOfDesktops() > 1)
    {
        a = addMenu(makeDesktopsMenu(t));

        if (showAll)
        {
            a = addAction(i18n("&To Current Desktop"),
                            t.data(), SLOT(toCurrentDesktop()));
            a->setEnabled(!t->isOnCurrentDesktop());
        }

        if (checkActions)
        {
            a->setEnabled(t->info().actionSupported(NET::ActionChangeDesktop));
        }
    }

    a = addAction(KIcon("transform-move"), i18n("&Move"), t.data(), SLOT(move()));
    a->setEnabled(!checkActions || t->info().actionSupported(NET::ActionMove));

    a = addAction(i18n("Re&size"), t.data(), SLOT(resize()));
    a->setEnabled(!checkActions || t->info().actionSupported(NET::ActionResize));

    a = addAction(i18n("Mi&nimize"), t.data(), SLOT(toggleIconified()));
    a->setCheckable(true);
    a->setChecked(t->isIconified());
    a->setEnabled(!checkActions || t->info().actionSupported(NET::ActionMinimize));

    a = addAction(i18n("Ma&ximize"), t.data(), SLOT(toggleMaximized()));
    a->setCheckable(true);
    a->setChecked(t->isMaximized());
    a->setEnabled(!checkActions || t->info().actionSupported(NET::ActionMax));

    a = addAction(i18n("&Shade"), t.data(), SLOT(toggleShaded()));
    a->setCheckable(true);
    a->setChecked(t->isShaded());
    a->setEnabled(!checkActions || t->info().actionSupported(NET::ActionShade));

    addSeparator();

    a = addAction(KIcon("window-close"), i18n("&Close"), t.data(), SLOT(close()));
    a->setEnabled(!checkActions || t->info().actionSupported(NET::ActionClose));
}

void TaskRMBMenu::fillMenu()
{
    QAction *a;

    TaskList::iterator itEnd = tasks.end();
    for (TaskList::iterator it = tasks.begin(); it != itEnd; ++it)
    {
        TaskPtr t = (*it);

        a = addMenu( new TaskRMBMenu(t, this) );
        a->setIcon( QIcon( t->pixmap() ) );
        a->setText( t->visibleNameWithState() );
        a->setChecked(t->isActive());
        connect( a, SIGNAL(triggered()), t.data(), SLOT( activateRaiseOrIconify() ) );
    }

    addSeparator();

    bool enable = false;

    if (TaskManager::self()->numberOfDesktops() > 1)
    {
        a = addMenu(makeDesktopsMenu());

        a = addAction(i18n("All &to Current Desktop"), this, SLOT(slotAllToCurrentDesktop()));
        TaskList::iterator itEnd = tasks.end();
        for (TaskList::iterator it = tasks.begin(); it != itEnd; ++it)
        {
            if (!(*it)->isOnCurrentDesktop())
            {
                enable = true;
                break;
            }
        }
        a->setEnabled(enable);
    }

    enable = false;

    a = addAction( i18n( "Mi&nimize All" ), this, SLOT( slotMinimizeAll() ) );
    itEnd = tasks.end();
    for (TaskList::iterator it = tasks.begin(); it != itEnd; ++it)
    {
        if( !(*it)->isIconified() ) {
            enable = true;
            break;
        }
    }
    a->setEnabled( enable );

    enable = false;

    a = addAction( i18n( "Ma&ximize All" ), this, SLOT( slotMaximizeAll() ) );
    itEnd = tasks.end();
    for (TaskList::iterator it = tasks.begin(); it != itEnd; ++it)
    {
        if( !(*it)->isMaximized() ) {
            enable = true;
            break;
        }
    }
    a->setEnabled( enable );

    enable = false;

    a = addAction( i18n( "&Restore All" ), this, SLOT( slotRestoreAll() ) );
    itEnd = tasks.end();
    for (TaskList::iterator it = tasks.begin(); it != itEnd; ++it)
    {
        if( (*it)->isIconified() || (*it)->isMaximized() ) {
            enable = true;
            break;
        }
    }
    a->setEnabled( enable );

    addSeparator();

    enable = false;

    addAction( KIcon( "list-remove" ), i18n( "&Close All" ), this, SLOT( slotCloseAll() ) );
}

QMenu* TaskRMBMenu::makeAdvancedMenu(TaskPtr t)
{
    QAction *a;
    QMenu* menu = new QMenu(this);
    menu->setTitle(i18n("Ad&vanced"));

    a = menu->addAction(KIcon("go-up"),
                          i18n("Keep &Above Others"),
                          t.data(), SLOT(toggleAlwaysOnTop()));
    a->setCheckable(true);
    a->setChecked(t->isAlwaysOnTop());

    a = menu->addAction(KIcon("go-down"),
                          i18n("Keep &Below Others"),
                          t.data(), SLOT(toggleKeptBelowOthers()));
    a->setCheckable(true);
    a->setChecked(t->isKeptBelowOthers());

    a = menu->addAction(KIcon("view-fullscreen"),
                          i18n("&Fullscreen"),
                          t.data(), SLOT(toggleFullScreen()));
    a->setCheckable(true);
    a->setChecked(t->isFullScreen());

    if (KWindowSystem::allowedActionsSupported())
    {
        a->setEnabled(t->info().actionSupported(NET::ActionFullScreen));
    }

    return menu;
}

QMenu* TaskRMBMenu::makeDesktopsMenu(TaskPtr t)
{
    QMenu* m = new QMenu(this);
    m->setTitle(i18n("To &Desktop"));

    QAction *a = m->addAction(i18n("&All Desktops"), this, SLOT(slotToDesktop()));
    a->setCheckable(true);
    toDesktopMap.append(QPair<TaskPtr, int>(t, ALL_DESKTOPS));
    a->setData(ALL_DESKTOPS);
    a->setChecked(t->isOnAllDesktops());

    m->addSeparator();

    for (int i = 1; i <= TaskManager::self()->numberOfDesktops(); i++) {
        QString name = QString("&%1 %2").arg(i).arg(TaskManager::self()->desktopName(i).replace('&', "&&"));
        a = m->addAction( name, this, SLOT( slotToDesktop() ) );
        a->setCheckable(true);
        toDesktopMap.append( QPair<TaskPtr, int>( t, i ) );
        a->setData( i );
        a->setChecked( !t->isOnAllDesktops() && t->desktop() == i );
    }

    return m;
}

QMenu* TaskRMBMenu::makeDesktopsMenu()
{
    QMenu* m = new QMenu(this);
    m->setTitle(i18n("All to &Desktop"));
    QAction *a;

    a = m->addAction(i18n("&All Desktops"), this, SLOT(slotAllToDesktop()));
    a->setData(ALL_DESKTOPS);

    m->addSeparator();

    for (int i = 1; i <= TaskManager::self()->numberOfDesktops(); i++) {
        QString name = QString("&%1 %2").arg(i).arg(TaskManager::self()->desktopName(i).replace('&', "&&"));
        a = m->addAction(name, this, SLOT(slotAllToDesktop()));
        a->setData(i);
    }

    return m;
}

void TaskRMBMenu::slotMinimizeAll()
{
    TaskList::iterator itEnd = tasks.end();
    for (TaskList::iterator it = tasks.begin(); it != itEnd; ++it)
    {
        (*it)->setIconified(true);
    }
}

void TaskRMBMenu::slotMaximizeAll()
{
    TaskList::iterator itEnd = tasks.end();
    for (TaskList::iterator it = tasks.begin(); it != itEnd; ++it)
    {
        (*it)->setMaximized(true);
    }
}

void TaskRMBMenu::slotRestoreAll()
{
    TaskList::iterator itEnd = tasks.end();
    for (TaskList::iterator it = tasks.begin(); it != itEnd; ++it)
    {
        (*it)->restore();
    }
}

void TaskRMBMenu::slotShadeAll()
{
    TaskList::iterator itEnd = tasks.end();
    for (TaskList::iterator it = tasks.begin(); it != itEnd; ++it)
    {
        (*it)->setShaded( !(*it)->isShaded() );
    }
}

void TaskRMBMenu::slotCloseAll()
{
    TaskList::iterator itEnd = tasks.end();
    for (TaskList::iterator it = tasks.begin(); it != itEnd; ++it)
    {
        (*it)->close();
    }
}

void TaskRMBMenu::slotAllToDesktop()
{
    QAction *action = qobject_cast<QAction *>( sender() );
    if( action ) {
      int desktop = action->data().toInt();
      TaskList::iterator itEnd = tasks.end();
      for (TaskList::iterator it = tasks.begin(); it != itEnd; ++it)
      {
          (*it)->toDesktop( desktop );
      }
    }
}

void TaskRMBMenu::slotAllToCurrentDesktop()
{
    TaskList::iterator itEnd = tasks.end();
    for (TaskList::iterator it = tasks.begin(); it != itEnd; ++it)
    {
        (*it)->toCurrentDesktop();
    }
}

void TaskRMBMenu::slotToDesktop()
{
    QAction *action = qobject_cast<QAction *>( sender() );
    if( action ) {
        QPair<TaskPtr, int> pair = toDesktopMap[action->data().toInt()];
        pair.first->toDesktop( pair.second );
    }
}

} // TaskManager namespace


#include "taskrmbmenu.moc"
