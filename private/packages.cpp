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

#include "plasma/private/packages_p.h"

#include <math.h>
#include <float.h> // FLT_MAX

#include <QCoreApplication>
#include <QFileInfo>

#include <kconfiggroup.h>
#include <kdesktopfile.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>

#include "kdeclarative.h"
#include "private/wallpaper_p.h"
#include "package.h"
#include "config-plasma.h"

namespace Plasma
{

void ChangeableMainScriptPackage::initPackage(Package *package)
{
    package->addFileDefinition("mainscript", "code/main", i18n("Main Script File"));
    package->setRequired("mainscript", true);
}

QString ChangeableMainScriptPackage::findMainScript(Package *package)
{
    Q_UNUSED(package)
    return QString();
}

void ChangeableMainScriptPackage::pathChanged(Package *package)
{
    KDesktopFile config(package->path() + "/metadata.desktop");
    KConfigGroup cg = config.desktopGroup();
    QString mainScript = cg.readEntry("X-Plasma-MainScript", QString());
    if (mainScript.isEmpty()) {
        mainScript = findMainScript(package);

        if (mainScript.isEmpty()) {
            mainScript = package->path() + "/code/main.js";
            if (!QFile::exists(mainScript)) {
                mainScript.clear();
            }
        }
    }

    if (!mainScript.isEmpty()) {
        package->addFileDefinition("mainscript", mainScript, i18n("Main Script File"));
    }
}

QString PlasmoidPackage::findMainScript(Package *package)
{
    const QString mainScript = package->path() + "/ui/main.qml";
    if (QFile::exists(mainScript)) {
        return mainScript;
    }

    return QString();
}

void PlasmoidPackage::initPackage(Package *package)
{
    QStringList platform = KDeclarative::runtimePlatform();
    if (!platform.isEmpty()) {
        QMutableStringListIterator it(platform);
        while (it.hasNext()) {
            it.next();
            it.setValue("platformcontents/" + it.value());
        }

        platform.append("contents");
        package->setContentsPrefixPaths(platform);
    }

    package->setServicePrefix("plasma-applet-");
    package->setDefaultPackageRoot("plasma/plasmoids");

    package->addDirectoryDefinition("images", "images", i18n("Images"));
    QStringList mimetypes;
    mimetypes << "image/svg+xml" << "image/png" << "image/jpeg";
    package->setMimeTypes("images", mimetypes);

    package->addDirectoryDefinition("config", "config", i18n("Configuration Definitions"));
    mimetypes.clear();
    mimetypes << "text/xml";
    package->setMimeTypes("config", mimetypes);

    package->addDirectoryDefinition("ui", "ui", i18n("User Interface"));
    package->setMimeTypes("ui", mimetypes);

    package->addDirectoryDefinition("data", "data", i18n("Data Files"));

    package->addDirectoryDefinition("scripts", "code", i18n("Executable Scripts"));
    mimetypes.clear();
    mimetypes << "text/plain";
    package->setMimeTypes("scripts", mimetypes);

    package->addDirectoryDefinition("translations", "locale", i18n("Translations"));

    package->addFileDefinition("mainconfigui", "ui/config.ui", i18n("Main Config UI File"));
    package->addFileDefinition("mainconfigxml", "config/main.xml", i18n("Configuration XML file"));
    package->addDirectoryDefinition("animations", "animations", i18n("Animation scripts"));
}

void DataEnginePackage::initPackage(Package *package)
{
    ChangeableMainScriptPackage::initPackage(package);
    package->setServicePrefix("plasma-dataengine-");
    package->setDefaultPackageRoot("plasma/dataengines/");

    package->addDirectoryDefinition("data", "data", i18n("Data Files"));

    package->addDirectoryDefinition("scripts", "code", i18n("Executable Scripts"));
    QStringList mimetypes;
    mimetypes << "text/plain";
    package->setMimeTypes("scripts", mimetypes);

    package->addDirectoryDefinition("services", "services/", i18n("Service Descriptions"));
    package->setMimeTypes("services", mimetypes);

    package->addDirectoryDefinition("translations", "locale", i18n("Translations"));
}

void RunnerPackage::initPackage(Package *package)
{
    ChangeableMainScriptPackage::initPackage(package);
    package->setServicePrefix("plasma-runner-");
    package->setDefaultPackageRoot("plasma/runners/");

    package->addDirectoryDefinition("data", "data", i18n("Data Files"));

    package->addDirectoryDefinition("scripts", "code", i18n("Executable Scripts"));
    QStringList mimetypes;
    mimetypes << "text/plain";
    package->setMimeTypes("scripts", mimetypes);

    package->addDirectoryDefinition("translations", "locale", i18n("Translations"));
}

void ThemePackage::initPackage(Package *package)
{
    package->addDirectoryDefinition("dialogs", "dialogs/", i18n("Images for dialogs"));
    package->addFileDefinition("dialogs/background", "dialogs/background.svg",
                      i18n("Generic dialog background"));
    package->addFileDefinition("dialogs/background", "dialogs/background.svgz",
                      i18n("Generic dialog background"));
    package->addFileDefinition("dialogs/shutdowndialog", "dialogs/shutdowndialog.svg",
                      i18n("Theme for the logout dialog"));
    package->addFileDefinition("dialogs/shutdowndialog", "dialogs/shutdowndialog.svgz",
                      i18n("Theme for the logout dialog"));

    package->addDirectoryDefinition("wallpapers", "wallpapers/", i18n("Wallpaper packages"));
    package->addDirectoryDefinition("animations", "animations/", i18n("Animation scripts"));

    package->addDirectoryDefinition("widgets", "widgets/", i18n("Images for widgets"));
    package->addFileDefinition("widgets/background", "widgets/background.svg",
                      i18n("Background image for widgets"));
    package->addFileDefinition("widgets/background", "widgets/background.svgz",
                      i18n("Background image for widgets"));
    package->addFileDefinition("widgets/clock", "widgets/clock.svg",
                      i18n("Analog clock face"));
    package->addFileDefinition("widgets/clock", "widgets/clock.svgz",
                      i18n("Analog clock face"));
    package->addFileDefinition("widgets/panel-background", "widgets/panel-background.svg",
                      i18n("Background image for panels"));
    package->addFileDefinition("widgets/panel-background", "widgets/panel-background.svgz",
                      i18n("Background image for panels"));
    package->addFileDefinition("widgets/plot-background", "widgets/plot-background.svg",
                      i18n("Background for graphing widgets"));
    package->addFileDefinition("widgets/plot-background", "widgets/plot-background.svgz",
                      i18n("Background for graphing widgets"));
    package->addFileDefinition("widgets/tooltip", "widgets/tooltip.svg",
                      i18n("Background image for tooltips"));
    package->addFileDefinition("widgets/tooltip", "widgets/tooltip.svgz",
                      i18n("Background image for tooltips"));

    package->addDirectoryDefinition("opaque/dialogs", "opaque/dialogs/", i18n("Opaque images for dialogs"));
    package->addFileDefinition("opaque/dialogs/background", "opaque/dialogs/background.svg",
                      i18n("Opaque generic dialog background"));
    package->addFileDefinition("opaque/dialogs/background", "opaque/dialogs/background.svgz",
                      i18n("Opaque generic dialog background"));
    package->addFileDefinition("opaque/dialogs/shutdowndialog", "opaque/dialogs/shutdowndialog.svg",
                      i18n("Opaque theme for the logout dialog"));
    package->addFileDefinition("opaque/dialogs/shutdowndialog", "opaque/dialogs/shutdowndialog.svgz",
                      i18n("Opaque theme for the logout dialog"));

    package->addDirectoryDefinition("opaque/widgets", "opaque/widgets/", i18n("Opaque images for widgets"));
    package->addFileDefinition("opaque/widgets/panel-background", "opaque/widgets/panel-background.svg",
                      i18n("Opaque background image for panels"));
    package->addFileDefinition("opaque/widgets/panel-background", "opaque/widgets/panel-background.svgz",
                      i18n("Opaque background image for panels"));
    package->addFileDefinition("opaque/widgets/tooltip", "opaque/widgets/tooltip.svg",
                      i18n("Opaque background image for tooltips"));
    package->addFileDefinition("opaque/widgets/tooltip", "opaque/widgets/tooltip.svgz",
                      i18n("Opaque background image for tooltips"));

    package->addDirectoryDefinition("locolor/dialogs", "locolor/dialogs/",
                           i18n("Low color images for dialogs"));
    package->addFileDefinition("locolor/dialogs/background", "locolor/dialogs/background.svg",
                      i18n("Low color generic dialog background"));
    package->addFileDefinition("locolor/dialogs/background", "locolor/dialogs/background.svgz",
                      i18n("Low color generic dialog background"));
    package->addFileDefinition("locolor/dialogs/shutdowndialog", "locolor/dialogs/shutdowndialog.svg",
                      i18n("Low color theme for the logout dialog"));
    package->addFileDefinition("locolor/dialogs/shutdowndialog", "locolor/dialogs/shutdowndialog.svgz",
                      i18n("Low color theme for the logout dialog"));

    package->addDirectoryDefinition("locolor/widgets", "locolor/widgets/", i18n("Images for widgets"));
    package->addFileDefinition("locolor/widgets/background", "locolor/widgets/background.svg",
                      i18n("Low color background image for widgets"));
    package->addFileDefinition("locolor/widgets/background", "locolor/widgets/background.svgz",
                      i18n("Low color background image for widgets"));
    package->addFileDefinition("locolor/widgets/clock", "locolor/widgets/clock.svg",
                      i18n("Low color analog clock face"));
    package->addFileDefinition("locolor/widgets/clock", "locolor/widgets/clock.svgz",
                      i18n("Low color analog clock face"));
    package->addFileDefinition("locolor/widgets/panel-background", "locolor/widgets/panel-background.svg",
                      i18n("Low color background image for panels"));
    package->addFileDefinition("locolor/widgets/panel-background", "locolor/widgets/panel-background.svgz",
                      i18n("Low color background image for panels"));
    package->addFileDefinition("locolor/widgets/plot-background", "locolor/widgets/plot-background.svg",
                      i18n("Low color background for graphing widgets"));
    package->addFileDefinition("locolor/widgets/plot-background", "locolor/widgets/plot-background.svgz",
                      i18n("Low color background for graphing widgets"));
    package->addFileDefinition("locolor/widgets/tooltip", "locolor/widgets/tooltip.svg",
                      i18n("Low color background image for tooltips"));
    package->addFileDefinition("locolor/widgets/tooltip", "locolor/widgets/tooltip.svgz",
                      i18n("Low color background image for tooltips"));

    package->addFileDefinition("colors", "colors", i18n("KColorScheme configuration file"));

    QStringList mimetypes;
    mimetypes << "image/svg+xml";
    package->setDefaultMimeTypes(mimetypes);
}

WallpaperPackage::WallpaperPackage(Wallpaper *paper)
    : PackageStructure(),
      m_fullPackage(true),
      m_targetSize(100000, 100000),
      m_resizeMethod(Wallpaper::ScaledResize)
{
    if (paper) {
        connect(paper, SIGNAL(renderHintsChanged()), this, SLOT(renderHintsChanged()));
    }
}

void WallpaperPackage::initPackage(Package *package)
{
    QStringList mimetypes;
    mimetypes << "image/svg" << "image/png" << "image/jpeg" << "image/jpg";
    package->setDefaultMimeTypes(mimetypes);

    package->addDirectoryDefinition("images", "images/", i18n("Images"));
    package->setRequired("images", true);
    package->addFileDefinition("screenshot", "screenshot.png", i18n("Screenshot"));
    package->setAllowExternalPaths(true);
}

void WallpaperPackage::renderHintsChanged()
{
    Wallpaper *paper = qobject_cast<Wallpaper *>(sender());
    if (!paper) {
        return;
    }

    if (m_fullPackage) {
        //FIXME: findBestPaper(packages.value(paper));
    }
}

void WallpaperPackage::pathChanged(Package *package)
{
    static bool guard = false;

    if (guard) {
        return;
    }

    guard = true;

    QFileInfo info(package->path());
    m_fullPackage = info.isDir();
    package->removeDefinition("preferred");
    package->setRequired("images", m_fullPackage);

    if (m_fullPackage) {
        package->setContentsPrefixPaths(QStringList() << "contents/");
        findBestPaper(package);
    } else {
        // dirty trick to support having a file passed in instead of a directory
        package->addFileDefinition("preferred", info.fileName(), i18n("Recommended wallpaper file"));
        package->setContentsPrefixPaths(QStringList());
        //kDebug() << "changing" << path() << "to" << info.path();
        package->setPath(info.path());
    }

    guard = false;
}

QSize WallpaperPackage::resSize(const QString &str) const
{
    const int index = str.indexOf('x');
    if (index != -1) {
        return QSize(str.left(index).toInt(), str.mid(index + 1).toInt());
    }

    return QSize();
}

void WallpaperPackage::findBestPaper(Package *package)
{
    QStringList images = package->entryList("images");
    if (images.empty()) {
        return;
    }

    //kDebug() << "wanted" << size;

    // choose the nearest resolution
    float best = FLT_MAX;

    QString bestImage;
    foreach (const QString &entry, images) {
        const QSize candidate = resSize(QFileInfo(entry).baseName());
        if (candidate == QSize()) {
            continue;
        }

        double dist = distance(candidate, m_targetSize, m_resizeMethod);
        //kDebug() << "candidate" << candidate << "distance" << dist;
        if (bestImage.isEmpty() || dist < best) {
            bestImage = entry;
            best = dist;
            //kDebug() << "best" << bestImage;
            if (dist == 0) {
                break;
            }
        }
    }

    //kDebug() << "best image" << bestImage;
    package->addFileDefinition("preferred", package->filePath("images") + bestImage, i18n("Recommended wallpaper file"));
}

float WallpaperPackage::distance(const QSize& size, const QSize& desired,
                                 Plasma::Wallpaper::ResizeMethod method) const
{
    // compute difference of areas
    float delta = size.width() * size.height() -
                  desired.width() * desired.height();
    // scale down to about 1.0
    delta /= ((desired.width() * desired.height())+(size.width() * size.height()))/2;


    switch (method) {
    case Plasma::Wallpaper::ScaledResize: {
        // Consider first the difference in aspect ratio,
        // then in areas. Prefer scaling down.
        float deltaRatio = 1.0;
        if (size.height() > 0 && desired.height() > 0) {
            deltaRatio = float(size.width()) / float(size.height()) -
                         float(desired.width()) / float(desired.height());
        }
        return fabs(deltaRatio) * 3.0 + (delta >= 0.0 ? delta : -delta + 5.0);
    }
    case Plasma::Wallpaper::ScaledAndCroppedResize:
        // Difference of areas, slight preference to scale down
        return delta >= 0.0 ? delta : -delta + 2.0;
    default:
        // Difference in areas
        return fabs(delta);
    }
}

void ContainmentActionsPackage::initPackage(Package *package)
{
    ChangeableMainScriptPackage::initPackage(package);
    package->setDefaultPackageRoot("plasma/containmentactions/");
}

void GenericPackage::initPackage(Package *package)
{
    package->setDefaultPackageRoot("plasma/packages/");
}

} // namespace Plasma

#include "moc_packages_p.cpp"

