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

#include <QMutex>
#include <QMutexLocker>

#include <KDebug>
#include <KServiceTypeTrader>

#include "searchcontext.h"

namespace Plasma
{

class AbstractRunner::Private
{
    public:
        bool hasMatchOptions;
        bool hasConfig;
        Priority priority;
        Speed speed;
        int tier;

    Private()
      : priority(NormalPriority),
        speed(NormalSpeed),
        tier(0)
    {}

    static QMutex serviceTypeTraderLock;
};

QMutex AbstractRunner::Private::serviceTypeTraderLock;

AbstractRunner::AbstractRunner(QObject* parent)
    : QObject(parent),
      d(new Private())
{
}

AbstractRunner::~AbstractRunner()
{
    delete d;
}

KConfigGroup AbstractRunner::config() const
{
    QString group = objectName();
    if (group.isEmpty()) {
        group = "UnnamedRunner";
    }

    KConfigGroup runners(KGlobal::config(), "Runners");
    return KConfigGroup(&runners, group);
}

void AbstractRunner::performMatch( Plasma::SearchContext &globalContext )
{
    Plasma::SearchContext localContext( 0, globalContext );
    //Keep track of global context list sizes so we know which pointers are our responsibility to delete
    int exactEnd = localContext.exactMatches().count();
    int possibleEnd = localContext.possibleMatches().count();
    int infoEnd = localContext.informationalMatches().count();

    match( &localContext );

    QList<SearchMatch *> exact = localContext.exactMatches().mid(exactEnd);
    QList<SearchMatch *> possible = localContext.possibleMatches().mid(possibleEnd);
    QList<SearchMatch *> info = localContext.informationalMatches().mid(infoEnd);

    //If matches were not added, delete items on the heap
    if (!globalContext.addMatches(localContext.searchTerm(), exact, possible, info)) {
        qDeleteAll(exact);
        qDeleteAll(possible);
        qDeleteAll(info);
    }
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

bool AbstractRunner::isConfigurable()
{
    return d->hasConfig;
}

void AbstractRunner::setIsConfigurable(bool hasConfig)
{
    d->hasConfig = hasConfig;
}

void AbstractRunner::createConfigurationInterface(QWidget *widget)
{
    Q_UNUSED(widget)
}

AbstractRunner::Speed AbstractRunner::speed() const
{
    return d->speed;
}

void AbstractRunner::setSpeed(Speed speed)
{
    d->speed = speed;
}

// For 4.1:
// int AbstractRunner::tier() const
// {
//     return d->tier;
// }
// 
// void AbstractRunner::setTier(int tier)
// {
//     d->tier = tier;
// }

AbstractRunner::Priority AbstractRunner::priority() const
{
    return d->priority;
}

void AbstractRunner::setPriority(Priority priority)
{
    d->priority = priority;
}

KService::List AbstractRunner::serviceQuery(const QString &serviceType, const QString &constraint) const
{
    QMutexLocker lock(&Private::serviceTypeTraderLock);
    return KServiceTypeTrader::self()->query(serviceType, constraint);
}

void AbstractRunner::exec(Plasma::SearchMatch *action)
{
    Q_UNUSED(action)
}

AbstractRunner::List AbstractRunner::loadRunners(QObject* parent, const QStringList& whitelist)
{
    List firstRunners;
    List runners;
    List lastRunners;

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Runner");
    QString error;
    foreach (KService::Ptr service, offers) {
        if( whitelist.empty() || whitelist.contains( service->name() ) ) {
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
    }

    firstRunners << runners << lastRunners;
    return firstRunners;
}

} // Plasma namespace

#include "abstractrunner.moc"
