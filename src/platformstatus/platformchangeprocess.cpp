/*
 *   Copyright (C) 2013 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "platformchangeprocess.h"

#include <QStringList>

class PlatformChangeProcess::Private {
public:
    int id;
    QStringList clients;
    QString platform;
};

PlatformChangeProcess::PlatformChangeProcess(int changeId,
        const QString & initiator, const QString & platform)
    : d(new Private())
{
    Q_UNUSED(initiator);

    d->id = changeId;
    d->platform = platform;
}

PlatformChangeProcess::~PlatformChangeProcess()
{

}

void PlatformChangeProcess::clientChangeStarted(const QString & client)
{
    if (!d->clients.contains(client)) {
        d->clients << client;
    }
}

void PlatformChangeProcess::clientChangeFinished(const QString & client)
{
    d->clients.removeAll(client);

    if (d->clients.isEmpty()) {
        emit finished(d->id);
    }
}

void PlatformChangeProcess::start()
{
    // Nothing so far
}

#include "platformchangeprocess.moc"

