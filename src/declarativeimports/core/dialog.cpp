/***************************************************************************
 *   Copyright 2011 Marco Martin <mart@kde.org>                            *
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

#include "dialog.h"
#include "framesvgitem.h"

#include <QApplication>
#include <QQuickItem>
#include <QDesktopWidget>
#include <QQuickItem>
#include <QQuickItem>
#include <QTimer>
#include <QLayout>
#include <QScreen>

#include <KWindowSystem>
#include <kwindoweffects.h>

#include <Plasma/Corona>
// #include <Plasma/Dialog>
//#include <Plasma/WindowEffects>
#include <QDebug>


DialogProxy::DialogProxy(QQuickItem *parent)
    : QQuickWindow(),
      m_activeWindow(false),
      m_location(Qt::AlignCenter)
{
    QSurfaceFormat format;
    format.setAlphaBufferSize(8);
    setFormat(format);
    setClearBeforeRendering(true);
    setColor(QColor(Qt::transparent));
    setFlags(Qt::FramelessWindowHint);

    m_flags = flags();

    m_syncTimer = new QTimer(this);
    m_syncTimer->setSingleShot(true);
    m_syncTimer->setInterval(250);
    connect(m_syncTimer, &QTimer::timeout, this,  &DialogProxy::syncToMainItemSize);

    //HACK: this property is invoked due to the initialization that gets done to contentItem() in the getter
    property("data");
    //Create the FrameSvg background.
    m_frameSvgItem = new Plasma::FrameSvgItem(contentItem());
    m_frameSvgItem->setImagePath("dialogs/background");
}

DialogProxy::~DialogProxy()
{
}

QQuickItem *DialogProxy::mainItem() const
{
    return m_mainItem.data();
}

void DialogProxy::setMainItem(QQuickItem *mainItem)
{
    qDebug() << "XXXXX Main ITem: " << mainItem->width() << mainItem->height();
    if (m_mainItem.data() != mainItem) {
        qDebug() << "YYYYY Main ITem: " << mainItem->width() << mainItem->height();
        if (m_mainItem) {
            m_mainItem.data()->setParent(parent());
        }


        m_mainItem = mainItem;

        if (mainItem) {
            //mainItem->setParentItem(0);
            mainItem->setParent(contentItem());
            mainItem->setProperty("parent", QVariant::fromValue(contentItem()));

            if (mainItem->metaObject()->indexOfSignal("widthChanged")) {
                connect(mainItem, SIGNAL(widthChanged()), m_syncTimer, SIGNAL(start()));
            }
            if (mainItem->metaObject()->indexOfSignal("heightChanged")) {
                connect(mainItem, SIGNAL(heightChanged()), m_syncTimer, SIGNAL(start()));
            }
            if (mainItem->metaObject()->indexOfSignal("minimumWidthChanged")) {
                connect(mainItem, SIGNAL(minimumWidthChanged()), this, SIGNAL(minimumWidthChanged()));
            }
            if (mainItem->metaObject()->indexOfSignal("minimumHeightChanged")) {
                connect(mainItem, SIGNAL(minimumHeightChanged()), this, SIGNAL(minimumHeightChanged()));
            }
            if (mainItem->metaObject()->indexOfSignal("maximumWidthChanged")) {
                connect(mainItem, SIGNAL(maximumWidthChanged()), this, SIGNAL(maximumWidthChanged()));
            }
            if (mainItem->metaObject()->indexOfSignal("maximumHeightChanged")) {
                connect(mainItem, SIGNAL(maximumHeightChanged()), this, SIGNAL(maximumHeightChanged()));
            }
        }

        //if this is called in Compenent.onCompleted we have to wait a loop the item is added to a scene
        emit mainItemChanged();
    }
}

QQuickItem *DialogProxy::visualParent() const
{
    return m_visualParent.data();
}

void DialogProxy::setVisualParent(QQuickItem *visualParent)
{
    if (m_visualParent.data() == visualParent) {
        return;
    }

    if (visualParent) {
        setPosition(popupPosition(visualParent, Qt::AlignCenter));
    }

    m_visualParent = visualParent;
    emit visualParentChanged();
}

bool DialogProxy::isVisible() const
{
    return QQuickWindow::isVisible();
}

void DialogProxy::setVisible(const bool visible)
{
    qDebug() << visible;
    if (visible) {
        syncToMainItemSize();

        if (m_visualParent) {
            setPosition(popupPosition(m_visualParent.data(), Qt::AlignCenter));
        }

        const QRect workArea(KWindowSystem::workArea());
        if (!workArea.contains(geometry())) {
            const int _x = qBound(workArea.left(), x(), workArea.right() - width());
            const int _y = qBound(workArea.top(), y(), workArea.bottom() - height());
            setPosition(_x, _y);
             //);
            qDebug() << "workarea l/r: " << workArea.left() << workArea.right();
            qDebug() << "workarea x/y: " << x() << y();
            qDebug() << "workarea t/b: " << workArea.top() << workArea.bottom();
            qDebug() << "XXX Workspace. Pop up at: " << _x << "," << _y << workArea << geometry();;
        } else if (!m_visualParent) {
            const int _x = (workArea.width() - width()) / 2;
            const int _y = (workArea.height() - height()) / 2;
            qDebug() << "Positioning inside workarea: " << workArea << geometry();
            qDebug() << "workarea l/r: " << workArea.left() << workArea.right();
            qDebug() << "workarea x/y: " << x() << y();
            qDebug() << "workarea t/b: " << workArea.top() << workArea.bottom();
            qDebug() << "XXX Workspace. Pop up at: " << _x << "," << _y << workArea << geometry();;
            setPosition(_x, _y);
        }

        raise();
    }
    QQuickWindow::setVisible(visible);
}

QPoint DialogProxy::popupPosition(QQuickItem *item, Qt::AlignmentFlag alignment)
{
    if (!item) {
        //If no item was specified try to align at the center of the parent view
        QQuickItem *parentItem = qobject_cast<QQuickItem *>(parent());
        if (parentItem && parentItem->window()) {
            qDebug() << "no item, popping up at " << parentItem->window()->geometry().center() - QPoint(width()/2, height()/2);
            return parentItem->window()->geometry().center() - QPoint(width()/2, height()/2);
        } else {
            qDebug() << "no item, zero-zero";
            return QPoint();
        }
    }

    QPointF pos = item->mapToScene(QPointF(0, 0));
    qDebug() << " we have an item at " << pos;
    if (item->window() && item->window()->screen()) {
        pos = item->window()->mapToGlobal(pos.toPoint());
        qDebug() << " ========= popping up at " << pos;
    } else {
        qDebug() << " ========= popping up at zerozero";
        return QPoint();
    }

    //swap direction if necessary
    if (QApplication::isRightToLeft() && alignment != Qt::AlignCenter) {
        if (alignment == Qt::AlignRight) {
            alignment = Qt::AlignLeft;
        } else {
            alignment = Qt::AlignRight;
        }
    }

    int xOffset = 0;
    int yOffset = 0;

    int _x = 0;
    int _y = 0;

    if (location() == Qt::AlignBottom) {
        _y = item->y() + item->height();
    } else if (location() == Qt::AlignTop) {
        _y = item->y() - height();
    } else if ((location() & Qt::AlignCenter) || (location() & Qt::AlignVCenter)) {
        // align our item's vertical center with our own vertical center
        _y = (item->y() + item->height()/2) - height()/2;
    } else {

        // Location is left or right
    }




    if (alignment == Qt::AlignCenter) {
        xOffset = item->boundingRect().width()/2 - width()/2;
        yOffset = item->boundingRect().height()/2 - height()/2;
        qDebug() << "Centering..." << xOffset << yOffset;
    } else if (alignment == Qt::AlignRight) {
        xOffset = item->boundingRect().width() - width();
    }

    const QRect avail = item->window()->screen()->availableGeometry();
    //QPoint menuPos = pos.toPoint() + QPoint(xOffset, item->boundingRect().height());
    QPoint menuPos = pos.toPoint() + QPoint(xOffset, yOffset);

    if (menuPos.y() + height() > avail.bottom()) {
        menuPos = pos.toPoint() + QPoint(xOffset, -height());
    }
    qDebug() << "PUP" << menuPos;
    return menuPos;
}


bool DialogProxy::isActiveWindow() const
{
    return m_activeWindow;
}

void DialogProxy::activateWindow()
{
    qDebug();
    setWindowState(Qt::WindowActive);
    //m_dialog->activateWindow();
}

int DialogProxy::windowFlags() const
{
    return (int)m_flags;
}

#ifndef Q_WS_WIN
qulonglong DialogProxy::windowId() const
{
    return winId();
}
#endif

void DialogProxy::setWindowFlags(const int flags)
{
    m_flags = (Qt::WindowFlags)flags;
    setFlags(Qt::FramelessWindowHint|m_flags);
}

Qt::AlignmentFlag DialogProxy::location() const
{
    return (Qt::AlignmentFlag)m_location;
}

void DialogProxy::setLocation(Qt::AlignmentFlag location)
{
    if (m_location == location) {
        return;
    }
    m_location = (Qt::AlignmentFlag)location;
    emit locationChanged();
}


QObject *DialogProxy::margins() const
{
    return m_frameSvgItem->margins();
}

void DialogProxy::resizeEvent(QResizeEvent *re)
{
    syncMainItemToSize();
    QQuickWindow::resizeEvent(re);
}

void DialogProxy::syncMainItemToSize()
{
    m_frameSvgItem->setX(0);
    m_frameSvgItem->setY(0);
    m_frameSvgItem->setWidth(width());
    m_frameSvgItem->setHeight(height());
    KWindowEffects::enableBlurBehind(winId(), true, m_frameSvgItem->frameSvg()->mask());

    if (m_mainItem) {
        m_mainItem.data()->setX(m_frameSvgItem->margins()->left());
        m_mainItem.data()->setY(m_frameSvgItem->margins()->top());
        m_mainItem.data()->setWidth(width() - m_frameSvgItem->margins()->left() - m_frameSvgItem->margins()->right());
        m_mainItem.data()->setHeight(height() - m_frameSvgItem->margins()->top() - m_frameSvgItem->margins()->bottom());
    }
}

void DialogProxy::syncToMainItemSize()
{
    if (!m_mainItem) {
        return;
    }

    //FIXME: workaround to prevent dialogs of Popup type disappearing on the second show
    const QSize s = QSize(m_mainItem.data()->width(), m_mainItem.data()->height()) +
                    QSize(m_frameSvgItem->margins()->left() + m_frameSvgItem->margins()->right(),
                          m_frameSvgItem->margins()->top() + m_frameSvgItem->margins()->bottom());
    //resize(0,0);
    resize(s);
    emit widthChanged(s.width());
    emit heightChanged(s.height());
}

/*
bool DialogProxy::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this && event->type() == QEvent::Move) {
        QMoveEvent *me = static_cast<QMoveEvent *>(event);
        if (me->oldPos().x() != me->pos().x()) {
            emit xChanged();
        }
        if (me->oldPos().y() != me->pos().y()) {
            emit yChanged();
        }
        if ((me->oldPos().x() != me->pos().x()) || (me->oldPos().y() != me->pos().y())) {
            m_margins->checkMargins();
        }
    } else if (watched == this && event->type() == QEvent::Resize) {
        QResizeEvent *re = static_cast<QResizeEvent *>(event);
        if (re->oldSize().width() != re->size().width()) {
            emit widthChanged();
        }
        if (re->oldSize().height() != re->size().height()) {
            emit heightChanged();
        }
    } else if (watched == this && event->type() == QEvent::Show) {
        //Plasma::WindowEffects::slideWindow(m_dialog, m_location);
//         if (m_dialog->testAttribute(Qt::WA_X11NetWmWindowTypeDock)) {
//             KWindowSystem::setOnAllDesktops(m_dialog->winId(), true);
//         } else {
//             KWindowSystem::setOnAllDesktops(m_dialog->winId(), false);
//         }
        emit visibleChanged();
    } else if (watched == this && event->type() == QEvent::Hide) {
        //Plasma::WindowEffects::slideWindow(m_dialog, m_location);
        emit visibleChanged();
    } else if (watched == this && event->type() == QEvent::WindowActivate) {
        m_activeWindow = true;
        emit activeWindowChanged();
    } else if (watched == this && event->type() == QEvent::WindowDeactivate) {
        m_activeWindow = false;
        emit activeWindowChanged();
    }
    return false;
}*/

void DialogProxy::setAttribute(int attribute, bool on)
{
    //m_dialog->setAttribute((Qt::WidgetAttribute)attribute, on);

    if (attribute == Qt::WA_X11NetWmWindowTypeDock) {
        KWindowSystem::setOnAllDesktops(winId(), true);
    } else {
        KWindowSystem::setOnAllDesktops(winId(), false);
    }
}

void DialogProxy::focusInEvent(QFocusEvent *ev)
{
    Q_UNUSED(ev)
    m_activeWindow = true;
    emit activeWindowChanged();
}

void DialogProxy::focusOutEvent(QFocusEvent *ev)
{
    Q_UNUSED(ev)
    m_activeWindow = false;
    emit activeWindowChanged();
}

#include "moc_dialog.cpp"

