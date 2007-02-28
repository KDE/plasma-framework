/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#ifndef PLASMA_INTERFACE_H
#define PLASMA_INTERFACE_H

#include <QString>

#include <kdelibs_export.h>

namespace Plasma
{

class KDE_EXPORT Interface
{
    public:
        // NOTE: Fix this stuff, not sure what the design was supposed to be,
        //       but, this thing can't be a singleton because we can't create
        //       an Interface object due to the pure virtuals. Maybe make them
        //       just virtual? -MB

        static Interface* self();

        virtual bool loadDataEngine(const QString &name);
        virtual void unloadDataEngine(const QString &name);

    protected:
        Interface();
        virtual ~Interface();

        static Interface* m_interface;
};

} // Plasma namespace

#endif // multiple inclusion guard

