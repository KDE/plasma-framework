/*
 *   Copyright 2006-2007 Aaron Seigo <aseigo@kde.org>
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

#include "abstractrunner.h"

#include <KDebug>
#include <KServiceTypeTrader>

namespace Plasma
{

class AbstractRunner::Private
{
    public:
        bool hasMatchOptions;
        bool hasConfig;
};

AbstractRunner::AbstractRunner(QObject* parent)
    : QObject(parent),
      d(new Private())
{
}

AbstractRunner::~AbstractRunner()
{
    delete d;
}

bool AbstractRunner::hasMatchOptions()
{
    return d->hasMatchOptions;
}

void AbstractRunner::setHasMatchOptions(bool hasMatchOptions)
{
    d->hasMatchOptions = hasMatchOptions;
}

void AbstractRunner::createMatchOptions(QWidget *parent)
{
    Q_UNUSED(parent)
}

bool AbstractRunner::canBeConfigured()
{
    return d->hasConfig;
}

void AbstractRunner::setCanBeConfigured(bool hasConfig)
{
    d->hasConfig = hasConfig;
}

void AbstractRunner::createConfigurationInterface(QWidget *widget)
{
    Q_UNUSED(widget)
}

void AbstractRunner::exec(Plasma::SearchAction *action)
{
    Q_UNUSED(action)
}

AbstractRunner::List AbstractRunner::loadRunners(QObject* parent)
{
    List firstRunners;
    List runners;
    List lastRunners;

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Runner");
    QString error;
    foreach (KService::Ptr service, offers) {
        AbstractRunner* runner = service->createInstance<AbstractRunner>(parent, QVariantList(), &error);
        if (runner) {
            //kDebug() << "loaded runner : " << service->name();
            QString phase = service->property("X-Plasma-RunnerPhase").toString();
            if (phase == "last") {
                lastRunners.append(runner);
            } else if (phase == "first") {
                firstRunners.append(runner);
            } else {
                runners.append(runner);
            }
        }
        else {
            kDebug() << "failed to load runner : " << service->name() << ". error reported: " << error;
        }
    }

    firstRunners << runners << lastRunners;
    return firstRunners;
}

} // Plasma namespace

#include "abstractrunner.moc"
