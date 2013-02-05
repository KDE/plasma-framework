/*
 *  Copyright 2013 Marco Martin <mart@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "view.h"

#include <QDebug>
#include <QQuickItem>
#include <QQmlContext>
#include <QTimer>
#include "plasma/pluginloader.h"

View::View(QWindow *parent)
    : QQuickView(parent)
{
    m_package = Plasma::PluginLoader::self()->loadPackage("Plasma/Generic");
    m_package.setPath("org.kde.homescreen.desktop");

    if (!m_package.isValid()) {
        qWarning() << "Invalid home screen package";
    }

    setResizeMode(View::SizeRootObjectToView);
    setSource(QUrl::fromLocalFile(m_package.filePath("mainscript")));
    show();rootContext()->setContextProperty("plasmoid", this);
}

View::~View()
{
    
}



void View::setContainment(Plasma::Containment *cont)
{
    if (m_containment) {
        disconnect(m_containment.data(), 0, this, 0);
    }

    m_containment = cont;

    if (!cont) {
        return;
    }

    if (m_containment.data()->graphicObject()) {
        qDebug() << "using as graphic containment" << m_containment.data()->graphicObject()<<m_containment.data();
        if (!m_containment || !m_containment.data()->graphicObject()) {
            qWarning() << "Containment not valid (yet?)";
            return;
        }

        m_containment.data()->graphicObject()->setProperty("visible", false);
        m_containment.data()->graphicObject()->setProperty("parent", QVariant::fromValue(rootObject()));
        rootObject()->setProperty("containment", QVariant::fromValue(m_containment.data()->graphicObject()));
    }
}

Plasma::Containment *View::containment() const
{
    return m_containment.data();
}

#include "moc_view.cpp"
