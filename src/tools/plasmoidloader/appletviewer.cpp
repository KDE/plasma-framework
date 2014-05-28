/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  David Edmundson <david@davidedmundson.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "appletviewer.h"

#include <applet.h>
#include <pluginloader.h>

#include <appletquickitem.h>

#include <applet.h>
#include <plasma/scripting/appletscript.h>
#include "plasmaquick/appletquickitem.h"

#include <QDebug>
#include <QDateTime>
#include <iostream>

using namespace Plasma;
using namespace PlasmaQuick;

AppletViewer::AppletViewer(const QString& appletName, Representation mode):
    m_appletItem(0),
    m_frameCount(0),
    m_mode(FrameCount)
{
    m_startTime = QDateTime::currentMSecsSinceEpoch ();

    connect(this, SIGNAL(frameSwapped()), SLOT(onSwapBuffers()), Qt::QueuedConnection);

    //suicide after 10 seconds
    QTimer::singleShot(10000, this, SLOT(onTimeout()));

    Applet *applet = PluginLoader::self()->loadApplet(appletName);

    if (!applet) {
        qFatal("Could not load applet");
    }

    auto scriptEngine = loadScriptEngine("declarativeappletscript", applet, QVariantList());
    scriptEngine->init();

    m_appletItem = applet->property("_plasma_graphicObject").value<AppletQuickItem*>();

    if(!m_appletItem) {
        qDebug() << "could not load applet item";
    }

    m_appletItem->init();

    qDebug() << m_appletItem->fullRepresentation();

    QQuickItem *item = 0;
    switch (mode) {
        case FullRepresentation:
            item = m_appletItem->fullRepresentationItem();
            break;
        case CompactRepresentation:
            item = m_appletItem->compactRepresentationItem();
        default:
            item = m_appletItem;
    }
    if (item) {
        item->setParentItem(contentItem());
        item->setWidth(width());
        item->setHeight(height());
    } else {
        qDebug() << "could not load item";
    }

}

void AppletViewer::onSwapBuffers()
{
    if (m_mode == FrameCount) {
        m_frameCount++;
    }
    if (m_mode == StartupBenchmark) {
        std::cout <<  QDateTime::currentMSecsSinceEpoch() - m_startTime << std::endl;
        QCoreApplication::exit();
    }
}

void AppletViewer::onTimeout()
{
    std::cout << m_frameCount << std::endl;
    QCoreApplication::exit();
}



#include "appletviewer.moc"
