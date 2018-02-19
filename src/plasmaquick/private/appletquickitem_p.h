/*
 *   Copyright 2014 Marco Martin <mart@kde.org>
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

#ifndef APPLETQUICKITEM_P_H
#define APPLETQUICKITEM_P_H

#include <QQmlComponent>
#include <QQuickItem>
#include <QQmlEngine>
#include <KPackage/Package>

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
    //weight values for the logic for when or if to preload
    enum PreloadWeights {
        DefaultPreloadWeight = 50,
        DefaultLauncherPreloadWeight = 100,
        DelayedPreloadWeight = 25,
        PreloadWeightIncrement = 5,
        PreloadWeightDecrement = 8
    };

    enum PreloadPolicy {
        Uninitialized = -1,
        None = 0,
        Adaptive = 1,
        Aggressive = 2
    };

    AppletQuickItemPrivate(Plasma::Applet *a, AppletQuickItem *item);

    void init();

    int preloadWeight() const;

    QQuickItem *createCompactRepresentationItem();
    QQuickItem *createFullRepresentationItem();
    QQuickItem *createCompactRepresentationExpanderItem();

    //look into item, and return the Layout attached property, if found
    void connectLayoutAttached(QObject *item);
    void propagateSizeHint(const QByteArray &layoutProperty);

    //handlers of Layout signals, private slots
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

    //Attached layout objects: own and the representation's one
    QPointer<QObject> representationLayout;
    QPointer<QObject> ownLayout;

    Plasma::Applet *applet;
    KDeclarative::QmlObject *qmlObject;

    KPackage::Package appletPackage;
    KPackage::Package coronaPackage;
    KPackage::Package containmentPackage;

    bool expanded : 1;
    bool activationTogglesExpanded : 1;

    static QHash<QObject *, AppletQuickItem *> s_rootObjects;
};

}

#endif
