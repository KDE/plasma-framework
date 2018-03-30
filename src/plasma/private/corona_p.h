/*
 *   Copyright 2007-2011 Aaron Seigo <aseigo@kde.org>
 *
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

#ifndef PLASMA_CORONA_P_H
#define PLASMA_CORONA_P_H

#include <QTimer>

#include <kactioncollection.h>

#include "package.h"

class KShortcutsDialog;

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
};

}

#endif
