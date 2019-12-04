/*
*   Copyright 2005 by Aaron Seigo <aseigo@kde.org>
*   Copyright 2007 by Riccardo Iaconelli <riccardo@kde.org>
*   Copyright 2008 by Ménard Alexis <darktears31@gmail.com>
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

#ifndef PLASMA_APPLET_P_H
#define PLASMA_APPLET_P_H

#include <QAction>
#include <QBasicTimer>

#include <kactioncollection.h>
#include <kconfigskeleton.h>
#include <KNotification>

#include <KPackage/Package>

#include "plasma/applet.h"

class KKeySequenceWidget;

namespace Plasma
{

class AppletScript;
class Service;

class AppletPrivate
{
public:
    AppletPrivate(const KPluginMetaData &info, int uniqueID, Applet *applet);
    virtual ~AppletPrivate();

    void init(const QString &packagePath = QString(), const QVariantList &args = QVariantList());

    void setDestroyed(bool destroyed);
    void askDestroy();
    virtual void cleanUpAndDelete();

    // put all setup routines for script here. at this point we can assume that
    // package exists and that we have a script engin
    void setupPackage();
    void setupScripting();

    QString globalName() const;
    void scheduleConstraintsUpdate(Plasma::Types::Constraints c);
    void scheduleModificationNotification();
    KConfigGroup *mainConfigGroup();
    void resetConfigurationObject();
    void updateShortcuts();
    void globalShortcutChanged();
    void propagateConfigChanged();
    void setUiReady();

    static KActionCollection *defaultActions(QObject *parent);

    void requestConfiguration();

    static uint s_maxAppletId;

    uint appletId;
    Applet *q;

    // applet attributes
    Types::ImmutabilityType immutability;
    Types::ImmutabilityType oldImmutability;
    QString launchErrorMessage;

    // applet info we keep around in case its needed
    KPluginMetaData appletDescription;
    QString customTitle;
    QString icon;

    // bookkeeping
    KConfigGroup *mainConfig;
    Plasma::Types::Constraints pendingConstraints;

    // scripting and package stuff
    AppletScript *script;
    KPackage::Package package;
    KConfigLoader *configLoader;

    // actions stuff; put activationAction into actions?
    KActionCollection *actions;
    QAction *activationAction;

    Types::ItemStatus itemStatus;

    // timerEvent bookkeeping
    QBasicTimer constraintsTimer;
    QBasicTimer *modificationsTimer;

    QPointer <KNotification> deleteNotification;
    QTimer *deleteNotificationTimer;

    QString configurationRequiredReason;

    Types::BackgroundHints backgroundHints = Types::DefaultBackground;
    Types::BackgroundHints userBackgroundHints = Types::DefaultBackground;

    // a great green field of booleans :)
    bool userBackgroundHintsInitialized = false;
    bool hasConfigurationInterface : 1;
    bool failed : 1;
    bool transient : 1;
    bool needsConfig : 1;
    bool started : 1;
    bool globalShortcutEnabled : 1;
    bool userConfiguring : 1;
    bool busy : 1;
};

} // Plasma namespace

#endif
