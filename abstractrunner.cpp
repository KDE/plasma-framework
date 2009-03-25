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
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>

#include <kdebug.h>
#include <kicon.h>
#include <kplugininfo.h>
#include <kservicetypetrader.h>
#include <kstandarddirs.h>

#include <plasma/querymatch.h>
#include <plasma/package.h>

#include "scripting/runnerscript.h"

#include "runnercontext.h"

namespace Plasma
{

class AbstractRunnerPrivate
{
public:
    AbstractRunnerPrivate(AbstractRunner *r, KService::Ptr service)
      : priority(AbstractRunner::NormalPriority),
        speed(AbstractRunner::NormalSpeed),
        blackListed(0),
        script(0),
        runnerDescription(service),
        runner(r),
        fastRuns(0),
        package(0)
    {
        if (runnerDescription.isValid()) {
            const QString api = runnerDescription.property("X-Plasma-API").toString();
            if (!api.isEmpty()) {
                const QString path = KStandardDirs::locate("data",
                                    "plasma/runners/" + runnerDescription.pluginName() + '/');
                PackageStructure::Ptr structure =
                    Plasma::packageStructure(api, Plasma::RunnerComponent);
                structure->setPath(path);
                package = new Package(path, structure);

                script = Plasma::loadScriptEngine(api, runner);
                if (!script) {
                    kDebug() << "Could not create a(n)" << api << "ScriptEngine for the"
                             << runnerDescription.name() << "Runner.";
                    delete package;
                    package = 0;
                } else {
                    QTimer::singleShot(0, runner, SLOT(init()));
                }
            }
        }
    }

    ~AbstractRunnerPrivate()
    {
        delete script;
        script = 0;
        delete package;
        package = 0;
    }

    bool hasRunOptions;
    bool hasConfig;
    AbstractRunner::Priority priority;
    AbstractRunner::Speed speed;
    RunnerContext::Types blackListed;
    RunnerScript *script;
    KPluginInfo runnerDescription;
    AbstractRunner *runner;
    QTime runtime;
    int fastRuns;
    Package *package;
    QHash<QString, QAction*> actions;
    QList<RunnerSyntax> syntaxes;
};

K_GLOBAL_STATIC(QMutex, s_bigLock)

AbstractRunner::AbstractRunner(QObject *parent, const QString &serviceId)
    : QObject(parent),
    d(new AbstractRunnerPrivate(this, KService::serviceByStorageId(serviceId)))
{
}

AbstractRunner::AbstractRunner(QObject *parent, const QVariantList &args)
    : QObject(parent),
      d(new AbstractRunnerPrivate(this, KService::serviceByStorageId(args.count() > 0 ? args[0].toString() : QString())))
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

void AbstractRunner::reloadConfiguration()
{
}

void AbstractRunner::addSyntax(const RunnerSyntax &syntax)
{
    d->syntaxes.append(syntax);
}

void AbstractRunner::setSyntaxes(const QList<RunnerSyntax> &syntaxes)
{
    d->syntaxes = syntaxes;
}

QList<RunnerSyntax> AbstractRunner::syntaxes() const
{
    return d->syntaxes;
}

void AbstractRunner::performMatch(Plasma::RunnerContext &localContext)
{
    static const int reasonableRunTime = 1500;
    static const int fastEnoughTime = 250;

    d->runtime.restart();

    //The local copy is already obtained in the job
    match(localContext);

    // automatically rate limit runners that become slooow
    const int runtime = d->runtime.elapsed();
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
    Q_UNUSED(parent)
}

AbstractRunner::Speed AbstractRunner::speed() const
{
    return d->speed;
}

void AbstractRunner::setSpeed(Speed speed)
{
    d->speed = speed;
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
    if (!d->runnerDescription.isValid()) {
        return objectName();
    }

    return d->runnerDescription.name();
}

QIcon AbstractRunner::icon() const
{
    if (!d->runnerDescription.isValid()) {
        return QIcon();
    }

    return KIcon(d->runnerDescription.icon());
}

QString AbstractRunner::id() const
{
    if (!d->runnerDescription.isValid()) {
        return objectName();
    }
    return d->runnerDescription.pluginName();
}

QString AbstractRunner::description() const
{
    if (!d->runnerDescription.isValid()) {
        return objectName();
    }

    return d->runnerDescription.property("Comment").toString();
}

const Package* AbstractRunner::package() const
{
    return d->package;
}

void AbstractRunner::init()
{
    if (d->script) {
        d->script->init();
    }
}

} // Plasma namespace

#include "abstractrunner.moc"
