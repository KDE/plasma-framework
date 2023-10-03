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

    /**
     * The anchor item to place the popup relative to.
     */
    Q_PROPERTY(QQuickItem *visualParent READ visualParent WRITE setVisualParent NOTIFY visualParentChanged)

    /**
     * Defines the default direction to place the popup relative to the visualParent.
     */
    Q_PROPERTY(Qt::Edge popupDirection READ popupDirection WRITE setPopupDirection NOTIFY popupDirectionChanged)

    /**
     * Defines whether the popup can appaer (float) over the parent window. The default is false.
     */
    Q_PROPERTY(bool floating READ floating WRITE setFloating NOTIFY floatingChanged)

    /**
     * Defines whether the popup is animated on show and close. The default is false.
     */
    Q_PROPERTY(bool animated READ animated WRITE setAnimated NOTIFY animatedChanged)

    /**
     * Defines which borders should be enabled/disabled when the popup is shown.
     */
    Q_PROPERTY(RemoveBorders removeBorderStrategy READ removeBorderStrategy WRITE setRemoveBorderStrategy NOTIFY removeBorderStrategyChanged)

public:
    enum RemoveBorder { Never = 0x0, AtScreenEdges = 0x1, AtPanelEdges = 0x2 };
    Q_DECLARE_FLAGS(RemoveBorders, RemoveBorder)
    Q_ENUM(RemoveBorder);

    PopupPlasmaWindow();
    QQuickItem *visualParent() const;
    void setVisualParent(QQuickItem *parent);

    Qt::Edge popupDirection() const;
    void setPopupDirection(Qt::Edge popupDirection);

    bool floating() const;
    void setFloating(bool floating);

    bool animated();
    void setAnimated(bool animated);

    RemoveBorders removeBorderStrategy() const;
    void setRemoveBorderStrategy(RemoveBorders borders);

    bool event(QEvent *event) override;

Q_SIGNALS:
    void visualParentChanged();
    void popupDirectionChanged();
    void floatingChanged();
    void animatedChanged();
    void removeBorderStrategyChanged();

private:
    void queuePositionUpdate();
    void updateSlideEffect();
    void updatePosition();
    void updatePositionX11(const QPoint &position);
    void updatePositionWayland(const QPoint &position);
    void updateBorders(const QRect &globalPosition);

    QPointer<QQuickItem> m_visualParent;
    RemoveBorders m_removeBorderStrategy = Never;
    bool m_needsReposition = false;
    bool m_floating = false;
    bool m_animated = false;
    Qt::Edge m_popupDirection = Qt::TopEdge;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(PopupPlasmaWindow::RemoveBorders)
}
