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

#include <kconfiggroup.h>
#include <kdesktopfile.h>

namespace Plasma
{

class PackageMetadataPrivate
{
    public:
        PackageMetadataPrivate()
            : type("Service")
        {
        }

        QString name;
        QString icon;
        QString description;
        QStringList keywords;
        QString author;
        QString email;
        QString version;
        QString website;
        QString license;
        QString app;
        QString category;
        QString requiredVersion;
        QString pluginName;
        QString type;
        QString serviceType;
        QString api;
        KUrl location;
};

PackageMetadata::PackageMetadata(const PackageMetadata &other)
    : d(new PackageMetadataPrivate(*other.d))
{
}

PackageMetadata &PackageMetadata::operator=(const PackageMetadata &other)
{
    *d = *other.d;
    return *this;
}

PackageMetadata::PackageMetadata(const QString &path)
    : d(new PackageMetadataPrivate)
{
    read(path);
}

PackageMetadata::~PackageMetadata()
{
    delete d;
}

bool PackageMetadata::isValid() const
{
    return ! (d->name.isEmpty() ||
              d->author.isEmpty() ||
              d->license.isEmpty() ||
              d->type.isEmpty());
}

void PackageMetadata::write(const QString &filename) const
{
    KDesktopFile cfg(filename);
    KConfigGroup config = cfg.desktopGroup();
    config.writeEntry("Encoding", "UTF-8");

    config.writeEntry("Name", d->name);
    config.writeEntry("Comment", d->description);
    config.writeEntry("Keywords", d->keywords);
    config.writeEntry("X-KDE-ServiceTypes", d->serviceType);
    config.writeEntry("X-KDE-PluginInfo-Name", d->pluginName);
    config.writeEntry("X-KDE-PluginInfo-Author", d->author);
    config.writeEntry("X-KDE-PluginInfo-Email", d->email);
    config.writeEntry("X-KDE-PluginInfo-Version", d->version);
    config.writeEntry("X-KDE-PluginInfo-Website", d->website);
    config.writeEntry("X-KDE-PluginInfo-License", d->license);
    config.writeEntry("X-KDE-PluginInfo-Category", d->category);
    config.writeEntry("X-Plasma-API", d->api);
    config.writeEntry("X-KDE-ParentApp", d->app);
    config.writeEntry("Type", d->type);
    config.writeEntry("X-Plasma-RemoteLocation", d->location);
}

void PackageMetadata::read(const QString &filename)
{
    if (filename.isEmpty()) {
        return;
    }

    KDesktopFile cfg(filename);
    KConfigGroup config = cfg.desktopGroup();

    d->name = config.readEntry("Name", d->name);
    d->icon = config.readEntry("Icon", d->name);
    d->description = config.readEntry("Comment", d->description);
    d->keywords = config.readEntry("Keywords", d->keywords);
    d->serviceType = config.readEntry("X-KDE-ServiceTypes", d->serviceType);
    d->pluginName = config.readEntry("X-KDE-PluginInfo-Name", d->pluginName);
    d->author = config.readEntry("X-KDE-PluginInfo-Author", d->author);
    d->email = config.readEntry("X-KDE-PluginInfo-Email", d->email);
    d->version = config.readEntry("X-KDE-PluginInfo-Version", d->version);
    d->website = config.readEntry("X-KDE-PluginInfo-Website", d->website);
    d->license = config.readEntry("X-KDE-PluginInfo-License", d->license);
    d->category = config.readEntry("X-KDE-PluginInfo-Category", d->category);
    d->api = config.readEntry("X-Plasma-API", d->api);
    d->app = config.readEntry("X-KDE-ParentApp", d->app);
    d->type = config.readEntry("Type", d->type);
    d->location = config.readEntry("X-Plasma-RemoteLocation", d->location);
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

QString PackageMetadata::icon() const
{
    return d->icon;
}

void PackageMetadata::setIcon(const QString &icon)
{
    d->icon = icon;
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

QString PackageMetadata::application() const
{
    return d->app;
}

QString PackageMetadata::category() const
{
    return d->category;
}

void PackageMetadata::setKeywords(const QStringList &keywords)
{
    d->keywords = keywords;
}

QStringList PackageMetadata::keywords() const
{
    return d->keywords;
}

QString PackageMetadata::requiredVersion() const
{
    return d->requiredVersion;
}

KUrl PackageMetadata::remoteLocation() const
{
    return d->location;
}

QString PackageMetadata::type() const
{
    return d->type;
}

QString PackageMetadata::implementationApi() const
{
    return d->api;
}

void PackageMetadata::setImplementationApi(const QString &api)
{
    d->api = api;
}

QString PackageMetadata::pluginName() const
{
    return d->pluginName;
}

void PackageMetadata::setPluginName(const QString &pluginName)
{
    d->pluginName = pluginName;
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

void PackageMetadata::setApplication(const QString &application)
{
    d->app = application;
}

void PackageMetadata::setCategory(const QString &category)
{
    d->category = category;
}

void PackageMetadata::setRequiredVersion(const QString &requiredVersion)
{
    d->requiredVersion = requiredVersion;
}

void PackageMetadata::setRemoteLocation(const KUrl &location)
{
    d->location = location;
}

void PackageMetadata::setType(const QString &type)
{
    d->type = type;
}

} // namespace Plasma

