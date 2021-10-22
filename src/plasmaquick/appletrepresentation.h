/*
    SPDX-FileCopyrightText: 2021 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QTimer>

#include <plasmaquick/plasmaquick_export.h>

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
class AppletRepresentationPrivate;

class PLASMAQUICK_EXPORT AppletRepresentation : public QObject
{
    Q_OBJECT

    // in applet?
    Q_PROPERTY(int switchWidth READ switchWidth WRITE setSwitchWidth NOTIFY switchWidthChanged)
    Q_PROPERTY(int switchHeight READ switchHeight WRITE setSwitchHeight NOTIFY switchHeightChanged)

    Q_PROPERTY(QQmlComponent *compactRepresentation READ compactRepresentation WRITE setCompactRepresentation NOTIFY compactRepresentationChanged)
    Q_PROPERTY(QQuickItem *compactRepresentationItem READ compactRepresentationItem NOTIFY compactRepresentationItemChanged)

    Q_PROPERTY(QQmlComponent *fullRepresentation READ fullRepresentation WRITE setFullRepresentation NOTIFY fullRepresentationChanged)
    Q_PROPERTY(QQuickItem *fullRepresentationItem READ fullRepresentationItem NOTIFY fullRepresentationItemChanged)

    /**
     * this is supposed to be either one between compactRepresentation or fullRepresentation
     */
    Q_PROPERTY(QQmlComponent *preferredRepresentation READ preferredRepresentation WRITE setPreferredRepresentation NOTIFY preferredRepresentationChanged)

    /**
     * True when the applet wants the activation signal act in toggle mode, i.e. while being expanded
     * the signal shrinks the applet to its not expanded state instead of reexpanding it.
     */
    Q_PROPERTY(bool activationTogglesExpanded WRITE setActivationTogglesExpanded READ isActivationTogglesExpanded NOTIFY activationTogglesExpandedChanged)

    Q_PROPERTY(Plasma::Applet *plasmoid READ applet CONSTANT)

public:
    AppletRepresentation(QQuickItem *parent = nullptr);
    ~AppletRepresentation() override;

    void init(Plasma::Applet *applet, QQmlContext *rootContext);
    Plasma::Applet *applet() const;

    QQuickItem *compactRepresentationItem();
    QQuickItem *fullRepresentationItem();

    ////PROPERTY ACCESSORS
    int switchWidth() const;
    void setSwitchWidth(int width);

    int switchHeight() const;
    void setSwitchHeight(int width);

    QQmlComponent *compactRepresentation();
    void setCompactRepresentation(QQmlComponent *component);

    QQmlComponent *fullRepresentation();
    void setFullRepresentation(QQmlComponent *component);

    QQmlComponent *preferredRepresentation();
    void setPreferredRepresentation(QQmlComponent *component);

    QObject* testItem();

    bool isExpanded() const; // In Applet?
    void setExpanded(bool expanded);

    bool isActivationTogglesExpanded() const; // In Applet?
    void setActivationTogglesExpanded(bool activationTogglesExpanded);

    //FIXME: remove engine param
    static AppletRepresentation *representationForApplet(Plasma::Applet *applet, QQmlEngine *engine);

Q_SIGNALS:
    // Property signals
    void switchWidthChanged(int width);
    void switchHeightChanged(int height);

    void expandedChanged(bool expanded);
    void activationTogglesExpandedChanged(bool activationTogglesExpanded);

    void compactRepresentationChanged(QQmlComponent *compactRepresentation);
    void fullRepresentationChanged(QQmlComponent *fullRepresentation);
    void preferredRepresentationChanged(QQmlComponent *preferredRepresentation);

    void compactRepresentationItemChanged(QObject *compactRepresentationItem);
    void fullRepresentationItemChanged(QObject *fullRepresentationItem);

private:
    AppletRepresentationPrivate *const d;
};

}

