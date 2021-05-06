/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_APPLETSCRIPT_H
#define PLASMA_APPLETSCRIPT_H

#include <QObject>
#include <QRect>
#include <QSizeF>

#include <KPluginInfo>

#include <plasma/containment.h>
#include <plasma/plasma_export.h>
#include <plasma/scripting/scriptengine.h>

class QAction;
class QPainter;

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
    explicit AppletScript(QObject *parent = nullptr);
    ~AppletScript() override;

    /**
     * Sets the applet associated with this AppletScript
     */
    void setApplet(Plasma::Applet *applet);

    /**
     * Returns the Plasma::Applet associated with this script component
     */
    Plasma::Applet *applet() const;

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
    virtual void constraintsEvent(Plasma::Types::Constraints constraints);

    /**
     * Returns a list of context-related QAction instances.
     *
     * @return A list of actions. The default implementation returns an
     *         empty list.
     */
    virtual QList<QAction *> contextualActions();

    /**
     * Sets whether or not this script has a configuration interface or not
     *
     * @param hasInterface true if the applet is user configurable
     */
    void setHasConfigurationInterface(bool hasInterface);

    /**
     * @see Applet
     */
    void setConfigurationRequired(bool req, const QString &reason = QString());

    /**
     * @see Applet
     */
    void setLaunchErrorMessage(const QString &reason = QString());

    /**
     * @see Applet
     */
    void configNeedsSaving() const;

    /**
     * @see Containment
     * @since 4.7
     */
    Plasma::Types::ContainmentType containmentType() const;

    /**
     * @see Containment
     * @since 4.7
     */
    void setContainmentType(Plasma::Types::ContainmentType type);

Q_SIGNALS:
    /**
     * @see Applet
     */
    void saveState(KConfigGroup &group) const;

    /**
     * @param uiReady true if the UI for this applet is ready
     */
    void uiReadyChanged(bool uiReady);

public Q_SLOTS:

    /**
     * Configure was changed.
     */
    virtual void configChanged();

protected:
    /**
     * @return absolute path to the main script file for this plasmoid
     */
    QString mainScript() const override;

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 83)

    /**
     * @return the Package associated with this plasmoid which can
     *         be used to request resources, such as images and
     *         interface files.
     */
    PLASMA_DEPRECATED_VERSION(5, 83, "Use kpackage API instead")
    Package package() const override;
#endif

#if PLASMA_ENABLE_DEPRECATED_SINCE(5, 81)
    /**
     * @return the KPluginInfo associated with this plasmoid
     * @deprecated since 5.81, use applet()->pluginMetaData() instead.
     */
    PLASMA_DEPRECATED_VERSION(5, 81, "Use applet()->pluginMetaData() instead.")
    KPluginInfo description() const;
#endif

private:
    friend class Applet;

    AppletScriptPrivate *const d;
};

#define K_EXPORT_PLASMA_APPLETSCRIPTENGINE(libname, classname) K_PLUGIN_FACTORY(factory, registerPlugin<classname>();)

#define K_EXPORT_PLASMA_APPLETSCRIPTENGINE_WITH_JSON(libname, classname, jsonFile)                                                                             \
    K_PLUGIN_FACTORY_WITH_JSON(factory, jsonFile, registerPlugin<classname>();)                                                                                \
    K_EXPORT_PLUGIN_VERSION(PLASMA_VERSION)
} // Plasma namespace

#endif
