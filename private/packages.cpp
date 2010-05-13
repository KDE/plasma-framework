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

#include <QFileInfo>

#include <kconfiggroup.h>
#include <kdesktopfile.h>
#include <klocale.h>
#include <kmessagebox.h>

#ifndef PLASMA_NO_KNEWSTUFF
#include <knewstuff3/downloaddialog.h>
#endif

#include "plasma/private/wallpaper_p.h"

namespace Plasma
{

PlasmoidPackage::PlasmoidPackage(QObject *parent)
    : Plasma::PackageStructure(parent, QString("Plasmoid"))
{
    addDirectoryDefinition("images", "images/", i18n("Images"));
    QStringList mimetypes;
    mimetypes << "image/svg+xml" << "image/png" << "image/jpeg";
    setMimetypes("images", mimetypes);

    addDirectoryDefinition("config", "config/", i18n("Configuration Definitions"));
    mimetypes.clear();
    mimetypes << "text/xml";
    setMimetypes("config", mimetypes);

    addDirectoryDefinition("ui", "ui", i18n("User Interface"));
    setMimetypes("ui", mimetypes);

    addDirectoryDefinition("data", "data", i18n("Data Files"));

    addDirectoryDefinition("scripts", "code", i18n("Executable Scripts"));
    mimetypes.clear();
    mimetypes << "text/plain";
    setMimetypes("scripts", mimetypes);

    addDirectoryDefinition("translations", "locale", i18n("Translations"));

    addFileDefinition("mainconfigui", "ui/config.ui", i18n("Main Config UI File"));
    addFileDefinition("mainconfigxml", "config/main.xml", i18n("Configuration XML file"));
    addFileDefinition("mainscript", "code/main", i18n("Main Script File"));
    addFileDefinition("defaultconfig", "config/default-configrc", i18n("Default configuration"));
    addDirectoryDefinition("animations", "animations/", i18n("Animation scripts"));
    setRequired("mainscript", true);
}

PlasmoidPackage::~PlasmoidPackage()
{
#ifndef PLASMA_NO_KNEWSTUFF
    delete m_knsDialog.data();
#endif
}

void PlasmoidPackage::pathChanged()
{
    KDesktopFile config(path() + "/metadata.desktop");
    KConfigGroup cg = config.desktopGroup();
    QString mainScript = cg.readEntry("X-Plasma-MainScript", QString());
    if (!mainScript.isEmpty()) {
        addFileDefinition("mainscript", mainScript, i18n("Main Script File"));
        setRequired("mainscript", true);
    }
}

void PlasmoidPackage::createNewWidgetBrowser(QWidget *parent)
{
#ifndef PLASMA_NO_KNEWSTUFF
    KNS3::DownloadDialog *knsDialog = m_knsDialog.data();
    if (!knsDialog) {
        m_knsDialog = knsDialog = new KNS3::DownloadDialog("plasmoids.knsrc", parent);
        connect(knsDialog, SIGNAL(accepted()), this, SIGNAL(newWidgetBrowserFinished()));
    }

    knsDialog->show();
    knsDialog->raise();
#endif
}

ThemePackage::ThemePackage(QObject *parent)
    : Plasma::PackageStructure(parent, QString("Plasma Theme"))
{
    addDirectoryDefinition("dialogs", "dialogs/", i18n("Images for dialogs"));
    addFileDefinition("dialogs/background", "dialogs/background.svg",
                      i18n("Generic dialog background"));
    addFileDefinition("dialogs/shutdowndialog", "dialogs/shutdowndialog.svg",
                      i18n("Theme for the logout dialog"));

    addDirectoryDefinition("wallpapers", "wallpapers/", i18n("Wallpaper packages"));
    addDirectoryDefinition("animations", "animations/", i18n("Animation scripts"));

    addDirectoryDefinition("widgets", "widgets/", i18n("Images for widgets"));
    addFileDefinition("widgets/background", "widgets/background.svg",
                      i18n("Background image for widgets"));
    addFileDefinition("widgets/clock", "widgets/clock.svg",
                      i18n("Analog clock face"));
    addFileDefinition("widgets/panel-background", "widgets/panel-background.svg",
                      i18n("Background image for panels"));
    addFileDefinition("widgets/plot-background", "widgets/plot-background.svg",
                      i18n("Background for graphing widgets"));
    addFileDefinition("widgets/tooltip", "widgets/tooltip.svg",
                      i18n("Background image for tooltips"));

    addDirectoryDefinition("opaque/dialogs", "opaque/dialogs/", i18n("Opaque images for dialogs"));
    addFileDefinition("opaque/dialogs/background", "opaque/dialogs/background.svg",
                      i18n("Opaque generic dialog background"));
    addFileDefinition("opaque/dialogs/shutdowndialog", "opaque/dialogs/shutdowndialog.svg",
                      i18n("Opaque theme for the logout dialog"));

    addDirectoryDefinition("opaque/widgets", "opaque/widgets/", i18n("Opaque images for widgets"));
    addFileDefinition("opaque/widgets/panel-background", "opaque/widgets/panel-background.svg",
                      i18n("Opaque background image for panels"));
    addFileDefinition("opaque/widgets/tooltip", "opaque/widgets/tooltip.svg",
                      i18n("Opaque background image for tooltips"));

    addDirectoryDefinition("locolor/dialogs", "locolor/dialogs/",
                           i18n("Low color images for dialogs"));
    addFileDefinition("locolor/dialogs/background", "locolor/dialogs/background.svg",
                      i18n("Low color generic dialog background"));
    addFileDefinition("locolor/dialogs/shutdowndialog", "locolor/dialogs/shutdowndialog.svg",
                      i18n("Low color theme for the logout dialog"));

    addDirectoryDefinition("locolor/widgets", "locolor/widgets/", i18n("Images for widgets"));
    addFileDefinition("locolor/widgets/background", "locolor/widgets/background.svg",
                      i18n("Low color background image for widgets"));
    addFileDefinition("locolor/widgets/clock", "locolor/widgets/clock.svg",
                      i18n("Low color analog clock face"));
    addFileDefinition("locolor/widgets/panel-background", "locolor/widgets/panel-background.svg",
                      i18n("Low color background image for panels"));
    addFileDefinition("locolor/widgets/plot-background", "locolor/widgets/plot-background.svg",
                      i18n("Low color background for graphing widgets"));
    addFileDefinition("locolor/widgets/tooltip", "locolor/widgets/tooltip.svg",
                      i18n("Low color background image for tooltips"));

    addFileDefinition("colors", "colors", i18n("KColorScheme configuration file"));

    QStringList mimetypes;
    mimetypes << "image/svg+xml";
    setDefaultMimetypes(mimetypes);
}

WallpaperPackage::WallpaperPackage(Wallpaper *paper, QObject *parent)
    : PackageStructure(parent, "Background"),
      m_paper(paper),
      m_fullPackage(true)
{
    QStringList mimetypes;
    mimetypes << "image/svg" << "image/png" << "image/jpeg" << "image/jpg";
    setDefaultMimetypes(mimetypes);

    addDirectoryDefinition("images", "images/", i18n("Images"));
    setRequired("images", true);
    addFileDefinition("screenshot", "screenshot.png", i18n("Screenshot"));
    setAllowExternalPaths(true);

    if (m_paper) {
        connect(paper, SIGNAL(renderHintsChanged()), this, SLOT(renderHintsChanged()));
        connect(m_paper, SIGNAL(destroyed(QObject*)), this, SLOT(paperDestroyed()));
    }
}

void WallpaperPackage::renderHintsChanged()
{
    if (m_fullPackage) {
        findBestPaper();
    }
}

void WallpaperPackage::pathChanged()
{
    static bool guard = false;

    if (guard) {
        return;
    }

    guard = true;

    QFileInfo info(path());

    m_fullPackage = info.isDir();
    if (m_fullPackage) {
        setContentsPrefix("contents/");
        findBestPaper();
    } else {
        // dirty trick to support having a file passed in instead of a directory
        addFileDefinition("preferred", info.fileName(), i18n("Recommended wallpaper file"));
        setContentsPrefix(QString());
        //kDebug() << "changing" << path() << "to" << info.path();
        setPath(info.path());
    }

    guard = false;
}

QSize WallpaperPackage::resSize(const QString &str) const
{
    int index = str.indexOf('x');
    if (index != -1) {
        return QSize(str.left(index).toInt(),
                     str.mid(index + 1).toInt());
    } else {
        return QSize();
    }
}

void WallpaperPackage::findBestPaper()
{
    QStringList images = entryList("images");
    if (images.empty()) {
        return;
    }

    //kDebug() << "wanted" << size;

    // choose the nearest resolution
    float best = FLT_MAX;
    const QSize size = m_paper ? m_paper->d->targetSize.toSize() : QSize(100000, 100000);
    const Wallpaper::ResizeMethod method = m_paper ? m_paper->d->lastResizeMethod
                                                   : Wallpaper::ScaledResize;

    QString bestImage;
    foreach (const QString &entry, images) {
        QSize candidate = resSize(QFileInfo(entry).baseName());
        if (candidate == QSize()) {
            continue;
        }

        double dist = distance(candidate, size, method);
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
    addFileDefinition("preferred", path("images") + bestImage, i18n("Recommended wallpaper file"));
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

void WallpaperPackage::paperDestroyed()
{
    m_paper = 0;
}

ContainmentActionsPackage::ContainmentActionsPackage(QObject *parent)
    : Plasma::PackageStructure(parent, QString("ContainmentActions"))
{
    //FIXME how do I do the mimetypes stuff?
}

} // namespace Plasma

#include "packages_p.moc"

