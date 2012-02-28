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

#include "graphicsviewappletprivate_p.h"

#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>

#include "kwindowsystem.h"

#include "corona.h"
#include "dialog.h"
#include "paintutils.h"
#include "popupapplet.h"
#include "theme.h"
#include "tooltipmanager.h"
#include "widgets/busywidget.h"
#include "widgets/iconwidget.h"
#include "widgets/label.h"
#include "widgets/pushbutton.h"

namespace Plasma
{

GraphicsViewAppletPrivate::GraphicsViewAppletPrivate(KService::Ptr service, const KPluginInfo *info, int uniqueID, Applet *applet)
    : AppletPrivate(service, info, uniqueID, applet),
      messageOverlay(0),
      messageOverlayProxy(0),
      busyWidget(0)
{
    q->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    q->setAcceptsHoverEvents(true);
    q->setFlag(QGraphicsItem::ItemIsFocusable, true);
    q->setFocusPolicy(Qt::ClickFocus);
    // FIXME: adding here because nothing seems to be doing it in QGraphicsView,
    // but it doesn't actually work anyways =/
    q->setLayoutDirection(qApp->layoutDirection());
}

void GraphicsViewAppletPrivate::showMessage(const QIcon &icon, const QString &message, const MessageButtons buttons)
{
    if (message.isEmpty()) {
        destroyMessageOverlay();
        return;
    }

    Corona *corona = qobject_cast<Corona *>(q->scene());
    QGraphicsWidget *mainWidget = new QGraphicsWidget;

    QGraphicsLinearLayout *mainLayout = new QGraphicsLinearLayout(mainWidget);
    mainLayout->setOrientation(Qt::Vertical);
    mainLayout->addStretch();

    QGraphicsLinearLayout *messageLayout = new QGraphicsLinearLayout();
    messageLayout->setOrientation(Qt::Horizontal);

    QGraphicsLinearLayout *buttonLayout = new QGraphicsLinearLayout();
    buttonLayout->setOrientation(Qt::Horizontal);

    mainLayout->addItem(messageLayout);
    mainLayout->addItem(buttonLayout);
    mainLayout->addStretch();

    IconWidget *messageIcon = new IconWidget(mainWidget);
    Label *messageText = new Label(mainWidget);
    messageText->nativeWidget()->setWordWrap(true);

    messageLayout->addStretch();
    messageLayout->addItem(messageIcon);
    messageLayout->addItem(messageText);
    messageLayout->addStretch();

    messageIcon->setIcon(icon);
    messageText->setText(message);

    buttonLayout->addStretch();

    if (buttons & ButtonOk) {
        messageOkButton = new PushButton(mainWidget);
        messageOkButton.data()->setText(i18n("&OK"));
        messageOkButton.data()->setIcon(KIcon("dialog-ok"));
        buttonLayout->addItem(messageOkButton.data());
        QObject::connect(messageOkButton.data(), SIGNAL(clicked()), q, SLOT(destroyMessageOverlay()));
    }

    if (buttons & ButtonYes) {
        messageYesButton = new PushButton(mainWidget);
        messageYesButton.data()->setText(i18n("&Yes"));
        buttonLayout->addItem(messageYesButton.data());
        QObject::connect(messageYesButton.data(), SIGNAL(clicked()), q, SLOT(destroyMessageOverlay()));
    }

    if (buttons & ButtonNo) {
        messageNoButton = new PushButton(mainWidget);
        messageNoButton.data()->setText(i18n("&No"));
        buttonLayout->addItem(messageNoButton.data());
        QObject::connect(messageNoButton.data(), SIGNAL(clicked()), q, SLOT(destroyMessageOverlay()));
    }

    if (buttons & ButtonCancel) {
        messageCancelButton = new PushButton(mainWidget);
        messageCancelButton.data()->setText(i18n("&Cancel"));
        messageCancelButton.data()->setIcon(KIcon("dialog-cancel"));
        buttonLayout->addItem(messageCancelButton.data());
        QObject::connect(messageCancelButton.data(), SIGNAL(clicked()), q, SLOT(destroyMessageOverlay()));
    }

    messageCloseAction = new QAction(messageOverlay);
    messageCloseAction.data()->setShortcut(Qt::Key_Escape);
    mainWidget->addAction(messageCloseAction.data());
    QObject::connect(messageCloseAction.data(), SIGNAL(triggered()), q, SLOT(destroyMessageOverlay()));

    buttonLayout->addStretch();

    mainWidget->adjustSize();
    QSizeF hint = mainWidget->preferredSize();
    if (hint.height() > q->size().height() || hint.width() > q->size().width()) {
        // either a collapsed popup in h/v form factor or just too small,
        // so show it in a dialog associated with ourselves
        if (corona) {
            corona->addOffscreenWidget(mainWidget);
        }

        if (messageDialog) {
            delete messageDialog.data()->graphicsWidget();
        } else {
            messageDialog = new Plasma::Dialog;
        }

        ToolTipManager::self()->hide(q);
        KWindowSystem::setOnAllDesktops(messageDialog.data()->winId(), true);
        KWindowSystem::setState(messageDialog.data()->winId(), NET::SkipTaskbar | NET::SkipPager);
        messageDialog.data()->setGraphicsWidget(mainWidget);
        QObject::connect(messageDialog.data(), SIGNAL(destroyed(QObject*)), mainWidget, SLOT(deleteLater()));

        // if we are going to show it in a popup, then at least make sure it can be dismissed
        if (buttonLayout->count() < 1) {
            PushButton *ok = new PushButton(mainWidget);
            ok->setText(i18n("OK"));
            ok->setIcon(KIcon("dialog-ok"));
            buttonLayout->addItem(ok);
            QObject::connect(ok, SIGNAL(clicked()), q, SLOT(destroyMessageOverlay()));
        }
    } else {
        delete messageDialog.data();
        createMessageOverlay();
        messageOverlay->opacity = 0.8;
        mainWidget->setParentItem(messageOverlay);
        QGraphicsLinearLayout *l = new QGraphicsLinearLayout(messageOverlay);
        l->addItem(mainWidget);
    }

    if (messageDialog) {
        QPoint pos = q->geometry().topLeft().toPoint();
        if (corona) {
            pos = corona->popupPosition(q, messageDialog.data()->size());
        }

        messageDialog.data()->move(pos);
        messageDialog.data()->animatedShow(locationToDirection(q->location()));
    } else {
        messageOverlay->show();
    }
}

void GraphicsViewAppletPrivate::updateFailedToLaunch(const QString &reason)
{
    if (failed == failed) {
        if (failed && !reason.isEmpty()) {
            foreach (QGraphicsItem *item, q->QGraphicsItem::children()) {
                Label *l = dynamic_cast<Label *>(item);
                if (l) {
                    l->setText(visibleFailureText(reason));
                }
            }
        }
        return;
    }

    //FIXME: q->prepareGeometryChange();

    foreach (QGraphicsItem *item, q->childItems()) {
        if (!dynamic_cast<AppletHandle *>(item)) {
            delete item;
        }
    }

    messageOverlay = 0;
    if (messageDialog) {
        messageDialog.data()->deleteLater();
        messageDialog.clear();
    }

    q->setLayout(0);

    if (failed) {
        q->setBackgroundHints(StandardBackground);

        QGraphicsLinearLayout *failureLayout = new QGraphicsLinearLayout(q);
        failureLayout->setContentsMargins(0, 0, 0, 0);

        IconWidget *failureIcon = new IconWidget(q);
        failureIcon->setIcon(KIcon("dialog-error"));
        failureLayout->addItem(failureIcon);

        Label *failureWidget = new Plasma::Label(q);
        failureWidget->setText(visibleFailureText(reason));
        QLabel *label = failureWidget->nativeWidget();
        label->setWordWrap(true);
        failureLayout->addItem(failureWidget);

        Plasma::ToolTipManager::self()->registerWidget(failureIcon);
        Plasma::ToolTipContent data(i18n("Unable to load the widget"), reason,
                                    KIcon("dialog-error"));
        Plasma::ToolTipManager::self()->setContent(failureIcon, data);

        q->setLayout(failureLayout);
        q->resize(300, 250);
        background->resizeFrame(q->geometry().size());
    }

    q->update();
}

void GraphicsViewAppletPrivate::showConfigurationRequiredMessage(bool show, const QString &reason)
{
    if (!show) {
        destroyMessageOverlay();
        return;
    }

    createMessageOverlay(true);
    messageOverlay->opacity = 0.4;

    QGraphicsGridLayout *configLayout = new QGraphicsGridLayout(messageOverlay);
    configLayout->setContentsMargins(0, 0, 0, 0);

  //  configLayout->addStretch();
    configLayout->setColumnStretchFactor(0, 5);
    configLayout->setColumnStretchFactor(2, 5);
    configLayout->setRowStretchFactor(0, 5);
    configLayout->setRowStretchFactor(3, 5);

    int row = 1;
    if (!reason.isEmpty()) {
        Label *explanation = new Label(messageOverlay);
        explanation->setText(reason);
        configLayout->addItem(explanation, row, 1);
        configLayout->setColumnStretchFactor(1, 5);
        ++row;
        configLayout->setAlignment(explanation, Qt::AlignBottom | Qt::AlignCenter);
    }

    PushButton *configWidget = new PushButton(messageOverlay);
    QObject::connect(configWidget, SIGNAL(clicked()), q, SLOT(showConfigurationInterface()));

    const FormFactor formFactor = q->formFactor();
    if (!qobject_cast<Plasma::PopupApplet *>(q) && (formFactor == Plasma::Horizontal || formFactor == Plasma::Vertical)) {
        configWidget->setImage("widgets/configuration-icons", "configure");
        configWidget->setMaximumSize(24,24);
        configWidget->setMinimumSize(24,24);
    } else {
        configWidget->setText(i18n("Configure..."));
    }

    configLayout->addItem(configWidget, row, 1);

    //configLayout->setAlignment(configWidget, Qt::AlignTop | Qt::AlignCenter);
    //configLayout->addStretch();

    messageOverlay->show();
}


void GraphicsViewAppletPrivate::createMessageOverlay(bool usePopup)
{
    if (messageOverlay) {
        qDeleteAll(messageOverlay->children());
        messageOverlay->setLayout(0);
    }

    PopupApplet *popup = qobject_cast<Plasma::PopupApplet*>(q);

    if (!messageOverlay) {
        if (usePopup && popup) {
            if (popup->widget()) {
                messageOverlayProxy = new QGraphicsProxyWidget(q);
                messageOverlayProxy->setWidget(popup->widget());
                messageOverlay = new AppletOverlayWidget(messageOverlayProxy);
            } else if (popup->graphicsWidget()) {
                messageOverlay = new AppletOverlayWidget(popup->graphicsWidget());
            }
        }

        if (!messageOverlay) {
            messageOverlay = new AppletOverlayWidget(q);
        }
    }

    positionMessageOverlay();
}

void GraphicsViewAppletPrivate::positionMessageOverlay()
{
    if (!messageOverlay) {
        return;
    }

    PopupApplet *popup = qobject_cast<Plasma::PopupApplet*>(q);
    const bool usePopup = popup && (messageOverlay->parentItem() != q);
    QGraphicsItem *topItem = q;

    if (usePopup && popup->widget()) {
        // popupapplet with widget()
        //FIXME:topItem = popup->d->proxy.data();
        messageOverlay->setGeometry(popup->widget()->contentsRect());
    } else if (usePopup && popup->graphicsWidget()) {
        // popupapplet with graphicsWidget()
        topItem = popup->graphicsWidget();
        QGraphicsWidget *w = dynamic_cast<QGraphicsWidget *>(topItem);
        messageOverlay->setGeometry(w ? w->contentsRect() : topItem->boundingRect());
    } else {
        // normal applet
        messageOverlay->setGeometry(q->contentsRect());
    }

    // raise the overlay above all the other children!
    int zValue = 100;
    foreach (QGraphicsItem *child, topItem->children()) {
        if (child->zValue() > zValue) {
            zValue = child->zValue() + 1;
        }
    }
    messageOverlay->setZValue(zValue);
}

void GraphicsViewAppletPrivate::destroyMessageOverlay()
{
    if (messageDialog) {
        messageDialog.data()->animatedHide(Plasma::locationToInverseDirection(q->location()));
        //messageDialog.data()->deleteLater();
        messageDialog.clear();
    }

    if (!messageOverlay) {
        return;
    }

    messageOverlay->destroy();
    messageOverlay = 0;

    if (messageOverlayProxy) {
        messageOverlayProxy->setWidget(0);
        delete messageOverlayProxy;
        messageOverlayProxy = 0;
    }

    MessageButton buttonCode = ButtonNo;
    //find out if we're disappearing because of a button press
    PushButton *button = 0;
    //FIXME: PushButton *button = qobject_cast<PushButton *>(q->sender());
    if (button) {
        if (button == messageOkButton.data()) {
            buttonCode = ButtonOk;
        }

        if (button == messageYesButton.data()) {
            buttonCode = ButtonYes;
        }

        if (button == messageNoButton.data()) {
            buttonCode = ButtonNo;
        }

        if (button == messageCancelButton.data()) {
            buttonCode = ButtonCancel;
        }

        //FIXME: emit q->messageButtonPressed(buttonCode);
    } else if (/* FIXME: q->sender() == messageOverlay */ false) {
        //FIXME: emit q->messageButtonPressed(ButtonCancel);
    }
}

void GraphicsViewAppletPrivate::setBusy(bool busy)
{
    if (busy) {
        if (!busyWidget && !busyWidgetTimer.isActive()) {
            busyWidgetTimer.start(500, this);
        }
    } else {
        busyWidgetTimer.stop();
        if (busyWidget) {
            busyWidget = 0;
            destroyMessageOverlay();
        }
    }
}

bool GraphicsViewAppletPrivate::isBusy() const
{
    return busyWidgetTimer.isActive() || (busyWidget && busyWidget->isVisible());
}

void GraphicsViewAppletPrivate::timerEvent(QTimerEvent *event)
{
    if (transient) {
        busyWidgetTimer.stop();
        return;
    }

    if (event->timerId() == busyWidgetTimer.timerId()) {
        if (!busyWidget) {
            createMessageOverlay(false);
            messageOverlay->opacity = 0;

            QGraphicsLinearLayout *mainLayout = new QGraphicsLinearLayout(messageOverlay);
            busyWidget = new Plasma::BusyWidget(messageOverlay);
            busyWidget->setAcceptHoverEvents(false);
            busyWidget->setAcceptedMouseButtons(Qt::NoButton);
            messageOverlay->setAcceptHoverEvents(false);
            messageOverlay->setAcceptedMouseButtons(Qt::NoButton);

            mainLayout->addStretch();
            mainLayout->addItem(busyWidget);
            mainLayout->addStretch();
        }
    }
}

AppletOverlayWidget::AppletOverlayWidget(QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      opacity(0.4)
{
    resize(parent->size());
}

void AppletOverlayWidget::destroy()
{
    /*FIXME: a transitional animation would be nice here .. we used to do this:
    Animation *anim = Plasma::Animator::create(Plasma::Animator::DisappearAnimation);
    if (anim) {
        connect(anim, SIGNAL(finished()), this, SLOT(overlayAnimationComplete()));
        anim->setTargetWidget(this);
        anim->start();
    } else {
        overlayAnimationComplete();
    }
    */
    if (scene()) {
        scene()->removeItem(this);
    }
    deleteLater();
}

void AppletOverlayWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
}

void AppletOverlayWidget::paint(QPainter *painter,
                                const QStyleOptionGraphicsItem *option,
                                QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (qFuzzyCompare(1, 1+opacity)) {
        return;
    }

    QColor wash = Plasma::Theme::defaultTheme()->color(Theme::BackgroundColor);
    wash.setAlphaF(opacity);

    Applet *applet = qobject_cast<Applet *>(parentWidget());


    QPainterPath backgroundShape;
    if (!applet || applet->backgroundHints() == StandardBackground) {
        //FIXME: a resize here is nasty, but perhaps still better than an eventfilter just for that..
        if (parentWidget()->contentsRect().size() != size()) {
            resize(parentWidget()->contentsRect().size());
        }
        backgroundShape = PaintUtils::roundedRectangle(contentsRect(), 5);
    } else {
        backgroundShape = shape();
    }

    painter->setRenderHints(QPainter::Antialiasing);
    painter->fillPath(backgroundShape, wash);
}

} // namespace Plasma

#include "graphicsviewappletprivate_p.h"

