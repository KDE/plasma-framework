/*
    SPDX-FileCopyrightText: 2023 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "popupplasmawindow.h"

#include <QQuickItem>

#include <plasmaquick/plasmaquick_export.h>

namespace PlasmaQuick
{

class AppletQuickItem;

class PLASMAQUICK_EXPORT AppletPopup : public PopupPlasmaWindow
{
    Q_OBJECT
    /**
     * This property holds a pointer to the AppletInterface used by
     */
    Q_PROPERTY(QQuickItem *appletInterface READ appletInterface WRITE setAppletInterface NOTIFY appletInterfaceChanged)

    /**
     * Whether the dialog should be hidden when the dialog loses focus.
     *
     * The default value is @c false.
     **/
    Q_PROPERTY(bool hideOnWindowDeactivate READ hideOnWindowDeactivate WRITE setHideOnWindowDeactivate NOTIFY hideOnWindowDeactivateChanged)

    /**
     * Sets an implicitWidth for the window
     *
     * The window width may be modified by external factors, the implicitWidth remains the value set by the caller
     */
    Q_PROPERTY(int implicitWidth READ implicitWidth WRITE setImplicitWidth NOTIFY implicitWidthChanged)
    /**
     * Sets an implicitHeight for the window
     *
     * The window height may be modified by external factors, the implicitHeight remains the value set by the caller
     */
    Q_PROPERTY(int implicitHeight READ implicitHeight WRITE setImplicitHeight NOTIFY implicitHeightChanged)

public:
    AppletPopup();
    QQuickItem *appletInterface() const;
    void setAppletInterface(QQuickItem *appletInterface);

    bool hideOnWindowDeactivate() const;
    void setHideOnWindowDeactivate(bool hideOnWindowDeactivate);

    int implicitWidth() const;
    void setImplicitWidth(int implicitWidth);

    int implicitHeight() const;
    void setImplicitHeight(int implicitHeight);

Q_SIGNALS:
    void appletInterfaceChanged();
    void hideOnWindowDeactivateChanged();
    void implicitWidthChanged();
    void implicitHeightChanged();

protected:
    void hideEvent(QHideEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    void onMainItemChanged();
    void updateSize();

    QPointer<AppletQuickItem> m_appletInterface;
    int m_implicitWidth = 500;
    int m_implicitHeight = 500;
    bool m_hideOnWindowDeactivate = false;
};

}
