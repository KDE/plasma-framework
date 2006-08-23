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

namespace Plasma
{

class Interface
{
    public:
        static Interface* interface() { return m_interface; }

        virtual bool loadDataEngine(const QString& name) = 0;
        virtual bool unloadDataEngine(const QString& name) = 0;

    protected:
        Interface() : m_interface(0) {}
        static Interface* m_interface;
};

} // Plasma namespace

#endif // multiple inclusion guard
