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

#include "private/configcategory_p.h"
#include "configview.h"
#include "configmodel.h"
#include "Plasma/Applet"
#include "Plasma/Containment"
//#include "plasmoid/wallpaperinterface.h"
#include "kdeclarative/configpropertymap.h"

#include <QDebug>
#include <QDir>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickItem>

#include <KAuthorized>
#include <klocalizedstring.h>
#include <kdeclarative/kdeclarative.h>
#include <packageurlinterceptor.h>
#include <KQuickAddons/ConfigModule>

#include <Plasma/Corona>
#include <Plasma/PluginLoader>

//Unfortunately QWINDOWSIZE_MAX is not exported
#define DIALOGSIZE_MAX ((1<<24)-1)

namespace PlasmaQuick
{

//////////////////////////////ConfigView

class ConfigViewPrivate
{
public:
    ConfigViewPrivate(Plasma::Applet *appl, ConfigView *view);
    ~ConfigViewPrivate();

    void init();

    void updateMinimumWidth();
    void updateMinimumHeight();
    void updateMaximumWidth();
    void updateMaximumHeight();
    void mainItemLoaded();

    ConfigView *q;
    QWeakPointer <Plasma::Applet> applet;
    ConfigModel *configModel;
    ConfigModel *kcmConfigModel;
    Plasma::Corona *corona;

    //Attached Layout property of mainItem, if any
    QWeakPointer <QObject> mainItemLayout;
};

ConfigViewPrivate::ConfigViewPrivate(Plasma::Applet *appl, ConfigView *view)
    : q(view),
      applet(appl),
      corona(0)
{
}

void ConfigViewPrivate::init()
{
    if (!applet) {
        qWarning() << "Null applet passed to constructor";
        return;
    }
    if (!applet.data()->pluginMetaData().isValid()) {
        qWarning() << "Invalid applet passed to constructor";
        return;
    }

    applet.data()->setUserConfiguring(true);

    KDeclarative::KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(q->engine());
    const QString rootPath = applet.data()->pluginMetaData().value(QStringLiteral("X-Plasma-RootPath"));
    if (!rootPath.isEmpty()) {
        kdeclarative.setTranslationDomain("plasma_applet_" + rootPath);
    } else {
        kdeclarative.setTranslationDomain("plasma_applet_" + applet.data()->pluginMetaData().pluginId());
    }
    kdeclarative.setupBindings();

    //FIXME: problem on nvidia, all windows should be transparent or won't show
    q->setColor(Qt::transparent);
    q->setTitle(i18n("%1 Settings", applet.data()->title()));

    //systray case
    if (!applet.data()->containment()->corona()) {
        Plasma::Applet *a = qobject_cast<Plasma::Applet *>(applet.data()->containment()->parent());
        if (a) {
            corona = a->containment()->corona();
        }
    } else if (!applet.data()->containment()->corona()->kPackage().isValid()) {
        qWarning() << "Invalid home screen package";
    } else {
        corona = applet.data()->containment()->corona();
    }
    if (!corona) {
        qWarning() << "Cannot find a Corona, this should never happen!";
        return;
    }

    const auto pkg = corona->kPackage();
    if (pkg.isValid()) {
        PackageUrlInterceptor *interceptor = new PackageUrlInterceptor(q->engine(), pkg);
        interceptor->addAllowedPath(applet.data()->kPackage().path());
        q->engine()->setUrlInterceptor(interceptor);
    }

    q->setResizeMode(QQuickView::SizeViewToRootObject);

    //config model local of the applet
    QQmlComponent *component = new QQmlComponent(q->engine(), QUrl::fromLocalFile(applet.data()->kPackage().filePath("configmodel")), q);
    QObject *object = component->beginCreate(q->engine()->rootContext());
    configModel = qobject_cast<ConfigModel *>(object);

    if (configModel) {
        configModel->setApplet(applet.data());
    } else {
        delete object;
    }

    QStringList kcms = KPluginMetaData::readStringList(applet.data()->pluginMetaData().rawData(), QStringLiteral("X-Plasma-ConfigPlugins"));

    // filter out non-authorized KCMs
    // KAuthorized expects KCMs with .desktop suffix, so we can't just pass everything
    // to KAuthorized::authorizeControlModules verbatim
    kcms.erase(std::remove_if(kcms.begin(), kcms.end(), [](const QString &kcm) {
        return !KAuthorized::authorizeControlModule(kcm + QLatin1String(".desktop"));
    }), kcms.end());

    if (!kcms.isEmpty()) {
        if (!configModel) {
            configModel = new ConfigModel(q);
        }

        foreach (const QString &kcm, kcms) {
            KPluginLoader loader(KPluginLoader::findPlugin(QLatin1String("kcms/") + kcm));
            KPluginMetaData md(loader.fileName());

            if (!md.isValid()) {
                qWarning() << "Could not find" << kcm << "specified in X-Plasma-ConfigPlugins";
                continue;
            }

            configModel->appendCategory(md.iconName(), md.name(), QString(), loader.fileName());
        }
    }

    q->engine()->rootContext()->setContextProperty(QStringLiteral("plasmoid"), applet.data()->property("_plasma_graphicObject").value<QObject *>());
    q->engine()->rootContext()->setContextProperty(QStringLiteral("configDialog"), q);
    component->completeCreate();
    delete component;
}

void ConfigViewPrivate::updateMinimumWidth()
{
    if (mainItemLayout) {
        q->setMinimumWidth(mainItemLayout.data()->property("minimumWidth").toInt());
        //Sometimes setMinimumWidth doesn't actually resize: Qt bug?

        q->setWidth(qMax(q->width(), q->minimumWidth()));
    } else {
        q->setMinimumWidth(-1);
    }
}

void ConfigViewPrivate::updateMinimumHeight()
{
    if (mainItemLayout) {
        q->setMinimumHeight(mainItemLayout.data()->property("minimumHeight").toInt());
        //Sometimes setMinimumHeight doesn't actually resize: Qt bug?

        q->setHeight(qMax(q->height(), q->minimumHeight()));
    } else {
        q->setMinimumHeight(-1);
    }
}

void ConfigViewPrivate::updateMaximumWidth()
{
    if (mainItemLayout) {
        const int hint = mainItemLayout.data()->property("maximumWidth").toInt();

        if (hint > 0) {
            q->setMaximumWidth(hint);
        } else {
            q->setMaximumWidth(DIALOGSIZE_MAX);
        }
    } else {
        q->setMaximumWidth(DIALOGSIZE_MAX);
    }
}

void ConfigViewPrivate::updateMaximumHeight()
{
    if (mainItemLayout) {
        const int hint = mainItemLayout.data()->property("maximumHeight").toInt();

        if (hint > 0) {
            q->setMaximumHeight(hint);
        } else {
            q->setMaximumHeight(DIALOGSIZE_MAX);
        }
    } else {
        q->setMaximumHeight(DIALOGSIZE_MAX);
    }
}

void ConfigViewPrivate::mainItemLoaded()
{
    if (applet) {
        KConfigGroup cg = applet.data()->config();
        cg = KConfigGroup(&cg, "ConfigDialog");
        q->resize(cg.readEntry("DialogWidth", q->width()), cg.readEntry("DialogHeight", q->height()));
    }

    //Extract the representation's Layout, if any
    QObject *layout = 0;

    //Search a child that has the needed Layout properties
    //HACK: here we are not type safe, but is the only way to access to a pointer of Layout
    foreach (QObject *child, q->rootObject()->children()) {
        //find for the needed property of Layout: minimum/maximum/preferred sizes and fillWidth/fillHeight
        if (child->property("minimumWidth").isValid() && child->property("minimumHeight").isValid() &&
                child->property("preferredWidth").isValid() && child->property("preferredHeight").isValid() &&
                child->property("maximumWidth").isValid() && child->property("maximumHeight").isValid() &&
                child->property("fillWidth").isValid() && child->property("fillHeight").isValid()
            ) {
            layout = child;
            break;
        }
    }
    mainItemLayout = layout;

    if (layout) {
        QObject::connect(layout, SIGNAL(minimumWidthChanged()), q, SLOT(updateMinimumWidth()));
        QObject::connect(layout, SIGNAL(minimumHeightChanged()), q, SLOT(updateMinimumHeight()));
        QObject::connect(layout, SIGNAL(maximumWidthChanged()), q, SLOT(updateMaximumWidth()));
        QObject::connect(layout, SIGNAL(maximumHeightChanged()), q, SLOT(updateMaximumHeight()));

        updateMinimumWidth();
        updateMinimumHeight();
        updateMaximumWidth();
        updateMaximumHeight();
    }
}


ConfigView::ConfigView(Plasma::Applet *applet, QWindow *parent)
    : QQuickView(parent),
      d(new ConfigViewPrivate(applet, this))
{
    setIcon(QIcon::fromTheme(QStringLiteral("configure")));
    qmlRegisterType<ConfigModel>("org.kde.plasma.configuration", 2, 0, "ConfigModel");
    qmlRegisterType<ConfigCategory>("org.kde.plasma.configuration", 2, 0, "ConfigCategory");
    d->init();
    connect(applet, &QObject::destroyed, this, &ConfigView::close);
    connect(this, &QQuickView::statusChanged, [=](QQuickView::Status status) {
        if (status == QQuickView::Ready) {
            d->mainItemLoaded();
        }
    });
}

ConfigView::~ConfigView()
{
    if (d->applet) {
        d->applet.data()->setUserConfiguring(false);
        if (d->applet.data()->containment() && d->applet.data()->containment()->corona()) {
            d->applet.data()->containment()->corona()->requestConfigSync();
        }
    }
}

void ConfigView::init()
{
    setSource(QUrl::fromLocalFile(d->corona->kPackage().filePath("appletconfigurationui")));
}

Plasma::Applet *ConfigView::applet()
{
    return d->applet.data();
}

ConfigModel *ConfigView::configModel() const
{
    return d->configModel;
}

QString ConfigView::appletGlobalShortcut() const
{
    if (!d->applet) {
        return QString();
    }

    return d->applet.data()->globalShortcut().toString();
}

void ConfigView::setAppletGlobalShortcut(const QString &shortcut)
{
    if (!d->applet || d->applet.data()->globalShortcut().toString().toLower() == shortcut.toLower()) {
        return;
    }

    d->applet.data()->setGlobalShortcut(shortcut);
    emit appletGlobalShortcutChanged();
}

//To emulate Qt::WA_DeleteOnClose that QWindow doesn't have
void ConfigView::hideEvent(QHideEvent *ev)
{
    QQuickWindow::hideEvent(ev);
    deleteLater();
}

void ConfigView::resizeEvent(QResizeEvent *re)
{
    if (!rootObject()) {
        return;
    }
    rootObject()->setSize(re->size());

    if (d->applet) {
        KConfigGroup cg = d->applet.data()->config();
        cg = KConfigGroup(&cg, "ConfigDialog");
        cg.writeEntry("DialogWidth", re->size().width());
        cg.writeEntry("DialogHeight", re->size().height());
    }

    QQuickWindow::resizeEvent(re);
}

}

#include "moc_configview.cpp"
