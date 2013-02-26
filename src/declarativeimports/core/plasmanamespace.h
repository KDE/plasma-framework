/***************************************************************************
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>                       *
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
#ifndef PLASMA_NAMESPACE_CORE
#define PLASMA_NAMESPACE_CORE

#include <QObject>

class PlasmaNamespace : public QObject
{
    Q_OBJECT
    Q_ENUMS(Location)

public:
    enum Location {
        Floating = 0, /**< Free floating. Neither geometry or z-ordering
                        is described precisely by this value. */
        Desktop,      /**< On the planar desktop layer, extending across
                        the full screen from edge to edge */
        FullScreen,   /**< Full screen */
        TopEdge,      /**< Along the top of the screen*/
        BottomEdge,   /**< Along the bottom of the screen*/
        LeftEdge,     /**< Along the left side of the screen */
        RightEdge     /**< Along the right side of the screen */
    };

    PlasmaNamespace(QObject *parent = 0);
    ~PlasmaNamespace();
};

#endif
