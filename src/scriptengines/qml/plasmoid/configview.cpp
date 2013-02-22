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

#include "configview.h"
#include "plasmoid/appletinterface.h"

#include <QDebug>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlContext>

#include <KLocalizedString>

#include <Plasma/Corona>

ConfigView::ConfigView(AppletInterface *interface, QWindow *parent)
    : QQuickView(parent),
      m_appletInterface(interface)
{
    //FIXME: problem on nvidia, all windows should be transparent or won't show
    setColor(Qt::transparent);
    setTitle(i18n("%1 Settings", m_appletInterface->applet()->title()));


    if (!m_appletInterface->applet()->containment()->corona()->package().isValid()) {
        qWarning() << "Invalid home screen package";
    }

    setResizeMode(QQuickView::SizeRootObjectToView);


    //rootObject()->metaObject()->invokeMethod(rootObject(), "addConfigPage", Q_ARG(QVariant, QUrl::fromLocalFile(m_appletInterface->applet()->package().filePath("ui", "ConfigGeneral.qml"))));

    QVariantMap page;
    page["title"] = i18n("General");
    if (m_appletInterface->applet()->icon().isEmpty()) {
        page["icon"] = "configure";
    } else {
        page["icon"] = m_appletInterface->applet()->icon();
    }
    page["component"] = QVariant::fromValue(new QQmlComponent(engine(), QUrl::fromLocalFile(m_appletInterface->applet()->package().filePath("ui", "ConfigGeneral.qml")), this));
    m_configPages << page;

    engine()->rootContext()->setContextProperty("plasmoid", interface);
    engine()->rootContext()->setContextProperty("configDialog", this);
    setSource(QUrl::fromLocalFile(m_appletInterface->applet()->containment()->corona()->package().filePath("ui", "Configuration.qml")));


}

ConfigView::~ConfigView()
{
}


QVariantList ConfigView::configPages() const
{
    return m_configPages;
}

#include "moc_configview.cpp"
