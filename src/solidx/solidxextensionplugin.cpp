/*
 *   Copyright (C) 2013 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "solidxextensionplugin.h"

//#include <QtDeclarative/qdeclarative.h>
#include <QtQml>
#include <QtQml/QQmlExtensionPlugin>
#include <QDebug>

#include "interface.h"
#include "inputdevicemodel.h"

class SolidXExtensionPlugin: public QQmlExtensionPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.solidx")

public:
    SolidXExtensionPlugin(QObject * parent = Q_NULLPTR)
        : QQmlExtensionPlugin(parent)
    {
        qDebug() << "instantiated plugin object";
    }

    void registerTypes(const char * uri) Q_DECL_OVERRIDE
    {
        using namespace solidx;

        qDebug() << "plugin loaded, registering types " << uri;

        Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.solidx"));

        qmlRegisterType<Interface> (uri, 0, 1, "Interface");
        qmlRegisterType<InputDeviceModel> (uri, 0, 1, "InputDeviceModel");
    }

};

#include "solidxextensionplugin.moc"

