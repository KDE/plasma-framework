#include "currentcontainmentactionsmodel_p.h"
#include "containmentconfigview.h"

#include <kdeclarative/configpropertymap.h>

#include <QDebug>
#include <QDir>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlComponent>

#include <KLocalizedString>

#include <Plasma/Corona>
#include <Plasma/ContainmentActions>
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
    ConfigModel *doContainmentActionConfigModel();
    QStandardItemModel *doCurrentContainmentActionsModel();


    ContainmentConfigView *q;
    Plasma::Containment *containment;
    ConfigModel *wallpaperConfigModel;
    QString currentWallpaper;
    ConfigModel *containmentActionConfigModel;
    CurrentContainmentActionsModel *currentContainmentActionsModel;
    ConfigPropertyMap *currentWallpaperConfig;
    ConfigPropertyMap *ownWallpaperConfig;
};

ContainmentConfigViewPrivate::ContainmentConfigViewPrivate(Plasma::Containment *cont, ContainmentConfigView *view)
    : q(view),
      containment(cont),
      wallpaperConfigModel(0),
      containmentActionConfigModel(0),
      currentContainmentActionsModel(0),
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

ConfigModel *ContainmentConfigViewPrivate::doContainmentActionConfigModel()
{
    if (!containmentActionConfigModel) {
        containmentActionConfigModel = new ConfigModel(q);

        KPluginInfo::List actions = Plasma::PluginLoader::self()->listContainmentActionsInfo(QString());

        Plasma::Package pkg = Plasma::PluginLoader::self()->loadPackage("Plasma/Generic");

        foreach (const KPluginInfo &info, actions) {
            pkg.setDefaultPackageRoot(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "plasma/containmentactions", QStandardPaths::LocateDirectory));
            ConfigCategory *cat = new ConfigCategory(containmentActionConfigModel);
            cat->setName(info.name());
            cat->setIcon(info.icon());
            cat->setSource(pkg.filePath("ui", "config.qml"));
            cat->setPluginName(info.pluginName());
            containmentActionConfigModel->appendCategory(cat);
        }

    }
    return containmentActionConfigModel;
}

QStandardItemModel *ContainmentConfigViewPrivate::doCurrentContainmentActionsModel()
{
    if (!currentContainmentActionsModel) {
        currentContainmentActionsModel = new CurrentContainmentActionsModel(containment, q);
    }
    return currentContainmentActionsModel;
}



//////////////////////////////ContainmentConfigView
ContainmentConfigView::ContainmentConfigView(Plasma::Containment *cont, QWindow *parent)
    : ConfigView(cont, parent),
      d(new ContainmentConfigViewPrivate(cont, this))
{
    qmlRegisterType<QStandardItemModel>();
}

ContainmentConfigView::~ContainmentConfigView()
{
    delete d;
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

ConfigModel *ContainmentConfigView::containmentActionConfigModel()
{
    return d->doContainmentActionConfigModel();
}

QStandardItemModel *ContainmentConfigView::currentContainmentActionsModel()
{
    return d->doCurrentContainmentActionsModel();
}

#include "moc_containmentconfigview.cpp"
