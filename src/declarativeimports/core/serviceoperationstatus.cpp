/*
 *   Copyright 2013 Marco Martin <mart@kde.org>
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

#include "serviceoperationstatus.h"

ServiceOperationStatus::ServiceOperationStatus(QObject *parent)
    : QObject(parent),
      m_enabled(false)
{
}

ServiceOperationStatus::~ServiceOperationStatus()
{
}

void ServiceOperationStatus::setService(Plasma::Service *service)
{
    if (m_service.data() == service) {
        return;
    }

    if (m_service) {
        disconnect(m_service.data(), nullptr, this, nullptr);
    }
    if (service) {
        connect(service, &Plasma::Service::operationEnabledChanged,
                this, &ServiceOperationStatus::updateStatus);
    }

    m_service = service;
    updateStatus();
    emit serviceChanged();
}

Plasma::Service *ServiceOperationStatus::service() const
{
    return m_service.data();
}

void ServiceOperationStatus::setOperation(const QString &operation)
{
    if (m_operation == operation) {
        return;
    }

    m_operation = operation;
    updateStatus();
    emit operationChanged();
}

QString ServiceOperationStatus::operation() const
{
    return m_operation;
}

void ServiceOperationStatus::setEnabled(bool enabled)
{
    if (m_enabled == enabled) {
        return;
    }

    m_enabled = enabled;
    updateStatus();
    emit enabledChanged();
}

bool ServiceOperationStatus::isEnabled() const
{
    return m_enabled;
}

void ServiceOperationStatus::updateStatus()
{
    if (!m_service) {
        return;
    }

    bool enabled = m_service.data()->isOperationEnabled(m_operation);
    if (enabled != m_enabled) {
        m_enabled = enabled;
        emit enabledChanged();
    }
}

