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

#include "frontendobject_p.h"

#include <QtCore/QObject>

Solid::Control::FrontendObjectPrivate::FrontendObjectPrivate(QObject *parent)
    : m_parent(parent), m_backendObject(0)
{
}

Solid::Control::FrontendObjectPrivate::~FrontendObjectPrivate()
{
}

QObject *Solid::Control::FrontendObjectPrivate::parent() const
{
    return m_parent;
}

QObject *Solid::Control::FrontendObjectPrivate::backendObject() const
{
    return m_backendObject;
}

void Solid::Control::FrontendObjectPrivate::setBackendObject(QObject *object)
{
    if (m_backendObject)
    {
        QObject::disconnect(m_parent, 0, m_backendObject, 0);
        m_backendObject->disconnect(m_parent);
    }

    m_backendObject = object;

    if (m_backendObject)
    {
        QObject::connect(m_backendObject, SIGNAL(destroyed(QObject *)),
                         m_parent, SLOT(_k_destroyed(QObject *)));
    }
}

void Solid::Control::FrontendObjectPrivate::_k_destroyed(QObject *object)
{
    if (m_backendObject == object) {
        m_backendObject = 0;
    }
}
