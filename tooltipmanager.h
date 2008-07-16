/***************************************************************************
*   Copyright (C) 2007 by Alexis Ménard <darktears31@gmail.com>           *
*   Copyright 2007 by Dan Meltzer <hydrogen@notyetimplemented.com>        *
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

#ifndef PLASMA_TOOLTIP_MANAGER_H
#define PLASMA_TOOLTIP_MANAGER_H

//plasma
#include <plasma/plasma.h>
#include <plasma/plasma_export.h>

//KDE
#include <KWindowSystem>

#ifdef Q_WS_X11
#include <QX11Info>
#endif

//X11
#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

namespace Plasma
{
  class ToolTipManagerPrivate;

  /**
  * @short The class to manage a tooltip on a plasma applet
  *
  *	Manage tooltip on plasma applet, before using it you have to set information
  */
  class PLASMA_EXPORT ToolTipManager  : public QObject
  {
      Q_OBJECT
  public:
      struct ToolTipData
      {
	  ToolTipData() : windowToPreview( 0 ) {}
	  QString mainText; //Important information
	  QString subText; //Elaborates on the Main Text
	  QPixmap image; // Icon to show;
	  WId windowToPreview; // Id of window to show preview
      };

      static ToolTipManager *self();
      /**
        * Default constructor. Usually you want to use the singleton instead.
        */
      explicit ToolTipManager(QObject* parent = 0);
      ~ToolTipManager();
      void showToolTip(QGraphicsWidget *widget);
      bool isVisible(QGraphicsWidget *widget);
      void delayedHideToolTip();
      void hideToolTip(QGraphicsWidget *widget);
      void registerToolTipData(QGraphicsWidget *widget,const ToolTipData &data);
      void unregisterToolTip(QGraphicsWidget *widget);
      void updateToolTipData(QGraphicsWidget *widget,const ToolTipData &data);
      bool hasToolTip(QGraphicsWidget *widget);
      static QPoint popupPosition(const QGraphicsItem * item, const QSize &s);

  private Q_SLOTS:
    void themeUpdated();

  private:
      friend class ToolTipManagerSingleton;
      ToolTipManagerPrivate* const d;
      Q_PRIVATE_SLOT(d, void showToolTip())
      Q_PRIVATE_SLOT(d, void resetShownState())
  };
}
#endif
