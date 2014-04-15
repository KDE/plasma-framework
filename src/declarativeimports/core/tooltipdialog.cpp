/***************************************************************************
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>                       *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "tooltipdialog.h"

#include <QQmlEngine>
#include <QQuickItem>
#include <QDebug>
#include <QPropertyAnimation>

#include <KWindowSystem/KWindowInfo>
#include <KWindowSystem/KWindowSystem>

#include "framesvgitem.h"

#include <kdeclarative/qmlobject.h>

ToolTipDialog::ToolTipDialog(QWindow  *parent)
    : QQuickWindow(parent),
      m_qmlObject(0),
      m_animation(0),
      m_hideTimeout(4000),
      m_direction(Plasma::Types::Up),
      m_interactive(false)
{
    setFlags(Qt::ToolTip);
    setLocation(Plasma::Types::Floating);
    setDirection(Plasma::Types::Up);

    m_animation = new QPropertyAnimation(this);
    connect(m_animation, SIGNAL(valueChanged(QVariant)),
            this, SLOT(valueChanged(QVariant)));
    m_animation->setTargetObject(this);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    m_animation->setDuration(250);

    m_showTimer = new QTimer(this);
    m_showTimer->setSingleShot(true);
    connect(m_showTimer, &QTimer::timeout, [=]() {
        setVisible(false);
    });
}

ToolTipDialog::~ToolTipDialog()
{
}

QQuickItem *ToolTipDialog::loadDefaultItem()
{
    if (!m_qmlObject) {
        m_qmlObject = new KDeclarative::QmlObject(this);
    }

    if (!m_qmlObject->rootObject()) {
        //HACK: search our own import
        foreach (const QString &path, m_qmlObject->engine()->importPathList()) {
            if (QFile::exists(path + "/org/kde/plasma/core")) {
                m_qmlObject->setSource(QUrl::fromLocalFile(path + "/org/kde/plasma/core/private/DefaultToolTip.qml"));
                break;
            }
        }
    }

    return qobject_cast<QQuickItem *>(m_qmlObject->rootObject());
}

Plasma::Types::Direction ToolTipDialog::direction() const
{
    return m_direction;
}

void ToolTipDialog::setDirection(Plasma::Types::Direction dir)
{
    m_direction = dir;
}

void ToolTipDialog::showEvent(QShowEvent *event)
{
    m_showTimer->start(m_hideTimeout);

    setPosition(popupPosition(m_visualParent.data(), size()));
    m_animation->stop();
    QQuickWindow::showEvent(event);
}

void ToolTipDialog::hideEvent(QHideEvent *event)
{
    m_showTimer->stop();
    m_animation->stop();

    QQuickWindow::hideEvent(event);
}

void ToolTipDialog::resizeEvent(QResizeEvent *re)
{
    QQuickWindow::resizeEvent(re);
}

bool ToolTipDialog::event(QEvent *e)
{
    if (e->type() == QEvent::Enter) {
        if (m_interactive) {
            m_showTimer->stop();
        }
    } else if (e->type() == QEvent::Leave) {
        dismiss();
    }

    bool ret = QQuickWindow::event(e);
    setFlags(Qt::ToolTip | Qt::WindowDoesNotAcceptFocus | Qt::WindowStaysOnTopHint);
    return ret;
}

void ToolTipDialog::adjustGeometry(const QRect &geom)
{
    switch (m_direction) {
    case Plasma::Types::Right:
        setX(x() + (size().width() - geom.size().width()));
        break;
    case Plasma::Types::Up:
        setY(y() + (size().height() - geom.size().height()));
        break;
    default:
        break;
    }

    if (isVisible()) {

        resize(geom.size());
        m_animation->setStartValue(position());
        m_animation->setEndValue(geom.topLeft());
        m_animation->start();
    } else {
        setGeometry(geom);
    }
}

void ToolTipDialog::dismiss()
{
    m_showTimer->start(m_hideTimeout / 20); // pretty short: 200ms
}

void ToolTipDialog::keepalive()
{
    m_showTimer->start(m_hideTimeout);
}

bool ToolTipDialog::interactive()
{
    return m_interactive;
}

void ToolTipDialog::setInteractive(bool interactive)
{
    m_interactive = interactive;
}

void ToolTipDialog::valueChanged(const QVariant &value)
{
    setPosition(value.value<QPoint>());
}

Plasma::Types::Location ToolTipDialog::location() const
{
    return m_location;
}

void ToolTipDialog::setLocation(Plasma::Types::Location location)
{
    if (m_location == location) {
        return;
    }
    m_location = location;
    emit locationChanged();
}

QQuickItem *ToolTipDialog::mainItem() const
{
    return m_mainItem.data();
}

void ToolTipDialog::setMainItem(QQuickItem *mainItem)
{
    m_mainItem = mainItem;
    resize(QSize(mainItem->width(), mainItem->height()));
    adjustGeometry(QRect(popupPosition(m_visualParent.data(), size()), size()));
    mainItem->setParentItem(contentItem());
}

QPoint ToolTipDialog::popupPosition(QQuickItem *item, const QSize &size)
{
    if (!item) {
        return QPoint();
    }

    QPointF pos = item->mapToScene(QPointF(0, 0));

    if (item->window()) {
        pos = item->window()->mapToGlobal(pos.toPoint());
    } else {
        return QPoint();
    }

    //if the item is in a dock or in a window that ignores WM we want to position the popups outside of the dock
    const KWindowInfo winInfo = KWindowSystem::windowInfo(item->window()->winId(), NET::WMWindowType);
    const bool outsideParentWindow = (winInfo.windowType(NET::AllTypesMask) == NET::Dock) || (item->window()->flags() & Qt::X11BypassWindowManagerHint);

    QRect parentGeometryBounds;
    if (outsideParentWindow) {
        parentGeometryBounds = item->window()->geometry();
    } else {
        parentGeometryBounds = QRect(pos.toPoint(), QSize(item->width(), item->height()));
    }

    const QPoint topPoint(pos.x() + (item->boundingRect().width() - size.width())/2,
                                   parentGeometryBounds.top() - size.height());
    const QPoint bottomPoint(pos.x() + (item->boundingRect().width() - size.width())/2,
                             parentGeometryBounds.bottom());

    const QPoint leftPoint(parentGeometryBounds.left() - size.width(),
                           pos.y() + (item->boundingRect().height() - size.height())/2);

    const QPoint rightPoint(parentGeometryBounds.right(),
                            pos.y() + (item->boundingRect().height() - size.height())/2);

    QPoint dialogPos;
    if (m_location == Plasma::Types::TopEdge) {
        dialogPos = bottomPoint;
    } else if (m_location == Plasma::Types::LeftEdge) {
        dialogPos = rightPoint;
    } else if (m_location == Plasma::Types::RightEdge) {
        dialogPos = leftPoint;
    } else { // Types::BottomEdge
        dialogPos = topPoint;
    }

    //find the correct screen for the item
    //we do not rely on item->window()->screen() because
    //QWindow::screen() is always only the screen where the window gets first created
    //not actually the current window. See QWindow::screen() documentation
    //QRect avail = m_screenForItem(item)->availableGeometry();
    QRect avail = screen()->availableGeometry();

    if (outsideParentWindow ) {
        //make the panel look it's inside the panel, in order to not make it look cutted
        switch (m_location) {
        case Plasma::Types::LeftEdge:
        case Plasma::Types::RightEdge:
            avail.setTop(qMax(avail.top(), parentGeometryBounds.top()));
            avail.setBottom(qMin(avail.bottom(), parentGeometryBounds.bottom()));
            break;
        default:
            avail.setLeft(qMax(avail.left(), parentGeometryBounds.left()));
            avail.setRight(qMin(avail.right(), parentGeometryBounds.right()));
            break;
        }
    }

    if (dialogPos.x() < avail.left()) {
        // popup hits lhs
        if (m_location == Plasma::Types::TopEdge || m_location == Plasma::Types::BottomEdge) {
            // move it
            dialogPos.setX(avail.left());
        } else {
            // swap edge
            dialogPos.setX(rightPoint.x());
        }
    }
    if (dialogPos.x() + size.width() > avail.right()) {
        // popup hits rhs
        if (m_location == Plasma::Types::TopEdge || m_location == Plasma::Types::BottomEdge) {
            dialogPos.setX(avail.right() - size.width());
        } else {
            dialogPos.setX(leftPoint.x());
        }
    }
    if (dialogPos.y() < avail.top()) {
        // hitting top
        if (m_location == Plasma::Types::LeftEdge || m_location == Plasma::Types::RightEdge) {
            dialogPos.setY(avail.top());
        } else {
            dialogPos.setY(bottomPoint.y());
        }
    }

    if (dialogPos.y() + size.height() > avail.bottom()) {
        // hitting bottom
        if (m_location == Plasma::Types::TopEdge || m_location == Plasma::Types::BottomEdge) {
            dialogPos.setY(topPoint.y());
        } else {
            dialogPos.setY(avail.bottom() - size.height());
        }
    }

    return dialogPos;
}

QQuickItem *ToolTipDialog::visualParent() const
{
    return m_visualParent.data();
}

void ToolTipDialog::setVisualParent(QQuickItem *visualParent)
{
    if (visualParent) {
        adjustGeometry(QRect(popupPosition(visualParent, size()), size()));
    }
    if (m_visualParent.data() == visualParent) {
        return;
    }

    m_visualParent = visualParent;
    emit visualParentChanged();
    if (visualParent) {
        if (visualParent->window()) {
            setTransientParent(visualParent->window());
        }
    }
}

#include "moc_tooltipdialog.cpp"
