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

#ifndef PLASMA_REMOTESERVICEJOB_P_H
#define PLASMA_REMOTESERVICEJOB_P_H

#include <QtCore/QByteArray>
#include <kurl.h>

#include "../servicejob.h"

class KConfigGroup;

namespace Jolie
{
    class PendingCallWatcher;
}

namespace Plasma
{

class RemoteService;

class RemoteServiceJob : public Plasma::ServiceJob
{
    Q_OBJECT

    public:
        RemoteServiceJob(KUrl location,
                  const QString& destination,
                  const QString& operation,
                  QMap<QString,QVariant>& parameters,
                  QByteArray initialToken,
                  RemoteService *parent);
        ~RemoteServiceJob();

        void start();
        void setDelayedDescription(const KConfigGroup &desc);

    private Q_SLOTS:
        void callCompleted(Jolie::PendingCallWatcher *watcher);
        void timeout();

    private:
        void checkValidity();

        QByteArray m_token;
        KUrl m_location;
        RemoteService *m_service;
        KConfigGroup *m_delayedDesc;
};

} // namespace Plasma

#endif // PLASMA_REMOTESERVICEJOB_P_H
