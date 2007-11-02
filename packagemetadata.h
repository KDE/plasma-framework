/******************************************************************************
*   Copyright 2007 by Riccardo Iaconelli  <riccardo@kde.org>                  *
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

#ifndef PACKAGEMETADATA_H
#define PACKAGEMETADATA_H

#include <QtCore/QString>

#include <plasma/plasma_export.h>

namespace Plasma
{

class PLASMA_EXPORT PackageMetadata
{
public:
    /**
     * Default constructor
     **/
    PackageMetadata();

    /**
     * Constructs a metadata object using the values in the file at path
     *
     * @param path path to a metadata.desktop file
     **/
    PackageMetadata(const QString& path);
    ~PackageMetadata();

    bool isComplete() const;

    /**
     * Writes out the metadata to filename, which should be a .desktop
     * file. It writes out the information in a format that is compatible
     * with KPluginInfo
     * @see KPluginInfo
     *
     * @arg filename path to the file to write to
     **/
    void write(const QString& filename) const;

    /**
     * Reads in metadata from a file, which should be a .desktop
     * file. It writes out the information in a format that is compatible
     * with KPluginInfo
     * @see KPluginInfo
     *
     * @arg filename path to the file to write to
     **/
    void read(const QString& filename);

    QString name() const;
    QString description() const;
    QString serviceType() const;
    QString author() const;
    QString email() const;
    QString version() const;
    QString website() const;
    QString license() const;
    QString mainFile() const;
    QString application() const;
    QString requiredVersion() const;
    QString releaseNotes() const;
    QString filenameToSave() const;
    // filename
    QString icon() const;

    /**
     * @return the name of the screenshot file, screenshot.png by default
     */
    QString screenshot() const;

    /**
     * Path to a PNG file containing a preview image.
     * This might be a screenshot, for instance.
     *
     * @return path to a local image file, or QString() if no
     *         preview is available
     **/
    QString preview() const;

    QString type() const;

    void setName(const QString &);
    void setDescription(const QString &);
    void setServiceType(const QString &);
    void setAuthor(const QString &);
    void setEmail(const QString &);
    void setVersion(const QString &);
    void setWebsite(const QString &);
    void setLicense(const QString &);
    void setMainFile(const QString &);
    void setApplication(const QString &);
    void setRequiredVersion(const QString &);
    void setReleaseNotes(const QString &);
    void setFilenameToSave(const QString &);
    // filename
    void setIcon(const QString &);

    /**
     * Sets the name of the screenshot file.
     *
     * @param file the name of the file
     */
    void setScreenshot(const QString &file);

    /**
     * Sets the path for the file containing a preview image.
     * This might be a screenshot, for instance.
     * A PNG is expected.
     *
     * @args path path to the preview image file
     **/
    void setPreview(const QString & path);

    void setType(const QString& type);

private:
    class Private;
    Private * const d;
};

}
#endif
