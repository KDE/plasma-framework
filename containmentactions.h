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
#include <plasma/packagestructure.h>
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
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString pluginName READ pluginName)
    Q_PROPERTY(QString icon READ icon)

    public:
        /**
         * Default constructor for an empty or null containmentactions
         */
        explicit ContainmentActions(QObject * parent = 0);

        ~ContainmentActions();

        /**
         * Returns a list of all known containmentactions plugins.
         *
         * @return list of containmentactions plugins
         **/
        static KPluginInfo::List listContainmentActionsInfo();

        /**
         * Attempts to load a containmentactions
         *
         * Returns a pointer to the containmentactions if successful.
         * The caller takes responsibility for the containmentactions, including
         * deleting it when no longer needed.
         *
         * @param parent the parent containment. Required; if you send null you'll get back null.
         * @param name the plugin name, as returned by KPluginInfo::pluginName()
         * @param args to send the containmentactions extra arguments
         * @return a pointer to the loaded containmentactions, or 0 on load failure
         **/
        static ContainmentActions *load(Containment *parent, const QString &name, const QVariantList &args = QVariantList());

        /**
         * Attempts to load a containmentactions
         *
         * Returns a pointer to the containmentactions if successful.
         * The caller takes responsibility for the containmentactions, including
         * deleting it when no longer needed.
         *
         * @param parent the parent containment. Required; if you send null you'll get back null.
         * @param info KPluginInfo object for the desired containmentactions
         * @param args to send the containmentactions extra arguments
         * @return a pointer to the loaded containmentactions, or 0 on load failure
         **/
        static ContainmentActions *load(Containment *parent, const KPluginInfo &info, const QVariantList &args = QVariantList());

        /**
         * Returns the Package specialization for containmentactions.
         */
        static PackageStructure::Ptr packageStructure();

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
         * @return true if initialized (usually by calling restore), false otherwise
         */
        bool isInitialized() const;

        /**
         * This method should be called once the plugin is loaded or settings are changed.
         * @param config Config group to load settings
         * @see init
         **/
        void restore(const KConfigGroup &config);

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
         * This method is called when the user's configuration changes are accepted
         */
        virtual void configurationAccepted();

        /**
         * Implement this to respond to events.
         * The user can configure whatever button and modifier they like, so please don't look at
         * those parameters.
         * So far the event could be a QGraphicsSceneMouseEvent or a QGraphicsSceneWheelEvent.
         */
        virtual void contextEvent(QEvent *event);

        /**
         * Implement this to provide a list of actions that can be added to another menu
         * for example, when right-clicking an applet, the "Activity Options" submenu is populated
         * with this.
         */
        virtual QList<QAction*> contextualActions();

        /**
         * Loads the given DataEngine
         *
         * Tries to load the data engine given by @p name.  Each engine is
         * only loaded once, and that instance is re-used on all subsequent
         * requests.
         *
         * If the data engine was not found, an invalid data engine is returned
         * (see DataEngine::isValid()).
         *
         * Note that you should <em>not</em> delete the returned engine.
         *
         * @param name Name of the data engine to load
         * @return pointer to the data engine if it was loaded,
         *         or an invalid data engine if the requested engine
         *         could not be loaded
         *
         */
        Q_INVOKABLE DataEngine *dataEngine(const QString &name) const;

        /**
         * @return true if the containmentactions currently needs to be configured,
         *         otherwise, false
         */
        bool configurationRequired() const;

        /**
         * Turns a mouse or wheel event into a string suitable for a ContainmentActions
         * @return the string representation of the event
         */
        static QString eventToString(QEvent *event);

        bool event(QEvent *e);

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

        /**
         * This method is called once the containmentactions is loaded or settings are changed.
         *
         * @param config Config group to load settings
         **/
        virtual void init(const KConfigGroup &config);

        /**
         * When the containmentactions needs to be configured before being usable, this
         * method can be called to denote that action is required
         *
         * @param needsConfiguring true if the applet needs to be configured,
         *                         or false if it doesn't
         */
        void setConfigurationRequired(bool needsConfiguring = true);

        /**
         * @return the containment the plugin is associated with.
         */
        Containment *containment();

        /**
         * pastes the clipboard at a given location
         */
        void paste(QPointF scenePos, QPoint screenPos);

    private:
        friend class ContainmentActionsPackage;
        friend class ContainmentActionsPrivate;
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
