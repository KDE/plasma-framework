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

#include <QCheckBox>
#include <QFile>
#include <QHash>
#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
#include <QTimer>

#include <KDebug>
#include <KStandardDirs>

#include <Plasma/Containment>
#include <Plasma/Theme>
#include <Plasma/Applet>
#include <Plasma/Wallpaper>
#include <plasma/pluginloader.h>

#include "plasmakpartcorona.h"
#include "plasmakpartview.h"

K_PLUGIN_FACTORY(plasmaKPartFactory, registerPlugin<PlasmaKPart>();)
K_EXPORT_PLUGIN(plasmaKPartFactory("plasma-kpart","plasma-kpart") )

PlasmaKPart::PlasmaKPart(QWidget *parentWidget, QObject *parent, const QVariantList &args)
    : KParts::ReadOnlyPart(parent),
      m_corona(0),
      m_view(new PlasmaKPartView(0, 1, parentWidget))
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

    QTimer::singleShot(0, this, SLOT(initCorona()));
}

PlasmaKPart::~PlasmaKPart()
{
    delete m_view;
    m_view = 0;

    if (!m_configFile.isEmpty()) {
        m_corona->saveLayout();
    }

    delete m_corona;
    m_corona = 0;

    //TODO: This manual sync() should not be necessary?
    syncConfig();
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

void PlasmaKPart::syncConfig()
{
    KGlobal::config()->sync();
}

void PlasmaKPart::initCorona()
{
    if (m_corona) {
        return;
    }

    m_corona = new PlasmaKPartCorona(this);
    connect(m_corona, SIGNAL(containmentAdded(Plasma::Containment*)), this, SLOT(createView(Plasma::Containment*)));
    connect(m_corona, SIGNAL(configSynced()), this, SLOT(syncConfig()));

    m_corona->setItemIndexMethod(QGraphicsScene::NoIndex);
    m_corona->initializeLayout(m_configFile);

    m_view->show();
}

PlasmaKPartCorona* PlasmaKPart::corona() const
{
    return m_corona;
}

void PlasmaKPart::createView(Plasma::Containment *containment)
{
    m_view->setContainment(containment);
}

void PlasmaKPart::addApplet(const QString& name, const QVariantList& args, const QRectF& geometry )
{
    containment()->addApplet(name, args, geometry);
}

Plasma::Applet::List PlasmaKPart::listActiveApplets() const
{
    return containment()->applets();
}

QString PlasmaKPart::configFile() const
{
    return m_configFile;
}

void PlasmaKPart::setConfigFile(const QString &file)
{
    m_configFile = file;
    if (m_corona && QFile::exists(m_configFile)) {
        m_corona->initializeLayout(m_configFile);
    }
}

Plasma::Containment* PlasmaKPart::containment() const
{
    return corona()->containments().first();
}

#include "plasmakpart.moc"
