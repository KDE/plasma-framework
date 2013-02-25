/*
 *   Copyright (c) 2009 Chani Armitage <chani@kde.org>

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

#ifndef PLASMA_CONTAINMENTACTIONS_H
#define PLASMA_CONTAINMENTACTIONS_H

#include <QList>

#include <kplugininfo.h>

#include <plasma/plasma.h>
#include <plasma/version.h>

class QAction;

namespace Plasma
{

class DataEngine;
class Containment;
class ContainmentActionsPrivate;

/**
 * @class ContainmentActions plasma/containmentactions.h <Plasma/ContainmentActions>
 *
 * @short The base ContainmentActions class
 *
 * "ContainmentActions" are components that provide actions (usually displaying a contextmenu) in
 * response to an event with a position (usually a mouse event).
 *
 * ContainmentActions plugins are registered using .desktop files. These files should be
 * named using the following naming scheme:
 *
 *     plasma-containmentactions-\<pluginname\>.desktop
 *
 */

class PLASMA_EXPORT ContainmentActions : public QObject
{
    Q_OBJECT

    public:
        /**
         * Where the actions come from.
         * Containments can use a global set of actions or a personalized set,
         * with granularity to Activity or Containment
         */
        enum ContainmentActionsSource {
            Global = 0,
            Activity,
            Local
        };
        Q_ENUMS(ContainmentActionsSource)

        /**
         * Default constructor for an empty or null containmentactions
         */
        explicit ContainmentActions(QObject * parent = 0);

        ~ContainmentActions();

        /**
         * Returns the user-visible name for the containmentactions, as specified in the
         * .desktop file.
         *
         * @return the user-visible name for the containmentactions.
         **/
        QString name() const;

        /**
         * Returns the plugin name for the containmentactions
         */
        QString pluginName() const;

        /**
         * Returns the icon related to this containmentactions
         **/
        QString icon() const;

        /**
         * Returns the configurations of this containmentactions
         * @since 5.0
         */
        KConfigGroup config() const;

        /**
         * Sets the source, for this containmentactions Global, Activity or Local
         * @param source @see ContainmentActionsSource
         * @since 5.0
         */
        void setSource(ContainmentActionsSource source);

        /**
         * @return the source of this containmentactions
         * @since 5.0
         */
        ContainmentActionsSource source() const;

        /**
         * This method should be called once the plugin is loaded or settings are changed.
         * @param config Config group to load settings
         * @see init
         **/
        virtual void restore(const KConfigGroup &config);

        /**
         * This method is called when settings need to be saved.
         * @param config Config group to save settings
         **/
        virtual void save(KConfigGroup &config);

        /**
         * Returns the widget used in the configuration dialog.
         * Add the configuration interface of the containmentactions to this widget.
         */
        virtual QWidget *createConfigurationInterface(QWidget *parent);

        /**
         * Called when a "next" action is triggered (e.g. by mouse wheel scroll). This
         * can be used to scroll through a list of items this plugin manages such as
         * windows, virtual desktops, activities, etc.
         * @see performPrevious
         */
        virtual void performNextAction();

        /**
         * Called when a "previous" action is triggered (e.g. by mouse wheel scroll). This
         * can be used to scroll through a list of items this plugin manages such as
         * windows, virtual desktops, activities, etc.
         * @see performNext
         */
        virtual void performPreviousAction();

        /**
         * Implement this to provide a list of actions that can be added to another menu
         * for example, when right-clicking an applet, the "Activity Options" submenu is populated
         * with this.
         */
        virtual QList<QAction*> contextualActions();

        /**
         * Turns a mouse or wheel event into a string suitable for a ContainmentActions
         * @return the string representation of the event
         */
        static QString eventToString(QEvent *event);

        /**
         * @p newContainment the containment the plugin should be associated with.
         * @since 4.6
         */
        void setContainment(Containment *newContainment);

        /**
         * @return the containment the plugin is associated with.
         */
        Containment *containment();

    protected:
        /**
         * This constructor is to be used with the plugin loading systems
         * found in KPluginInfo and KService. The argument list is expected
         * to have one element: the KService service ID for the desktop entry.
         *
         * @param parent a QObject parent; you probably want to pass in 0
         * @param args a list of strings containing one entry: the service id
         */
        ContainmentActions(QObject *parent, const QVariantList &args);

    private:
        ContainmentActionsPrivate *const d;
};

} // Plasma namespace

/**
 * Register a containmentactions when it is contained in a loadable module
 */
#define K_EXPORT_PLASMA_CONTAINMENTACTIONS(libname, classname) \
K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(factory("plasma_containmentactions_" #libname)) \
K_EXPORT_PLUGIN_VERSION(PLASMA_VERSION)

#endif // PLASMA_CONTAINMENTACTIONS_H
