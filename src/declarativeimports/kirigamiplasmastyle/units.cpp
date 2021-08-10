/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmunsdon@kde.org>
    SPDX-FileCopyrightText: 2021 Jonah Brüchert <jbb@kaidan.im>

    SPDX-License-Identifier: LGPL-2.0-or-later

*/

#include "units.h"

#include <KConfigGroup>
#include <KSharedConfig>

constexpr int defaultLongDuration = 200;

Units::Units(QObject *parent)
    : Kirigami::Units(parent)
    , m_animationSpeedWatcher(KConfigWatcher::create(KSharedConfig::openConfig()))
{
    connect(m_animationSpeedWatcher.data(), &KConfigWatcher::configChanged, this, [this](const KConfigGroup &group, const QByteArrayList &names) {
        if (group.name() == QLatin1String("KDE") && names.contains(QByteArrayLiteral("AnimationDurationFactor"))) {
            updateAnimationSpeed();
        }
    });

    updateAnimationSpeed();
}

// It'd be nice if we could somehow share this with core/units.cpp
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

    setVeryShortDuration(longDuration / 4);
    setShortDuration(longDuration / 2);
    setLongDuration(longDuration);
    setVeryLongDuration(longDuration * 2);
}
