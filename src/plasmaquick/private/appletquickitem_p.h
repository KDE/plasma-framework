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
    AppletQuickItemPrivate(Plasma::Applet *a, AppletQuickItem *item);

    QObject *createCompactRepresentationItem();
    QObject *createFullRepresentationItem();
    QObject *createCompactRepresentationExpanderItem();

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

    int switchWidth;
    int switchHeight;

    QWeakPointer<QQmlComponent> compactRepresentation;
    QWeakPointer<QQmlComponent> fullRepresentation;
    QWeakPointer<QQmlComponent> preferredRepresentation;
    QWeakPointer<QQmlComponent> compactRepresentationExpander;

    QWeakPointer<QObject> compactRepresentationItem;
    QWeakPointer<QObject> fullRepresentationItem;
    QWeakPointer<QObject> compactRepresentationExpanderItem;
    QWeakPointer<QObject> currentRepresentationItem;

    //Attached layout objects: own and the representation's one
    QWeakPointer<QObject> representationLayout;
    QWeakPointer<QObject> ownLayout;

    QTimer compactRepresentationCheckTimer;
    QTimer fullRepresentationResizeTimer;

    Plasma::Applet *applet;
    KDeclarative::QmlObject *qmlObject;

    Plasma::Package appletPackage;
    Plasma::Package coronaPackage;

    bool expanded : 1;
    bool creatingFullRepresentation : 1;

    static QHash<QObject *, AppletQuickItem *> s_rootObjects;
};

}

#endif
