/*
 *   Copyright 2011 by Aaron Seigo <aseigo@kde.org>
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

#ifndef PLASMA_GRAPHICSVIEWAPPLETPRIVATE_H
#define PLASMA_GRAPHICSVIEWAPPLETPRIVATE_H

#include "private/applet_p.h"

namespace Plasma
{

class AppletOverlayWidget : public QGraphicsWidget
{
    Q_OBJECT

public:
    AppletOverlayWidget(QGraphicsWidget *parent);
    void destroy();

    qreal opacity;

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
};

class GraphicsViewAppletPrivate : public QObject, public AppletPrivate
{
    Q_OBJECT

public:
    GraphicsViewAppletPrivate(KService::Ptr service, const KPluginInfo *info, int uniqueID, Applet *applet);
    void cleanUpAndDelete();
    void showConfigurationRequiredMessage(bool show, const QString &reason);
    void showMessage(const QIcon &icon, const QString &message, const MessageButtons buttons);
    void positionMessageOverlay();
    void setBusy(bool busy);
    bool isBusy() const;
    void updateFailedToLaunch(const QString &reason);

protected:
    void timerEvent(QTimerEvent *event);

private:
    void createMessageOverlay(bool usePopup = true);

private Q_SLOTS:
    void destroyMessageOverlay();

private:
    // overlays and messages
    QWeakPointer<Plasma::Dialog> messageDialog;
    AppletOverlayWidget *messageOverlay;
    QGraphicsProxyWidget *messageOverlayProxy;
    Plasma::BusyWidget *busyWidget;
    QWeakPointer<Plasma::PushButton> messageOkButton;
    QWeakPointer<Plasma::PushButton> messageYesButton;
    QWeakPointer<Plasma::PushButton> messageNoButton;
    QWeakPointer<Plasma::PushButton> messageCancelButton;
    QWeakPointer<QAction> messageCloseAction;
};

} // namespace Plasma

#endif

