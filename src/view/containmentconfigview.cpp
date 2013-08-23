/*
 *   Copyright 2013 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "containmentconfigview.h"
#include <Plasma/Containment>
//#include "plasmoid/wallpaperinterface.h"
#include <kdeclarative/configpropertymap.h>

#include <QDebug>
#include <QDir>
#include <QQmlContext>
#include <QQmlEngine>


#include <KLocalizedString>

#include <Plasma/Corona>
#include <Plasma/PluginLoader>

class ContainmentConfigViewPrivate
{
public:
    ContainmentConfigViewPrivate(Plasma::Containment *cont, ContainmentConfigView *view);
    ~ContainmentConfigViewPrivate();

    void syncWallpaperObjects();
    void applyWallpaper();
    ConfigModel *doWallpaperConfigModel();
    void setCurrentWallpaper(const QString &wallpaper);

    ContainmentConfigView *q;
    Plasma::Containment *containment;
    ConfigModel *wallpaperConfigModel;
    QString currentWallpaper;
    ConfigPropertyMap *currentWallpaperConfig;
    ConfigPropertyMap *ownWallpaperConfig;
};

ContainmentConfigViewPrivate::ContainmentConfigViewPrivate(Plasma::Containment *cont, ContainmentConfigView *view)
    : q(view),
      containment(cont),
      wallpaperConfigModel(0),
      currentWallpaperConfig(0),
      ownWallpaperConfig(0)
{
    q->engine()->rootContext()->setContextProperty("configDialog", q);
    setCurrentWallpaper(cont->containment()->wallpaper());

    Plasma::Package pkg = Plasma::PluginLoader::self()->loadPackage("Plasma/Generic");
    pkg.setDefaultPackageRoot("plasma/wallpapers");
    pkg.setPath(containment->wallpaper());
    QFile file(pkg.filePath("config", "main.xml"));
    KConfigGroup cfg = containment->config();
    cfg = KConfigGroup(&cfg, "Wallpaper");

    syncWallpaperObjects();
}

ContainmentConfigViewPrivate::~ContainmentConfigViewPrivate()
{
}

void ContainmentConfigViewPrivate::syncWallpaperObjects()
{
    QObject *wallpaperGraphicsObject = containment->property("wallpaperGraphicsObject").value<QObject *>();
    q->engine()->rootContext()->setContextProperty("wallpaper", wallpaperGraphicsObject);

    //FIXME: why m_wallpaperGraphicsObject->property("configuration").value<ConfigPropertyMap *>() doesn't work?
    currentWallpaperConfig = static_cast<ConfigPropertyMap *>(wallpaperGraphicsObject->property("configuration").value<QObject *>());
}

void ContainmentConfigViewPrivate::applyWallpaper()
{
   containment->setWallpaper(currentWallpaper);

    delete ownWallpaperConfig;
    ownWallpaperConfig = 0;

    syncWallpaperObjects();
    emit q->wallpaperConfigurationChanged();
}

ConfigModel *ContainmentConfigViewPrivate::doWallpaperConfigModel()
{
    if (!wallpaperConfigModel) {
        wallpaperConfigModel = new ConfigModel(q);
        QStringList dirs(QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, "plasma/wallpapers", QStandardPaths::LocateDirectory));
        Plasma::Package pkg = Plasma::PluginLoader::self()->loadPackage("Plasma/Generic");
        foreach (const QString &dirPath, dirs) {
            QDir dir(dirPath);
            pkg.setDefaultPackageRoot(dirPath);
            QStringList packages;

            foreach (const QString &sdir, dir.entryList(QDir::AllDirs | QDir::Readable)) {
                QString metadata = dirPath + '/' + sdir + "/metadata.desktop";
                if (QFile::exists(metadata)) {
                    packages << sdir;
                }
            }

            foreach (const QString &package, packages) {
                pkg.setPath(package);
                if (!pkg.isValid()) {
                    continue;
                }
                ConfigCategory *cat = new ConfigCategory(wallpaperConfigModel);
                cat->setName(pkg.metadata().name());
                cat->setIcon(pkg.metadata().icon());
                cat->setSource(pkg.filePath("ui", "config.qml"));
                cat->setPluginName(package);
                wallpaperConfigModel->appendCategory(cat);
            }
        }
    }
    return wallpaperConfigModel;
}

void ContainmentConfigViewPrivate::setCurrentWallpaper(const QString &wallpaper)
{
    if (currentWallpaper == wallpaper) {
        return;
    }

    delete ownWallpaperConfig;
    ownWallpaperConfig = 0;

    if (containment->wallpaper() == wallpaper) {
        syncWallpaperObjects();
    } else {
        //we have to construct an independent ConfigPropertyMap when we want to configure wallpapers that are not the current one
        Plasma::Package pkg = Plasma::PluginLoader::self()->loadPackage("Plasma/Generic");
        pkg.setDefaultPackageRoot("plasma/wallpapers");
        pkg.setPath(wallpaper);
        QFile file(pkg.filePath("config", "main.xml"));
        KConfigGroup cfg = containment->config();
        cfg = KConfigGroup(&cfg, "Wallpaper");
        currentWallpaperConfig = ownWallpaperConfig = new ConfigPropertyMap(new Plasma::ConfigLoader(&cfg, &file), q);
    }

    currentWallpaper = wallpaper;
    emit q->currentWallpaperChanged();
    emit q->wallpaperConfigurationChanged();
}

//////////////////////////////ContainmentConfigView
ContainmentConfigView::ContainmentConfigView(Plasma::Containment *cont, QWindow *parent)
    : ConfigView(cont, parent),
      d(new ContainmentConfigViewPrivate(cont, this))
{
}

ContainmentConfigView::~ContainmentConfigView()
{
}

void ContainmentConfigView::init()
{
    setSource(QUrl::fromLocalFile(d->containment->containment()->corona()->package().filePath("containmentconfigurationui")));
}

ConfigModel *ContainmentConfigView::wallpaperConfigModel()
{
    return d->doWallpaperConfigModel();
}

ConfigPropertyMap *ContainmentConfigView::wallpaperConfiguration() const
{
    return d->currentWallpaperConfig;
}

QString ContainmentConfigView::currentWallpaper() const
{
    return d->currentWallpaper;
}

void ContainmentConfigView::setCurrentWallpaper(const QString &wallpaper)
{
    d->setCurrentWallpaper(wallpaper);
}

void ContainmentConfigView::applyWallpaper()
{
    d->applyWallpaper();
}

void ContainmentConfigView::syncWallpaperObjects()
{
    d->syncWallpaperObjects();
}

#include "moc_containmentconfigview.cpp"
