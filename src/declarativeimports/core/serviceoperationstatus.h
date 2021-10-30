/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef SERVICEOPERATIONSTATUS_P
#define SERVICEOPERATIONSTATUS_P

#include <QObject>
#include <QPointer>

#include "plasma/service.h"

namespace Plasma
{
class Service;
}

/**
 * @class ServiceOperationStatus
 * @short Monitors services
 */
class ServiceOperationStatus : public QObject
{
    Q_OBJECT

    /**
     * The service instance we want to monitor
     */
    Q_PROPERTY(Plasma::Service *service READ service WRITE setService NOTIFY serviceChanged)

    /**
     * the service operation we want to monitor for enabled or disabled status
     */
    Q_PROPERTY(QString operation READ operation WRITE setOperation NOTIFY operationChanged)

    /**
     * true if the service operation is enabled
     */
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)

public:
    explicit ServiceOperationStatus(QObject *parent = nullptr);
    ~ServiceOperationStatus() override;

    void setService(Plasma::Service *service);
    Plasma::Service *service() const;

    void setOperation(const QString &operation);
    QString operation() const;

    void setEnabled(bool enabled);
    bool isEnabled() const;

Q_SIGNALS:
    void serviceChanged();
    void operationChanged();
    void enabledChanged();

private Q_SLOTS:
    void updateStatus();

private:
    QPointer<Plasma::Service> m_service;
    QString m_operation;
    bool m_enabled;
};

#endif
