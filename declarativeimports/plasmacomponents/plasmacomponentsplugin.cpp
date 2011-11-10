/*
 *   Copyright 2011 by Marco Martin <mart@kde.org>

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

#include "plasmacomponentsplugin.h"

#include <QtDeclarative/qdeclarative.h>
#include <QtDeclarative/QDeclarativeEngine>

#include "qrangemodel.h"

#include <KSharedConfig>

#include "enums.h"
#include "qmenu.h"
#include "qmenuitem.h"
#include "kdialogproxy.h"

void PlasmaComponentsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.plasma.components"));

    QString componentsPlatform = getenv("KDE_PLASMA_COMPONENTS_PLATFORM");
    if (componentsPlatform.isEmpty()) {
        KConfigGroup cg(KSharedConfig::openConfig("kdeclarativerc"), "Components-platform");
        componentsPlatform = cg.readEntry("name", "desktop");
    }

    if (componentsPlatform == "desktop") {
        qmlRegisterType<KDialogProxy>(uri, 0, 1, "QueryDialog");

        qmlRegisterType<QMenuProxy>(uri, 0, 1, "Menu");
        qmlRegisterType<QMenuProxy>(uri, 0, 1, "ContextMenu");
        qmlRegisterType<QMenuItem>(uri, 0, 1, "MenuItem");
    }

    qmlRegisterType<Plasma::QRangeModel>(uri, 0, 1, "RangeModel");

    qmlRegisterUncreatableType<DialogStatus>(uri, 0, 1, "DialogStatus", "");
    qmlRegisterUncreatableType<PageOrientation>(uri, 0, 1, "PageOrientation", "");
    qmlRegisterUncreatableType<PageStatus>(uri, 0, 1, "PageStatus", "");
}


#include "plasmacomponentsplugin.moc"

