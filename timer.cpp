/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#include "timer.h"
#include "sharedtimer_p.h"

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QTimerEvent>

#include <KDE/KDebug>
#include <KDE/KGlobal>

namespace Plasma
{

class Timer::Private
{
public:
    Private()
        : interval(0),
          active(false),
          singleShot(false)
    {
    }

    int interval;
    bool active;
    bool singleShot;
};

Timer::Timer(QObject *parent)
    : QObject(parent),
      d(new Private)
{
}

Timer::~Timer()
{
    TimerDrive::self()->unregisterTimer(this, d->interval);
    delete d;
}

void Timer::setInterval(int msec)
{
    if (d->active) {
        // unregister with the new time
        TimerDrive::self()->unregisterTimer(this, d->interval);
    }

    d->interval = msec;

    if (d->active) {
        // register with the new time
        TimerDrive::self()->registerTimer(this, d->interval);
    }
}

int Timer::interval() const
{
    return d->interval;
}

void Timer::setSingleShot(bool singleShot)
{
    d->singleShot = singleShot;
}

bool Timer::isSingleShot() const
{
    return d->singleShot;
}

bool Timer::isActive() const
{
    return d->active;
}

void Timer::start(int msec)
{
    if (d->interval != msec) {
        setInterval(msec);
    }

    start();
}

void Timer::start()
{
    if (d->active) {
        return;
    }

    d->active = true;
    TimerDrive::self()->registerTimer(this, d->interval);
}

void Timer::stop()
{
    TimerDrive::self()->unregisterTimer(this, d->interval);
    d->active = false;
}

void Timer::activate() const
{
    emit timeout();

    if (d->singleShot) {
        TimerDrive::self()->unregisterTimer(this, d->interval);
        d->active = false;
    }
}

class TimerBundle
{
public:
    TimerBundle()
        : multiple(-1),
          currentCount(-1)
    {
    }

    void setMultiple(int msec, int nextMsec)
    {
        if (nextMsec > 0) {
            currentCount = multiple = nextMsec / msec;
        } else {
            currentCount = multiple = -1;
        }
        //kDebug() << "setting msec of" << msec << "with" << nextMsec << multiple;
    }

    int multiple;
    int currentCount;
    QList<const Timer*> timers;
};

class TimerDrive::Private
{
public:
    Private()
        : interval(0),
          timerId(-1)
    {
    }

    // msec -> TimerBundle
    QMap<int, TimerBundle> bundles;
    int interval;
    int timerId;
};

class TimerDriveSingleton
{
public:
   TimerDrive self;
};

K_GLOBAL_STATIC(TimerDriveSingleton, privateTimerDriveSelf)

TimerDrive* TimerDrive::self()
{
    return &privateTimerDriveSelf->self;
}

TimerDrive::TimerDrive(QObject *parent)
    : QObject(parent),
      d(new Private)
{
}

TimerDrive::~TimerDrive()
{
    delete d;
}

void TimerDrive::registerTimer(const Timer *t, int msec)
{
    kDebug() << "registering" << t << "for" << msec;
    QMap<int, TimerBundle>::iterator it = d->bundles.find(msec);

    if (it == d->bundles.end()) {
        kDebug( ) << "creating a new bundle";
        it = d->bundles.insert(msec, TimerBundle());

        if (it != d->bundles.begin()) {
            QMap<int, TimerBundle>::iterator prev = it - 1;
            prev.value().setMultiple(prev.key(), msec);
        }

        QMap<int, TimerBundle>::iterator next = it + 1;
        if (next != d->bundles.end()) {
            it.value().setMultiple(msec, next.key());
        }

        if (d->bundles.count() == 1) {
            kDebug() << "kickstarting the timer";
            d->interval = msec;

            if (d->timerId != -1) {
                // this should never happen, but better safe than sorry?
                killTimer(d->timerId);
            }

            d->timerId = startTimer(msec);
        } else if (d->timerId != -1) {
            //TODO: figure out if we're injecting an item between the two current timers
        }
    } else {
        // prevent multiple inclusions
        foreach (const Timer *existingTimer, it.value().timers) {
            if (t == existingTimer) {
                return;
            }
        }
    }

    kDebug() << "appending our timer";
    it.value().timers.append(t);
}

void TimerDrive::unregisterTimer(const Timer *t, int msec)
{
    kDebug() << "unregistering" << t << "for" << msec;
    // hash of intervals to timer ids
    QMap<int, TimerBundle>::iterator it = d->bundles.find(msec);
    if (it != d->bundles.end()) {
        TimerBundle &bundle = it.value();
        bundle.timers.removeAll(t);

        if (bundle.timers.count() == 0) {
            kDebug() << "no more timers, removing this bundle";
            if (it != d->bundles.begin()) {
                // update the previous entries multiple
                kDebug() << "reseting previous interval";
                int nextMsec = 0;
                if (it != d->bundles.end()) {
                    QMap<int, TimerBundle>::iterator next = it + 1;
                    if (next != d->bundles.end()) {
                        nextMsec = next.key();
                    }
                }

                QMap<int, TimerBundle>::iterator prev = it - 1;
                if (prev != d->bundles.end()) {
                    prev.value().setMultiple(prev.key(), nextMsec);
                }
            }

            d->bundles.erase(it);
        }
    }

    if (d->bundles.count() == 0) {
        if (d->timerId != -1) {
            killTimer(d->timerId);
            d->timerId = -1;
        }
    } else if (d->timerId != -1) {
        //TODO: figure out if we were going to call this guy next, and if so,
        //      repair the timings
    }
}

void TimerDrive::timerEvent(QTimerEvent *event)
{
    int id = event->timerId();
    if (d->timerId != -1) {
        killTimer(d->timerId);
    }

    QMap<int, TimerBundle>::iterator it = d->bundles.begin();
    QMap<int, TimerBundle>::iterator begin = d->bundles.begin();
    QMap<int, TimerBundle>::iterator end = d->bundles.end();

    if (it == end) {
        return;
    }

    kDebug() << id << d->timerId << "got interval of" << d->interval;
    int effectiveInterval = d->interval;

    while (it != end) {
        int msec = it.key();
        TimerBundle &bundle = it.value();
        if (msec <= effectiveInterval && bundle.currentCount != 0) {
            kDebug() << "     +++ activating" << msec << bundle.currentCount << effectiveInterval;
            foreach (const Timer *t, bundle.timers) {
                t->activate();
            }

            if (bundle.currentCount >= 0) {
                --bundle.currentCount;
            }

            if (bundle.currentCount >= 0) {
                // we haven't reached our multiple, so don't go further.
                break;
            }
        } else {
            //kDebug() << "    --- skipping" << msec << bundle.currentCount << effectiveInterval;
            bundle.currentCount = bundle.multiple;
        }

        effectiveInterval = msec * bundle.multiple + d->interval;
        ++it;
    }

    d->interval = 0;
    int intervalAdjustment = 0;

    if (it == end) {
        d->interval = begin.key();
    } else {
        int currentCount = it.value().currentCount;
        int prevMsec = it.key() * it.value().multiple;

        if (++it == end) {
            //kDebug() << "    end of the line";
            d->interval = begin.key();
            // take into consideration time spent in the last moments prior to reset.
            if (--it != begin) {
                int prevInterval = it.key();
                while (--it != begin) {
                    int gap = prevInterval - it.key();
                    if (gap + intervalAdjustment > d->interval) {
                        break;
                    }

                    intervalAdjustment += gap;
                    prevInterval = it.key();
                }
            }

            // ensure all our current counts are reset
            while (begin != end) {
                begin.value().currentCount = begin.value().multiple;
                ++begin;
            }
        } else {
            // calculate hop to the next value
            int interval = it.key() - prevMsec;
            if (currentCount != 0) {
                // start at the beginning, because we have to take another lap
                // before continuing on to the next item
                //kDebug() << "    reseting";
                d->interval = begin.key();

                // take into consideration time spent in the last moments prior to reset.
                if (--it != begin) {
                    int prevInterval = it.key();
                    while (--it != begin) {
                        int gap = prevInterval - it.key();
                        if (gap + intervalAdjustment > d->interval) {
                            break;
                        }

                        intervalAdjustment += gap;
                        prevInterval = it.key();
                    }
                }
            } else if (interval > begin.key()) {
                // start at the beginning, because our next interval is longer
                // than some of our earlier timers
                //kDebug() << "    looping back";
                d->interval = begin.key();
                while (begin != it) {
                    if (begin.key() < interval) {
                        begin.value().currentCount = begin.value().multiple;
                    } else {
                        begin.value().currentCount = 0;
                    }
                    ++begin;
                }
            } else if (it.value().currentCount == 0) {
                // in this branch, we've looped back once due to the next step
                // interval being larger than our beginning interval
                //kDebug() << "    loop back detected";
                while (begin != it) {
                    begin.value().currentCount = 0;
                    ++begin;
                }

                while (it.value().currentCount == 0) {
                    interval = it.key() * it.value().multiple;
                    ++it;
                }

                d->interval = it.key() - interval;
            } else {
                // short hop to the next entry
                //kDebug() << "    short hop to the next stop";
                d->interval = interval;
            }
        }
    }

    if (d->interval) {
        d->timerId = startTimer(d->interval - intervalAdjustment);
        kDebug() << "    starting interval of" << d->interval
                 << " (really" << d->interval - intervalAdjustment << ")" << d->timerId;
    } else {
        d->timerId = -1;
    }
}

} // namespace Plasma

#include "timer.moc"
#include "sharedtimer_p.moc"

