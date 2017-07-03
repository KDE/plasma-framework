/******************************************************************************
*   Copyright 2007-2011 by Aaron Seigo <aseigo@kde.org>                       *
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

#ifndef PLASMA_PACKAGE_H
#define PLASMA_PACKAGE_H

#include <QtCore/QStringList>

#include <kplugininfo.h>

#include <plasma/plasma.h>
#include <plasma/plasma_export.h>

class KJob;

#ifndef PLASMA_NO_DEPRECATED

namespace KPackage {
    class Package;
}

namespace Plasma
{

/**
 * @class Package plasma/package.h <Plasma/Package>
 *
 * @short object representing an installed Plasma package
 *
 * @deprecated USe KPackage::Package instead
 * Package defines what is in a package and provides easy access to the contents.
 *
 * To define a package, one might write the following code:
 *
 @code
    Package package;

    package.addDirectoryDefinition("images", "pics/", i18n("Images"));
    QStringList mimeTypes;
    mimeTypes << "image/svg" << "image/png" << "image/jpeg";
    package.setMimeTypes("images", mimeTypes);

    package.addDirectoryDefinition("scripts", "code/", i18n("Executable Scripts"));
    mimeTypes.clear();
    mimeTypes << "text/\*";
    package.setMimeTypes("scripts", mimeTypes);

    package.addFileDefinition("mainscript", "code/main.js", i18n("Main Script File"));
    package.setRequired("mainscript", true);
 @endcode
 * One may also choose to create a subclass of PackageStructure and include the setup
 * in the constructor.
 *
 * Either way, Package creates a self-documenting contract between the packager and
 * the application without exposing package internals such as actual on-disk structure
 * of the package or requiring that all contents be explicitly known ahead of time.
 *
 * Subclassing PackageStructure does have provide a number of potential const benefits:
 *    * the package can be notified of path changes via the virtual pathChanged() method
 *    * the subclass may implement mechanisms to install and remove packages using the
 *      virtual install and uninstall methods
 *    * subclasses can be compiled as plugins for easy re-use
 **/
//TODO: write documentation on USING a package

class PackagePrivate;
class PackageStructure;

class PLASMA_EXPORT Package
{
public:
    /**
     * Default constructor
     *
     * @param structure if a NULL pointer is passed in, this will creates an empty (invalid) Package;
     * otherwise the structure is allowed to set up the Package's initial layout
     * @since 4.6
     */
    PLASMA_DEPRECATED explicit Package(PackageStructure *structure = 0);

    /**
     * Copy constructore
     * @since 4.6
     */
    PLASMA_DEPRECATED Package(const KPackage::Package &other);

    /**
     * Copy constructore
     * @since 4.6
     */
    PLASMA_DEPRECATED Package(const Package &other);

    ~Package();

    /**
     * Assignment operator
     * @since 4.6
     */
    PLASMA_DEPRECATED Package &operator=(const Package &rhs);

    /**
     * @return true if this package has a valid PackageStructure associatedw it with it.
     * A package may not be valid, but have a valid structure. Useful when dealing with
     * Package objects in a semi-initialized state (e.g. before calling setPath())
     * @since 5.1
     */
    PLASMA_DEPRECATED bool hasValidStructure() const;

    /**
     * @return true if all the required components exist
     **/
    PLASMA_DEPRECATED bool isValid() const;

    /**
     * Sets the path to the root of this package
     * @param path an absolute path, or a relative path to the default package root
     * @since 4.3
     */
    PLASMA_DEPRECATED void setPath(const QString &path);

    /**
     * @return the path to the root of this particular package
     */
    PLASMA_DEPRECATED const QString path() const;

    /**
     * Get the path to a given file based on the key and an optional filename.
     * Example: finding the main script in a scripting package:
     *      filePath("mainscript")
     *
     * Example: finding a specific image in the images directory:
     *      filePath("images", "myimage.png")
     *
     * @param key the key of the file type to look for,
     * @param filename optional name of the file to locate within the package
     * @return path to the file on disk. QString() if not found.
     **/
    PLASMA_DEPRECATED QString filePath(const char *key, const QString &filename = QString()) const;

    /**
     * Get the list of files of a given type.
     *
     * @param fileType the type of file to look for, as defined in the
     *               package structure.
     * @return list of files by name, suitable for passing to filePath
     **/
    PLASMA_DEPRECATED QStringList entryList(const char *key) const;

    /**
     * @return user visible name for the given entry
     **/
    PLASMA_DEPRECATED QString name(const char *key) const;

    /**
     * @return true if the item at path exists and is required
     **/
    PLASMA_DEPRECATED bool isRequired(const char *key) const;

    /**
     * @return the mimeTypes associated with the path, if any
     **/
    PLASMA_DEPRECATED QStringList mimeTypes(const char *key) const;

    /**
     * @return the prefix paths inserted between the base path and content entries, in order of priority.
     *         When searching for a file, all paths will be tried in order.
     * @since 4.6
     */
    PLASMA_DEPRECATED QStringList contentsPrefixPaths() const;

    /**
     * @return preferred package root. This defaults to plasma/plasmoids/
     */
    PLASMA_DEPRECATED QString defaultPackageRoot() const;

    /**
     * @return service prefix used in desktop files. This defaults to plasma-applet-
     */
    PLASMA_DEPRECATED QString servicePrefix() const;

    /**
     * @return true if paths/symlinks outside the package itself should be followed.
     * By default this is set to false for security reasons.
     */
    PLASMA_DEPRECATED bool allowExternalPaths() const;

    /**
      * @return the package metadata object.
      */
    PLASMA_DEPRECATED KPluginInfo metadata() const;

    /**
     * @return a SHA1 hash digest of the contents of the package in hexadecimal form
     * @since 4.4
     */
    PLASMA_DEPRECATED QString contentsHash() const;

    /**
     * Adds a directory to the structure of the package. It is added as
     * a not-required element with no associated mimeTypes.
     *
     * Starting in 4.6, if an entry with the given key
     * already exists, the path is added to it as a search alternative.
     *
     * @param key  used as an internal label for this directory
     * @param path the path within the package for this directory
     * @param name the user visible (translated) name for the directory
     **/
    PLASMA_DEPRECATED void addDirectoryDefinition(const char *key, const QString &path, const QString &name);

    /**
     * Adds a file to the structure of the package. It is added as
     * a not-required element with no associated mimeTypes.
     *
     * Starting in 4.6, if an entry with the given key
     * already exists, the path is added to it as a search alternative.
     *
     * @param key  used as an internal label for this file
     * @param path the path within the package for this file
     * @param name the user visible (translated) name for the file
     **/
    PLASMA_DEPRECATED void addFileDefinition(const char *key, const QString &path, const QString &name);

    /**
     * Removes a definition from the structure of the package.
     * @since 4.6
     * @param key the internal label of the file or directory to remove
     */
    PLASMA_DEPRECATED void removeDefinition(const char *key);

    /**
     * Sets whether or not a given part of the structure is required or not.
     * The path must already have been added using addDirectoryDefinition
     * or addFileDefinition.
     *
     * @param key the entry within the package
     * @param required true if this entry is required, false if not
     */
    PLASMA_DEPRECATED void setRequired(const char *key, bool required);

    /**
     * Defines the default mimeTypes for any definitions that do not have
     * associated mimeTypes. Handy for packages with only one or predominantly
     * one file type.
     *
     * @param mimeTypes a list of mimeTypes
     **/
    PLASMA_DEPRECATED void setDefaultMimeTypes(QStringList mimeTypes);

    /**
     * Define mimeTypes for a given part of the structure
     * The path must already have been added using addDirectoryDefinition
     * or addFileDefinition.
     *
     * @param key the entry within the package
     * @param mimeTypes a list of mimeTypes
     **/
    PLASMA_DEPRECATED void setMimeTypes(const char *key, QStringList mimeTypes);

    /**
     * Sets the prefixes that all the contents in this package should
     * appear under. This defaults to "contents/" and is added automatically
     * between the base path and the entries as defined by the package
     * structure. Multiple entries can be added.
     * In this case each file request will be searched in all prefixes in order,
     * and the first found will be returned.
     *
     * @param prefix paths the directory prefix to use
     * @since 4.6
     */
    PLASMA_DEPRECATED void setContentsPrefixPaths(const QStringList &prefixPaths);

    /**
     * Sets service prefix.
     */
    PLASMA_DEPRECATED void setServicePrefix(const QString &servicePrefix);

    /**
     * Sets whether or not external paths/symlinks can be followed by a package
     * @param allow true if paths/symlinks outside of the package should be followed,
     *             false if they should be rejected.
     */
    PLASMA_DEPRECATED void setAllowExternalPaths(bool allow);

    /**
     * Sets preferred package root.
     */
    PLASMA_DEPRECATED void setDefaultPackageRoot(const QString &packageRoot);

    /**
     * Sets the fallback package root path
     * If a file won't be found in this package, it will search it in the package
     * with the same structure identified by path
     * It is intended to be used by the packageStructure
     * @param path package root path @see setPath
     */
    PLASMA_DEPRECATED void setFallbackPackage(const Plasma::Package &package);

    /**
     * @return The fallback package root path
     */
    PLASMA_DEPRECATED Plasma::Package fallbackPackage() const;

    // Content structure description methods
    /**
     * @return all directories registered as part of this Package's structure
     */
    PLASMA_DEPRECATED QList<const char *> directories() const;

    /**
     * @return all directories registered as part of this Package's required structure
     */
    PLASMA_DEPRECATED QList<const char *> requiredDirectories() const;

    /**
     * @return all files registered as part of this Package's structure
     */
    PLASMA_DEPRECATED QList<const char *> files() const;

    /**
     * @return all files registered as part of this Package's required structure
     */
    PLASMA_DEPRECATED QList<const char *> requiredFiles() const;

    /**
     * Installs a package matching this package structure. By default installs a
     * native Plasma::Package.
     *
     * @return KJob to track installation progress and result
     **/
    PLASMA_DEPRECATED KJob *install(const QString &sourcePackage, const QString &packageRoot = QString());

    /**
     * Uninstalls a package matching this package structure.
     *
     * @return KJob to track removal progress and result
     */
    PLASMA_DEPRECATED KJob *uninstall(const QString &packageName, const QString &packageRoot);

    /**
     * @returns the wrapped KPackage::Package instance, which deprecated this class
     *
     * @since 5.28
     */
    KPackage::Package kPackage() const;

private:
    QExplicitlySharedDataPointer<PackagePrivate> d;
    friend class PackagePrivate;
    friend class PackageStructure;
    friend class AppletPrivate;
    friend class Applet;
    friend class Corona;
};

}
#endif
Q_DECLARE_METATYPE(Plasma::Package)
#endif

