/*
    SPDX-FileCopyrightText: 2021 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KPackage/Package>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQmlContext>

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
class AppletRepresentation;

class AppletRepresentationPrivate
{
public:
    AppletRepresentationPrivate(AppletRepresentation *item);

    QQuickItem *createCompactRepresentationItem();
    QQuickItem *createFullRepresentationItem();

    AppletRepresentation *q;

    int switchWidth;
    int switchHeight;

    QPointer<QQmlComponent> compactRepresentation;
    QPointer<QQmlComponent> fullRepresentation;
    QPointer<QQmlComponent> preferredRepresentation;

    QPointer<QQuickItem> compactRepresentationItem;
    QPointer<QQuickItem> fullRepresentationItem;
    QPointer<QQuickItem> currentRepresentationItem;
    QPointer<QObject> testItem;

    Plasma::Applet *applet;
    QQmlContext *rootContext;

    KPackage::Package appletPackage;
    KPackage::Package coronaPackage;
    KPackage::Package containmentPackage;

    static QHash<Plasma::Applet *, AppletRepresentation *> s_appletRepresentations;

    bool expanded : 1;
    bool activationTogglesExpanded : 1;
};
}

