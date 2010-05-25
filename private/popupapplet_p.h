/*
 * Copyright 2008 by Montel Laurent <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#ifndef POPUPAPPLET_P_H
#define POPUPAPPLET_P_H

#include <QWeakPointer>

#include <plasma/plasma.h>

namespace Plasma
{

class PopupAppletPrivate
{
public:
    PopupAppletPrivate(PopupApplet *applet);
    ~PopupAppletPrivate();

    void iconSizeChanged(int group);
    void internalTogglePopup();
    void hideTimedPopup();
    void clearPopupLostFocus();
    void dialogSizeChanged();
    void dialogStatusChanged(bool status);
    void restoreDialogSize();
    void updateDialogPosition();
    void popupConstraintsEvent(Plasma::Constraints constraints);
    void checkExtenderAppearance(Plasma::FormFactor f);
    KConfigGroup popupConfigGroup();
    void appletActivated();

    PopupApplet *q;
    Plasma::IconWidget *icon;
    QWeakPointer<Plasma::Dialog> dialogPtr;
    QWeakPointer<QGraphicsProxyWidget>proxy;
    QWidget *widget;
    QGraphicsWidget *graphicsWidget;
    Plasma::PopupPlacement popupPlacement;
    Plasma::AspectRatioMode savedAspectRatio;
    QTimer *timer;
    QPoint clicked;
    bool popupLostFocus : 1;
    bool passive : 1;
};

} // Plasma namespace

#endif

