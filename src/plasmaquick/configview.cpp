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

#include <KGlobal>
#include <KLocalizedString>
#include <kdeclarative/kdeclarative.h>

#include <Plasma/Corona>
#include <Plasma/PluginLoader>



//////////////////////////////ConfigView

class ConfigViewPrivate
{
public:
    ConfigViewPrivate(Plasma::Applet *appl, ConfigView *view);
    ~ConfigViewPrivate();

    void init();
    ConfigView *q;
    Plasma::Applet *applet;
    ConfigModel *configModel;
};

ConfigViewPrivate::ConfigViewPrivate(Plasma::Applet *appl, ConfigView *view)
    : q(view),
      applet(appl)
{
}

void ConfigViewPrivate::init()
{
    applet->setUserConfiguring(true);

    KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(q->engine());
    kdeclarative.setupBindings();
    qmlRegisterType<ConfigModel>("org.kde.plasma.configuration", 2, 0, "ConfigModel");
    qmlRegisterType<ConfigCategory>("org.kde.plasma.configuration", 2, 0, "ConfigCategory");

    //FIXME: problem on nvidia, all windows should be transparent or won't show
    q->setColor(Qt::transparent);
    q->setTitle(i18n("%1 Settings", applet->title()));


    if (!applet->containment()->corona()->package().isValid()) {
        qWarning() << "Invalid home screen package";
    }

    q->setResizeMode(QQuickView::SizeViewToRootObject);


    //config model local of the applet
    QQmlComponent *component = new QQmlComponent(q->engine(), QUrl::fromLocalFile(applet->package().filePath("configmodel")), q);
    QObject *object = component->beginCreate(q->engine()->rootContext());
    configModel = qobject_cast<ConfigModel *>(object);
    if (configModel) {
        configModel->setApplet(applet);
    } else {
        delete object;
    }

    q->engine()->rootContext()->setContextProperty("plasmoid", applet->property("graphicObject").value<QObject*>());
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
}

ConfigView::~ConfigView()
{
    d->applet->setUserConfiguring(false);
}

void ConfigView::init()
{
    setSource(QUrl::fromLocalFile(d->applet->containment()->corona()->package().filePath("appletconfigurationui")));
}

ConfigModel *ConfigView::configModel() const
{
    return d->configModel;
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


#include "moc_configview.cpp"
