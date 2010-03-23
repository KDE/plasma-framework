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

#include <QAction>
#include <QHash>
#include <QMenu>
#include <QMimeData>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>

#include <kdebug.h>
#include <kicon.h>
#include <kplugininfo.h>
#include <kservicetypetrader.h>
#include <kstandarddirs.h>

#include <plasma/package.h>
#include <plasma/querymatch.h>

#include "private/abstractrunner_p.h"
#include "runnercontext.h"
#include "scripting/runnerscript.h"

namespace Plasma
{

K_GLOBAL_STATIC(QMutex, s_bigLock)

AbstractRunner::AbstractRunner(QObject *parent, const QString &path)
    : QObject(parent),
      d(new AbstractRunnerPrivate(this))
{
    d->init(path);
}

AbstractRunner::AbstractRunner(const KService::Ptr service, QObject *parent)
    : QObject(parent),
      d(new AbstractRunnerPrivate(this))
{
    d->init(service);
}

AbstractRunner::AbstractRunner(QObject *parent, const QVariantList &args)
    : QObject(parent),
      d(new AbstractRunnerPrivate(this))
{
    if (args.count() > 0) {
        KService::Ptr service = KService::serviceByStorageId(args[0].toString());
        if (service) {
            d->init(service);
        }
    }
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

void AbstractRunner::reloadConfiguration()
{
    if (d->script) {
        emit d->script->reloadConfiguration();
    }
}

void AbstractRunner::addSyntax(const RunnerSyntax &syntax)
{
    d->syntaxes.append(syntax);
}

void AbstractRunner::setDefaultSyntax(const RunnerSyntax &syntax)
{
    d->syntaxes.append(syntax);
    d->defaultSyntax = &(d->syntaxes.last());
}

void AbstractRunner::setSyntaxes(const QList<RunnerSyntax> &syntaxes)
{
    d->syntaxes = syntaxes;
}

QList<RunnerSyntax> AbstractRunner::syntaxes() const
{
    return d->syntaxes;
}

RunnerSyntax *AbstractRunner::defaultSyntax() const
{
    return d->defaultSyntax;
}

void AbstractRunner::performMatch(Plasma::RunnerContext &localContext)
{
    static const int reasonableRunTime = 1500;
    static const int fastEnoughTime = 250;

    QTime time;
    time.restart();

    //The local copy is already obtained in the job
    match(localContext);

    // automatically rate limit runners that become slooow
    const int runtime = time.elapsed();
    bool slowed = speed() == SlowSpeed;

    if (!slowed && runtime > reasonableRunTime) {
        // we punish runners that return too slowly, even if they don't bring
        // back matches
        kDebug() << id() << "runner is too slow, putting it on the back burner.";
        d->fastRuns = 0;
        setSpeed(SlowSpeed);
    }

    if (slowed && runtime < fastEnoughTime && localContext.query().size() > 2) {
        ++d->fastRuns;

        if (d->fastRuns > 2) {
            // we reward slowed runners who bring back matches fast enough
            // 3 times in a row
            kDebug() << id() << "runner is faster than we thought, kicking it up a notch";
            setSpeed(NormalSpeed);
        }
    }
}

QList<QAction*> AbstractRunner::actionsForMatch(const Plasma::QueryMatch &match)
{
    Q_UNUSED(match)
    QList<QAction*> ret;
    if (d->script) {
        emit d->script->actionsForMatch(match, &ret);
    }
    return ret;
}

QAction* AbstractRunner::addAction(const QString &id, const QIcon &icon, const QString &text)
{
    QAction *a = new QAction(icon, text, this);
    d->actions.insert(id, a);
    return a;
}

void AbstractRunner::addAction(const QString &id, QAction *action)
{
    d->actions.insert(id, action);
}

void AbstractRunner::removeAction(const QString &id)
{
    QAction *a = d->actions.take(id);
    delete a;
}

QAction* AbstractRunner::action(const QString &id) const
{
    return d->actions.value(id);
}

QHash<QString, QAction*> AbstractRunner::actions() const
{
    return d->actions;
}

void AbstractRunner::clearActions()
{
    qDeleteAll(d->actions);
    d->actions.clear();
}

QMimeData * AbstractRunner::mimeDataForMatch(const QueryMatch *match)
{
    Q_UNUSED(match)
    return 0;
}

bool AbstractRunner::hasRunOptions()
{
    return d->hasRunOptions;
}

void AbstractRunner::setHasRunOptions(bool hasRunOptions)
{
    d->hasRunOptions = hasRunOptions;
}

void AbstractRunner::createRunOptions(QWidget *parent)
{
    if (d->script) {
        emit d->script->createRunOptions(parent);
    }
}

AbstractRunner::Speed AbstractRunner::speed() const
{
    // the only time the read lock will fail is if we were slow are going to speed up
    // or if we were fast and are going to slow down; so don't wait in this case, just
    // say we're slow. we either will be soon or were just a moment ago and it doesn't
    // hurt to do one more run the slow way
    if (!d->speedLock.tryLockForRead()) {
        return SlowSpeed;
    }
    Speed s = d->speed;
    d->speedLock.unlock();
    return s;
}

void AbstractRunner::setSpeed(Speed speed)
{
    d->speedLock.lockForWrite();
    d->speed = speed;
    d->speedLock.unlock();
}

AbstractRunner::Priority AbstractRunner::priority() const
{
    return d->priority;
}

void AbstractRunner::setPriority(Priority priority)
{
    d->priority = priority;
}

RunnerContext::Types AbstractRunner::ignoredTypes() const
{
    return d->blackListed;
}

void AbstractRunner::setIgnoredTypes(RunnerContext::Types types)
{
    d->blackListed = types;
}

KService::List AbstractRunner::serviceQuery(const QString &serviceType, const QString &constraint) const
{
    return KServiceTypeTrader::self()->query(serviceType, constraint);
}

QMutex* AbstractRunner::bigLock()
{
    return s_bigLock;
}

void AbstractRunner::run(const Plasma::RunnerContext &search, const Plasma::QueryMatch &action)
{
    if (d->script) {
        return d->script->run(search, action);
    }
}

void AbstractRunner::match(Plasma::RunnerContext &search)
{
    if (d->script) {
        return d->script->match(search);
    }
}

QString AbstractRunner::name() const
{
    if (d->runnerDescription.isValid()) {
        return d->runnerDescription.name();
    }

    if (d->package) {
        return d->package->metadata().name();
    }

    return objectName();
}

QIcon AbstractRunner::icon() const
{
    if (d->runnerDescription.isValid()) {
        return KIcon(d->runnerDescription.icon());
    }

    if (d->package) {
        return KIcon(d->package->metadata().icon());
    }

    return QIcon();
}

QString AbstractRunner::id() const
{
    if (d->runnerDescription.isValid()) {
        return d->runnerDescription.pluginName();
    }

    if (d->package) {
        return d->package->metadata().pluginName();
    }

    return objectName();
}

QString AbstractRunner::description() const
{
    if (d->runnerDescription.isValid()) {
        return d->runnerDescription.property("Comment").toString();
    }

    if (d->package) {
        return d->package->metadata().description();
    }

    return objectName();
}

const Package* AbstractRunner::package() const
{
    return d->package;
}


void AbstractRunner::init()
{
    if (d->script) {
        d->setupScriptSupport();
        d->script->init();
    }
}

DataEngine *AbstractRunner::dataEngine(const QString &name) const
{
    return d->dataEngine(name);
}

AbstractRunnerPrivate::AbstractRunnerPrivate(AbstractRunner *r)
    : priority(AbstractRunner::NormalPriority),
      speed(AbstractRunner::NormalSpeed),
      blackListed(0),
      script(0),
      runner(r),
      fastRuns(0),
      package(0),
      hasRunOptions(false),
      defaultSyntax(0)
{
}

AbstractRunnerPrivate::~AbstractRunnerPrivate()
{
    delete script;
    script = 0;
    delete package;
    package = 0;
}

void AbstractRunnerPrivate::init(const KService::Ptr service)
{
    runnerDescription = KPluginInfo(service);
    if (runnerDescription.isValid()) {
        const QString api = runnerDescription.property("X-Plasma-API").toString();
        if (!api.isEmpty()) {
            const QString path = KStandardDirs::locate("data", "plasma/runners/" + runnerDescription.pluginName() + '/');
            prepScripting(path, api);
            if (!script) {
                kDebug() << "Could not create a(n)" << api << "ScriptEngine for the" << runnerDescription.name() << "Runner.";
            }
        }
    }
}

void AbstractRunnerPrivate::init(const QString &path)
{
    prepScripting(path);
}

void AbstractRunnerPrivate::prepScripting(const QString &path, QString api)
{
    if (script) {
        return;
    }

    if (package) {
        delete package;
    }

    PackageStructure::Ptr structure = Plasma::packageStructure(api, Plasma::RunnerComponent);
    structure->setPath(path);
    package = new Package(path, structure);

    if (!package->isValid()) {
        kDebug() << "Invalid Runner package at" << path;
        delete package;
        package = 0;
        return;
    }

    if (api.isEmpty()) {
        api = package->metadata().implementationApi();
    }

    script = Plasma::loadScriptEngine(api, runner);
    if (!script) {
        delete package;
        package = 0;
    }
}

// put all setup routines for script here. at this point we can assume that
// package exists and that we have a script engine
void AbstractRunnerPrivate::setupScriptSupport()
{
    if (!package) {
        return;
    }

    kDebug() << "setting up script support, package is in" << package->path()
             << "which is a" << package->structure()->type() << "package"
             << ", main script is" << package->filePath("mainscript");

    QString translationsPath = package->filePath("translations");
    if (!translationsPath.isEmpty()) {
        //FIXME: we should _probably_ use a KComponentData to segregate the applets
        //       from each other; but I want to get the basics working first :)
        KGlobal::dirs()->addResourceDir("locale", translationsPath);
        KGlobal::locale()->insertCatalog(package->metadata().pluginName());
    }
}

} // Plasma namespace

#include "abstractrunner.moc"
