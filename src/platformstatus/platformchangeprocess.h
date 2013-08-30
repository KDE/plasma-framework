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

#ifndef PLATFORMCHANGEPROCESS_H
#define PLATFORMCHANGEPROCESS_H

#include <QObject>

/**
 * PlatformChangeProcess
 * Class handles a platform change - all the
 * asynchronous sub-tasks.
 */
class PlatformChangeProcess: public QObject {
    Q_OBJECT

public:
    PlatformChangeProcess(int changeId, const QString & initiator, const QString & platform);
    virtual ~PlatformChangeProcess();

public Q_SLOTS:
    /**
     * Called when a client reports that it started
     * adapting to the platform change
     */
    void clientChangeStarted(const QString & client);

    /**
     * Called when a client's adaptation is finished
     */
    void clientChangeFinished(const QString & client);

    /**
     * Starts the process
     */
    void start();

Q_SIGNALS:
    /**
     * All the clients have finished
     */
    void finished(int changeId);

private:
    class Private;
    const QScopedPointer<Private> d;
};


#endif /* PLATFORMCHANGEPROCESS_H */

