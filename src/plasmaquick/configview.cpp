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

#include <klocalizedstring.h>
#include <kdeclarative/kdeclarative.h>

#include <Plasma/Corona>
#include <Plasma/PluginLoader>

namespace PlasmaQuick
{

//////////////////////////////ConfigView

class ConfigViewPrivate
{
public:
    ConfigViewPrivate(Plasma::Applet *appl, ConfigView *view);
    ~ConfigViewPrivate();

    void init();
    ConfigView *q;
    QWeakPointer <Plasma::Applet> applet;
    ConfigModel *configModel;
    Plasma::Corona *corona;
};

ConfigViewPrivate::ConfigViewPrivate(Plasma::Applet *appl, ConfigView *view)
    : q(view),
      applet(appl),
      corona(0)
{
}

void ConfigViewPrivate::init()
{
    applet.data()->setUserConfiguring(true);

    KDeclarative::KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(q->engine());
    kdeclarative.setupBindings();
    qmlRegisterType<ConfigModel>("org.kde.plasma.configuration", 2, 0, "ConfigModel");
    qmlRegisterType<ConfigCategory>("org.kde.plasma.configuration", 2, 0, "ConfigCategory");

    //FIXME: problem on nvidia, all windows should be transparent or won't show
    q->setColor(Qt::transparent);
    q->setTitle(i18n("%1 Settings", applet.data()->title()));

    //systray case
    if (!applet.data()->containment()->corona()) {
        Plasma::Applet *a = qobject_cast<Plasma::Applet *>(applet.data()->containment()->parent());
        if (a) {
            corona = a->containment()->corona();
        }
    } else if (!applet.data()->containment()->corona()->package().isValid()) {
        qWarning() << "Invalid home screen package";
    } else {
        corona = applet.data()->containment()->corona();
    }

    q->setResizeMode(QQuickView::SizeViewToRootObject);

    //config model local of the applet
    QQmlComponent *component = new QQmlComponent(q->engine(), QUrl::fromLocalFile(applet.data()->package().filePath("configmodel")), q);
    QObject *object = component->beginCreate(q->engine()->rootContext());
    configModel = qobject_cast<ConfigModel *>(object);
    if (configModel) {
        configModel->setApplet(applet.data());
    } else {
        delete object;
    }

    q->engine()->rootContext()->setContextProperty("plasmoid", applet.data()->property("_plasma_graphicObject").value<QObject *>());
    q->engine()->rootContext()->setContextProperty("configDialog", q);
    component->completeCreate();
    delete component;
}

ConfigView::ConfigView(Plasma::Applet *applet, QWindow *parent)
    : QQuickView(parent),
      d(new ConfigViewPrivate(applet, this))
{
    d->init();
    qmlRegisterType<ConfigModel>("org.kde.plasma.configuration", 2, 0, "ConfigModel");
    qmlRegisterType<ConfigCategory>("org.kde.plasma.configuration", 2, 0, "ConfigCategory");
    connect(applet, &QObject::destroyed, this, &ConfigView::close);
}

ConfigView::~ConfigView()
{
    if (d->applet) {
        d->applet.data()->setUserConfiguring(false);
    }
}

void ConfigView::init()
{
    setSource(QUrl::fromLocalFile(d->corona->package().filePath("appletconfigurationui")));
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
    rootObject()->setWidth(re->size().width());
    rootObject()->setHeight(re->size().height());
    QQuickWindow::resizeEvent(re);
}

}

#include "moc_configview.cpp"
