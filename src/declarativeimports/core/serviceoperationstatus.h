/***************************************************************************
 *   Copyright 2013 Marco Martin <mart@kde.org>                            *
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
#ifndef SERVICEOPERATIONSTATUS_P
#define SERVICEOPERATIONSTATUS_P

#include <QObject>
#include <QSharedPointer>

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
    ~ServiceOperationStatus();

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
    QWeakPointer<Plasma::Service> m_service;
    QString m_operation;
    bool m_enabled;
};

#endif
