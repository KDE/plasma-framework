/***************************************************************************
 *   Copyright (C) 2008 by Montel Laurent <montel@kde.org>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#include "popupapplet.h"

#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
#include <QVBoxLayout>
#include <QTimer>

#include <KIcon>
#include <KIconLoader>
#include <KWindowSystem>

#include <plasma/dialog.h>
#include <plasma/corona.h>
#include <plasma/containment.h>
#include <plasma/extender.h>
#include <plasma/widgets/icon.h>

namespace Plasma
{

class PopupAppletPrivate
{
public:
    PopupAppletPrivate(PopupApplet *applet)
        : q(applet),
          icon(0),
          dialog(0),
          layout(0),
          proxy(0),
          savedAspectRatio(Plasma::InvalidAspectRatioMode),
          timer(0)
    {
    }

    ~PopupAppletPrivate()
    {
        if (proxy) {
            proxy->setWidget(0);
        }

        delete dialog;
        delete icon;
    }

    void togglePopup();
    void hideTimedPopup();
    void dialogSizeChanged();
    void dialogStatusChanged(bool status);

    PopupApplet *q;
    Plasma::Icon *icon;
    Plasma::Dialog *dialog;
    QGraphicsLinearLayout *layout;
    QGraphicsProxyWidget *proxy;
    Plasma::AspectRatioMode savedAspectRatio;
    QTimer *timer;
};

PopupApplet::PopupApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      d(new PopupAppletPrivate(this))
{
    int iconSize = IconSize(KIconLoader::Desktop);
    resize(iconSize, iconSize);
}

PopupApplet::~PopupApplet()
{
    delete d;
}

void PopupApplet::setIcon(const QIcon &icon)
{
    if (!d->icon) {
        d->icon = new Plasma::Icon(icon, QString(), this);
    } else {
        d->icon->setIcon(icon);
    }
}

void PopupApplet::setIcon(const QString &iconName)
{
    if (!d->icon) {
        d->icon = new Plasma::Icon(KIcon(iconName), QString(), this);
    } else {
        d->icon->setIcon(iconName);
    }
}

QIcon PopupApplet::icon() const
{
    return d->icon->icon();
}

QWidget *PopupApplet::widget()
{
    return 0;
}

QGraphicsWidget *PopupApplet::graphicsWidget()
{
    return 0;
}

void PopupApplet::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::StartupCompletedConstraint) {
        if (!d->icon) {
            d->icon = new Plasma::Icon(KIcon("icons"), QString(), this);
        }

        d->layout = new QGraphicsLinearLayout(this);
        d->layout->setContentsMargins(0, 0, 0, 0);
        d->layout->setSpacing(0);
        d->layout->setOrientation(Qt::Horizontal);
        setAspectRatioMode(Plasma::ConstrainedSquare);
        setLayout(d->layout);

        connect(d->icon, SIGNAL(clicked()), this, SLOT(togglePopup()));
    }

    //since we call this function when an extender's geometry gets updated, we want to avoid doing
    //anything if the StartupCompletedConstraint hasn't been called yet.
    if (!d->layout) {
        return;
    }

    if (constraints & Plasma::FormFactorConstraint) {
        d->layout->removeAt(0);

        switch (formFactor()) {
        case Plasma::Planar:
        case Plasma::MediaCenter: {
            if (d->icon) {
                d->icon->hide();
            }

            if (d->savedAspectRatio != Plasma::InvalidAspectRatioMode) {
                setAspectRatioMode(d->savedAspectRatio);
            }

            if (d->dialog) {
                if (d->dialog->layout() && widget()) {
                    //we dont want to delete Widget inside the dialog layout
                    d->dialog->layout()->removeWidget(widget());
                }

                delete d->dialog;
                d->dialog = 0;
            }

            //get the margins
            QSizeF marginSize = size() - contentsRect().size();

            if (graphicsWidget()) {
                d->layout->addItem(graphicsWidget());
                setMinimumSize(graphicsWidget()->minimumSize() + marginSize);
                graphicsWidget()->installEventFilter(this);
            }
            else {
                if (!d->proxy) {
                    d->proxy = new QGraphicsProxyWidget(this);
                    d->proxy->setWidget(widget());
                    d->proxy->show();
                }

                d->layout->addItem(d->proxy);
                setMinimumSize(widget() ? widget()->minimumSize() + marginSize : QSizeF(300, 200));
            }

            break;
        }
        case Plasma::Horizontal:
        case Plasma::Vertical:
            //save the aspect ratio mode in case we drag'n drop in the Desktop later
            d->savedAspectRatio = aspectRatioMode();
            setAspectRatioMode(Plasma::ConstrainedSquare);

            if (d->icon) {
                d->icon->show();
            }

            if (d->proxy) {
                d->proxy->setWidget(0); // prevent it from deleting our widget!
                delete d->proxy;
                d->proxy = 0;
            }

            if (!d->dialog) {
                d->dialog = new Plasma::Dialog();

                //no longer use Qt::Popup since that seems to cause a lot of problem when you drag
                //stuff out of your Dialog (extenders). Monitor WindowDeactivate events so we can
                //emulate the same kind of behavior as Qt::Popup (close when you click somewhere
                //else.
                d->dialog->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
                KWindowSystem::setState(d->dialog->winId(), NET::SkipTaskbar | NET::SkipPager);
                d->dialog->installEventFilter(this);

                connect(d->dialog, SIGNAL(dialogResized()), this, SLOT(dialogSizeChanged()));
                connect(d->dialog, SIGNAL(dialogVisible(bool)), this , SLOT(dialogStatusChanged(bool)));
                if (graphicsWidget()) {
                    Corona *corona = qobject_cast<Corona *>(graphicsWidget()->scene());

                    //could that cast ever fail??
                    if (corona) {
                        corona->addOffscreenWidget(graphicsWidget());
                        graphicsWidget()->resize(graphicsWidget()->preferredSize());
                        graphicsWidget()->setMinimumSize(graphicsWidget()->preferredSize());
                        d->dialog->setGraphicsWidget(graphicsWidget());
                    }
                } else {
                    QVBoxLayout *l_layout = new QVBoxLayout(d->dialog);
                    l_layout->setSpacing(0);
                    l_layout->setMargin(0);
                    l_layout->addWidget(widget());
                }
            }

            d->dialog->adjustSize();
            d->layout->addItem(d->icon);

            break;
        }
    }
}

bool PopupApplet::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d->dialog && (event->type() == QEvent::WindowDeactivate)) {
        hidePopup();
    }

    if (watched == graphicsWidget() && (event->type() == QEvent::GraphicsSceneResize)) {
        //sizes are recalculated in the constraintsevent so let's just call that.
        constraintsEvent(Plasma::FormFactorConstraint);

        //resize vertically if necesarry.
        if (formFactor() == Plasma::MediaCenter || formFactor() == Plasma::Planar) {
            resize(QSizeF(size().width(), minimumHeight()));
        }
    }

    return Applet::eventFilter(watched, event);
}

QVariant PopupApplet::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemParentHasChanged && d->dialog) {
        d->dialog->setParent(view());
    }

    return Applet::itemChange(change, value);
}

void PopupApplet::showPopup(uint popupDuration)
{
    if (d->dialog && (formFactor() == Horizontal || formFactor() == Vertical)) {
        d->dialog->move(popupPosition(d->dialog->size()));
        d->dialog->show();
        KWindowSystem::setState(d->dialog->winId(), NET::SkipTaskbar | NET::SkipPager);

        if (d->timer) {
            d->timer->stop();
        }

        if (popupDuration > 0) {
            if (!d->timer) {
                d->timer = new QTimer(this);
                connect(d->timer, SIGNAL(timeout()), this, SLOT(hideTimedPopup()));
            }

            d->timer->start(popupDuration);
        }
    }
}

void PopupApplet::hidePopup()
{
    if (d->dialog && (formFactor() == Horizontal || formFactor() == Vertical)) {
        d->dialog->hide();
    }
}

void PopupApplet::popupEvent(bool)
{

}

void PopupAppletPrivate::togglePopup()
{
   if (dialog) {
       if (timer) {
           timer->stop();
       }

       dialog->move(q->popupPosition(dialog->size()));

       if (dialog->isVisible()) {
           dialog->hide();
       } else {
           dialog->show();
           KWindowSystem::setState(dialog->winId(), NET::SkipTaskbar | NET::SkipPager);
       }

       dialog->clearFocus();
    }
}

void PopupAppletPrivate::hideTimedPopup()
{
    timer->stop();
    q->hidePopup();
}

void PopupAppletPrivate::dialogSizeChanged()
{
    //Reposition the dialog.
    if (dialog) {
        dialog->updateGeometry();
        dialog->move(q->popupPosition(dialog->size()));
    }
}

void PopupAppletPrivate::dialogStatusChanged(bool status)
{
    q->popupEvent(status);
}

} // Plasma namespace

#include "popupapplet.moc"

