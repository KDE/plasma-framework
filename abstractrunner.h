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

#include <QtGui/QAction>
#include <QtCore/QList>
#include <QtCore/QObject>

#include <plasma/plasma_export.h>

class KActionCollection;
class QAction;

namespace Plasma
{

class AbstractRunner;
class SearchAction;

class PLASMA_EXPORT SearchContext : public QObject
{
    Q_OBJECT

    public:
        enum Type { UnknownType = 0,
                    Directory,
                    File,
                    NetworkLocation,
                    Executable,
                    ShellCommand,
                    Help
                  };

        explicit SearchContext(QObject *parent = 0);
        ~SearchContext();

        void setTerm(const QString&);
        QString term() const;
        Type type() const;
        QString mimetype() const;

        SearchAction* addInformationalMatch(AbstractRunner *runner);
        SearchAction* addExactMatch(AbstractRunner *runner);
        SearchAction* addPossibleMatch(AbstractRunner *runner);

        QList<SearchAction *> informationalMatches() const;
        QList<SearchAction *> exactMatches() const;
        QList<SearchAction *> possibleMatches() const;

    private:
        class Private;
        Private * const d;
};

class PLASMA_EXPORT SearchAction : public QAction
{
    Q_OBJECT

    public:
        enum Type { InformationalMatch,
                    ExactMatch,
                    PossibleMatch };

        SearchAction(SearchContext *search, AbstractRunner *runner);
        ~SearchAction();

        /**
         * Sets the type of match this action represents.
         */
        void setType(Type type);

        /**
         * The type of action this is. Defaults to ExactMatch.
         */
        Type type() const;

        /**
         * Sets the mimetype, if any, associated with this match
         *
         * @arg mimetype the mimetype
         */
        void setMimetype(const QString &mimetype);

        /**
         * The mimetype associated with this action, if any
         */
        QString mimetype() const;

        /**
         * The search term that triggered this action
         */
        QString term() const;

        /**
         * Sets the relevance of this action for the search
         * it was created for.
         *
         * @param relevance a number between 0 and 1.
         */
        void setRelevance(qreal relevance);

        /**
         * The relevance of this action to the search. By default,
         * the relevance is 1.
         *
         * @return a number between 0 and 1
         */
        qreal relevance() const;

        /**
         * The runner associated with this action
         */
        AbstractRunner* runner() const;

        bool operator<(const SearchAction& other) const;

    protected Q_SLOTS:
        void exec();

    private:
        class Private;
        Private * const d;
};

/**
 * A abstract super-class for Plasma Runners
 */
class PLASMA_EXPORT AbstractRunner : public QObject
{
    Q_OBJECT

    public:
        typedef QList<AbstractRunner*> List;

        /**
         * Static method is called to load and get a list available of Runners.
         */
        static List loadRunners(QObject* parent);

        /**
         * Constrcuts an Runner object. Since AbstractRunner has pure virtuals,
         * this constructor can not be called directly. Rather a subclass must
         * be created
         */
        explicit AbstractRunner(QObject* parent = 0);
        virtual ~AbstractRunner();

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
        bool canBeConfigured();

        /**
         * If canBeConfigured() returns true, this method may to get
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
        virtual void exec(Plasma::SearchAction *action);

    protected:
        /**
         * Sets whether or not the the runner has options for matches
         */
        void setHasMatchOptions(bool hasMatchOptions);

        /**
         * Sets whether or not the runner has configuration options itself
         */
        void setCanBeConfigured(bool canBeConfigured);

    private:
        class Private;
        Private* const d;
};

} // Plasma namespace

#define K_EXPORT_PLASMA_RUNNER( libname, classname )     \
K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(factory("plasma_runner_" #libname))

#endif
