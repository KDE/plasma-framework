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

#ifndef PLASMA_RUNNERSCRIPT_H
#define PLASMA_RUNNERSCRIPT_H

#include <kgenericfactory.h>
#include <kplugininfo.h>

#include <plasma/plasma_export.h>
#include <plasma/abstractrunner.h>
#include <plasma/scripting/scriptengine.h>

namespace Plasma
{

class RunnerScriptPrivate;

/**
 * @class RunnerScript plasma/scripting/runnerscript.h <Plasma/Scripting/RunnerScript>
 *
 * @short Provides a restricted interface for scripting a runner.
 */
class PLASMA_EXPORT RunnerScript : public ScriptEngine
{
    Q_OBJECT

public:
    /**
     * Default constructor for a RunnerScript.
     * Subclasses should not attempt to access the Plasma::AbstractRunner
     * associated with this RunnerScript in the constructor. All
     * such set up that requires the AbstractRunner itself should be done
     * in the init() method.
     */
    explicit RunnerScript(QObject *parent = 0);
    ~RunnerScript();

    /**
     * Sets the Plasma::AbstractRunner associated with this RunnerScript
     */
    void setRunner(AbstractRunner *runner);

    /**
     * Returns the Plasma::AbstractRunner associated with this script component
     */
    AbstractRunner *runner() const;

    /**
     * Called when the script should create QueryMatch instances through
     * RunnerContext::addInformationalMatch, RunnerContext::addExactMatch, and
     * RunnerContext::addPossibleMatch.
     */
    virtual void match(Plasma::RunnerContext &search);

    /**
     * Called whenever an exact or possible match associated with this
     * runner is triggered.
     */
    virtual void run(const Plasma::RunnerContext &search, const Plasma::QueryMatch &action);


Q_SIGNALS:
    void prepare();
    void teardown();
    void createRunOptions(QWidget *widget);
    void reloadConfiguration();
    void actionsForMatch(const Plasma::QueryMatch &match, QList<QAction*>* actions);

protected:
    /**
     * @return absolute path to the main script file for this plasmoid
     */
    QString mainScript() const;

    /**
     * @return the Package associated with this plasmoid which can
     *         be used to request resources, such as images and
     *         interface files.
     */
    const Package *package() const;

    /**
     * @return the KPluginInfo associated with this plasmoid
     */
    KPluginInfo description() const;

    /**
     * @return a Plasma::DataEngine matchin name
     * @since 4.4
     */
    DataEngine *dataEngine(const QString &name);

    KConfigGroup config() const;
    void setIgnoredTypes(RunnerContext::Types types);
    void setHasRunOptions(bool hasRunOptions);
    void setSpeed(AbstractRunner::Speed newSpeed);
    void setPriority(AbstractRunner::Priority newPriority);
    KService::List serviceQuery(const QString &serviceType,
                                const QString &constraint = QString()) const;
    QAction* addAction(const QString &id, const QIcon &icon, const QString &text);
    void addAction(const QString &id, QAction *action);
    void removeAction(const QString &id);
    QAction* action(const QString &id) const;
    QHash<QString, QAction*> actions() const;
    void clearActions();
    void addSyntax(const RunnerSyntax &syntax);
    void setSyntaxes(const QList<RunnerSyntax> &syns);

private:
    friend class AbstractRunner;

    RunnerScriptPrivate *const d;
};

#define K_EXPORT_PLASMA_RUNNERSCRIPTENGINE(libname, classname) \
K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(factory("plasma_runnerscriptengine_" #libname))

} //Plasma namespace

#endif
