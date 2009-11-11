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

#ifndef PLASMA_APPLETSCRIPT_H
#define PLASMA_APPLETSCRIPT_H

#include <QtCore/QObject>
#include <QtCore/QRect>
#include <QtCore/QSizeF>

#include <kgenericfactory.h>
#include <kplugininfo.h>

#include <plasma/plasma_export.h>
#include <plasma/scripting/scriptengine.h>

class QAction;
class QPainter;
class QStyleOptionGraphicsItem;
class KConfigDialog;

namespace Plasma
{

class AppletScriptPrivate;
class Extender;

/**
 * @class AppletScript plasma/scripting/appletscript.h <Plasma/Scripting/AppletScript>
 *
 * @short Provides a restricted interface for scripted applets.
 */
class PLASMA_EXPORT AppletScript : public ScriptEngine
{
    Q_OBJECT

public:
    /**
     * Default constructor for an AppletScript.
     *
     * Subclasses should not attempt to access the Plasma::Applet
     * associated with this AppletScript in the constructor. All
     * such set up that requires the Applet itself should be done
     * in the init() method.
     */
    explicit AppletScript(QObject *parent = 0);
    ~AppletScript();

    /**
     * Sets the applet associated with this AppletScript
     */
    void setApplet(Plasma::Applet *applet);

    /**
     * Returns the Plasma::Applet associated with this script component
     */
    Plasma::Applet *applet() const;

    /**
     * Called when the script should paint the applet
     *
     * @param painter the QPainter to use
     * @param option the style option containing such flags as selection, level of detail, etc
     * @param contentsRect the rect to paint within; automatically adjusted for
     *                     the background, if any
     */
    virtual void paintInterface(QPainter *painter,
                                const QStyleOptionGraphicsItem *option,
                                const QRect &contentsRect);

    /**
     * Returns the area within which contents can be painted.
     **/
    Q_INVOKABLE QSizeF size() const;

    /**
     * Called when any of the geometry constraints have been updated.
     *
     * This is always called prior to painting and should be used as an
     * opportunity to layout the widget, calculate sizings, etc.
     *
     * Do not call update() from this method; an update() will be triggered
     * at the appropriate time for the applet.
     *
     * @param constraints the type of constraints that were updated
     */
    virtual void constraintsEvent(Plasma::Constraints constraints);

    /**
     * Returns a list of context-related QAction instances.
     *
     * @return A list of actions. The default implementation returns an
     *         empty list.
     */
    virtual QList<QAction*> contextualActions();

    /**
     * Returns the shape of the widget, defaults to the bounding rect
     */
    virtual QPainterPath shape() const;

    /**
     * Sets whether or not this script has a configuration interface or not
     *
     * @arg hasInterface true if the applet is user configurable
     */
    void setHasConfigurationInterface(bool hasInterface);

    /**
     * @see Applet
     */
    void setConfigurationRequired(bool req, const QString &reason = QString());

    /**
     * @see Applet
     */
    void setFailedToLaunch(bool failed, const QString &reason = QString());

    /**
     * @see Applet
     */
    void configNeedsSaving() const;

    /**
     * @see Applet
     */
    Extender *extender() const;

Q_SIGNALS:
    /**
     * @see Applet
     */
    void saveState(KConfigGroup &group) const;

public Q_SLOTS:

    /**
     * Show a configuration dialog.
     */
    virtual void showConfigurationInterface();

    /**
     * Configure was changed.
     */
    virtual void configChanged();

protected:
    /**
     * @arg engine name of the engine
     * @return a data engine associated with this plasmoid
     */
    Q_INVOKABLE DataEngine *dataEngine(const QString &engine) const;

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
     * @return a standard Plasma applet configuration dialog, ready
     * to have pages added to it.
     *
     * Note that the dialog returned is set to delete on close.
     */
    KConfigDialog *standardConfigurationDialog();

    /**
     * This method should be called after a scripting applet has added
     * its own pages to a configuration dialog
     *
     * @since 4.3.1
     */
    void addStandardConfigurationPages(KConfigDialog *dialog);

    /**
     * @see Applet
     */
    void showMessage(const QIcon &icon, const QString &message, const MessageButtons buttons);

    /**
     * @see Applet
     */
    void registerAsDragHandle(QGraphicsItem *item);

    /**
     * @see Applet
     */
    void unregisterAsDragHandle(QGraphicsItem *item);

    /**
     * @see Applet
     */
    bool isRegisteredAsDragHandle(QGraphicsItem *item);

private:
    friend class Applet;

    AppletScriptPrivate *const d;
};

#define K_EXPORT_PLASMA_APPLETSCRIPTENGINE(libname, classname) \
K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(factory("plasma_appletscriptengine_" #libname))

} //Plasma namespace

#endif
