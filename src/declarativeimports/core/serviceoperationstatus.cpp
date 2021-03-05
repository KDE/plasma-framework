/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "serviceoperationstatus.h"

ServiceOperationStatus::ServiceOperationStatus(QObject *parent)
    : QObject(parent)
    , m_enabled(false)
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
        connect(service, &Plasma::Service::operationEnabledChanged, this, &ServiceOperationStatus::updateStatus);
    }

    m_service = service;
    updateStatus();
    Q_EMIT serviceChanged();
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
    Q_EMIT operationChanged();
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
    Q_EMIT enabledChanged();
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
        Q_EMIT enabledChanged();
    }
}
