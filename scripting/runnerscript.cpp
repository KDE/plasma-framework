/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
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

#include "plasma/scripting/runnerscript.h"

#include "plasma/abstractrunner.h"
#include "plasma/dataenginemanager.h"
#include "plasma/package.h"
#include "plasma/private/abstractrunner_p.h"

namespace Plasma
{

class RunnerScriptPrivate
{
public:
    AbstractRunner *runner;
};

RunnerScript::RunnerScript(QObject *parent)
    : ScriptEngine(parent),
      d(new RunnerScriptPrivate)
{
}

RunnerScript::~RunnerScript()
{
    delete d;
}

void RunnerScript::setRunner(AbstractRunner *runner)
{
    d->runner = runner;
    connect(runner, SIGNAL(prepare()), this, SIGNAL(prepare()));
    connect(runner, SIGNAL(teardown()), this, SIGNAL(teardown()));
}

AbstractRunner *RunnerScript::runner() const
{
    return d->runner;
}

void RunnerScript::match(Plasma::RunnerContext &search)
{
    Q_UNUSED(search);
}

void RunnerScript::run(const Plasma::RunnerContext &search, const Plasma::QueryMatch &action)
{
    Q_UNUSED(search);
    Q_UNUSED(action);
}

DataEngine *RunnerScript::dataEngine(const QString &name)
{
    if (d->runner) {
        return d->runner->dataEngine(name);
    }

    return DataEngineManager::self()->engine(QString());
}

KConfigGroup RunnerScript::config() const
{
    if (d->runner) {
        return d->runner->config();
    }
    return KConfigGroup();
}

void RunnerScript::setIgnoredTypes(RunnerContext::Types types)
{
    if (d->runner) {
        d->runner->setIgnoredTypes(types);
    }
}

void RunnerScript::setHasRunOptions(bool hasRunOptions)
{
    if (d->runner) {
        d->runner->setHasRunOptions(hasRunOptions);
    }
}

void RunnerScript::setSpeed(AbstractRunner::Speed newSpeed)
{
    if (d->runner) {
        d->runner->setSpeed(newSpeed);
    }
}

void RunnerScript::setPriority(AbstractRunner::Priority newPriority)
{
    if (d->runner) {
        d->runner->setPriority(newPriority);
    }
}

KService::List RunnerScript::serviceQuery(const QString &serviceType,
                                          const QString &constraint) const
{
    if (d->runner) {
        return d->runner->serviceQuery(serviceType, constraint);
    }
    return KService::List();
}

QAction* RunnerScript::addAction(const QString &id, const QIcon &icon, const QString &text)
{
    if (d->runner) {
        return d->runner->addAction(id, icon, text);
    }
    return 0;
}

void RunnerScript::addAction(const QString &id, QAction *action)
{
    if (d->runner) {
        d->runner->addAction(id, action);
    }
}

void RunnerScript::removeAction(const QString &id)
{
    if (d->runner) {
        d->runner->removeAction(id);
    }
}

QAction* RunnerScript::action(const QString &id) const
{
    if (d->runner) {
        return d->runner->action(id);
    }
    return 0;
}

QHash<QString, QAction*> RunnerScript::actions() const
{
    if (d->runner) {
        return d->runner->actions();
    }
    return QHash<QString, QAction*>();
}

void RunnerScript::clearActions()
{
    if (d->runner) {
        d->runner->clearActions();
    }
}

void RunnerScript::addSyntax(const RunnerSyntax &syntax)
{
    if (d->runner) {
        d->runner->addSyntax(syntax);
    }
}

void RunnerScript::setSyntaxes(const QList<RunnerSyntax> &syns)
{
    if (d->runner) {
        d->runner->setSyntaxes(syns);
    }
}

const Package *RunnerScript::package() const
{
    return d->runner ? d->runner->package() : 0;
}

KPluginInfo RunnerScript::description() const
{
    return d->runner ? d->runner->d->runnerDescription : KPluginInfo();
}

QString RunnerScript::mainScript() const
{
    if (!package()) {
        return QString();
    } else {
        return package()->filePath("mainscript");
    }
}

} // Plasma namespace

#include "runnerscript.moc"
