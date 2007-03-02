/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include <QObject>
#include <QList>

#include <kdemacros.h>

class KActionCollection;

class KDE_EXPORT Runner : public QObject
{
    Q_OBJECT

    public:
        typedef QList<Runner*> List;

        explicit Runner( QObject* parent = 0 );
        virtual ~Runner();

        /**
         * If the runner can run precisely this term, return a QAction, else
         * return 0. The first runner that returns a QAction will be the
         * default runner. Other runner's actions will be suggested in the
         * interface. Non-exact matches should be offered via findMatches.
         * The action will be activated if the user selects it.
         */
        QAction* exactMatch( const QString& command );

        /**
         * If the runner has options that the user can interact with to modify
         * what happens when exec or one of the actions created in fillMatches
         * is called, the runner should return true
         */
        virtual bool hasOptions( );

        /**
         * If the hasOptions() returns true, this method will be called to get
         * the widget displaying the options the user can interact with.
         */
        virtual QWidget* options( );

        KActionCollection* matches( const QString& term, int max, int offset );

	/**
	 * Static method is called to load and get a list available of Runners.
	 */
	static List loadRunners( QWidget* parent );

    signals:
        /**
         * When emitted, the interface will update itself to show the new
         * matches. This is meant to be used by asynchronous runners that will
         * only be able to start a query on fillMatches being called with
         * response (and therefore matches) coming later
         */
        void matchesUpdated( KActionCollection* matches );

    protected:
        /**
         * This method is called when there is text input to match. The runner
         * should fill the matches action collection with one action per match
         * to a maximium of max matches starting at offset in the data set
         */
        virtual void fillMatches( KActionCollection* matches,
                                  const QString& term,
                                  int max, int offset );

        /**
         * If the runner can run precisely this term, return a QAction, else
         * return 0. The first runner that returns a QAction will be the
         * default runner. Other runner's actions will be suggested in the
         * interface. Non-exact matches should be offered via findMatches.
         * The action will be activated if the user selects it.
         */
        virtual QAction* accepts( const QString& term ) = 0;

        /**
         * Take action on the command. What this means is dependant on the
         * particular runner implementation, e.g. some runners may treat
         * command as a shell command, while others may treat it as an
         * equation or a user name or ...
         * This will be called automatically when the exact match
         * QAction is triggered
         */
        virtual bool exec( const QString& command ) = 0;

    private:
        class Private;
        Private* d;

    private Q_SLOTS:
        void runExactMatch();
};

#define K_EXPORT_KRUNNER_RUNNER( libname, classname )     \
    K_EXPORT_COMPONENT_FACTORY(                           \
        krunner_##libname,                                \
        KGenericFactory<classname>("krunner_" #libname) )

#endif
