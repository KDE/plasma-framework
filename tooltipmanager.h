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
  * @short The class to manage tooltips on QGraphicsWidget in Plasma
  *
  *	Manage tooltips on QGraphicsWidget. First you have to register your widget by calling registerWidget on the ToolTipManager singleton. After this you have to set the content of the tooltip by calling setWidgetToolTipContent using the struct ToolTipContent. The tooltip manager unregister automatically the widget when it is destroyed but if you want to do it manually call unregisterWidget. If you use plasma's applets show/hide of tooltip is automatically manage but if you use custom QGraphicsWidget, please call showToolTip on you hoverEnterEvent or hideToolTip on you hoverLeaveEvent.
  */
  class PLASMA_EXPORT ToolTipManager  : public QObject
  {
      Q_OBJECT
  public:

     /**
      * Content of a tooltip
      */
      struct ToolTipContent
      {
	  ToolTipContent() : windowToPreview( 0 ) {}
	  QString mainText; /**Important information, e.g. the title*/
	  QString subText; /** Elaborates on the Main Text */
	  QPixmap image; /** Icon to show */
	  WId windowToPreview; /** Id of a window if you want to show a preview */
      };

      /**
        *
        * @return The singleton instance of the manager.
        **/
      static ToolTipManager *self();
      
      /**
        * Default constructor. Usually you want to use the singleton instead.
        */
      explicit ToolTipManager(QObject* parent = 0);
      
      /**
        * Default destructor.
        */
      ~ToolTipManager();
      
      /**
      * Function to show a tooltip of a widget registered in the tooltip manager
      * @param widget the widget on which the tooltip will be displayed
      */
      void showToolTip(QGraphicsWidget *widget);

      /**
      * Function to know if a tooltip of a widget is displayed
      * @param widget the desired widget
      * @return true if te tooltip of the widget is currently displayed, otherwhise no
      */
      bool isWidgetToolTipDisplayed(QGraphicsWidget *widget);

      /**
      * Function to launch a delayed hide of the current displayed tooltip
      * @param widget the desired widget
      */
      void delayedHideToolTip();

      /**
      * Function to hide the tooltip of the desired widget
      * @param widget the desired widget
      */
      void hideToolTip(QGraphicsWidget *widget);

      /**
      * Function to register a widget to the tooltip manager, after this a setWidgetToolTipContent
      * must be called
      * @param widget the desired widget
      */
      void registerWidget(QGraphicsWidget *widget);

      /**
      * Function to unregister a widget in the tooltip manager, i.e. means that memory will be free
      * @param widget the desired widget to delete
      */
      void unregisterWidget(QGraphicsWidget *widget);

      /**
      * Function to set the content of a tooltip on a desired widget
      * @param widget the desired widget
      * @param data the content of the tooltip
      */
      void setWidgetToolTipContent(QGraphicsWidget *widget,const ToolTipContent &data);
      
      /**
      * Function to know if widget has a tooltip registered in the tooltip manager
      * @param widget the widget
      * @return true if this widget has a tooltip
      */
      bool widgetHasToolTip(QGraphicsWidget *widget) const;
      
      /**
      * Reccomended position for a popup window like a menu or a tooltip
      * given its size
      * @param s size of the popup
      * @returns reccomended position
      */
      static QPoint popupPosition(const QGraphicsItem * item, const QSize &s);

  private:
      friend class ToolTipManagerSingleton;
      ToolTipManagerPrivate* const d;
      Q_PRIVATE_SLOT(d, void showToolTip())
      Q_PRIVATE_SLOT(d, void resetShownState())
      Q_PRIVATE_SLOT(d, void onWidgetDestroyed(QObject*))
      Q_PRIVATE_SLOT(d, void themeUpdated())
  };
}
#endif
