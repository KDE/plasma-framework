/*****************************************************************

Copyright (c) 2000 Matthias Elter <elter@kde.org>

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
#include "taskmanager_p.h"

// KDE
#include <KConfig>
#include <KConfigGroup>
#include <KGlobal>
#include <KLocale>

#ifdef Q_WS_X11
#include <QX11Info>
#endif
#include <QApplication>
#include <QDesktopWidget>

namespace TaskManager
{

class TaskManagerSingleton
{
public:
   TaskManager self;
};

K_GLOBAL_STATIC( TaskManagerSingleton, privateTaskManagerSelf )

TaskManager* TaskManager::self()
{
    return &privateTaskManagerSelf->self;
}

class TaskManager::Private
{
public:
    Private()
        : active(0),
          startupInfo(0),
          trackGeometry(false)
    {
    }

    TaskPtr active;
    KStartupInfo* startupInfo;
    TaskDict tasksByWId;
    StartupList startups;
    WindowList skiptaskbarWindows;
    bool trackGeometry;
};

TaskManager::TaskManager()
    : QObject(),
      d(new Private)
{
    KGlobal::locale()->insertCatalog("libtaskmanager");
    connect(KWindowSystem::self(), SIGNAL(windowAdded(WId)),
            this,       SLOT(windowAdded(WId)));
    connect(KWindowSystem::self(), SIGNAL(windowRemoved(WId)),
            this,       SLOT(windowRemoved(WId)));
    connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)),
            this,       SLOT(activeWindowChanged(WId)));
    connect(KWindowSystem::self(), SIGNAL(currentDesktopChanged(int)),
            this,       SLOT(currentDesktopChanged(int)));
    connect(KWindowSystem::self(), SIGNAL(windowChanged(WId,unsigned int)),
            this,       SLOT(windowChanged(WId,unsigned int)));

    // register existing windows
    const QList<WId> windows = KWindowSystem::windows();
    QList<WId>::ConstIterator end(windows.end());
    for (QList<WId>::ConstIterator it = windows.begin(); it != end; ++it)
    {
        windowAdded(*it);
    }

    // set active window
    WId win = KWindowSystem::activeWindow();
    activeWindowChanged(win);
    configure_startup();
}

TaskManager::~TaskManager()
{
    KGlobal::locale()->removeCatalog("libtaskmanager");
    delete d;
}

void TaskManager::configure_startup()
{
    KConfig _c( "klaunchrc" );
    KConfigGroup c(&_c, "FeedbackStyle");
    if (!c.readEntry("TaskbarButton", true))
        return;
    d->startupInfo = new KStartupInfo( KStartupInfo::CleanOnCantDetect, this );
    connect( d->startupInfo,
        SIGNAL( gotNewStartup( const KStartupInfoId&, const KStartupInfoData& )),
        SLOT( gotNewStartup( const KStartupInfoId&, const KStartupInfoData& )));
    connect( d->startupInfo,
        SIGNAL( gotStartupChange( const KStartupInfoId&, const KStartupInfoData& )),
        SLOT( gotStartupChange( const KStartupInfoId&, const KStartupInfoData& )));
    connect( d->startupInfo,
        SIGNAL( gotRemoveStartup( const KStartupInfoId&, const KStartupInfoData& )),
        SLOT( killStartup( const KStartupInfoId& )));
    c=KConfigGroup(&_c, "TaskbarButtonSettings");
    d->startupInfo->setTimeout( c.readEntry( "Timeout", 30 ));
}

TaskPtr TaskManager::findTask(WId w)
{
    // TODO: might be able to be made more efficient if
    // we check to see if w is a transient first?
    // profiling would say whether this is worth the effort

    TaskDict::iterator it = d->tasksByWId.begin();
    TaskDict::iterator itEnd = d->tasksByWId.end();

    for (; it != itEnd; ++it)
    {
        if (it.key() == w || it.value()->hasTransient(w))
        {
            return it.value();
        }
    }

    return TaskPtr();
}

TaskPtr TaskManager::findTask(int desktop, const QPoint& p)
{
    QList<WId> list = KWindowSystem::stackingOrder();

    TaskPtr task;
    int currentIndex = -1;
    TaskDict::iterator itEnd = d->tasksByWId.end();
    for (TaskDict::iterator it = d->tasksByWId.begin(); it != itEnd; ++it)
    {
        TaskPtr t = it.value();
        if (!t->isOnAllDesktops() && t->desktop() != desktop)
        {
            continue;
        }

        if (t->isIconified() || t->isShaded())
        {
            continue;
        }

        if (t->geometry().contains(p))
        {
            int index = list.indexOf(t->window());
            if (index > currentIndex)
            {
                currentIndex = index;
                task = t;
            }
        }
    }

    return task;
}

void TaskManager::windowAdded(WId w )
{
    NETWinInfo info(QX11Info::display(), w, QX11Info::appRootWindow(),
                    NET::WMWindowType | NET::WMPid | NET::WMState);

    // ignore NET::Tool and other special window types
    NET::WindowType wType =
        info.windowType( NET::NormalMask | NET::DesktopMask | NET::DockMask |
                         NET::ToolbarMask | NET::MenuMask | NET::DialogMask |
                         NET::OverrideMask | NET::TopMenuMask |
                         NET::UtilityMask | NET::SplashMask );

    if (wType != NET::Normal &&
        wType != NET::Override &&
        wType != NET::Unknown &&
        wType != NET::Dialog &&
        wType != NET::Utility)
    {
        return;
    }

    // ignore windows that want to be ignored by the taskbar
    if ((info.state() & NET::SkipTaskbar) != 0)
    {
        d->skiptaskbarWindows.push_front( w ); // remember them though
        return;
    }

    Window transient_for_tmp;
    if (XGetTransientForHint( QX11Info::display(), (Window) w, &transient_for_tmp ))
    {
        WId transient_for = (WId) transient_for_tmp;

        // check if it's transient for a skiptaskbar window
        if( d->skiptaskbarWindows.contains( transient_for ))
            return;

        // lets see if this is a transient for an existing task
        if( transient_for != QX11Info::appRootWindow()
            && transient_for != 0
            && wType != NET::Utility )
        {
            TaskPtr t = findTask(transient_for);
            if (t)
            {
                if (t->window() != w)
                {
                    t->addTransient(w, info);
                    // kDebug() << "TM: Transient " << w << " added for Task: " << t->window();
                }
                return;
            }
        }
    }

    TaskPtr t( new Task( w, 0 ) );
    d->tasksByWId[w] = t;

    if (d->startupInfo) {
        KStartupInfoId startupInfoId;
        // checkStartup modifies startupInfoId
        d->startupInfo->checkStartup(w, startupInfoId);
        foreach (StartupPtr startup, d->startups) {
            if (startup->id() == startupInfoId) {
                startup->addWindowMatch(w);
            }
        }
    }

    // kDebug() << "TM: Task added for WId: " << w;

    emit taskAdded(t);
}

void TaskManager::windowRemoved(WId w)
{
    d->skiptaskbarWindows.removeAll(w);

    // find task
    TaskPtr t = findTask(w);
    if (!t)
    {
        return;
    }

    if (t->window() == w)
    {
        d->tasksByWId.remove(w);
        emit taskRemoved(t);

        if (t == d->active)
        {
            d->active = 0;
        }

        //kDebug() << "TM: Task for WId " << w << " removed.";
    }
    else
    {
        t->removeTransient(w);
        //kDebug() << "TM: Transient " << w << " for Task " << t->window() << " removed.";
    }
}

void TaskManager::windowChanged(WId w, unsigned int dirty)
{
    if (dirty & NET::WMState) {
        NETWinInfo info (QX11Info::display(), w, QX11Info::appRootWindow(),
                         NET::WMState | NET::XAWMState);

        if (info.state() & NET::SkipTaskbar) {
            windowRemoved(w);
            d->skiptaskbarWindows.push_front(w);
            return;
        } else {
            d->skiptaskbarWindows.removeAll(w);
            if (info.mappingState() != NET::Withdrawn && !findTask(w)) {
                // skipTaskBar state was removed and the window is still
                // mapped, so add this window
                windowAdded( w );
            }
        }
    }

    // check if any state we are interested in is marked dirty
    if (!(dirty & (NET::WMVisibleName |NET::WMName |
                   NET::WMState | NET::WMIcon |
                   NET::XAWMState | NET::WMDesktop) ||
          (d->trackGeometry && dirty & NET::WMGeometry))) {
        return;
    }

    // find task
    TaskPtr t = findTask(w);
    if (!t) {
        return;
    }

    //kDebug() << "TaskManager::windowChanged " << w << " " << dirty;

    if (dirty & NET::WMState) {
        t->updateDemandsAttentionState(w);
    }

    // refresh icon pixmap if necessary
    if (dirty & NET::WMIcon) {
        //TODO should we forward on icon changes, too?
        t->refreshIcon();
        dirty ^= NET::WMIcon;
    }

    TaskChanges changes = TaskUnchanged;

    //kDebug() << "got changes, but will we refresh?" << dirty;
    if (dirty) {
        // only refresh this stuff if we have other changes besides icons
        changes = t->refresh(dirty);
    }

    if (changes != TaskUnchanged) {
        emit windowChanged(t, changes);
    }
}

void TaskManager::updateWindowPixmap(WId w)
{
    if (!KWindowSystem::compositingActive())
    {
        return;
    }

    TaskPtr task = findTask(w);
    if (task)
    {
        task->updateWindowPixmap();
    }
}

void TaskManager::activeWindowChanged(WId w )
{
    //kDebug() << "TaskManager::activeWindowChanged";

    TaskPtr t = findTask( w );
    if (!t) {
        if (d->active) {
            d->active->setActive(false);
            d->active = 0;
        }
    }
    else {
        if (d->active)
            d->active->setActive(false);

        d->active = t;
        d->active->setActive(true);
    }
}

void TaskManager::currentDesktopChanged(int desktop)
{
    emit desktopChanged(desktop);
}

void TaskManager::gotNewStartup( const KStartupInfoId& id, const KStartupInfoData& data )
{
    StartupPtr s( new Startup( id, data, 0 ) );
    d->startups.append(s);

    emit startupAdded(s);
}

void TaskManager::gotStartupChange( const KStartupInfoId& id, const KStartupInfoData& data )
{
    StartupList::iterator itEnd = d->startups.end();
    for (StartupList::iterator sIt = d->startups.begin(); sIt != itEnd; ++sIt)
    {
        if ((*sIt)->id() == id)
        {
            (*sIt)->update(data);
            return;
        }
    }
}

void TaskManager::killStartup( const KStartupInfoId& id )
{
    StartupList::iterator sIt = d->startups.begin();
    StartupList::iterator itEnd = d->startups.end();
    StartupPtr s;
    for (; sIt != itEnd; ++sIt)
    {
        if ((*sIt)->id() == id)
        {
            s = *sIt;
            break;
        }
    }

    if (!s)
    {
        return;
    }

    d->startups.erase(sIt);
    emit startupRemoved(s);
}

void TaskManager::killStartup(StartupPtr s)
{
    if (!s)
    {
        return;
    }

    StartupList::iterator sIt = d->startups.begin();
    StartupList::iterator itEnd = d->startups.end();
    for (; sIt != itEnd; ++sIt)
    {
        if ((*sIt) == s)
        {
            d->startups.erase(sIt);
            break;
        }
    }

    emit startupRemoved(s);
}

QString TaskManager::desktopName(int desk) const
{
    return KWindowSystem::desktopName(desk);
}

TaskDict TaskManager::tasks() const
{
    return d->tasksByWId;
}

StartupList TaskManager::startups() const
{
    return d->startups;
}

int TaskManager::numberOfDesktops() const
{
    return KWindowSystem::numberOfDesktops();
}

bool TaskManager::isOnTop(const Task* task)
{
    if (!task)
    {
        return false;
    }

    QList<WId> list = KWindowSystem::stackingOrder();
    QList<WId>::const_iterator begin(list.constBegin());
    QList<WId>::const_iterator it = list.begin() + (list.size() - 1);
    do
    {
        TaskDict::iterator taskItEnd = d->tasksByWId.end();
        for (TaskDict::iterator taskIt = d->tasksByWId.begin();
             taskIt != taskItEnd; ++taskIt)
        {
            TaskPtr t = taskIt.value();
            if ((*it) == t->window())
            {
                if (t == task)
                {
                    return true;
                }

                if (!t->isIconified() &&
                    (t->isAlwaysOnTop() == task->isAlwaysOnTop()))
                {
                    return false;
                }

                break;
            }
        }
    } while (it-- != begin);

    return false;
}

void TaskManager::trackGeometry(bool state)
{
    d->trackGeometry = state;
}

bool TaskManager::isOnScreen(int screen, const WId wid)
{
    if (screen == -1)
    {
        return true;
    }

    KWindowInfo wi = KWindowSystem::windowInfo(wid, NET::WMFrameExtents);

    // for window decos that fudge a bit and claim to extend beyond the
    // edge of the screen, we just contract a bit.
    QRect window = wi.frameGeometry();
    QRect desktop = QApplication::desktop()->screenGeometry(screen);
    desktop.adjust(5, 5, -5, -5);
    return window.intersects(desktop);
}

} // TaskManager namespace


#include "taskmanager.moc"
