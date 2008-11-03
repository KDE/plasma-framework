/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef SOLID_MANAGERBASE_P_H
#define SOLID_MANAGERBASE_P_H

#include <QObject>
#include <QString>

#include "solid/control/solid_control_export.h"

namespace Solid
{
namespace Control
{
    class ManagerBasePrivate
    {
    public:
        ManagerBasePrivate();
        virtual ~ManagerBasePrivate();
        void loadBackend(const QString &description, const char *serviceName, const char *backendClassName);

        SOLIDCONTROL_EXPORT static void _k_forcePreloadedBackend(const char *backendClassName, QObject *backend);

        QObject *managerBackend() const;
        QString errorText() const;

    private:
        QObject *m_backend;
        QString m_errorText;
    };
}
}

#endif
