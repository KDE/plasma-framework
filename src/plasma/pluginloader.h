/*
    SPDX-FileCopyrightText: 2010 Ryan Rix <ry@n.rix.si>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLUGIN_LOADER_H
#define PLUGIN_LOADER_H

#include <plasma/plasma_export.h>

#include <plasma/plasma.h>

#include <QVariant>

class KPluginMetaData;
namespace Plasma
{
class Applet;
class Containment;
class ContainmentActions;
class DataEngine;
class Service;

class PluginLoaderPrivate;

// TODO:
// * add loadWallpaper
// * add KPluginInfo listing support for Containments (already loaded via the applet loading code)

/**
 * @class PluginLoader plasma/pluginloader.h <Plasma/PluginLoader>
 *
 * This is an abstract base class which defines an interface to which Plasma's
 * Applet Loading logic can communicate with a parent application. The plugin loader
 * must be set before any plugins are loaded, otherwise (for safety reasons), the
 * default PluginLoader implementation will be used. The reimplemented version should
 * not do more than simply returning a loaded plugin. It should not init() it, and it should not
 * hang on to it. The associated methods will be called only when a component of Plasma
 * needs to load a _new_ plugin. (e.g. DataEngine does its own caching).
 *
 * @author Ryan Rix <ry@n.rix.si>
 * @since 4.6
 **/
class PLASMA_EXPORT PluginLoader
{
public:
    /**
     * Load an Applet plugin.
     *
     * @param name the plugin name, as returned by KPluginInfo::pluginName()
     * @param appletId unique ID to assign the applet, or zero to have one
     *        assigned automatically.
     * @param args to send the applet extra arguments
     * @return a pointer to the loaded applet, or 0 on load failure
     **/
    Applet *loadApplet(const QString &name, uint appletId = 0, const QVariantList &args = QVariantList());

    /**
     * Load a Service plugin.
     *
     * @param name the plugin name of the service to load
     * @param args a list of arguments to supply to the service plugin when loading it
     * @param parent the parent object, if any, for the service
     *
     * @return a Service object, unlike Plasma::Service::loadService, this can return null.
     **/
    Service *loadService(const QString &name, const QVariantList &args, QObject *parent = nullptr);

    /**
     * Load a ContainmentActions plugin.
     *
     * Returns a pointer to the containmentactions if successful.
     * The caller takes responsibility for the containmentactions, including
     * deleting it when no longer needed.
     *
     * @param parent the parent containment. @since 4.6 null is allowed.
     * @param name the plugin name, as returned by KPluginInfo::pluginName()
     * @param args to send the containmentactions extra arguments
     * @return a ContainmentActions object
     **/
    ContainmentActions *loadContainmentActions(Containment *parent, const QString &containmentActionsName, const QVariantList &args = QVariantList());

    /**
     * Returns a list of all known applets.
     * This may skip applets based on security settings and ExcludeCategories in the application's config.
     *
     * @param category Only applets matching this category will be returned.
     *                 If "Misc" is passed in, then applets without a
     *                 Categories= entry are also returned.
     *                 If an empty string is passed in, all applets are
     *                 returned.
     * @return list of applets
     *
     * @since 5.28
     **/
    QList<KPluginMetaData> listAppletMetaData(const QString &category);

    /**
     * Returns a list of all known applets associated with a certain mimetype.
     *
     * @return list of applets
     * @since 5.36
     **/
    QList<KPluginMetaData> listAppletMetaDataForMimeType(const QString &mimetype);

    /**
     * Returns a list of all known applets associated with a certain URL.
     *
     * @return list of applets
     * @since 5.36
     **/
    QList<KPluginMetaData> listAppletMetaDataForUrl(const QUrl &url);

    /**
     * Returns a list of all the categories used by installed applets.
     *
     * @param parentApp the application to filter applets on. Uses the
     *                  X-KDE-ParentApp entry (if any) in the plugin info.
     *                  The default value of QString() will result in a
     *                  list of all Applets.
     * @return list of categories
     * @param visibleOnly true if it should only return applets that are marked as visible
     */
    QStringList listAppletCategories(const QString &parentApp = QString(), bool visibleOnly = true);

    /**
     * Sets the list of custom categories that are used in addition to the default
     * set of categories known to libplasma for applets.
     * @param categories a list of categories
     * @since 4.3
     */
    void setCustomAppletCategories(const QStringList &categories);

    /**
     * @return the list of custom categories known to libplasma
     * @since 4.3
     */
    QStringList customAppletCategories() const;

    /**
     * Get the category of the given applet
     *
     * @param appletName the name of the applet
     */
    QString appletCategory(const QString &appletName);

    /**
     * Returns a list of all known containments.
     *
     * @param filter An optional predicate that can be used for filtering.
     *
     * @return list of containments
     */
    static QList<KPluginMetaData> listContainmentsMetaData(std::function<bool(const KPluginMetaData &)> filter = {});

    /**
     * Returns a list of containments of the specified type.
     *
     * @param type The target containment type
     *
     * @return list of containments
     */
    static QList<KPluginMetaData> listContainmentsMetaDataOfType(const QString &type);

    /**
     * Returns a list of all known dataengines.
     *
     * @param parentApp the application to filter dataengines on. Uses the
     *                  X-KDE-ParentApp entry (if any) in the plugin info.
     *                  The default value of QString() will result in a
     *                  list of all dataengines
     * @return list of dataengines
     * @since 5.77
     **/
    QVector<KPluginMetaData> listDataEngineMetaData(const QString &parentApp = QString());

    /**
     * Returns a list of all known ContainmentActions.
     *
     * @param parentApp the application to filter ContainmentActions on. Uses the
     *                  X-KDE-ParentApp entry (if any) in the plugin metadata.
     *                  The default value of QString() will result in a
     *                  list of all ContainmentActions.
     * @return list of ContainmentActions
     * @since 5.77
     **/
    QVector<KPluginMetaData> listContainmentActionsMetaData(const QString &parentApp);

    /**
     * Return the active plugin loader
     **/
    static PluginLoader *self();

    PluginLoader();
    virtual ~PluginLoader();

private:
    PluginLoaderPrivate *const d;
};

}

Q_DECLARE_METATYPE(Plasma::PluginLoader *)

#endif
