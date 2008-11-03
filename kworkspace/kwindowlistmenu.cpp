/*****************************************************************

Copyright (c) 2000 Matthias Elter <elter@kde.org>
                   Matthias Ettrich <ettrich@kde.org>

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

#include "kwindowlistmenu.h"

#include <QtCore/QBool>

#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtGui/QPainter>
#include <QtGui/QX11Info>

#include <QtDBus/QtDBus>
#include <klocale.h>
#include <kstringhandler.h>
#include <kstyle.h>
#include <kwindowsystem.h>
#include <netwm.h>

#undef Bool
#include "kwindowlistmenu.moc"
#include "kwin_interface.h"

static bool compareKWinWindowInfo( KWindowInfo* firstInfo, KWindowInfo* secondInfo )
{
  QString firstTitle, secondTitle;

  if ( firstInfo )
    firstTitle = firstInfo->visibleNameWithState().toLower();

  if ( secondInfo )
    secondTitle = secondInfo->visibleNameWithState().toLower();

  return firstTitle.compare( secondTitle ) >= 0;
}

class KWindowListMenu::Private
{
};

KWindowListMenu::KWindowListMenu( QWidget *parent )
  : KMenu( parent ), d( new Private )
{
}

KWindowListMenu::~KWindowListMenu()
{
  delete d;
}

static bool standaloneDialog( const KWindowInfo* info, const QList<KWindowInfo*>& list )
{
  WId group = info->groupLeader();

  if ( group == 0 )
    return info->transientFor() == QX11Info::appRootWindow();

  foreach ( KWindowInfo* info, list )
    if ( info->groupLeader() == group )
      return false;

  return true;
}

void KWindowListMenu::init()
{
  int numberOfDesktops = KWindowSystem::numberOfDesktops();
  int currentDesktop = KWindowSystem::currentDesktop();
  WId activeWindow = KWindowSystem::activeWindow();

  // Make sure the popup is not too wide, otherwise clicking in the middle of kdesktop
  // wouldn't leave any place for the popup, and release would activate some menu entry.
  int maxwidth = qApp->desktop()->screenGeometry( this ).width() / 2 - 100;

  clear();

  QAction* unclutter = addAction( i18n("Unclutter Windows"),
                              this, SLOT( slotUnclutterWindows() ) );
  QAction* cascade = addAction( i18n("Cascade Windows"),
                            this, SLOT( slotCascadeWindows() ) );

  // if we only have one desktop we won't be showing titles, so put a separator in
  if ( numberOfDesktops == 1 )
    addSeparator();

  QList<KWindowInfo> windows;
  foreach ( WId id, KWindowSystem::windows() )
    windows.append( KWindowSystem::windowInfo( id, NET::WMDesktop ) );

  bool showAllDesktopsGroup = ( numberOfDesktops > 1 );

  int i = 0;
  for ( int j = 1; j <= numberOfDesktops + (showAllDesktopsGroup ? 1 : 0); j++ ) {
    bool onAllDesktops = ( j > numberOfDesktops );
    int items = 0;

    // KDE4 porting - huh? didn't know you could set an item checked before it's created?
    //if (!activeWindow && d == cd)
        //setItemChecked(1000 + d, true);

    QList<KWindowInfo*> list;

    foreach (const KWindowInfo &wi, windows) {
      if ( (wi.desktop() == j) || (onAllDesktops && wi.onAllDesktops())
           || (!showAllDesktopsGroup && wi.onAllDesktops()) ) {
        list.append( new KWindowInfo( wi.win(),
                         NET::WMVisibleName | NET::WMState | NET::XAWMState | NET::WMWindowType,
                         NET::WM2GroupLeader | NET::WM2TransientFor ) );
      }
    }

    qStableSort( list.begin(), list.end(), compareKWinWindowInfo );

    foreach ( KWindowInfo* info, list ) {
      ++i;
      QString itemText = fontMetrics().elidedText(info->visibleNameWithState(), Qt::ElideMiddle, maxwidth);

      NET::WindowType windowType = info->windowType( NET::NormalMask | NET::DesktopMask
          | NET::DockMask | NET::ToolbarMask | NET::MenuMask | NET::DialogMask
          | NET::OverrideMask | NET::TopMenuMask | NET::UtilityMask | NET::SplashMask );

      if ( (windowType == NET::Normal || windowType == NET::Unknown
              || (windowType == NET::Dialog && standaloneDialog( info, list )))
              && !(info->state() & NET::SkipTaskbar) ) {

        QPixmap pm = KWindowSystem::icon( info->win(), 16, 16, true );
        items++;

        // ok, we have items on this desktop, let's show the title
        if ( items == 1 && numberOfDesktops > 1 ) {
          if( !onAllDesktops )
              addTitle( KWindowSystem::desktopName( j ) );
          else
              addTitle( i18n( "On All Desktops" ) );
        }

        // Avoid creating unwanted accelerators.
        itemText.replace( '&', QLatin1String( "&&" ));

        QAction* action = addAction( pm, itemText, this, SLOT( slotForceActiveWindow() ) );
        action->setData( (int)info->win() );

        if ( info->win() == activeWindow )
          action->setChecked( true );
      }
    }

    if ( j == currentDesktop ) {
      unclutter->setEnabled( items > 0 );
      cascade->setEnabled( items > 0 );
    }

    qDeleteAll( list );
  }

  // no windows?
  if ( i == 0 ) {
    if ( numberOfDesktops > 1 )
      addSeparator(); // because we don't have any titles, nor a separator

    addAction( i18n( "No Windows" ) )->setEnabled( false );
  }
}

void KWindowListMenu::slotForceActiveWindow()
{
    QAction* window = qobject_cast<QAction*>(sender());
    if (!window || !window->data().canConvert(QVariant::Int))
        return;

    KWindowSystem::forceActiveWindow(window->data().toInt());
}

void KWindowListMenu::slotSetCurrentDesktop()
{
    QAction* window = qobject_cast<QAction*>(sender());
    if (!window || !window->data().canConvert(QVariant::Int))
        return;

    KWindowSystem::setCurrentDesktop(window->data().toInt());
}

// This popup is much more useful from keyboard if it has the active
// window active by default - however, QPopupMenu tries hard to resist.
// QPopupMenu::popup() resets the active item, so this needs to be
// called after popup().
void KWindowListMenu::selectActiveWindow()
{
    foreach (QAction* action, actions())
        if (action->isChecked()) {
            setActiveAction(action);
            break;
        }
}

void KWindowListMenu::slotUnclutterWindows()
{
    org::kde::KWin kwin("org.kde.kwin", "/KWin", QDBusConnection::sessionBus());
    kwin.unclutterDesktop();
}

void KWindowListMenu::slotCascadeWindows()
{
    org::kde::KWin kwin("org.kde.kwin", "/KWin", QDBusConnection::sessionBus());
    kwin.cascadeDesktop();
}

