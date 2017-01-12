/***************************************************************************
 *   Copyright 2013 Marco Martin <mart@kde.org>                            *
 *   Copyright 2014 Sebastian Kügler <sebas@kde.org>                       *
 *   Copyright 2014 David Edmundson <davidedmunsdon@kde.org>               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "units.h"

#include <QGuiApplication>
#include <QDebug>
#include <QtGlobal>
#include <QQuickItem>
#include <QQuickWindow>
#include <QScreen>
#include <QFontMetrics>
#include <cmath>

#include <KDirWatch>
#include <KIconLoader>

QString plasmarc() { return QStringLiteral("plasmarc"); }
QString groupName() { return QStringLiteral("Units"); }
const int defaultLongDuration = 120;


SharedAppFilter::SharedAppFilter(QObject *parent)
    : QObject(parent)
{
    QCoreApplication::instance()->installEventFilter(this);
}

SharedAppFilter::~SharedAppFilter()
{}

bool SharedAppFilter::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == QCoreApplication::instance()) {
        if (event->type() == QEvent::ApplicationFontChange) {
            emit fontChanged();
        }
    }
    return QObject::eventFilter(watched, event);
}

SharedAppFilter *Units::s_sharedAppFilter = nullptr;

Units::Units(QObject *parent)
    : QObject(parent),
      m_gridUnit(-1),
      m_devicePixelRatio(-1),
      m_smallSpacing(-1),
      m_largeSpacing(-1),
      m_longDuration(defaultLongDuration) // default base value for animations
{
    if (!s_sharedAppFilter) {
        s_sharedAppFilter = new SharedAppFilter();
    }

    m_iconSizes = new QQmlPropertyMap(this);
    updateDevicePixelRatio(); // also updates icon sizes
    updateSpacing(); // updates gridUnit and *Spacing properties

    connect(KIconLoader::global(), &KIconLoader::iconLoaderSettingsChanged, this, &Units::iconLoaderSettingsChanged);
    QObject::connect(s_sharedAppFilter, SIGNAL(fontChanged()), this, SLOT(updateSpacing()));

    const QString configFile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/') + plasmarc();
    KDirWatch::self()->addFile(configFile);

    // Catch both, direct changes to the config file ...
    connect(KDirWatch::self(), &KDirWatch::dirty, this, &Units::settingsFileChanged);
    // ... but also remove/recreate cycles, like KConfig does it
    connect(KDirWatch::self(), &KDirWatch::created, this, &Units::settingsFileChanged);
    // read configuration
    updatePlasmaRCSettings();
}

Units::~Units()
{

}

Units &Units::instance()
{
    static Units units;
    return units;
}

void Units::settingsFileChanged(const QString &file)
{
    if (file.endsWith(plasmarc())) {
        KSharedConfigPtr cfg = KSharedConfig::openConfig(plasmarc());
        cfg->reparseConfiguration();
        updatePlasmaRCSettings();
    }
}

void Units::updatePlasmaRCSettings()
{
    KConfigGroup cfg = KConfigGroup(KSharedConfig::openConfig(plasmarc()), groupName());
    // Animators with a duration of 0 do not fire reliably
    // see Bug 357532 and QTBUG-39766
    const int longDuration = qMax(1, cfg.readEntry("longDuration", defaultLongDuration));

    if (longDuration != m_longDuration) {
        m_longDuration = longDuration;
        emit durationChanged();
    }
}


void Units::iconLoaderSettingsChanged()
{
    m_iconSizes->insert(QStringLiteral("desktop"), devicePixelIconSize(KIconLoader::global()->currentSize(KIconLoader::Desktop)));

    m_iconSizes->insert(QStringLiteral("tiny"), devicePixelIconSize(KIconLoader::SizeSmall) / 2);
    m_iconSizes->insert(QStringLiteral("small"), devicePixelIconSize(KIconLoader::SizeSmall));
    m_iconSizes->insert(QStringLiteral("smallMedium"), devicePixelIconSize(KIconLoader::SizeSmallMedium));
    m_iconSizes->insert(QStringLiteral("medium"), devicePixelIconSize(KIconLoader::SizeMedium));
    m_iconSizes->insert(QStringLiteral("large"), devicePixelIconSize(KIconLoader::SizeLarge));
    m_iconSizes->insert(QStringLiteral("huge"), devicePixelIconSize(KIconLoader::SizeHuge));
    m_iconSizes->insert(QStringLiteral("enormous"), devicePixelIconSize(KIconLoader::SizeEnormous));

    emit iconSizesChanged();
}

QQmlPropertyMap *Units::iconSizes() const
{
    return m_iconSizes;
}

int Units::roundToIconSize(int size)
{
    /*Do *not* use devicePixelIconSize here, we want to use the sizes of the pixmaps of the smallest icons on the disk. And those are unaffected by dpi*/
    if (size <= 0) {
        return 0;
    } else if (size < KIconLoader::SizeSmall) {
        return KIconLoader::SizeSmall/2;
    } else if (size < KIconLoader::SizeSmallMedium) {
        return KIconLoader::SizeSmall;

    } else if (size < KIconLoader::SizeMedium) {
        return KIconLoader::SizeSmallMedium;

    } else if (size < KIconLoader::SizeLarge) {
        return KIconLoader::SizeMedium;

    } else if (size < KIconLoader::SizeHuge) {
        return KIconLoader::SizeLarge;

    } else {
        return size;
    }
}

int Units::devicePixelIconSize(const int size) const
{
    /* in kiconloader.h
    enum StdSizes {
        SizeSmall=16,
        SizeSmallMedium=22,
        SizeMedium=32,
        SizeLarge=48,
        SizeHuge=64,
        SizeEnormous=128
    };
    */
    // Scale the icon sizes up using the devicePixelRatio
    // This function returns the next stepping icon size
    // and multiplies the global settings with the dpi ratio.
    const qreal ratio = devicePixelRatio();

    if (ratio < 1.5) {
        return size;
    } else if (ratio < 2.0) {
        return size * 1.5;
    } else if (ratio < 2.5) {
        return size * 2.0;
    } else if (ratio < 3.0) {
        return size * 2.5;
    } else if (ratio < 3.5) {
        return size * 3.0;
    } else {
        return size * ratio;
    }
    // FIXME: Add special casing for < 64 cases: align to kiconloader size
}

qreal Units::devicePixelRatio() const
{
    return m_devicePixelRatio;
}

void Units::updateDevicePixelRatio()
{
    // Using QGuiApplication::devicePixelRatio() gives too coarse values,
    // i.e. it directly jumps from 1.0 to 2.0. We want tighter control on
    // sizing, so we compute the exact ratio and use that.
    // TODO: make it possible to adapt to the dpi for the current screen dpi
    //  instead of assuming that all of them use the same dpi which applies for
    //  X11 but not for other systems.
    QScreen *primary = QGuiApplication::primaryScreen();
    if (!primary) {
        return;
    }
    const qreal dpi = primary->logicalDotsPerInchX();
    // Usual "default" is 96 dpi
    // that magic ratio follows the definition of "device independent pixel" by Microsoft
    m_devicePixelRatio = (qreal)dpi / (qreal)96;
    iconLoaderSettingsChanged();
    emit devicePixelRatioChanged();
}

int Units::gridUnit() const
{
    return m_gridUnit;
}

int Units::smallSpacing() const
{
    return m_smallSpacing;
}

int Units::largeSpacing() const
{
    return m_largeSpacing;
}

void Units::updateSpacing()
{
    int gridUnit = QFontMetrics(QGuiApplication::font()).boundingRect(QStringLiteral("M")).height();

    if (gridUnit % 2 != 0) {
        gridUnit++;
    }
    if (gridUnit != m_gridUnit) {
        m_gridUnit = gridUnit;
        emit gridUnitChanged();
    }

    if (gridUnit != m_largeSpacing) {
        m_smallSpacing = qMax(2, (int)(gridUnit / 4)); // 1/4 of gridUnit, at least 2
        m_largeSpacing = gridUnit; // msize.height
        emit spacingChanged();
    }
}

int Units::longDuration() const
{
    return m_longDuration;
}

int Units::shortDuration() const
{
    return qMax(1, m_longDuration / 5);
}

#include "moc_units.cpp"

