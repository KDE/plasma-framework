/*
    SPDX-FileCopyrightText: 2011 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_PACKAGESTRUCTURE_H
#define PLASMA_PACKAGESTRUCTURE_H

#include <QStringList>

#include <plasma/package.h>
#include <plasma/plasma.h>
#include <plasma/plasma_export.h>

#if PLASMA_ENABLE_DEPRECATED_SINCE(5, 6)

namespace Plasma
{
class PackageStructurePrivate;

class PLASMA_EXPORT PackageStructure : public QObject
{
    Q_OBJECT

public:
    PLASMA_DEPRECATED_VERSION(5, 6, "Use KPackage API")
    explicit PackageStructure(QObject *parent = nullptr, const QVariantList &args = QVariantList());

    ~PackageStructure() override;

    /**
     * Called when a the PackageStructure should initialize a Package with the initial
     * structure. This allows setting paths before setPath is called.
     *
     * Note: one special value is "metadata" which can be set to the location of KPluginInfo
     * compatible .desktop file within the package. If not defined, it is assumed that this file
     * exists under the top level directory of the package.
     *
     * @param package the Package to set up. The object is empty of all definition when
     *        first passed in.
     */
    virtual void initPackage(Package *package);

    /**
     * Called whenever the path changes so that subclasses may take
     * package specific actions.
     */
    virtual void pathChanged(Package *package);

    /**
     * Installs a package matching this package structure. By default installs a
     * native Plasma::Package.
     *
     * @param package the instance of Package that is being used for the install; useful for
     * accessing file paths
     * @param archivePath path to the package archive file
     * @param packageRoot path to the directory where the package should be
     *                    installed to
     * @return KJob* to track the installation status
     **/
    virtual KJob *install(Package *package, const QString &archivePath, const QString &packageRoot);

    /**
     * Uninstalls a package matching this package structure.
     *
     * @param package the instance of Package that is being used for the install; useful for
     * accessing file paths
     * @param packageName the name of the package to remove
     * @param packageRoot path to the directory where the package should be installed to
     * @return KJob* to track the installation status
     */
    virtual KJob *uninstall(Package *package, const QString &packageRoot);

private:
    PackageStructurePrivate *d;
    friend class Package;
    friend class PluginLoader;
    friend class PackageStructurePrivate;

    Q_PRIVATE_SLOT(d, void installPathChanged(const QString &path))
};

} // Plasma namespace

/**
 * Register a Package class when it is contained in a loadable module
 */
/* clang-format off */
#define K_EXPORT_PLASMA_PACKAGE(libname, classname) \
    K_PLUGIN_FACTORY(factory, registerPlugin<classname>();)

#define K_EXPORT_PLASMA_PACKAGE_WITH_JSON(classname, jsonFile) \
    K_PLUGIN_FACTORY_WITH_JSON(factory, jsonFile, registerPlugin<classname>();)
/* clang-format on */

#endif // PLASMA_ENABLE_DEPRECATED_SINCE(5, 6)

#endif
