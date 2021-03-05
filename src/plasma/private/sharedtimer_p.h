/*
    SPDX-FileCopyrightText: 2008 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_SHAREDTIMER_P_H
#define PLASMA_SHAREDTIMER_P_H

#include <QObject>

namespace Plasma
{
class Timer;

class TimerDrive : public QObject
{
    Q_OBJECT

public:
    static TimerDrive *self();
    void registerTimer(const Timer *t, int msec);
    void unregisterTimer(const Timer *t, int msec);

protected:
    void timerEvent(QTimerEvent *event);

private:
    friend class TimerDriveSingleton;
    explicit TimerDrive(QObject *parent = nullptr);
    ~TimerDrive();
    class Private;
    Private *const d;
};

} // namespace Plasma

#endif
