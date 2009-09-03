/*
 *   Copyright © 2009 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
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

#ifndef SERVICEJOB_P_H
#define SERVICEJOB_P_H

#include "../remote/credentials.h"
#include "../servicejob.h"

#include <QString>
#include <QVariant>
#include <QMap>

namespace Plasma
{

class ServiceJobPrivate
{
public:
    ServiceJobPrivate(ServiceJob *owner,
                      const QString &dest,
                      const QString &op,
                      const QMap<QString, QVariant> &params);

    void slotStart();

    ServiceJob *q;
    QString destination;
    QString operation;
    QMap<QString, QVariant> parameters;
    QVariant result;
    Credentials identity;
};

}
#endif // SERVICEJOB_P_H
