/***************************************************************************
 *   Copyright 2013 Marco Martin <mart@kde.org>                            *
 *   Copyright 2014 Sebastian Kügler <sebas@kde.org>                       *
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

const QString plasmarc = QStringLiteral("plasmarc");
const QString groupName = QStringLiteral("Units");
const int defaultLongDuration = 250;

Units::Units(QObject *parent)
    : QObject(parent),
      m_gridUnit(-1),
      m_devicePixelRatio(-1),
      m_smallSpacing(-1),
      m_largeSpacing(-1),
      m_longDuration(defaultLongDuration) // default base value for animations
{
    m_iconSizes = new QQmlPropertyMap(this);
    updateDevicePixelRatio(); // also updates icon sizes
    updateSpacing(); // updates gridUnit and *Spacing properties

    connect(KIconLoader::global(), &KIconLoader::iconLoaderSettingsChanged, this, &Units::iconLoaderSettingsChanged);
    connect(&m_theme, &Plasma::Theme::themeChanged, this, &Units::updateSpacing);
    QCoreApplication::instance()->installEventFilter(this);

    const QString configFile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/') + plasmarc;
    KDirWatch::self()->addFile(configFile);

    // Catch both, direct changes to the config file ...
    connect(KDirWatch::self(), &KDirWatch::dirty, this, &Units::settingsFileChanged);
    // ... but also remove/recreate cycles, like KConfig does it
    connect(KDirWatch::self(), &KDirWatch::created, this, &Units::settingsFileChanged);
    // Trigger configuration read
    settingsFileChanged(plasmarc);
}

Units::~Units()
{
}

void Units::settingsFileChanged(const QString &file)
{
    if (file.endsWith(plasmarc)) {

        KConfigGroup cfg = KConfigGroup(KSharedConfig::openConfig(plasmarc), groupName);
        cfg.config()->reparseConfiguration();
        const int longDuration = cfg.readEntry("longDuration", defaultLongDuration);

        if (longDuration != m_longDuration) {
            m_longDuration = longDuration;
            emit durationChanged();
        }
    }
}

void Units::iconLoaderSettingsChanged()
{
    // These are not scaled, we respect the user's setting over dpi scaling
    m_iconSizes->insert("desktop", QVariant(KIconLoader::global()->currentSize(KIconLoader::Desktop)));

    // The following icon sizes are fully scaled to dpi
    m_iconSizes->insert("tiny", devicePixelIconSize(KIconLoader::SizeSmall) / 2);
    m_iconSizes->insert("small", devicePixelIconSize(KIconLoader::SizeSmall));
    m_iconSizes->insert("smallMedium", devicePixelIconSize(KIconLoader::SizeSmallMedium));
    m_iconSizes->insert("medium", devicePixelIconSize(KIconLoader::SizeMedium));
    m_iconSizes->insert("large", devicePixelIconSize(KIconLoader::SizeLarge));
    m_iconSizes->insert("huge", devicePixelIconSize(KIconLoader::SizeHuge));
    m_iconSizes->insert("enormous", devicePixelIconSize(KIconLoader::SizeEnormous));

    emit iconSizesChanged();
}

QQmlPropertyMap *Units::iconSizes() const
{
    return m_iconSizes;
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
    qreal dpi = QGuiApplication::primaryScreen()->physicalDotsPerInchX();
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
    int gridUnit = QFontMetrics(QGuiApplication::font()).boundingRect("M").height();
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
    return m_longDuration / 5;
}

bool Units::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == QCoreApplication::instance()) {
        if (event->type() == QEvent::ApplicationFontChange || event->type() == QEvent::FontChange) {
            updateSpacing();
        }
    }
    return QObject::eventFilter(watched, event);
}

