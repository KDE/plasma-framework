/*
    SPDX-FileCopyrightText: 2009 Chani Armitage <chani@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_CONTAINMENTACTIONS_H
#define PLASMA_CONTAINMENTACTIONS_H

#include <QList>

#include <plasma/plasma_export.h>
#if PLASMA_ENABLE_DEPRECATED_SINCE(5, 94)
#include <KPluginInfo>
#endif

#include <plasma/plasma.h>

class QAction;
class KPluginMetaData;
class KConfigGroup;

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
     * Default constructor for an empty or null containmentactions
     */
    explicit ContainmentActions(QObject *parent = nullptr);

    ~ContainmentActions() override;

#if PLASMA_ENABLE_DEPRECATED_SINCE(5, 67)
    /**
     * @return the plugin info for this ContainmentActions instance,
     *         including name, pluginName and icon
     * @since 5.0
     *
     * @deprecated since 5.67, use metadata
     */
    PLASMA_DEPRECATED_VERSION(5, 67, "use metadata()")
    KPluginInfo pluginInfo() const;
#endif

    /**
     * @return metadata for this ContainmentActions instance
     *         including name, pluginName and icon
     * @since 5.67
     */
    KPluginMetaData metadata() const;

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
     * This method is called when the user's configuration changes are accepted
     */
    virtual void configurationAccepted();

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
    virtual QList<QAction *> contextualActions();

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

#if PLASMA_ENABLE_DEPRECATED_SINCE(5, 88)
/**
 * Register a containmentactions when it is contained in a loadable module
 * @deprecated Since 5.88, use K_PLUGIN_CLASS_WITH_JSON instead
 */
/* clang-format off */
#define K_EXPORT_PLASMA_CONTAINMENTACTIONS_WITH_JSON(libname, classname, jsonFile) \
    K_PLUGIN_FACTORY_WITH_JSON(factory, jsonFile, registerPlugin<classname>();)
/* clang-format on */
#endif

#endif // PLASMA_CONTAINMENTACTIONS_H
