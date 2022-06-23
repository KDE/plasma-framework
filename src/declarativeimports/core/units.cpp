/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmunsdon@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "units.h"

#include <QDebug>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QQuickItem>
#include <QQuickWindow>
#include <QScreen>
#include <QtGlobal>
#include <cmath>

#include <KIconLoader>

const int defaultLongDuration = 200;

SharedAppFilter::SharedAppFilter(QObject *parent)
    : QObject(parent)
{
    QCoreApplication::instance()->installEventFilter(this);
}

SharedAppFilter::~SharedAppFilter()
{
}

bool SharedAppFilter::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == QCoreApplication::instance()) {
        if (event->type() == QEvent::ApplicationFontChange) {
            Q_EMIT fontChanged();
        }
    }
    return QObject::eventFilter(watched, event);
}

SharedAppFilter *Units::s_sharedAppFilter = nullptr;

Units::Units(QObject *parent)
    : QObject(parent)
    , m_gridUnit(-1)
    , m_devicePixelRatio(-1)
    , m_smallSpacing(-1)
    , m_mediumSpacing(-1)
    , m_largeSpacing(-1)
    , m_longDuration(defaultLongDuration) // default base value for animations
{
    if (!s_sharedAppFilter) {
        s_sharedAppFilter = new SharedAppFilter();
    }

    m_iconSizes = new QQmlPropertyMap(this);
    m_iconSizeHints = new QQmlPropertyMap(this);
    updateDevicePixelRatio(); // also updates icon sizes
    updateSpacing(); // updates gridUnit and *Spacing properties

    connect(KIconLoader::global(), &KIconLoader::iconLoaderSettingsChanged, this, &Units::iconLoaderSettingsChanged);
    QObject::connect(s_sharedAppFilter, &SharedAppFilter::fontChanged, this, &Units::updateSpacing);

    m_animationSpeedWatcher = KConfigWatcher::create(KSharedConfig::openConfig());
    connect(m_animationSpeedWatcher.data(), &KConfigWatcher::configChanged, this, [this](const KConfigGroup &group, const QByteArrayList &names) {
        if (group.name() == QLatin1String("KDE") && names.contains(QByteArrayLiteral("AnimationDurationFactor"))) {
            updateAnimationSpeed();
        }
    });
    updateAnimationSpeed();
}

Units::~Units()
{
}

Units &Units::instance()
{
    static Units units;
    return units;
}

void Units::updateAnimationSpeed()
{
    KConfigGroup generalCfg = KConfigGroup(KSharedConfig::openConfig(), QStringLiteral("KDE"));
    const qreal animationSpeedModifier = qMax(0.0, generalCfg.readEntry("AnimationDurationFactor", 1.0));

    // Read the old longDuration value for compatibility
    KConfigGroup cfg = KConfigGroup(KSharedConfig::openConfig(QStringLiteral("plasmarc")), QStringLiteral("Units"));
    int longDuration = cfg.readEntry("longDuration", defaultLongDuration);

    longDuration = qRound(longDuration * animationSpeedModifier);

    // Animators with a duration of 0 do not fire reliably
    // see Bug 357532 and QTBUG-39766
    longDuration = qMax(1, longDuration);

    if (longDuration != m_longDuration) {
        m_longDuration = longDuration;
        Q_EMIT durationChanged();
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
    // We deliberately don't feed the result into devicePixelIconSize() because
    // roundToIconSize() already does that internally.
    m_iconSizes->insert(QStringLiteral("sizeForLabels"), roundToIconSize(QFontMetrics(QGuiApplication::font()).height()));

    m_iconSizeHints->insert(QStringLiteral("panel"), devicePixelIconSize(KIconLoader::global()->currentSize(KIconLoader::Panel)));
    m_iconSizeHints->insert(QStringLiteral("desktop"), devicePixelIconSize(KIconLoader::global()->currentSize(KIconLoader::Desktop)));

    Q_EMIT iconSizesChanged();
    Q_EMIT iconSizeHintsChanged();
}

QQmlPropertyMap *Units::iconSizes() const
{
    return m_iconSizes;
}

QQmlPropertyMap *Units::iconSizeHints() const
{
    return m_iconSizeHints;
}

int Units::roundToIconSize(int size)
{
    // If not using Qt scaling (e.g. on X11 by default), manually scale all sizes
    // according to the DPR because Qt hasn't done it for us. Otherwise all
    // returned sizes are too small and icons are only the right size by pure
    // chance for the larger sizes due to their large differences in size
    qreal multiplier = 1.0;
    QScreen *primary = QGuiApplication::primaryScreen();
    if (primary) {
        // Note that when using Qt scaling, the multiplier will always be 1
        // because Qt will scale everything for us. This is good and intentional.
        multiplier = bestIconScaleForDevicePixelRatio((qreal)primary->logicalDotsPerInchX() / (qreal)96);
    }

    if (size <= 0) {
        return 0;

    } else if (size < KIconLoader::SizeSmall * multiplier) {
        return qRound(KIconLoader::SizeSmall * multiplier);

    } else if (size < KIconLoader::SizeSmallMedium * multiplier) {
        return qRound(KIconLoader::SizeSmall * multiplier);

    } else if (size < KIconLoader::SizeMedium * multiplier) {
        return qRound(KIconLoader::SizeSmallMedium * multiplier);

    } else if (size < KIconLoader::SizeLarge * multiplier) {
        return qRound(KIconLoader::SizeMedium * multiplier);

    } else if (size < KIconLoader::SizeHuge * multiplier) {
        return qRound(KIconLoader::SizeLarge * multiplier);

    } else if (size < KIconLoader::SizeEnormous * multiplier) {
        return qRound(KIconLoader::SizeHuge * multiplier);

    } else {
        return size;
    }
}

qreal Units::bestIconScaleForDevicePixelRatio(const qreal ratio)
{
    if (ratio < 1.5) {
        return 1;
    } else if (ratio < 2.0) {
        return 1.5;
    } else if (ratio < 2.5) {
        return 2.0;
    } else if (ratio < 3.0) {
        return 2.5;
    } else if (ratio < 3.5) {
        return 3.0;
    } else {
        return ratio;
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

    return size * bestIconScaleForDevicePixelRatio(ratio);
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
    Q_EMIT devicePixelRatioChanged();
}

int Units::gridUnit() const
{
    return m_gridUnit;
}

int Units::smallSpacing() const
{
    return m_smallSpacing;
}

int Units::mediumSpacing() const
{
    return m_mediumSpacing;
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
        Q_EMIT gridUnitChanged();
    }

    if (gridUnit != m_largeSpacing) {
        m_smallSpacing = qMax(2, (int)(gridUnit / 4)); // 1/4 of gridUnit, at least 2
        m_mediumSpacing = std::round(m_smallSpacing * 1.5);
        m_largeSpacing = gridUnit; // msize.height
        Q_EMIT spacingChanged();
    }
}

int Units::longDuration() const
{
    return m_longDuration;
}

int Units::shortDuration() const
{
    return qMax(1, qRound(m_longDuration * 0.5));
}

int Units::veryShortDuration() const
{
    return qRound(m_longDuration * 0.25);
}

int Units::veryLongDuration() const
{
    return m_longDuration * 2;
}

int Units::humanMoment() const
{
    return 2000;
}

#include "moc_units.cpp"
