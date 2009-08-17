/*
 *   Copyright 2008 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Petri Damsten <damu@iki.fi>

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

#ifndef PLASMA_CONTEXTACTION_H
#define PLASMA_CONTEXTACTION_H

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
class ContextActionPrivate;

/**
 * @class ContextAction plasma/contextaction.h <Plasma/ContextAction>
 *
 * @short The base ContextAction class
 *
 * "ContextActions" are components that provide an action (usually displaying a contextmenu) in
 * response to an event with a position (usually a mouse event).
 *
 * ContextAction plugins are registered using .desktop files. These files should be
 * named using the following naming scheme:
 *
 *     plasma-contextaction-\<pluginname\>.desktop
 *
 */

class PLASMA_EXPORT ContextAction : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString pluginName READ pluginName)
    Q_PROPERTY(QString icon READ icon)

    public:
        /**
         * Default constructor for an empty or null contextaction
         */
        explicit ContextAction(QObject * parent = 0);

        ~ContextAction();

        /**
         * Returns a list of all known contextactions.
         *
         * @return list of contextactions
         **/
        static KPluginInfo::List listContextActionInfo();

        /**
         * Attempts to load a contextaction
         *
         * Returns a pointer to the contextaction if successful.
         * The caller takes responsibility for the contextaction, including
         * deleting it when no longer needed.
         *
         * @param name the plugin name, as returned by KPluginInfo::pluginName()
         * @param args to send the contextaction extra arguments
         * @return a pointer to the loaded contextaction, or 0 on load failure
         **/
        static ContextAction *load(const QString &name, const QVariantList &args = QVariantList());

        /**
         * Attempts to load a contextaction
         *
         * Returns a pointer to the contextaction if successful.
         * The caller takes responsibility for the contextaction, including
         * deleting it when no longer needed.
         *
         * @param info KPluginInfo object for the desired contextaction
         * @param args to send the contextaction extra arguments
         * @return a pointer to the loaded contextaction, or 0 on load failure
         **/
        static ContextAction *load(const KPluginInfo &info, const QVariantList &args = QVariantList());

        /**
         * Returns the Package specialization for contextactions.
         */
        static PackageStructure::Ptr packageStructure();

        /**
         * Returns the user-visible name for the contextaction, as specified in the
         * .desktop file.
         *
         * @return the user-visible name for the contextaction.
         **/
        QString name() const;

        /**
         * Returns the plugin name for the contextaction
         */
        QString pluginName() const;

        /**
         * Returns the icon related to this contextaction
         **/
        QString icon() const;

        /**
         * @return true if initialized (usually by calling restore), false otherwise
         */
        bool isInitialized() const;

        /**
         * This method should be called once the wallpaper is loaded or mode is changed.
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
         * Add the configuration interface of the contextaction to this widget.
         * To signal that settings have changed connect to
         * settingsChanged(bool modified) in @p parent.
         * @code connect(this, SIGNAL(settingsChanged(bool), parent, SLOT(settingsChanged(bool)))
         * @endcode
         * Emit settingsChanged(true) when the settings are changed and false when the original state is restored.
         */
        virtual QWidget *createConfigurationInterface(QWidget *parent);

        /**
         * This method is called when the user's configuration changes are accepted
         */
        virtual void configurationAccepted();

        /**
         * Implement this to respond to a mouse button event.
         * The user can configure whatever button and modifier they like, so please don't look at
         * those parameters.
         * @param event the mouse event object
         */
        virtual void contextEvent(QGraphicsSceneMouseEvent *event);

        /**
         * Implement this to respond to a wheel event.
         * The user can configure which wheel and whatever modifier they like, so please don't look at
         * those parameters.
         * @param event the mousewheel event object
         */
        virtual void wheelEvent(QGraphicsSceneWheelEvent *event);

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
         * @return true if the contextaction currently needs to be configured,
         *         otherwise, false
         */
        bool configurationRequired() const;

        /**
         * @return a config action if the contextaction currently needs to be configured,
         *         otherwise, null
         */
        QAction *configurationAction();

    protected:
        /**
         * This constructor is to be used with the plugin loading systems
         * found in KPluginInfo and KService. The argument list is expected
         * to have one element: the KService service ID for the desktop entry.
         *
         * @param parent a QObject parent; you probably want to pass in 0
         * @param args a list of strings containing one entry: the service id
         */
        ContextAction(QObject *parent, const QVariantList &args);

        /**
         * This method is called once the contextaction is loaded or mode is changed.
         *
         * The mode can be retrieved using the renderingMode() method.
         *
         * @param config Config group to load settings
         **/
        virtual void init(const KConfigGroup &config);

        /**
         * When the contextaction needs to be configured before being usable, this
         * method can be called to denote that action is required
         *
         * @param needsConfiguring true if the applet needs to be configured,
         *                         or false if it doesn't
         * @param reason a translated message for the user explaining that the
         *               applet needs configuring; this should note what needs
         *               to be configured
         */
        void setConfigurationRequired(bool needsConfiguring, const QString &reason = QString());

        /**
         * return the containment the plugin is associated with, if any.
         */
        Containment *containment();

    private:
        friend class ContextActionPackage;
        friend class ContextActionPrivate;
        ContextActionPrivate *const d;
};

} // Plasma namespace

/**
 * Register a contextaction when it is contained in a loadable module
 */
#define K_EXPORT_PLASMA_CONTEXTACTION(libname, classname) \
K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(factory("plasma_contextaction_" #libname)) \
K_EXPORT_PLUGIN_VERSION(PLASMA_VERSION)

#endif // multiple inclusion guard
