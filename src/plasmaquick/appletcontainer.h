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

namespace PlasmaQuick
{
class AppletContainerPrivate;
class AppletRepresentation;

class PLASMAQUICK_EXPORT AppletContainer : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(Plasma::Applet *plasmoid READ applet WRITE setApplet NOTIFY appletChanged)
    Q_PROPERTY(AppletRepresentation *plasmoidRepresentation READ appletRepresentation NOTIFY appletRepresentationChanged)
    Q_PROPERTY(QQuickItem *contentItem READ contentItem NOTIFY contentItemChanged)
    Q_PROPERTY(QQuickItem *temporaryExpansionItem READ temporaryExpansionItem NOTIFY temporaryExpansionItemChanged)
    

public:
    AppletContainer(QQuickItem *parent = nullptr);
    ~AppletContainer() override;

    void setApplet(Plasma::Applet *applet);
    Plasma::Applet *applet() const;

    AppletRepresentation *appletRepresentation();

    QQuickItem *contentItem();
    QQuickItem *temporaryExpansionItem();

Q_SIGNALS:
    void appletChanged(Plasma::Applet *applet);
    void appletRepresentationChanged(AppletRepresentation *representation);
    void contentItemChanged(QQuickItem *contentItem);
    void temporaryExpansionItemChanged(QQuickItem *temporaryExpansionItem);

protected:
    // Reimplementation
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void itemChange(ItemChange change, const ItemChangeData &value) override;

private:
    AppletContainerPrivate *const d;
};

}

