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
#include <QtCore/QStringList>

#include <KDE/KConfigGroup>

#include <plasma/plasma_export.h>
#include <plasma/searchmatch.h>
#include <plasma/searchcontext.h>

class KCompletion;

namespace Plasma
{

/**
 * An abstract base class for Plasma Runner plugins
 */
class PLASMA_EXPORT AbstractRunner : public QObject
{
    Q_OBJECT

    public:
        enum Speed { NormalSpeed,
                     SlowSpeed
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
        static List loadRunners(QObject* parent, const QStringList& whitelist = QStringList() );

        /**
         * Constructs a Runner object. Since AbstractRunner has pure virtuals,
         * this constructor can not be called directly. Rather a subclass must
         * be created
         */
        explicit AbstractRunner(QObject* parent = 0);
        virtual ~AbstractRunner();

        /**
         * Provides access to the runner's configuration object.
         */
        KConfigGroup config() const;

        /**
         * If the runner can run precisely this term, return a QAction, else
         * return 0. The first runner that returns a QAction will be the
         * default runner. Other runner's actions will be suggested in the
         * interface. Non-exact matches should be offered via findMatches.
         * The action will be activated if the user selects it.
         * If the action is informational only and should not be executed,
         * disable the action with setEnabled( false ).
         *
         * If this runner's exact match is selected, the action will not
         * be triggered, but it will be passed into the exec method.
         * @see exec
         *
         * Ownership of the action passes to the AbstractRunner class.
         */
        virtual void match(Plasma::SearchContext *search) = 0;

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
        virtual void exec(Plasma::SearchMatch *action);

        /**
         * The nominal speed of the runner.
         * @see setSpeed
         */
        Speed speed() const;

// For 4.1
//         /**
//          * The tier of the runner.
//          * @see setTier
//          */
//         int tier() const;

        /**
         * The priority of the runner.
         * @see setPriority
         */
        Priority priority() const;

    protected:
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

    private:
        class Private;
        Private* const d;
};

} // Plasma namespace

#define K_EXPORT_PLASMA_RUNNER( libname, classname )     \
K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(factory("plasma_runner_" #libname))

#endif
