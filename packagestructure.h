/******************************************************************************
*   Copyright 2011 by Aaron Seigo <aseigo@kde.org>                            *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#ifndef PLASMA_PACKAGESTRUCTURE_H
#define PLASMA_PACKAGESTRUCTURE_H

#include <QtCore/QStringList>

#include <kplugininfo.h>

#include <plasma/plasma.h>
#include <plasma/plasma_export.h>
#include <plasma/package.h>

namespace Plasma
{

class PackageStructurePrivate;

class PLASMA_EXPORT PackageStructure : public QObject
{
    Q_OBJECT

public:
    explicit PackageStructure(QObject *parent = 0, const QVariantList &args = QVariantList());

    ~PackageStructure();

    /**
     * Called when a the PackageStructure should initialize a Package with the initial
     * structure. This allows setting paths before setPath is called.
     *
     * Note: one special value is "metadata" which can be set to the location of KPluginInfo
     * compatible .desktop file within the package. If not defined, it is assumed that this file
     * exists under the top level directory of the package.
     *
     * @arg package the Package to set up. The object is empty of all definition when
     *      first passed in.
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
     * @return true on successful installation, false otherwise
     **/
    virtual bool installPackage(Package *package, const QString &archivePath, const QString &packageRoot);

    /**
     * Uninstalls a package matching this package structure.
     *
     * @param package the instance of Package that is being used for the install; useful for
     * accessing file paths
     * @param packageName the name of the package to remove
     * @param packageRoot path to the directory where the package should be installed to
     * @return true on successful removal of the package, false otherwise
     */
    virtual bool uninstallPackage(Package *package, const QString &packageName, const QString &packageRoot);

private:
    PackageStructurePrivate *const d;
};

} // Plasma namespace

/**
 * Register a Package class when it is contained in a loadable module
 */
#define K_EXPORT_PLASMA_PACKAGE(libname, classname) \
K_PLUGIN_FACTORY(factory, registerPlugin<classname>();) \
K_EXPORT_PLUGIN(factory("plasma_packagestructure_" #libname)) \
K_EXPORT_PLUGIN_VERSION(PLASMA_VERSION)

#endif

