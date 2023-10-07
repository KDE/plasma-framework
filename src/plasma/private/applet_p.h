/*
    SPDX-FileCopyrightText: 2005 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2007 Riccardo Iaconelli <riccardo@kde.org>
    SPDX-FileCopyrightText: 2008 Ménard Alexis <darktears31@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_APPLET_P_H
#define PLASMA_APPLET_P_H

#include <QAction>
#include <QBasicTimer>
#include <QPointer>

#include <KConfigPropertyMap>
#include <KConfigSkeleton>
#include <KNotification>

#include <KPackage/Package>
#include <qaction.h>
#include <qtypes.h>

#include "plasma/applet.h"

class KKeySequenceWidget;

namespace Plasma
{
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

    void setupPackage();

    QString globalName() const;
    void scheduleConstraintsUpdate(Applet::Constraints c);
    void scheduleModificationNotification();
    KConfigGroup *mainConfigGroup();
    void resetConfigurationObject();
    void globalShortcutChanged();
    void propagateConfigChanged();
    void setUiReady();

    static QMap<QString, QAction *> defaultActions(QObject *parent);

    static void contextualActions_append(QQmlListProperty<QAction> *prop, QAction *action);
    static qsizetype contextualActions_count(QQmlListProperty<QAction> *prop);
    static QAction *contextualActions_at(QQmlListProperty<QAction> *prop, qsizetype idx);
    static void contextualActions_clear(QQmlListProperty<QAction> *prop);
    static void contextualActions_replace(QQmlListProperty<QAction> *prop, qsizetype idx, QAction *action);
    static void contextualActions_removeLast(QQmlListProperty<QAction> *prop);

    void requestConfiguration();

    static uint s_maxAppletId;

    uint appletId;
    Applet *q;

    // applet attributes
    Types::ImmutabilityType immutability;
    Types::ImmutabilityType oldImmutability;
    QString launchErrorMessage;

    KPluginMetaData appletDescription;
    QString customTitle;
    QString icon;

    QVariantList startupArguments;

    // bookkeeping
    KConfigGroup *mainConfig;
    Applet::Constraints pendingConstraints;

    // config and package stuff
    KPackage::Package package;
    KConfigLoader *configLoader = nullptr;
    KConfigPropertyMap *configPropertyMap = nullptr;

    // It's a map to have values() as a stable list
    QMap<QString, QAction *> actions;
    QList<QAction *> contextualActions;
    QAction *activationAction;
    QHash<QString, QActionGroup *> actionGroups;

    Types::ItemStatus itemStatus;

    // timerEvent bookkeeping
    QBasicTimer constraintsTimer;
    QBasicTimer *modificationsTimer;

    QPointer<KNotification> deleteNotification;
    QTimer *deleteNotificationTimer;

    QString configurationRequiredReason;

    Types::BackgroundHints backgroundHints = Types::DefaultBackground;
    Types::BackgroundHints userBackgroundHints = Types::DefaultBackground;
    Types::ConstraintHints constraintHints = Types::NoHint;

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
