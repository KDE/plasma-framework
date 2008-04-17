/***************************************************************************
 *   Copyright (C) 2007 by Alexis Ménard <darktears31@gmail.com>           *
 *   Copyright (C) 2007 Sebastian Kuegler <sebas@kde.org>                  *
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>                       *
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

#ifndef PLASMA_DIALOG_H
#define PLASMA_DIALOG_H

#include <QtGui/QWidget>
#include <QtGui/QGraphicsSceneEvent>
#include <QtGui/QGraphicsView>

#include <plasma/plasma_export.h>

namespace Plasma
{

/**
 * @short A dialog that uses the Plasma style
 *
 * Dialog provides a dialog-like widget that can be used to display additional
 * information.
 *
 * Dialog uses the plasma theme, and usually has no window decoration. It's meant
 * as an interim solution to display widgets as extension to plasma applets, for
 * example when you click on an applet like the devicenotifier or the clock, the
 * widget that is then displayed, is a Dialog.
 */
class PLASMA_EXPORT Dialog : public QWidget
{
    Q_OBJECT

    public:
        /**
         * @arg parent the parent widget, for plasmoids, this is usually 0.
         * @arg f the Qt::WindowFlags, default is to not show a windowborder.
         */
        explicit Dialog( QWidget * parent = 0,Qt::WindowFlags f =  Qt::Window );
        virtual ~Dialog();
        /**
         * @arg event the event that is used to position the dialog. Usually, you want
         * to pass this on from the mouseevent.
         * @arg boundingRect the boundingRect() from the applet.
         * @arg scenePos the absolute position on the scene.
         */
        void position(QGraphicsSceneEvent *event, const QRectF boundingRect, QPointF scenePos);
	
	/**
         * @arg view The QGV where is displayed the applet
         * @arg scenePos the absolute position on the scene.
	 * @arg boundingRect the boundingRect() from the applet.
         */
        void position(QGraphicsView * view,const QRectF boundingRect,QPointF scenePos);

    protected:
        /**
         * Reimplemented from QWidget
         */
        void paintEvent(QPaintEvent *e);
        void resizeEvent(QResizeEvent *e);

    private:
        class Private;
        Private * const d;
        /**
         * React to theme changes 
         */
        Q_PRIVATE_SLOT(d,void themeUpdated(Dialog * dialog));
};

} // Plasma namespace

#endif
