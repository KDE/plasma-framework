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

#ifndef RUNNER_H
#define RUNNER_H

#include <QtCore/QObject>
#include <QtCore/QMutex>
#include <QtCore/QStringList>

#include <KDE/KConfigGroup>
#include <KDE/KService>

#include <plasma/plasma_export.h>
#include <plasma/package.h>
#include <plasma/searchmatch.h>
#include <plasma/searchcontext.h>

class KCompletion;

namespace Plasma
{

class Package;
class RunnerScript;

/**
 * An abstract base class for Plasma Runner plugins
 *
 * Be aware that runners have to be thread-safe. This is due to
 * the fact that each runner is executed in its own thread for
 * each new term. Thus, a runner may be executed more than once
 * at the same time.
 */
class PLASMA_EXPORT AbstractRunner : public QObject
{
    Q_OBJECT

    public:
        enum Speed { SlowSpeed,
                     NormalSpeed
                   };

        enum Priority { LowestPriority = 0,
                        LowPriority,
                        NormalPriority,
                        HighPriority,
                        HighestPriority
                      };

        typedef QList<AbstractRunner*> List;

        /**
         * Static method is called to load and get a list available of Runners.
         *
         * @param whitelist An optional whitelist of runners to load
         */
        static List load(QObject* parent, const QStringList& whitelist = QStringList());

        virtual ~AbstractRunner();

        /**
         * This is the main query method. It should trigger creation of
         * SearchMatch instances through SearchContext::addInformationalMatch,
         * SearchContext::addExactMatch, and SearchContext::addPossibleMatch.
         *
         * If the runner can run precisely the requested term (SearchContext::searchTerm),
         * it should create an exact match (SearchContext::addExactMatch).
         * The first runner that creates a SearchMatch will be the
         * default runner. Other runner's matches will be suggested in the
         * interface. Non-exact matches should be offered via SearchContext::addPossibleMatch.
         *
         * The match will be activated if the user selects it.
         *
         * If this runner's exact match is selected, it will be passed into
         * the exec method.
         * @see exec
         *
         * Since each runner is executed in its own thread there is no need
         * to return from this method right away, nor to create all matches
         * here.
         */
        virtual void match(Plasma::SearchContext *search);

        /**
         * Triggers a call to match.
         *
         * @arg globalContext the search context used in executing this match.
         */
        void performMatch(Plasma::SearchContext &globalContext);

        /**
         * If the runner has options that the user can interact with to modify
         * what happens when exec or one of the actions created in fillMatches
         * is called, the runner should return true
         */
        bool hasMatchOptions();

        /**
         * If hasMatchOptions() returns true, this method may be called to get
         * a widget displaying the options the user can interact with to modify
         * the behaviour of what happens when a given match is selected.
         *
         * @param widget the parent of the options widgets.
         */
        virtual void createMatchOptions(QWidget *widget);

        /**
         * If the runner itself has configuration options, this method returns true
         */
        bool isConfigurable();

        /**
         * If isConfigurable() returns true, this method may to get
         * a widget displaying the options the user can interact with to modify
         * the behaviour of what happens when a given match is selected.
         *
         * @param widget the parent of the options widgets.
         */
        virtual void createConfigurationInterface(QWidget *widget);

        /**
         * Called whenever an exact or possible match associated with this
         * runner is triggered.
         */
        virtual void exec(const Plasma::SearchContext *context, const Plasma::SearchMatch *action);

        /**
         * The nominal speed of the runner.
         * @see setSpeed
         */
        Speed speed() const;

        /**
         * The priority of the runner.
         * @see setPriority
         */
        Priority priority() const;

        /**
          * Returns the engine name for the Runner
          */
        QString name() const;

        /**
         * Accessor for the associated Package object if any.
         *
         * @return the Package object, or 0 if none
         **/
        const Package* package() const;

    protected:
        /**
         * Constructs a Runner object. Since AbstractRunner has pure virtuals,
         * this constructor can not be called directly. Rather a subclass must
         * be created
         */
        explicit AbstractRunner(QObject* parent = 0, const QString& serviceId = QString());
        AbstractRunner(QObject* parent, const QVariantList& args);

        /**
         * Provides access to the runner's configuration object.
         */
        KConfigGroup config() const;

        /**
         * Sets whether or not the the runner has options for matches
         */
        void setHasMatchOptions(bool hasMatchOptions);

        /**
         * Sets whether or not the runner has configuration options itself
         */
        void setIsConfigurable(bool canBeConfigured);

        /**
         * Sets the nominal speed of the runner. Only slow runners need
         * to call this within their constructor because the default
         * speed is NormalSpeed. Runners that use DBUS should call
         * this within their constructors.
         */
        void setSpeed(Speed newSpeed);

//For 4.1
//         /**
//          * Sets the run tier of the runner. Higher tier runners execute only
//          * after all lower-level runners execute. Call this method if your runner
//          * depends on the output of previous runners.
//          */
//         void setTier(int tier);

        /**
         * Sets the priority of the runner. Lower priority runners are executed
         * only after higher priority runners.
         */
        void setPriority(Priority newPriority);

        /**
         * A blocking method to do queries of installed Services which can provide
         * a measure of safety for runners running their own threads. This should
         * be used instead of calling KServiceTypeTrader::query(..) directly.
         *
         * @arg serviceType a service type like "Plasma/Applet" or "KFilePlugin"
         * @arg constraint a constraint to limit the the choices returned.
         * @see KServiceTypeTrader::query(const QString&, const QString&)
         *
         * @return a list of services that satisfy the query.
         */
        KService::List serviceQuery(const QString &serviceType,
                                    const QString &constraint = QString()) const;

        QMutex* bigLock() const;

    protected Q_SLOTS:
        void init();

    private:
        class Private;
        Private* const d;
};

} // Plasma namespace

#define K_EXPORT_PLASMA_RUNNER( libname, classname )     \
K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(factory("plasma_runner_" #libname))

#endif
