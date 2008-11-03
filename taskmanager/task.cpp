/*****************************************************************

Copyright (c) 2000-2001 Matthias Elter <elter@kde.org>
Copyright (c) 2001 Richard Moore <rich@kde.org>

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
#include "task.h"

// Qt
#include <QMimeData>
#include <QTimer>
#include <QApplication>
#include <QDesktopWidget>

#ifdef Q_WS_X11
#include <QX11Info>
#endif

// KDE
#include <KDebug>
#include <KIconLoader>
#include <KLocale>

#include "taskmanager.h"
#include "taskmanager_p.h"

namespace TaskManager
{

class Task::Private
{
public:
    Private(WId w)
     : active(false),
       win(w),
       frameId(w),
       info(KWindowSystem::windowInfo(w,
            NET::WMState | NET::XAWMState | NET::WMDesktop |
            NET::WMVisibleName | NET::WMGeometry | NET::WMWindowType,
            NET::WM2AllowedActions)),
       lastWidth(0),
       lastHeight(0),
       lastResize(false),
       lastIcon(),
       thumbSize(0.2),
       thumb(),
       grab()
    {
    }

    bool active;
    WId win;
    WId frameId;
    QPixmap pixmap;
    KWindowInfo info;
    WindowList transients;
    WindowList transientsDemandingAttention;

    int lastWidth;
    int lastHeight;
    bool lastResize;
    QPixmap lastIcon;
    QIcon icon;

    double thumbSize;
    QPixmap thumb;
    QPixmap grab;
    QRect iconGeometry;

    Pixmap windowPixmap;
};

Task::Task(WId w, QObject *parent, const char *name)
  : QObject(parent),
    d(new Private(w))
{
    setObjectName( name );

    // try to load icon via net_wm
    d->pixmap = KWindowSystem::icon(d->win, 16, 16, true);

    // try to guess the icon from the classhint
    if (d->pixmap.isNull())
    {
        KIconLoader::global()->loadIcon(className().toLower(),
                                                    KIconLoader::Small,
                                                    KIconLoader::Small,
                                                    KIconLoader::DefaultState,
                                                    QStringList(), 0, true);
    }

    // load the icon for X applications
    if (d->pixmap.isNull())
    {
        d->pixmap = SmallIcon("xorg");
    }

#ifdef THUMBNAILING_POSSIBLE
    d->windowPixmap = 0;
    findWindowFrameId();

    if (KWindowSystem::compositingActive())
    {
        updateWindowPixmap();
    }
#endif // THUMBNAILING_POSSIBLE
}

Task::~Task()
{
#ifdef THUMBNAILING_POSSIBLE
    //be sure we have something to delete
    if (d->windowPixmap && QX11Info::display())
    {
        XFreePixmap(QX11Info::display(), d->windowPixmap);
    }
#endif // THUMBNAILING_POSSIBLE
    delete d;
}

// Task::findWindowFrameId()
// Code was copied from Kompose.
// Copyright (C) 2004 Hans Oischinger
// Permission granted on 2005-04-27.
void Task::findWindowFrameId()
{
#ifdef THUMBNAILING_POSSIBLE
    Window targetWin, parent, root;
    Window *children;
    uint nchildren;

    targetWin = d->win;
    for (;;)
    {
        if (!XQueryTree(QX11Info::display(), targetWin, &root,
                        &parent, &children, &nchildren))
        {
            break;
        }

        if (children)
        {
            XFree(children); // it's a list, that's deallocated!
        }

        if (!parent || parent == root)
        {
            break;
        }
        else
        {
            targetWin = parent;
        }
    }

    d->frameId = targetWin;
#endif // THUMBNAILING_POSSIBLE
}

void Task::refreshIcon()
{
    // try to load icon via net_wm
    d->pixmap = KWindowSystem::icon(d->win, 16, 16, true);

    // try to guess the icon from the classhint
    if(d->pixmap.isNull())
    {
        KIconLoader::global()->loadIcon(className().toLower(),
                                                    KIconLoader::Small,
                                                    KIconLoader::Small,
                                                    KIconLoader::DefaultState,
                                                    QStringList(), 0, true);
    }

    // load the icon for X applications
    if (d->pixmap.isNull())
    {
        d->pixmap = SmallIcon("xorg");
    }

    d->lastIcon = QPixmap();
    d->icon = QIcon();
    emit changed(IconChanged);
}

::TaskManager::TaskChanges Task::refresh(unsigned int dirty)
{
    KWindowInfo info = KWindowSystem::windowInfo(d->win,
        NET::WMState | NET::XAWMState | NET::WMDesktop | NET::WMVisibleName | NET::WMGeometry | NET::WMWindowType,
        NET::WM2AllowedActions);

    TaskChanges changes = TaskUnchanged;

    if (d->info.visibleName() != info.visibleName() ||
        d->info.visibleNameWithState() != info.visibleNameWithState() ||
        d->info.name() != info.name()) {
        changes |= NameChanged;
    }

    d->info = info;

    if (dirty & NET::WMState || dirty & NET::XAWMState) {
        changes |= StateChanged;
    }

    if (dirty & NET::WMDesktop) {
        changes |= DesktopChanged;
    }

    if (dirty & NET::WMGeometry) {
        changes |= GeometryChanged;
    }

    if (dirty & NET::WMWindowType) {
        changes |= WindowTypeChanged;
    }

    if (dirty & NET::WM2AllowedActions) {
        changes |= ActionsChanged;
    }

    if (changes != TaskUnchanged) {
        emit changed(changes);
    }

    return changes;
}

void Task::setActive(bool a)
{
    d->active = a;
    emit changed(StateChanged);
    if (a) {
      emit activated();
    } else {
      emit deactivated();
    }
}

double Task::thumbnailSize() const { return d->thumbSize; }


void Task::setThumbnailSize( double size )
{
    d->thumbSize = size;
}

bool Task::hasThumbnail() const
{
    return !d->thumb.isNull();
}

QPixmap Task::thumbnail() const
{
    return d->thumb;
}

bool Task::isMaximized() const
{
    return d->info.valid() && (d->info.state() & NET::Max);
}

bool Task::isMinimized() const
{
    return d->info.valid() && d->info.isMinimized();
}

bool Task::isIconified() const
{
    return d->info.valid() && d->info.isMinimized();
}

bool Task::isAlwaysOnTop() const
{
    return d->info.valid() && (d->info.state() & NET::StaysOnTop);
}

bool Task::isKeptBelowOthers() const
{
    return d->info.valid() && (d->info.state() & NET::KeepBelow);
}

bool Task::isFullScreen() const
{
    return d->info.valid() && (d->info.state() & NET::FullScreen);
}

bool Task::isShaded() const
{
    return d->info.valid() && (d->info.state() & NET::Shaded);
}

bool Task::isOnCurrentDesktop() const
{
    return d->info.valid() && d->info.isOnCurrentDesktop();
}

bool Task::isOnAllDesktops() const
{
    return d->info.valid() && d->info.onAllDesktops();
}

bool Task::isActive() const
{
    return d->active;
}

bool Task::isOnTop() const
{
    return TaskManager::self()->isOnTop(this);
}

bool Task::isModified() const
{
  static QString modStr = QString::fromUtf8("[") +
                          i18n("modified") +
                          QString::fromUtf8("]");
  int modStrPos = d->info.visibleName().indexOf(modStr);

  return ( modStrPos != -1 );
}

int Task::desktop() const
{
    return d->info.desktop();
}

bool Task::demandsAttention() const
{
    return (d->info.valid() && (d->info.state() & NET::DemandsAttention)) ||
            d->transientsDemandingAttention.count() > 0;
}

bool Task::isOnScreen( int screen ) const
{
    return TaskManager::isOnScreen( screen, d->win );
}

bool Task::showInTaskbar() const
{
    return d->info.state() ^ NET::SkipTaskbar;
}

bool Task::showInPager() const
{
    return d->info.state() ^ NET::SkipPager;
}

QRect Task::geometry() const
{
    return d->info.geometry();
}

void Task::updateDemandsAttentionState( WId w )
{
    if (window() != w)
    {
        // 'w' is a transient for this task
        NETWinInfo i( QX11Info::display(), w, QX11Info::appRootWindow(), NET::WMState );
        if(i.state() & NET::DemandsAttention)
        {
            if (!d->transientsDemandingAttention.contains(w))
            {
                d->transientsDemandingAttention.append(w);
            }
        }
        else
        {
            d->transientsDemandingAttention.removeAll( w );
        }
    }
}

void Task::addTransient( WId w, const NETWinInfo& info )
{
    d->transients.append(w);
    if (info.state() & NET::DemandsAttention)
    {
        d->transientsDemandingAttention.append(w);
        emit changed(TransientsChanged);
    }
}

void Task::removeTransient(WId w)
{
    d->transients.removeAll(w);
    d->transientsDemandingAttention.removeAll(w);
}

bool Task::hasTransient(WId w) const
{
    return d->transients.indexOf(w) != -1;
}

WId Task::window() const
{
    return d->win;
}

KWindowInfo Task::info() const
{
    return d->info;
}

QString Task::visibleName() const
{
    return d->info.visibleName();
}

QString Task::visibleNameWithState() const
{
    return d->info.visibleNameWithState();
}

QString Task::name() const
{
    return d->info.name();
}

QString Task::className() const
{
    XClassHint hint;
    if(XGetClassHint(QX11Info::display(), d->win, &hint)) {
        QString nh( hint.res_name );
        XFree( hint.res_name );
        XFree( hint.res_class );
        return nh;
    }
    return QString();
}

QString Task::classClass() const
{
    XClassHint hint;
    if(XGetClassHint(QX11Info::display(), d->win, &hint)) {
        QString ch( hint.res_class );
        XFree( hint.res_name );
        XFree( hint.res_class );
        return ch;
    }
    return QString();
}

QPixmap Task::icon( int width, int height, bool allowResize )
{
  if ( (width == d->lastWidth)
       && (height == d->lastHeight)
       && (allowResize == d->lastResize )
       && (!d->lastIcon.isNull()) )
    return d->lastIcon;

  QPixmap newIcon = KWindowSystem::icon( d->win, width, height, allowResize );
  if ( !newIcon.isNull() ) {
    d->lastIcon = newIcon;
    d->lastWidth = width;
    d->lastHeight = height;
    d->lastResize = allowResize;
  }

  return newIcon;
}

QIcon Task::icon()
{
    if ( !d->icon.isNull() )
        return d->icon;

    d->icon.addPixmap(KWindowSystem::icon( d->win, KIconLoader::SizeSmall, KIconLoader::SizeSmall, false));
    d->icon.addPixmap(KWindowSystem::icon( d->win, KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium, false));
    d->icon.addPixmap(KWindowSystem::icon( d->win, KIconLoader::SizeMedium, KIconLoader::SizeMedium, false));
    d->icon.addPixmap(KWindowSystem::icon( d->win, KIconLoader::SizeLarge, KIconLoader::SizeLarge, false));

    return d->icon;
}



WindowList Task::transients() const
{
    return d->transients;
}

QPixmap Task::pixmap() const
{
    return d->pixmap;
}

QPixmap Task::bestIcon( int size, bool &isStaticIcon )
{
  QPixmap pixmap;
  isStaticIcon = false;

  switch( size ) {
  case KIconLoader::SizeSmall:
    {
      pixmap = icon( 16, 16, true  );

      // Icon of last resort
      if( pixmap.isNull() ) {
        pixmap = KIconLoader::global()->loadIcon( "xorg",
                                                  KIconLoader::NoGroup,
                                                  KIconLoader::SizeSmall );
        isStaticIcon = true;
      }
    }
    break;
  case KIconLoader::SizeMedium:
    {
      //
      // Try 34x34 first for KDE 2.1 icons with shadows, if we don't
      // get one then try 32x32.
      //
      pixmap = icon( 34, 34, false  );

      if ( (( pixmap.width() != 34 ) || ( pixmap.height() != 34 )) &&
           (( pixmap.width() != 32 ) || ( pixmap.height() != 32 )) )
      {
        pixmap = icon( 32, 32, true  );
      }

      // Icon of last resort
      if( pixmap.isNull() ) {
        pixmap = KIconLoader::global()->loadIcon( "xorg",
                            KIconLoader::NoGroup,
                            KIconLoader::SizeMedium );
        isStaticIcon = true;
      }
    }
    break;
  case KIconLoader::SizeLarge:
    {
      // If there's a 48x48 icon in the hints then use it
      pixmap = icon( size, size, false  );

      // If not, try to get one from the classname
      if ( pixmap.isNull() || pixmap.width() != size || pixmap.height() != size ) {
        pixmap = KIconLoader::global()->loadIcon( className(),
                            KIconLoader::NoGroup,
                            size,
                            KIconLoader::DefaultState,
                            QStringList(), 0L,
                            true );
        isStaticIcon = true;
      }

      // If we still don't have an icon then scale the one in the hints
      if ( pixmap.isNull() || ( pixmap.width() != size ) || ( pixmap.height() != size ) ) {
        pixmap = icon( size, size, true  );
        isStaticIcon = false;
      }

      // Icon of last resort
      if( pixmap.isNull() ) {
        pixmap = KIconLoader::global()->loadIcon( "xorg",
                                                  KIconLoader::NoGroup,
                                                  size );
        isStaticIcon = true;
      }
    }
  }

  return pixmap;
}

bool Task::idMatch( const QString& id1, const QString& id2 )
{
  if ( id1.isEmpty() || id2.isEmpty() )
    return false;

  if ( id1.contains( id2 ) > 0 )
    return true;

  if ( id2.contains( id1 ) > 0 )
    return true;

  return false;
}

void Task::move()
{
    bool on_current = d->info.isOnCurrentDesktop();

    if (!on_current)
    {
        KWindowSystem::setCurrentDesktop(d->info.desktop());
        KWindowSystem::forceActiveWindow(d->win);
    }

    if (d->info.isMinimized())
    {
        KWindowSystem::unminimizeWindow(d->win);
    }

    QRect geom = d->info.geometry();
    QCursor::setPos(geom.center());

    NETRootInfo ri(QX11Info::display(), NET::WMMoveResize);
    ri.moveResizeRequest(d->win, geom.center().x(),
                         geom.center().y(), NET::Move);
}

void Task::resize()
{
    bool on_current = d->info.isOnCurrentDesktop();

    if (!on_current)
    {
        KWindowSystem::setCurrentDesktop(d->info.desktop());
        KWindowSystem::forceActiveWindow(d->win);
    }

    if (d->info.isMinimized())
    {
        KWindowSystem::unminimizeWindow(d->win);
    }

    QRect geom = d->info.geometry();
    QCursor::setPos(geom.bottomRight());

    NETRootInfo ri(QX11Info::display(), NET::WMMoveResize);
    ri.moveResizeRequest(d->win, geom.bottomRight().x(),
                         geom.bottomRight().y(), NET::BottomRight);
}

void Task::setMaximized(bool maximize)
{
    KWindowInfo info = KWindowSystem::windowInfo(d->win, NET::WMState | NET::XAWMState | NET::WMDesktop);
    bool on_current = info.isOnCurrentDesktop();

    if (!on_current)
    {
        KWindowSystem::setCurrentDesktop(info.desktop());
    }

    if (info.isMinimized())
    {
        KWindowSystem::unminimizeWindow(d->win);
    }

    NETWinInfo ni(QX11Info::display(), d->win, QX11Info::appRootWindow(), NET::WMState);

    if (maximize)
    {
        ni.setState(NET::Max, NET::Max);
    }
    else
    {
        ni.setState(0, NET::Max);
    }

    if (!on_current)
    {
        KWindowSystem::forceActiveWindow(d->win);
    }
}

void Task::toggleMaximized()
{
    setMaximized(!isMaximized());
}

void Task::restore()
{
    KWindowInfo info = KWindowSystem::windowInfo(d->win, NET::WMState | NET::XAWMState | NET::WMDesktop);
    bool on_current = info.isOnCurrentDesktop();

    if (!on_current)
    {
        KWindowSystem::setCurrentDesktop(info.desktop());
    }

    if( info.isMinimized())
    {
        KWindowSystem::unminimizeWindow(d->win);
    }

    NETWinInfo ni(QX11Info::display(), d->win, QX11Info::appRootWindow(), NET::WMState);
    ni.setState(0, NET::Max);

    if (!on_current)
    {
        KWindowSystem::forceActiveWindow( d->win );
    }
}

void Task::setIconified(bool iconify)
{
    if (iconify)
    {
        KWindowSystem::minimizeWindow(d->win);
    }
    else
    {
        KWindowInfo info = KWindowSystem::windowInfo(d->win, NET::WMState | NET::XAWMState | NET::WMDesktop);
        bool on_current = info.isOnCurrentDesktop();

        if (!on_current)
        {
            KWindowSystem::setCurrentDesktop(info.desktop());
        }

        KWindowSystem::unminimizeWindow(d->win);

        if (!on_current)
        {
            KWindowSystem::forceActiveWindow(d->win);
        }
    }
}

void Task::toggleIconified()
{
    setIconified(!isIconified());
}

void Task::close()
{
    NETRootInfo ri( QX11Info::display(), NET::CloseWindow );
    ri.closeWindowRequest( d->win );
}

void Task::raise()
{
//    kDebug(1210) << "Task::raise(): " << name();
    KWindowSystem::raiseWindow( d->win );
}

void Task::lower()
{
//    kDebug(1210) << "Task::lower(): " << name();
    KWindowSystem::lowerWindow( d->win );
}

void Task::activate()
{
//    kDebug(1210) << "Task::activate():" << name();
    WId w = d->win;
    if (d->transientsDemandingAttention.count() > 0)
    {
        w = d->transientsDemandingAttention.last();
    }
    KWindowSystem::forceActiveWindow( w );
}

void Task::activateRaiseOrIconify()
{
    if (!isActive() || isIconified())
    {
        activate();
    }
    else if (!isOnTop())
    {
       raise();
    }
    else
    {
       setIconified(true);
    }
}

void Task::toDesktop(int desk)
{
    NETWinInfo ni(QX11Info::display(), d->win, QX11Info::appRootWindow(), NET::WMDesktop);
    if (desk == 0)
    {
        if (d->info.valid() && d->info.onAllDesktops())
        {
            ni.setDesktop(KWindowSystem::currentDesktop());
            KWindowSystem::forceActiveWindow(d->win);
        }
        else
        {
            ni.setDesktop(NETWinInfo::OnAllDesktops);
        }

        return;
    }
    ni.setDesktop(desk);
    if(desk == KWindowSystem::currentDesktop())
        KWindowSystem::forceActiveWindow(d->win);
}

void Task::toCurrentDesktop()
{
    toDesktop(KWindowSystem::currentDesktop());
}

void Task::setAlwaysOnTop(bool stay)
{
    NETWinInfo ni( QX11Info::display(), d->win, QX11Info::appRootWindow(), NET::WMState);
    if(stay)
        ni.setState( NET::StaysOnTop, NET::StaysOnTop );
    else
        ni.setState( 0, NET::StaysOnTop );
}

void Task::toggleAlwaysOnTop()
{
    setAlwaysOnTop( !isAlwaysOnTop() );
}

void Task::setKeptBelowOthers(bool below)
{
    NETWinInfo ni(QX11Info::display(), d->win, QX11Info::appRootWindow(), NET::WMState);

    if (below)
    {
        ni.setState(NET::KeepBelow, NET::KeepBelow);
    }
    else
    {
        ni.setState(0, NET::KeepBelow);
    }
}

void Task::toggleKeptBelowOthers()
{
    setKeptBelowOthers(!isKeptBelowOthers());
}

void Task::setFullScreen(bool fullscreen)
{
    NETWinInfo ni(QX11Info::display(), d->win, QX11Info::appRootWindow(), NET::WMState);

    if (fullscreen)
    {
        ni.setState(NET::FullScreen, NET::FullScreen);
    }
    else
    {
        ni.setState(0, NET::FullScreen);
    }
}

void Task::toggleFullScreen()
{
    setFullScreen(!isFullScreen());
}

void Task::setShaded(bool shade)
{
    NETWinInfo ni( QX11Info::display(), d->win, QX11Info::appRootWindow(), NET::WMState);
    if(shade)
        ni.setState( NET::Shaded, NET::Shaded );
    else
        ni.setState( 0, NET::Shaded );
}

void Task::toggleShaded()
{
    setShaded( !isShaded() );
}

void Task::publishIconGeometry(QRect rect)
{
    if (rect == d->iconGeometry)
    {
        return;
    }

    d->iconGeometry = rect;
    NETWinInfo ni(QX11Info::display(), d->win, QX11Info::appRootWindow(), 0);
    NETRect r;

    if (rect.isValid())
    {
        r.pos.x = rect.x();
        r.pos.y = rect.y();
        r.size.width = rect.width();
        r.size.height = rect.height();
    }
    ni.setIconGeometry(r);
}

#ifdef THUMBNAILING_POSSIBLE
QPixmap Task::thumbnail(int maxDimension)
{
    if (!KWindowSystem::compositingActive() || !d->windowPixmap)
    {
        return QPixmap();
    }

    Display *dpy = QX11Info::display();

    XWindowAttributes winAttr;
    XGetWindowAttributes(dpy, d->frameId, &winAttr);
    XRenderPictFormat *format = XRenderFindVisualFormat(dpy, winAttr.visual);

    XRenderPictureAttributes picAttr;
    ::memset(&picAttr, 0, sizeof(picAttr));
    picAttr.subwindow_mode = IncludeInferiors; // Don't clip child widgets

    Picture picture = XRenderCreatePicture(dpy, d->windowPixmap, format,
                                           CPSubwindowMode, &picAttr);

    // Get shaped windows handled correctly.
    XserverRegion region = XFixesCreateRegionFromWindow(dpy, d->frameId,
                                                        WindowRegionBounding);
    XFixesSetPictureClipRegion(dpy, picture, 0, 0, region);
    XFixesDestroyRegion(dpy, region);

    double factor;
    if (winAttr.width > winAttr.height)
    {
        factor = (double)maxDimension / (double)winAttr.width;
    }
    else
    {
        factor = (double)maxDimension / (double)winAttr.height;
    }
    int thumbnailWidth = (int)(winAttr.width * factor);
    int thumbnailHeight = (int)(winAttr.height * factor);

    QPixmap thumbnail(thumbnailWidth, thumbnailHeight);
    thumbnail.fill(QApplication::palette().color(QPalette::Active,QPalette::Background));

#if 0 // QImage::smoothScale() scaling
    QPixmap full(winAttr.width, winAttr.height);
    full.fill(QApplication::palette().active().background());

    bool hasAlpha = format->type == PictTypeDirect && format->direct.alphaMask;

    XRenderComposite(dpy,
                     hasAlpha ? PictOpOver : PictOpSrc,
                     picture, // src
                     None, // mask
                     full.x11RenderHandle(), // dst
                     0, 0, // src offset
                     0, 0, // mask offset
                     0, 0, // dst offset
                     winAttr.width, winAttr.height);

    KPixmapIO io;
    QImage image = io.toImage(full);
    thumbnail = io.convertToPixmap(image.smoothScale(thumbnailWidth,
                                                     thumbnailHeight));
#else // XRENDER scaling
    // Scaling matrix
    XTransform transformation = {{
        { XDoubleToFixed(1), XDoubleToFixed(0), XDoubleToFixed(     0) },
        { XDoubleToFixed(0), XDoubleToFixed(1), XDoubleToFixed(     0) },
        { XDoubleToFixed(0), XDoubleToFixed(0), XDoubleToFixed(factor) }
    }};

    XRenderSetPictureTransform(dpy, picture, &transformation);
    XRenderSetPictureFilter(dpy, picture, FilterBest, 0, 0);

    XRenderComposite(QX11Info::display(),
                     PictOpOver, // we're filtering, alpha values are probable
                     picture, // src
                     None, // mask
                     thumbnail.x11PictureHandle(), // dst
                     0, 0, // src offset
                     0, 0, // mask offset
                     0, 0, // dst offset
                     thumbnailWidth, thumbnailHeight);
#endif
    XRenderFreePicture(dpy, picture);

    return thumbnail;
}
#else // THUMBNAILING_POSSIBLE
QPixmap Task::thumbnail(int /* maxDimension */)
{
    return QPixmap();
}
#endif // THUMBNAILING_POSSIBLE

void Task::updateWindowPixmap()
{
#ifdef THUMBNAILING_POSSIBLE
    if (!KWindowSystem::compositingActive() || !isOnCurrentDesktop() ||
        isMinimized())
    {
        return;
    }

    Display *dpy = QX11Info::display();

    if (d->windowPixmap)
    {
        XFreePixmap(dpy, d->windowPixmap);
    }

    d->windowPixmap = XCompositeNameWindowPixmap(dpy, d->frameId);
#endif // THUMBNAILING_POSSIBLE
}

int TaskManager::currentDesktop() const
{
    return KWindowSystem::currentDesktop();
}

TaskDrag::TaskDrag(const TaskList& tasks, QWidget* source)
  : QDrag(source),
    d(0)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream.setVersion(QDataStream::Qt_3_1);

    TaskList::const_iterator itEnd = tasks.constEnd();
    for (TaskList::const_iterator it = tasks.constBegin(); it != itEnd; ++it)
    {
        stream << (quint32)(*it)->window();
    }

    QMimeData* mimeData = new QMimeData();
    mimeData->setData("taskbar/task", data);
    setMimeData(mimeData);
}

TaskDrag::~TaskDrag()
{
}

bool TaskDrag::canDecode(const QMimeData* e)
{
    return e->hasFormat("taskbar/task");
}

TaskList TaskDrag::decode( const QMimeData* e )
{
    QByteArray data(e->data("taskbar/task"));
    TaskList tasks;

    if (data.size())
    {
        QDataStream stream(data);
        while (!stream.atEnd())
        {
            quint32 id;
            stream >> id;
            if (TaskPtr task = TaskManager::self()->findTask(id))
            {
                tasks.append(task);
            }
        }
    }

    return tasks;
}

} // TaskManager namespace

#include "task.moc"

