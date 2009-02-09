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
#include "private/popupapplet_p.h"

#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
#include <QVBoxLayout>
#include <QTimer>
#include <QApplication>

#include <kicon.h>
#include <kiconloader.h>
#include <kwindowsystem.h>
#include <kglobalsettings.h>

#include "plasma/private/applet_p.h"
#include "plasma/corona.h"
#include "plasma/containment.h"
#include "plasma/dialog.h"
#include "plasma/extender.h"
#include "plasma/tooltipmanager.h"
#include "plasma/widgets/iconwidget.h"

namespace Plasma
{

PopupApplet::PopupApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      d(new PopupAppletPrivate(this))
{
    int iconSize = IconSize(KIconLoader::Desktop);
    resize(iconSize, iconSize);
    connect(this, SIGNAL(activate()), this, SLOT(internalTogglePopup()));
}

PopupApplet::~PopupApplet()
{
    delete widget();
    delete d;
}

void PopupApplet::setPopupIcon(const QIcon &icon)
{
    if (icon.isNull()) {
        if (d->icon) {
            delete d->icon;
            d->icon = 0;
            setLayout(0);
            setAspectRatioMode(d->savedAspectRatio);
        }

        return;
    }

    if (!d->icon) {
        d->icon = new Plasma::IconWidget(icon, QString(), this);
        connect(d->icon, SIGNAL(clicked()), this, SLOT(internalTogglePopup()));

        QGraphicsLinearLayout *layout = new QGraphicsLinearLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        layout->setOrientation(Qt::Horizontal);

        if (formFactor() == Plasma::Vertical || formFactor() == Plasma::Horizontal ) {
            d->savedAspectRatio = aspectRatioMode();
            setAspectRatioMode(Plasma::ConstrainedSquare);
        }

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

void PopupAppletPrivate::checkExtenderAppearance(Plasma::FormFactor f)
{
    Extender *extender = qobject_cast<Extender*>(q->graphicsWidget());
    if (extender) {
        if (f != Plasma::Horizontal && f != Plasma::Vertical) {
            extender->setAppearance(Extender::NoBorders);
        } else if (q->location() == TopEdge) {
            extender->setAppearance(Extender::TopDownStacked);
        } else {
            extender->setAppearance(Extender::BottomUpStacked);
        }

        if (dialog) {
            dialog->setGraphicsWidget(extender);
        }
    }
}

void PopupAppletPrivate::popupConstraintsEvent(Plasma::Constraints constraints)
{
    Plasma::FormFactor f = q->formFactor();

    if (constraints & Plasma::LocationConstraint) {
        checkExtenderAppearance(f);
    }

    if (constraints & Plasma::FormFactorConstraint ||
        constraints & Plasma::StartupCompletedConstraint ||
        (constraints & Plasma::SizeConstraint &&
         (f == Plasma::Vertical || f == Plasma::Horizontal))) {
        QGraphicsLinearLayout *lay = dynamic_cast<QGraphicsLinearLayout *>(q->layout());

        if (icon && !icon->icon().isNull() && lay) {
            lay->removeAt(0);
        }

        QSizeF minimum;
        QSizeF containmentSize;

        QGraphicsWidget *gWidget = q->graphicsWidget();
        kDebug() << "graphics widget is" << (QObject*)gWidget;
        QWidget *qWidget = q->widget();

        if (gWidget) {
            minimum = gWidget->minimumSize();
            // our layout may have been replaced on us in the call to graphicsWidget!
            lay = dynamic_cast<QGraphicsLinearLayout *>(q->layout());

            if (!(constraints & LocationConstraint)) {
                checkExtenderAppearance(f);
            }
        } else if (qWidget) {
            minimum = qWidget->minimumSizeHint();
        }

        if (q->containment()) {
            containmentSize = q->containment()->size();
        }

        //Applet on desktop
        if (icon && !icon->icon().isNull() && ((f != Plasma::Vertical && f != Plasma::Horizontal) ||
            ((f == Plasma::Vertical && containmentSize.width() >= minimum.width()) ||
             (f == Plasma::Horizontal && containmentSize.height() >= minimum.height())))) {
            kDebug() << "we are expanding the popupapplet";

            // we only switch to expanded if we aren't horiz/vert constrained and
            // this applet has an icon.
            // otherwise, we leave it up to the applet itself to figure it out
            if (icon) {
                icon->hide();
            }

            if (savedAspectRatio != Plasma::InvalidAspectRatioMode) {
                q->setAspectRatioMode(savedAspectRatio);
            }

            if (dialog) {
                if (dialog->layout() && qWidget) {
                    //we don't want to delete Widget inside the dialog layout
                    dialog->layout()->removeWidget(qWidget);
                }

                if (qWidget) {
                    qWidget->setParent(0);
                }

                delete dialog;
                dialog = 0;
            }

            if (!lay) {
                lay = new QGraphicsLinearLayout();
                lay->setContentsMargins(0, 0, 0, 0);
                lay->setSpacing(0);
                lay->setOrientation(Qt::Horizontal);
                q->setLayout(lay);
            }

            QSize prefSize;

            if (gWidget) {
                Corona *corona = qobject_cast<Corona *>(gWidget->scene());

                if (corona) {
                    corona->removeOffscreenWidget(gWidget);
                }

                lay->addItem(gWidget);
                prefSize = gWidget->preferredSize().toSize();
            } else if (qWidget) {
                if (!proxy) {
                    proxy = new QGraphicsProxyWidget(q);
                    proxy->setWidget(qWidget);
                    proxy->show();
                }

                lay->addItem(proxy);
                prefSize = qWidget->sizeHint();
            }

            //we could be on a big panel, but in that case we will be able to resize
            //more than the natural minimum size, because we'll transform into an icon
            if (f == Plasma::Horizontal) {
                minimum.setHeight(0);
            } else if (f == Plasma::Vertical) {
                minimum.setWidth(0);
            }

            qreal left, top, right, bottom;
            q->getContentsMargins(&left, &top, &right, &bottom);
            QSizeF oldSize(q->size());
            q->setMinimumSize(minimum + QSizeF(left+right, top+bottom));
            //size not saved/invalid size saved
            if (oldSize.width() < q->minimumSize().width() || oldSize.height() < q->minimumSize().height()) {
                q->resize(prefSize);
                emit q->appletTransformedItself();
            }

            //FIXME: this will be automatically propagated by the qgraphicslayout in the future
            lay->setPreferredSize(prefSize);
        //Applet on popup
        } else {
            kDebug() << "about to switch to a popup";
            //save the aspect ratio mode in case we drag'n drop in the Desktop later
            savedAspectRatio = q->aspectRatioMode();

            if (icon) {
                icon->show();
                q->setAspectRatioMode(Plasma::ConstrainedSquare);
            }

            if (proxy) {
                proxy->setWidget(0); // prevent it from deleting our widget!
                delete proxy;
                proxy = 0;
            }

            if (!dialog) {
                dialog = new Plasma::Dialog();

                //no longer use Qt::Popup since that seems to cause a lot of problem when you drag
                //stuff out of your Dialog (extenders). Monitor WindowDeactivate events so we can
                //emulate the same kind of behavior as Qt::Popup (close when you click somewhere
                //else.
                Qt::WindowFlags wflags = Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint;

                if (passive) {
                    wflags |= Qt::X11BypassWindowManagerHint;
                }

                dialog->setWindowFlags(wflags);
                KWindowSystem::setState(dialog->winId(), NET::SkipTaskbar | NET::SkipPager);
                dialog->installEventFilter(q);

                q->setMinimumSize(QSize(0, 0));
                if (gWidget) {
                    Corona *corona = qobject_cast<Corona *>(gWidget->scene());

                    //could that cast ever fail??
                    if (corona) {
                        corona->addOffscreenWidget(gWidget);
                        dialog->setGraphicsWidget(gWidget);
                    }
                } else if (qWidget) {
                    QVBoxLayout *l_layout = new QVBoxLayout(dialog);
                    l_layout->setSpacing(0);
                    l_layout->setMargin(0);
                    l_layout->addWidget(qWidget);
                    dialog->adjustSize();
                }

                QObject::connect(dialog, SIGNAL(dialogResized()), q, SLOT(dialogSizeChanged()));
                QObject::connect(dialog, SIGNAL(dialogVisible(bool)), q, SLOT(dialogStatusChanged(bool)));
            }

            if (icon && lay) {
                lay->addItem(icon);
            }

            q->setMinimumSize(0,0);
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
    if (!d->icon &&
        (d->clicked - scenePos().toPoint()).manhattanLength() < KGlobalSettings::dndEventDelay()) {
        d->internalTogglePopup();
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

    /**
    if (layout() && watched == graphicsWidget() && (event->type() == QEvent::GraphicsSceneResize)) {
        //sizes are recalculated in the constraintsevent so let's just call that.
        d->popupConstraintsEvent(Plasma::FormFactorConstraint);

        //resize vertically if necesarry.
        if (formFactor() == Plasma::MediaCenter || formFactor() == Plasma::Planar) {
            resize(QSizeF(size().width(), minimumHeight()));
        }
    }
    */

    return Applet::eventFilter(watched, event);
}

void PopupApplet::showPopup(uint popupDuration)
{
    if (d->dialog) {
        // move the popup before its fist show, even if the show isn't triggered by
        // a click, this should fix the first random position seen in some widgets
        if (!d->dialog->isVisible()) {
            d->internalTogglePopup();
        }

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
    if (d->dialog) {
        d->dialog->hide();
    }
}

void PopupApplet::togglePopup()
{
    d->internalTogglePopup();
}

Plasma::PopupPlacement PopupApplet::popupPlacement() const
{
    return d->popupPlacement;
}

void PopupApplet::popupEvent(bool)
{
}

void PopupApplet::setPassivePopup(bool passive)
{
    d->passive = passive;

    if (d->dialog) {
        Qt::WindowFlags wflags = Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint;

        if (d->passive) {
            wflags |= Qt::X11BypassWindowManagerHint;
        }

        d->dialog->setWindowFlags(wflags);
    }
}

bool PopupApplet::isPassivePopup() const
{
    return d->passive;
}

bool PopupApplet::isPopupShowing() const
{
    return d->dialog && d->dialog->isVisible();
}

PopupAppletPrivate::PopupAppletPrivate(PopupApplet *applet)
        : q(applet),
          icon(0),
          dialog(0),
          proxy(0),
          popupPlacement(Plasma::FloatingPopup),
          savedAspectRatio(Plasma::InvalidAspectRatioMode),
          timer(0),
          popupLostFocus(false),
          passive(false)
{
}

PopupAppletPrivate::~PopupAppletPrivate()
{
    if (proxy) {
        proxy->setWidget(0);
    }

    delete dialog;
    delete icon;
}

void PopupAppletPrivate::internalTogglePopup()
{
    if (!dialog) {
        return;
    }

    if (timer) {
        timer->stop();
    }

    if (dialog->isVisible()) {
        dialog->hide();
    } else {
        ToolTipManager::self()->hide(q);
        updateDialogPosition();
        KWindowSystem::setState(dialog->winId(), NET::SkipTaskbar | NET::SkipPager);
        dialog->show();
    }

    dialog->clearFocus();
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
        KConfigGroup sizeGroup = q->config();
        sizeGroup = KConfigGroup(&sizeGroup, "PopupApplet");
        sizeGroup.writeEntry("DialogHeight", dialog->height());
        sizeGroup.writeEntry("DialogWidth", dialog->width());

        updateDialogPosition();

        emit q->configNeedsSaving();
    }
}

void PopupAppletPrivate::dialogStatusChanged(bool status)
{
    q->popupEvent(status);
}

void PopupAppletPrivate::updateDialogPosition()
{
    QGraphicsView *view = q->view();

    if (!view) {
        return;
    }

    KConfigGroup sizeGroup = q->config();
    sizeGroup = KConfigGroup(&sizeGroup, "PopupApplet");

    Q_ASSERT(q->containment());
    Q_ASSERT(q->containment()->corona());

    int preferredWidth = 0;
    int preferredHeight = 0;
    if (dialog->graphicsWidget()) {
        preferredWidth = dialog->graphicsWidget()->preferredSize().width();
        preferredHeight = dialog->graphicsWidget()->preferredSize().height();
    }

    const int width = qMin(sizeGroup.readEntry("DialogWidth", preferredWidth),
                           q->containment()->corona()->screenGeometry(-1).width() - 50);
    const int height = qMin(sizeGroup.readEntry("DialogHeight", preferredHeight),
                            q->containment()->corona()->screenGeometry(-1).height() - 50);

    QSize saved(width, height);

    if (saved.isNull()) {
        saved = dialog->sizeHint();
    } else {
        saved = saved.expandedTo(dialog->minimumSizeHint());
    }

    if (saved.width() != dialog->width() || saved.height() != dialog->height()) {
        dialog->resize(saved);
    }

    QSize s = dialog->size();
    QPoint pos = view->mapFromScene(q->scenePos());

    //try to access a corona
    Corona *corona = qobject_cast<Corona *>(q->scene());
    if (corona) {
        pos = corona->popupPosition(q, s);
    }

    bool reverse = false;
    if (q->formFactor() == Plasma::Vertical) {
        if (view->mapToGlobal(view->mapFromScene(q->scenePos())).y() + q->size().height()/2 < pos.y() + dialog->size().width()/2) {
            reverse = true;
        }
    } else {
        if (view->mapToGlobal(view->mapFromScene(q->scenePos())).x() + q->size().width()/2 < pos.x() + dialog->size().width()/2) {
            reverse = true;
        }
    }

    switch (q->location()) {
    case BottomEdge:
        if (pos.x() >= q->pos().x()) {
            dialog->setResizeHandleCorners(Dialog::NorthEast);
        } else {
            dialog->setResizeHandleCorners(Dialog::NorthWest);
        }

        if (reverse) {
            popupPlacement = Plasma::TopPosedLeftAlignedPopup;
        } else {
            popupPlacement = Plasma::TopPosedRightAlignedPopup;
        }
        break;
    case TopEdge:
        if (pos.x() >= q->pos().x()) {
            dialog->setResizeHandleCorners(Dialog::SouthEast);
        } else {
            dialog->setResizeHandleCorners(Dialog::SouthWest);
        }

        if (reverse) {
            popupPlacement = Plasma::BottomPosedLeftAlignedPopup;
        } else {
            popupPlacement = Plasma::BottomPosedRightAlignedPopup;
        }
        break;
    case LeftEdge:
        if (pos.y() >= q->pos().y()) {
            dialog->setResizeHandleCorners(Dialog::SouthEast);
        } else {
            dialog->setResizeHandleCorners(Dialog::NorthEast);
        }

        if (reverse) {
            popupPlacement = Plasma::RightPosedTopAlignedPopup;
        } else {
            popupPlacement = Plasma::RightPosedBottomAlignedPopup;
        }
        break;

    case RightEdge:
        if (pos.y() >= q->pos().y()) {
            dialog->setResizeHandleCorners(Dialog::SouthWest);
        } else {
            dialog->setResizeHandleCorners(Dialog::NorthWest);
        }

        if (reverse) {
            popupPlacement = Plasma::LeftPosedTopAlignedPopup;
        } else {
            popupPlacement = Plasma::LeftPosedBottomAlignedPopup;
        }
        break;
    default:
        dialog->setResizeHandleCorners(Dialog::NorthEast);
    }

    dialog->move(pos);
}
} // Plasma namespace

#include "popupapplet.moc"

