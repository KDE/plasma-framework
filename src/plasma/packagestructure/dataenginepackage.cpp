/******************************************************************************
*   Copyright 2007-2009 by Aaron Seigo <aseigo@kde.org>                       *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#include "plasma.h"

#include <KLocalizedString>
#include <kpackage/package.h>
#include <kpackage/packagestructure.h>
#include "packages_p.h"
#include "config-plasma.h"

class DataEnginePackage : public Plasma::ChangeableMainScriptPackage
{
    Q_OBJECT
public:
    DataEnginePackage(QObject *parent = nullptr, const QVariantList &args = QVariantList()) : ChangeableMainScriptPackage(parent, args) {}

    void initPackage(KPackage::Package *package) Q_DECL_OVERRIDE
    {
        ChangeableMainScriptPackage::initPackage(package);
        package->setDefaultPackageRoot(QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/dataengines/"));

        package->addDirectoryDefinition("data", QStringLiteral("data"), i18n("Data Files"));

        package->addDirectoryDefinition("scripts", QStringLiteral("code"), i18n("Executable Scripts"));
        QStringList mimetypes;
        mimetypes << QStringLiteral("text/plain");
        package->setMimeTypes("scripts", mimetypes);

        package->addDirectoryDefinition("services", QStringLiteral("services/"), i18n("Service Descriptions"));
        package->setMimeTypes("services", mimetypes);

        package->addDirectoryDefinition("translations", QStringLiteral("locale"), i18n("Translations"));
    }
};


K_EXPORT_KPACKAGE_PACKAGE_WITH_JSON(DataEnginePackage, "dataengine-packagestructure.json")

#include "dataenginepackage.moc"
