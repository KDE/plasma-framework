/******************************************************************************
*   Copyright 2007 by Riccardo Iaconelli  <riccardo@kde.org>                *
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

#include <packagemetadata.h>

#include <QDir>

#include <KConfig>
#include <KConfigGroup>

namespace Plasma
{

class PackageMetadata::Private
{
    public:
        Private()
            : screenshot("screenshot.png")
        {}

        QString name;
        QString description;
        QString author;
        QString email;
        QString version;
        QString website;
        QString license;
        QString mainFile;
        QString app;
        QString requiredVersion;
        QString releaseNotes;
        QString icon;
        QString screenshot;
        QString preview;
        QString type;
        QString serviceType;
};

PackageMetadata::PackageMetadata()
    : d(new Private)
{
}

PackageMetadata::PackageMetadata(const QString& path)
    : d(new Private)
{
    read(path);
}

PackageMetadata::~PackageMetadata()
{
    delete d;
}

bool PackageMetadata::isComplete() const
{
    return ! (d->name.isEmpty() ||
              d->author.isEmpty() ||
              d->version.isEmpty() ||
              d->license.isEmpty() ||
              d->app.isEmpty() ||
              d->type.isEmpty());
}

void PackageMetadata::write(const QString& filename) const
{
    KConfig cfg(filename);
    KConfigGroup config(&cfg, "Desktop Entry");
    config.writeEntry("Encoding", "UTF-8");

    //TODO: this will be a problem for localized names?
    config.writeEntry("Name", d->name);
    config.writeEntry("Description", d->description);
    config.writeEntry("Icon", d->icon);
    config.writeEntry("X-KDE-Screenshot", d->screenshot);
    config.writeEntry("ServiceTypes", d->serviceType);
    config.writeEntry("X-KDE-PluginInfo-Name", d->name);
    config.writeEntry("X-KDE-PluginInfo-Author", d->author);
    config.writeEntry("X-KDE-PluginInfo-Email", d->email);
    config.writeEntry("X-KDE-PluginInfo-Version", d->version);
    config.writeEntry("X-KDE-PluginInfo-Website", d->website);
    config.writeEntry("X-KDE-PluginInfo-License", d->license);
    config.writeEntry("X-KDE-PluginInfo-Category", d->type);
    config.writeEntry("X-KDE-Plasmagik-MainFile", d->mainFile);
    config.writeEntry("X-KDE-Plasmagik-ApplicationName", d->app);
    config.writeEntry("X-KDE-Plasmagik-RequiredVersion", d->requiredVersion);
}

void PackageMetadata::read(const QString& filename)
{
    KConfig cfg(filename);
    KConfigGroup config(&cfg, "Desktop Entry");

    //TODO: this will be a problem for localized names?
    d->name = config.readEntry("X-KDE-PluginInfo-Name", d->name);
    d->description = config.readEntry("Description", d->description);
    d->icon = config.readEntry("Icon", d->icon);
    d->screenshot= config.readEntry("X-KDE-Screenshot", d->screenshot);
    d->serviceType = config.readEntry("ServiceTypes", d->serviceType);
    d->author = config.readEntry("X-KDE-PluginInfo-Author", d->author);
    d->email = config.readEntry("X-KDE-PluginInfo-Email", d->email);
    d->version = config.readEntry("X-KDE-PluginInfo-Version", d->version);
    d->website = config.readEntry("X-KDE-PluginInfo-Website", d->website);
    d->license = config.readEntry("X-KDE-PluginInfo-License", d->license);
    d->type = config.readEntry("X-KDE-PluginInfo-Category", d->type);
    d->mainFile = config.readEntry("X-KDE-Plasmagik-MainFile", d->mainFile);
    d->app = config.readEntry("X-KDE-Plasmagik-ApplicationName", d->app);
    d->requiredVersion = config.readEntry("X-KDE-Plasmagik-RequiredVersion", d->requiredVersion);
}

QString PackageMetadata::name() const
{
    return d->name;
}

QString PackageMetadata::description() const
{
    return d->description;
}

QString PackageMetadata::serviceType() const
{
    return d->serviceType;
}

QString PackageMetadata::author() const
{
    return d->author;
}

QString PackageMetadata::email() const
{
    return d->email;
}

QString PackageMetadata::version() const
{
    return d->version;
}

QString PackageMetadata::website() const
{
    return d->website;
}

QString PackageMetadata::license() const
{
    return d->license;
}

QString PackageMetadata::mainFile() const
{
    return d->mainFile;
}

QString PackageMetadata::application() const
{
    return d->app;
}

QString PackageMetadata::requiredVersion() const
{
    return d->requiredVersion;
}

QString PackageMetadata::releaseNotes() const
{
    return d->releaseNotes;
}

QString PackageMetadata::icon() const
{
    return d->icon;
}

QString PackageMetadata::screenshot() const
{
    return d->screenshot;
}

QString PackageMetadata::preview() const
{
    return d->preview;
}

QString PackageMetadata::type() const
{
    return d->type;
}

void PackageMetadata::setName(const QString &name)
{
    d->name = name;
}

void PackageMetadata::setDescription(const QString &description)
{
    d->description = description;
}

void PackageMetadata::setServiceType(const QString &serviceType)
{
    d->serviceType = serviceType;
}

void PackageMetadata::setAuthor(const QString &author)
{
    d->author = author;
}

void PackageMetadata::setEmail(const QString &email)
{
    d->email = email;
}

void PackageMetadata::setVersion(const QString &version)
{
    d->version = version;
}

void PackageMetadata::setWebsite(const QString &website)
{
    d->website = website;
}

void PackageMetadata::setLicense(const QString &license)
{
    d->license = license;
}

void PackageMetadata::setMainFile(const QString &mainFile)
{
    d->mainFile = mainFile;
}
void PackageMetadata::setApplication(const QString &application)
{
    d->app = application;
}

void PackageMetadata::setRequiredVersion(const QString &requiredVersion)
{
    d->requiredVersion = requiredVersion;
}

void PackageMetadata::setReleaseNotes(const QString &releaseNotes)
{
    d->releaseNotes = releaseNotes;
}

void PackageMetadata::setIcon(const QString &icon)
{
    d->icon = icon;
}

void PackageMetadata::setScreenshot(const QString & screenshot)
{
    d->screenshot = screenshot;
}

void PackageMetadata::setPreview(const QString& path)
{
    d->preview = path;
}

void PackageMetadata::setType(const QString& type)
{
    d->type = type;
}

} // namespace Plasma


