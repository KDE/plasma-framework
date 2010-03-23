/*
 *   Copyright 2006-2009 Aaron Seigo <aseigo@kde.org>
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

#ifndef ABSTRACTRUNNER_P_H
#define ABSTRACTRUNNER_P_H

#include <QReadWriteLock>

#include "dataengineconsumer_p.h"

namespace Plasma
{

class AbstractRunner;

class AbstractRunnerPrivate : public DataEngineConsumer
{
public:
    AbstractRunnerPrivate(AbstractRunner *r);
    ~AbstractRunnerPrivate();
    void init(const KService::Ptr service);
    void init(const QString &path);
    void prepScripting(const QString &path, QString api = QString());
    void setupScriptSupport();

    AbstractRunner::Priority priority;
    AbstractRunner::Speed speed;
    RunnerContext::Types blackListed;
    RunnerScript *script;
    KPluginInfo runnerDescription;
    AbstractRunner *runner;
    int fastRuns;
    Package *package;
    QHash<QString, QAction*> actions;
    QList<RunnerSyntax> syntaxes;
    bool hasRunOptions;
    QReadWriteLock speedLock;
    RunnerSyntax *defaultSyntax;
};

} // namespace Plasma
#endif


