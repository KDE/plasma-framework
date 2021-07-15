/*
    SPDX-FileCopyrightText: 2007-2011 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_CORONA_P_H
#define PLASMA_CORONA_P_H

#include <QTimer>

#include <KActionCollection>

#include "package.h"

namespace Plasma
{
class Containment;

class CoronaPrivate
{
public:
    CoronaPrivate(Corona *corona);
    ~CoronaPrivate();

    void init();
    void toggleImmutability();
    void saveLayout(KSharedConfigPtr cg) const;
    void updateContainmentImmutability();
    void containmentDestroyed(QObject *obj);
    void syncConfig();
    void notifyContainmentsReady();
    void containmentReady(bool ready);
    Containment *addContainment(const QString &name, const QVariantList &args, uint id, int lastScreen, bool delayedInit = false);
    QList<Plasma::Containment *> importLayout(const KConfigGroup &conf, bool mergeConfig);

    Corona *q;
    KPackage::Package package;
    KConfigGroup desktopDefaultsConfig;
    Types::ImmutabilityType immutability;
    QString configName;
    KSharedConfigPtr config;
    QTimer *configSyncTimer;
    QList<Containment *> containments;
    KActionCollection actions;
    int containmentsStarting;
    bool editMode = false;
};

}

#endif
