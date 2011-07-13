/*
 *   Copyright 2006-2008 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
 *   Copyright 2010 Ryan Rix <ry@n.rix.si>
 *   Copyright 2010 Siddharth Sharma <siddharth.kde@gmail.com>
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

#include "plasmakpart.h"

#include "plasmakpartcorona.h"
#include "plasmakpartview.h"

#include <Plasma/Containment>
#include <Plasma/Theme>
#include <Plasma/Applet>
#include <Plasma/Wallpaper>
#include <plasma/pluginloader.h>

#include <KDebug>
#include <KStandardDirs>

#include <QCheckBox>
#include <QHash>
#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>

K_PLUGIN_FACTORY(plasmaKPartFactory, registerPlugin<PlasmaKPart>();)
K_EXPORT_PLUGIN(plasmaKPartFactory("plasma-kpart","plasma-kpart") )

PlasmaKPart::PlasmaKPart(QWidget *parentWidget, QObject *parent, const QVariantList &args)
    : KParts::ReadOnlyPart(parent),
      m_corona(0),
      m_view(new PlasmaKPartView(0, 1, parentWidget)),
      m_service(0)
{
    setComponentData(plasmaKPartFactory::componentData());

    KGlobal::locale()->insertCatalog("libplasma");
    KGlobal::locale()->insertCatalog("plasmagenericshell");
    KGlobal::locale()->insertCatalog("plasma-kpart");

    setThemeDefaults();

    setWidget(m_view);

    // initialize the plugin loader
    if (args.length() > 0) {
        Plasma::PluginLoader *loader = qVariantValue<Plasma::PluginLoader *>(args.first());
        if (loader) {
            Plasma::PluginLoader::setPluginLoader(loader);
        }
    }

    // this line initializes the corona.
    corona();
}

PlasmaKPart::~PlasmaKPart()
{
}

void PlasmaKPart::setThemeDefaults()
{
    KConfigGroup cg(KSharedConfig::openConfig("plasmarc"), "Theme-plasma-kpart" );
    const QString themeName = cg.readEntry("name", "appdashboard");
    Plasma::Theme::defaultTheme()->setUseGlobalSettings(false);
    Plasma::Theme::defaultTheme()->setThemeName(themeName);

    cg = KConfigGroup(KGlobal::config(), "General");

    Plasma::Theme::defaultTheme()->setFont(cg.readEntry("desktopFont", QFont("Sans") ));
}

void PlasmaKPart::cleanup()
{
    if (m_corona) {
        m_corona->saveLayout();
    }

    if (!m_view->containment()) {
        return;
    }

    // save the mapping of Views to Containments at the moment
    // of application exit so we can restore that when we start again.
    KConfigGroup viewIds(KGlobal::config(), "ViewIds");
    viewIds.deleteGroup();
    viewIds.writeEntry(QString::number(m_view->containment()->id()), 1);

    delete m_view;
    m_view = 0;

    delete m_corona;
    m_corona = 0;

    //TODO: This manual sync() should not be necessary?
    syncConfig();
}

void PlasmaKPart::syncConfig()
{
    KGlobal::config()->sync();
}

PlasmaKPartCorona* PlasmaKPart::corona()
{
    if (!m_corona) {
        m_corona = new PlasmaKPartCorona(this);
        connect(m_corona, SIGNAL(containmentAdded(Plasma::Containment*)), this, SLOT(createView(Plasma::Containment*)));
        connect(m_corona, SIGNAL(configSynced()), this, SLOT(syncConfig()));

        m_corona->setItemIndexMethod(QGraphicsScene::NoIndex);
        m_corona->initializeLayout();

        m_view->show();
    }

    return m_corona;
}

void PlasmaKPart::createView(Plasma::Containment *containment)
{
    KConfigGroup viewIds(KGlobal::config(), "ViewIds");
    int id = viewIds.readEntry(QString::number(containment->id()), 1);

    kDebug() << "new containment" << (QObject*)containment << containment->id()<<"view id"<<id;

    m_view->setContainment(containment);
    emit viewCreated();
}

void PlasmaKPart::addApplet(const QString& name, const QVariantList& args, const QRectF& geometry )
{
    containment()->addApplet(name, args, geometry);
}

Plasma::Applet::List PlasmaKPart::listActiveApplets( )
{
    return containment()->applets();
}

Plasma::Containment* PlasmaKPart::containment()
{
    return corona()->containments().first();
}

bool PlasmaKPart::setPluginLoader(Plasma::PluginLoader *loader)
{
    if (Plasma::PluginLoader::pluginLoader()) {
        return false;
    }

    Plasma::PluginLoader::setPluginLoader(loader);
    return true;
}

#include "plasmakpart.moc"
