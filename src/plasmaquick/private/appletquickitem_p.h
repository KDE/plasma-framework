/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef APPLETQUICKITEM_P_H
#define APPLETQUICKITEM_P_H

#include <KPackage/Package>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the public Plasma API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace Plasma
{
class Applet;
}

namespace KDeclarative
{
class QmlObject;
}

namespace PlasmaQuick
{
class AppletQuickItem;

class AppletQuickItemPrivate
{
public:
    // weight values for the logic for when or if to preload
    enum PreloadWeights {
        DefaultPreloadWeight = 50,
        DefaultLauncherPreloadWeight = 100,
        DelayedPreloadWeight = 25,
        PreloadWeightIncrement = 5,
        PreloadWeightDecrement = 8,
    };

    enum PreloadPolicy {
        Uninitialized = -1,
        None = 0,
        Adaptive = 1,
        Aggressive = 2,
    };

    AppletQuickItemPrivate(Plasma::Applet *a, AppletQuickItem *item);

    void init();

    int preloadWeight() const;

    QQuickItem *createCompactRepresentationItem();
    QQuickItem *createFullRepresentationItem();
    QQuickItem *createCompactRepresentationExpanderItem();

    // true if the applet is at a size in which it should be expanded,
    // false if is too small and should be an icon
    bool appletShouldBeExpanded() const;
    // ensures the popup is preloaded, don't expand yet
    void preloadForExpansion();

    // look into item, and return the Layout attached property, if found
    void connectLayoutAttached(QObject *item);
    void propagateSizeHint(const QByteArray &layoutProperty);

    // handlers of Layout signals, private slots
    static void anchorsFillParent(QQuickItem *item, QQuickItem *parent);
    void compactRepresentationCheck();
    void minimumWidthChanged();
    void minimumHeightChanged();
    void preferredWidthChanged();
    void preferredHeightChanged();
    void maximumWidthChanged();
    void maximumHeightChanged();
    void fillWidthChanged();
    void fillHeightChanged();

    AppletQuickItem *q;

    static PreloadPolicy s_preloadPolicy;
    int switchWidth;
    int switchHeight;

    QPointer<QQmlComponent> compactRepresentation;
    QPointer<QQmlComponent> fullRepresentation;
    QPointer<QQmlComponent> preferredRepresentation;
    QPointer<QQmlComponent> compactRepresentationExpander;

    QPointer<QQuickItem> compactRepresentationItem;
    QPointer<QQuickItem> fullRepresentationItem;
    QPointer<QQuickItem> compactRepresentationExpanderItem;
    QPointer<QQuickItem> currentRepresentationItem;
    QPointer<QObject> testItem;

    // Attached layout objects: own and the representation's one
    QPointer<QObject> representationLayout;
    QPointer<QObject> ownLayout;

    Plasma::Applet *applet;
    KDeclarative::QmlObject *qmlObject;

    KPackage::Package appletPackage;
    KPackage::Package coronaPackage;
    KPackage::Package containmentPackage;

    bool expanded : 1;
    bool activationTogglesExpanded : 1;
    bool initComplete : 1;
    bool compactRepresentationCheckGuard : 1;
};

}

#endif
