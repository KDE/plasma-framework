/*
 * Copyright 2008 by Montel Laurent <montel@kde.org>
 * Copyright 2008 by Marco Martin <notmart@gmail.com>
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

#include "popupapplet.h"

#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QApplication>
#include <QDesktopWidget>

#include <KIcon>
#include <KIconLoader>
#include <KWindowSystem>
#include <KGlobalSettings>

#include "plasma/private/applet_p.h"
#include "plasma/dialog.h"
#include "plasma/corona.h"
#include "plasma/containment.h"
#include "plasma/extender.h"
#include "plasma/widgets/icon.h"

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
          popupPlacement(Plasma::FloatingPopup),
          savedAspectRatio(Plasma::InvalidAspectRatioMode),
          timer(0),
          startupComplete(false),
          popupLostFocus(false)
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
    void clearPopupLostFocus();
    void dialogSizeChanged();
    void dialogStatusChanged(bool status);
    void updateDialogPosition();

    PopupApplet *q;
    Plasma::Icon *icon;
    Plasma::Dialog *dialog;
    QGraphicsLinearLayout *layout;
    QGraphicsProxyWidget *proxy;
    Plasma::PopupPlacement popupPlacement;
    Plasma::AspectRatioMode savedAspectRatio;
    QTimer *timer;
    QPoint clicked;
    bool startupComplete : 1;
    bool popupLostFocus : 1;
};

PopupApplet::PopupApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      d(new PopupAppletPrivate(this))
{
    int iconSize = IconSize(KIconLoader::Desktop);
    resize(iconSize, iconSize);
    if (!icon().isNull()) {
        setPopupIcon(KIcon(icon()));
    } else {
        setPopupIcon(KIcon("icons"));
    }

    connect(this, SIGNAL(activate()), this, SLOT(togglePopup()));
}

PopupApplet::~PopupApplet()
{
    delete d;
}

void PopupApplet::setPopupIcon(const QIcon &icon)
{
    if (icon.isNull()) {
        if (d->icon) {
            delete d->icon;
            d->icon = 0;
            setLayout(0);
        }

        return;
    }

    if (!d->icon) {
        d->icon = new Plasma::Icon(icon, QString(), this);
        connect(d->icon, SIGNAL(clicked()), this, SLOT(togglePopup()));

        QGraphicsLinearLayout *layout = new QGraphicsLinearLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        layout->setOrientation(Qt::Horizontal);
        setAspectRatioMode(Plasma::ConstrainedSquare);
        setLayout(layout);
    } else {
        d->icon->setIcon(icon);
    }
}

void PopupApplet::setPopupIcon(const QString &iconName)
{
    setPopupIcon(KIcon(iconName));
}

QIcon PopupApplet::popupIcon() const
{
    return d->icon ? d->icon->icon() : QIcon();
}

QWidget *PopupApplet::widget()
{
    return 0;
}

QGraphicsWidget *PopupApplet::graphicsWidget()
{
    return static_cast<Applet*>(this)->d->extender;
}

void PopupApplet::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::StartupCompletedConstraint) {
        d->startupComplete = true;
    }

    if (!d->startupComplete) {
        return;
    }

    QGraphicsLinearLayout *lay = dynamic_cast<QGraphicsLinearLayout *>(layout());
    Plasma::FormFactor f = formFactor();

    if (constraints & Plasma::FormFactorConstraint ||
        (constraints & Plasma::SizeConstraint && (f == Plasma::Vertical || f == Plasma::Horizontal))) {
        if (lay) {
            lay->removeAt(0);
        }

        QSizeF minimum;
        QSizeF containmentSize;

        QGraphicsWidget *gWidget = graphicsWidget();
        QWidget *qWidget = widget();

        if (gWidget) {
            minimum = gWidget->minimumSize();
        } else if (qWidget) {
            minimum = qWidget->minimumSizeHint();
        }

        if (containment()) {
            containmentSize = containment()->size();
        }

        if (d->icon &&
            ((f != Plasma::Vertical && f != Plasma::Horizontal) ||
             ((f == Plasma::Vertical && containmentSize.width() >= minimum.width()) ||(f == Plasma::Horizontal && containmentSize.height() >= minimum.height())))) {
            // we only switch to expanded if we aren't horiz/vert constrained and
            // this applet has an icon.
            // otherwise, we leave it up to the applet itself to figure it out
            d->icon->hide();

            if (d->savedAspectRatio != Plasma::InvalidAspectRatioMode) {
                setAspectRatioMode(d->savedAspectRatio);
            }

            if (d->dialog) {
                if (d->dialog->layout() && qWidget) {
                    //we dont want to delete Widget inside the dialog layout
                    d->dialog->layout()->removeWidget(qWidget);
                }

                delete d->dialog;
                d->dialog = 0;
            }

            //get the margins
            QSizeF marginSize = size() - contentsRect().size();

            if (gWidget) {
                if (lay) {
                    lay->addItem(gWidget);
                }
                setMinimumSize(gWidget->minimumSize() + marginSize);
                gWidget->installEventFilter(this);
            } else if (qWidget) {
                if (!d->proxy) {
                    d->proxy = new QGraphicsProxyWidget(this);
                    d->proxy->setWidget(qWidget);
                    d->proxy->show();
                }

                if (lay) {
                    lay->addItem(d->proxy);
                }

                setMinimumSize(qWidget ? qWidget->minimumSize() + marginSize : QSizeF(300, 200));
            }
        } else {
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
                kDebug() << "making dialog with view" << view();
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
                setMinimumSize(QSize(0, 0));
                if (gWidget) {
                    Corona *corona = qobject_cast<Corona *>(gWidget->scene());

                    //could that cast ever fail??
                    if (corona) {
                        corona->addOffscreenWidget(gWidget);
                        graphicsWidget()->resize(gWidget->preferredSize());
                        graphicsWidget()->setMinimumSize(gWidget->preferredSize());
                        d->dialog->setGraphicsWidget(gWidget);
                    }
                } else if (qWidget) {
                    QVBoxLayout *l_layout = new QVBoxLayout(d->dialog);
                    l_layout->setSpacing(0);
                    l_layout->setMargin(0);
                    l_layout->addWidget(qWidget);
                }
            }

            d->dialog->adjustSize();

            if (d->icon && lay) {
                lay->addItem(d->icon);
            }
        }
    }
}

void PopupApplet::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!d->icon && !d->popupLostFocus && event->buttons() == Qt::LeftButton) {
        d->clicked = scenePos().toPoint();
        event->setAccepted(true);
        return;
    } else {
        d->popupLostFocus = false;
        Applet::mousePressEvent(event);
    }
}

void PopupApplet::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!d->icon && (d->clicked - scenePos().toPoint()).manhattanLength() < KGlobalSettings::dndEventDelay()) {
        d->togglePopup();
    } else {
        Applet::mouseReleaseEvent(event);
    }
}

bool PopupApplet::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d->dialog && (event->type() == QEvent::WindowDeactivate)) {
        d->popupLostFocus = true;
        hidePopup();
        QTimer::singleShot(100, this, SLOT(clearPopupLostFocus()));
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
        d->updateDialogPosition();
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

Plasma::PopupPlacement PopupApplet::popupPlacement() const
{
    return d->popupPlacement;
}

void PopupApplet::popupEvent(bool)
{

}

void PopupAppletPrivate::togglePopup()
{
    kDebug();
    if (dialog) {
        if (timer) {
            timer->stop();
        }

        if (dialog->isVisible()) {
            dialog->hide();
        } else {
            updateDialogPosition();
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

void PopupAppletPrivate::clearPopupLostFocus()
{
    popupLostFocus = false;
}

void PopupAppletPrivate::dialogSizeChanged()
{
    //Reposition the dialog
    if (dialog) {
        dialog->updateGeometry();
        dialog->move(q->popupPosition(dialog->size()));

        KConfigGroup sizeGroup = q->config();
        sizeGroup = KConfigGroup(&sizeGroup, "PopupApplet");
        sizeGroup.writeEntry("DialogHeight", dialog->height());
        sizeGroup.writeEntry("DialogWidth", dialog->width());

        emit q->configNeedsSaving();
    }
}

void PopupAppletPrivate::dialogStatusChanged(bool status)
{
    q->popupEvent(status);
}

void PopupAppletPrivate::updateDialogPosition()
{
    KConfigGroup sizeGroup = q->config();
    sizeGroup = KConfigGroup(&sizeGroup, "PopupApplet");
    const int width = qMin(sizeGroup.readEntry("DialogWidth", 0), QApplication::desktop()->screen()->width() - 50);
    const int height = qMin(sizeGroup.readEntry("DialogHeight", 0), QApplication::desktop()->screen()->height() - 50);

    QSize saved(width, height);

    if (saved.isNull()) {
        dialog->adjustSize();
    } else {
        saved = saved.expandedTo(dialog->minimumSizeHint());
        dialog->resize(saved);
    }

    QSize s = dialog->size();
    QPoint pos = q->view()->mapFromScene(q->scenePos());
    pos = q->view()->mapToGlobal(pos);

    switch (q->location()) {
    case BottomEdge:
        pos = QPoint(pos.x(), pos.y() - s.height());
        popupPlacement = Plasma::TopPosedLeftAlignedPopup;
        dialog->setResizeHandleCorners(Dialog::NorthEast);

        break;
    case TopEdge:
        pos = QPoint(pos.x(), pos.y() + (int)q->boundingRect().size().height());
        popupPlacement = Plasma::BottomPosedLeftAlignedPopup;
        dialog->setResizeHandleCorners(Dialog::SouthEast);

        break;
    case LeftEdge:
        pos = QPoint(pos.x() + (int)q->boundingRect().size().width(), pos.y());
        popupPlacement = Plasma::RightPosedTopAlignedPopup;
        dialog->setResizeHandleCorners(Dialog::SouthEast);

        break;

    case RightEdge:
        pos = QPoint(pos.x() - s.width(), pos.y());
        popupPlacement = Plasma::LeftPosedTopAlignedPopup;
        dialog->setResizeHandleCorners(Dialog::SouthWest);

        break;
    default:
        if (pos.y() - s.height() > 0) {
            pos = QPoint(pos.x(), pos.y() - s.height());
        } else {
            pos = QPoint(pos.x(), pos.y() + (int)q->boundingRect().size().height());
        }

        dialog->setResizeHandleCorners(Dialog::NorthEast);
    }
    //are we out of screen?

    QRect screenRect = QApplication::desktop()->screenGeometry(q->containment() ? q->containment()->screen() : -1);
    //kDebug() << "==> rect for" << (containment() ? containment()->screen() : -1) << "is" << screenRect;

    if (pos.rx() + s.width() > screenRect.right()) {
        pos.rx() += (int)q->boundingRect().size().width() - s.width();

        if (q->location() == BottomEdge) {
            popupPlacement = Plasma::TopPosedRightAlignedPopup;
            dialog->setResizeHandleCorners(Dialog::NorthWest);
        } else if (q->location() == TopEdge) {
            popupPlacement = Plasma::BottomPosedRightAlignedPopup;
            dialog->setResizeHandleCorners(Dialog::SouthWest);
        }
    }

    if (pos.ry() + s.height() > screenRect.bottom()) {
        pos.ry() += (int)q->boundingRect().size().height() - s.height();

        if (q->location() == LeftEdge) {
            popupPlacement = Plasma::RightPosedBottomAlignedPopup;
            dialog->setResizeHandleCorners(Dialog::NorthEast);
        } else if (q->location() == RightEdge) {
            popupPlacement = Plasma::LeftPosedBottomAlignedPopup;
            dialog->setResizeHandleCorners(Dialog::NorthWest);
        }
    }

    pos.rx() = qMax(0, pos.rx());

    dialog->move(pos);
}
} // Plasma namespace

#include "popupapplet.moc"

