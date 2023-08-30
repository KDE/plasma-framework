/*
    SPDX-FileCopyrightText: 2023 David Edmundson <davidedmundson@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "plasmawindow.h"

#include <QQuickItem>
#include <plasmaquick/plasmaquick_export.h>

namespace PlasmaQuick
{
class PopupPlasmaWindowPrivate;

/**
 * @brief The PopupPlasmaWindow class is a styled Plasma window that can be positioned
 * relative to an existing Item on another window. When shown the popup is placed correctly.
 *
 * On Wayland this is currently an XdgTopLevel with the PlasmaShellSurface used on top.
 * Do not rely on that implementation detail
 */
class PLASMAQUICK_EXPORT PopupPlasmaWindow : public PlasmaWindow
{
    Q_OBJECT
    Q_PROPERTY(QQuickItem *visualParent READ visualParent WRITE setVisualParent NOTIFY visualParentChanged)
    Q_PROPERTY(Qt::Edge popupDirection READ popupDirection WRITE setPopupDirection NOTIFY popupDirectionChanged)
    Q_PROPERTY(bool floating READ floating WRITE setFloating NOTIFY floatingChanged)
    Q_PROPERTY(bool animated READ animated WRITE setAnimated NOTIFY animatedChanged)

public:
    PopupPlasmaWindow();
    QQuickItem *visualParent() const;
    void setVisualParent(QQuickItem *parent);

    Qt::Edge popupDirection() const;
    void setPopupDirection(Qt::Edge popupDirection);

    bool floating() const;
    void setFloating(bool floating);

    bool animated();
    void setAnimated(bool animated);

    bool event(QEvent *event) override;

Q_SIGNALS:
    void visualParentChanged();
    void popupDirectionChanged();
    void floatingChanged();
    void animatedChanged();

private:
    void queuePositionUpdate();
    void updateSlideEffect();
    void updatePosition();
    void updatePositionX11(const QPoint &position);
    void updatePositionWayland(const QPoint &position);

    QPointer<QQuickItem> m_visualParent;
    bool m_needsReposition = false;
    bool m_floating = false;
    bool m_animated = false;
    Qt::Edge m_popupDirection = Qt::TopEdge;
};
}
