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
class LayoutChangedProxy;

/**
 * @brief The AppletPopup class shows a popup for an applet either in the panel or on the desktop
 *
 * In addition to the new API this class is resizable and can forward any input events recieved
 * on the margin to the main item
 *
 * Size hints are transferred from the mainItem's size hints.
 */
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

public:
    AppletPopup(QWindow *parent = nullptr);
    ~AppletPopup() override;
    QQuickItem *appletInterface() const;
    void setAppletInterface(QQuickItem *appletInterface);

    bool hideOnWindowDeactivate() const;
    void setHideOnWindowDeactivate(bool hideOnWindowDeactivate);

Q_SIGNALS:
    void appletInterfaceChanged();
    void hideOnWindowDeactivateChanged();

protected:
    void hideEvent(QHideEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    void onMainItemChanged();
    void updateMinSize();
    void updateMaxSize();
    void updateSize();

    QPointer<AppletQuickItem> m_appletInterface;
    bool m_hideOnWindowDeactivate = false;
    bool m_sizeExplicitlySetFromConfig = false;
    QScopedPointer<LayoutChangedProxy> m_layoutChangedProxy;
};

}
