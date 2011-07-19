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

namespace Plasma
{

/**
 * @class Package plasma/package.h <Plasma/Package>
 *
 * @short object representing an installed Plasma package
 *
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
 *      virtual installPackage and uninstallPackage methods
 *    * subclasses can be compiled as plugins for easy re-use
 **/
//TODO: write documentation on USING a package

class PackagePrivate;
class PackageStructure;

class PLASMA_EXPORT Package
{
public:
    /**
     * Loads a package format by name.
     *
     * @param format If not empty, attempts to locate the given format, either
     *             from built-ins or via plugins.
     * @param specialization used to find script extensions for the given format, e.g. "QML" for "Plasma/Applet"
     * @return a package that matches the format, if available, or an invalid package if not
     */
    static Package load(const QString &format, const QString &specialization = QString());

    /**
     * Default constructor
     *
     * @arg structure if a NULL pointer is passed in, this will creates an empty (invalid) Package;
     * otherwise the structure is allowed to set up the Package's initial layout
     * @since 4.6
     */
    explicit Package(PackageStructure *structure = 0);

    /**
     * Copy constructore
     * @since 4.6
     */
    Package(const Package &other);

    ~Package();

    /**
     * Assignment operator
     * @since 4.6
     */
    Package &operator=(const Package &rhs);

    /**
     * @return true if all the required components exist
     **/
    bool isValid() const;

    /**
     * Sets the path to the root of this package
     * @param path and absolute path
     * @since 4.3
     */
    void setPath(const QString &path);

    /**
     * @return the path to the root of this particular package
     */
    const QString path() const;

    /**
     * Get the path to a given file based on the key and.an optional filename.
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
    QString filePath(const char *key, const QString &filename = QString()) const;

    /**
     * Get the list of files of a given type.
     *
     * @param fileType the type of file to look for, as defined in the
     *               package structure.
     * @return list of files by name, suitable for passing to filePath
     **/
    QStringList entryList(const char *key) const;

    /**
     * @return user visible name for the given entry
     **/
    QString name(const char *key) const;

    /**
     * @return true if the item at path exists and is required
     **/
    bool isRequired(const char *key) const;

    /**
     * @return the mimeTypes associated with the path, if any
     **/
    QStringList mimeTypes(const char *key) const;

    /**
     * @return the prefix paths inserted between the base path and content entries, in order of priority.
     *         When searching for a file, all paths will be tried in order.
     * @since 4.6
     */
    QStringList contentsPrefixPaths() const;

    /**
     * @return preferred package root. This defaults to plasma/plasmoids/
     */
    QString defaultPackageRoot() const;

    /**
     * @return service prefix used in desktop files. This defaults to plasma-applet-
     */
    QString servicePrefix() const;

    /**
     * @return true if paths/symlinks outside the package itself should be followed.
     * By default this is set to false for security reasons.
     */
    bool allowExternalPaths() const;

    /**
      * @return the package metadata object.
      */
    KPluginInfo metadata() const;

    /**
     * @return a SHA1 hash digest of the contents of the package in hexadecimal form
     * @since 4.4
     */
    QString contentsHash() const;

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
    void addDirectoryDefinition(const char *key, const QString &path, const QString &name);

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
    void addFileDefinition(const char *key, const QString &path, const QString &name);

    /**
     * Removes a definition from the structure of the package.
     * @since 4.6
     * @param key the internal label of the file or directory to remove
     */
    void removeDefinition(const char *key);

    /**
     * Sets whether or not a given part of the structure is required or not.
     * The path must already have been added using addDirectoryDefinition
     * or addFileDefinition.
     *
     * @param key the entry within the package
     * @param required true if this entry is required, false if not
     */
    void setRequired(const char *key, bool required);

    /**
     * Defines the default mimeTypes for any definitions that do not have
     * associated mimeTypes. Handy for packages with only one or predominantly
     * one file type.
     *
     * @param mimeTypes a list of mimeTypes
     **/
    void setDefaultMimeTypes(QStringList mimeTypes);

    /**
     * Define mimeTypes for a given part of the structure
     * The path must already have been added using addDirectoryDefinition
     * or addFileDefinition.
     *
     * @param key the entry within the package
     * @param mimeTypes a list of mimeTypes
     **/
    void setMimeTypes(const char *key, QStringList mimeTypes);

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
    void setContentsPrefixPaths(const QStringList &prefixPaths);

    /**
     * Sets service prefix.
     */
    void setServicePrefix(const QString &servicePrefix);

    /**
     * Sets whether or not external paths/symlinks can be followed by a package
     * @param allow true if paths/symlinks outside of the package should be followed,
     *             false if they should be rejected.
     */
    void setAllowExternalPaths(bool allow);

    /**
     * Sets preferred package root.
     */
    void setDefaultPackageRoot(const QString &packageRoot);

    /**
     * Called whenever the path changes so that subclasses may take
     * package specific actions.
     */
    void pathChanged();

    // Content structure description methods
    /**
     * @return all directories registered as part of this Package's structure
     */
    QList<const char*> directories() const;

    /**
     * @return all directories registered as part of this Package's required structure
     */
    QList<const char*> requiredDirectories() const;

    /**
     * @return all files registered as part of this Package's structure
     */
    QList<const char*> files() const;

    /**
     * @return all files registered as part of this Package's required structure
     */
    QList<const char*> requiredFiles() const;

    /**
     * Installs a package matching this package structure. By default installs a
     * native Plasma::Package.
     *
     * @param archivePath path to the package archive file
     * @param packageRoot path to the directory where the package should be
     *                    installed to
     * @return true on successful installation, false otherwise
     **/
    bool installPackage(const QString &archivePath, const QString &packageRoot);

    /**
     * Uninstalls a package matching this package structure.
     *
     * @param packageName the name of the package to remove
     * @param packageRoot path to the directory where the package should be installed to
     * @return true on successful removal of the package, false otherwise
     */
    bool uninstallPackage(const QString &packageName, const QString &packageRoot);

private:
    PackagePrivate * const d;
};

}

Q_DECLARE_METATYPE(Plasma::Package)
#endif

