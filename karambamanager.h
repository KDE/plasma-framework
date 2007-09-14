/*
 *   Copyright 2007 Alexander Wiedenbruch <wirr01@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef KARAMBA_MANAGER_H
#define KARAMBA_MANAGER_H

#include <QtCore/QObject>
#include <QtGui/QGraphicsScene>

#include <KUrl>

#include <plasma/plasma_export.h>

extern "C" {
    typedef QGraphicsItemGroup* (*startKaramba)(const KUrl &theme, QGraphicsView *view);
}

namespace KarambaManager
{
PLASMA_EXPORT QGraphicsItemGroup* loadKaramba(const KUrl &themePath, QGraphicsScene *scene);
}

#endif
