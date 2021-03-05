/*
    SPDX-FileCopyrightText: 2009 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SERVICEJOB_P_H
#define SERVICEJOB_P_H

#include "../servicejob.h"

#include <QHash>
#include <QString>
#include <QVariant>

namespace Plasma
{
class ServiceJobPrivate
{
public:
    ServiceJobPrivate(ServiceJob *owner, const QString &dest, const QString &op, const QVariantMap &params);

    void preventAutoStart();
    void autoStart();

    ServiceJob *q;
    QString destination;
    QString operation;
    QVariantMap parameters;
    QVariant result;
    bool m_allowAutoStart;
};

}
#endif // SERVICEJOB_P_H
