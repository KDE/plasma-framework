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
//#include "declarativeitemcontainer_p.h"

#include <QApplication>
#include <QQuickItem>
#include <QDesktopWidget>
#include <QQuickItem>
#include <QQuickItem>
#include <QTimer>
#include <QLayout>

#include <KWindowSystem>

#include <Plasma/Corona>
// #include <Plasma/Dialog>
//#include <Plasma/WindowEffects>
#include <QDebug>

int DialogProxy::offscreenX = 0;
int DialogProxy::offscreenY = 0;

DialogMargins::DialogMargins(QQuickWindow *dialog, QObject *parent)
    : QObject(parent),
      m_left(0), m_top(0), m_right(0), m_bottom(0),
      m_dialog(dialog)
{
    checkMargins();
}

void DialogMargins::checkMargins()
{
    int _m = 16;
    int top = _m;
    int bottom = _m;
    int left = _m;
    int right = _m;
    //m_dialog->getContentsMargins(&left, &top, &right, &bottom);

    if (left != m_left) {
        m_left = left;
        emit leftChanged();
    }
    if (top != m_top) {
        m_top = top;
        emit topChanged();
    }
    if (right != m_right) {
        m_right = right;
        emit rightChanged();
    }
    if (bottom != m_bottom) {
        m_bottom = bottom;
        emit bottomChanged();
    }
}

int DialogMargins::left() const
{
    return m_left;
}

int DialogMargins::top() const
{
    return m_top;
}

int DialogMargins::right() const
{
    return m_right;
}

int DialogMargins::bottom() const
{
    return m_bottom;
}

DialogProxy::DialogProxy(QQuickItem *parent)
    : QQuickWindow(),
      m_activeWindow(false),
      m_location(Plasma::Floating)
{
    QSurfaceFormat format;
    format.setAlphaBufferSize(8);
    setFormat(format);
    setClearBeforeRendering(true);
    setColor(QColor(Qt::transparent));
    setFlags(Qt::FramelessWindowHint);

    //m_dialog = new QQuickWindow();
    m_margins = new DialogMargins(m_dialog, this);
    //m_dialog->installEventFilter(this);
    m_flags = flags();
}

DialogProxy::~DialogProxy()
{
    //delete m_dialog;
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
            m_mainItem.data()->setParent(mainItem ? mainItem->parent() : 0);
        }

        m_mainItem = mainItem;

        if (mainItem) {
            //mainItem->setParentItem(0);
            mainItem->setParent(this);
        }

        //if this is called in Compenent.onCompleted we have to wait a loop the item is added to a scene
        emit mainItemChanged();
    }
}

bool DialogProxy::isVisible() const
{
    return QQuickWindow::isVisible();
}

void DialogProxy::setVisible(const bool visible)
{
    if (isVisible() != visible) {
        //FIXME: workaround to prevent dialogs of Popup type disappearing on the second show
        const QSize s = QSize(m_mainItem.data()->width(), m_mainItem.data()->height());
        //resize(0,0);
        resize(s);

        const QRect workArea(KWindowSystem::workArea());
        if (!workArea.contains(geometry())) {
            //FIXME
//             m_dialog->move(qBound(workArea.left(), m_dialog->x(), workArea.right() - m_dialog->width()),
//                              qBound(workArea.top(), m_dialog->y(), workArea.bottom() - m_dialog->height())
//             );
        }

        QQuickWindow::setVisible(visible);
        if (visible) {
            raise();
        }
    }
}

QPoint DialogProxy::popupPosition(QQuickItem *item, int alignment)
{
    /*
    QQuickItem *actualItem = item;

    //if no item is passed search the root item in order to figure out the view
    if (!actualItem) {
        actualItem = qobject_cast<QQuickItem *>(parent());

        //search the root object
        while (true) {
            QQuickItem *ancestor = qobject_cast<QQuickItem *>(actualItem->parent());

            if (ancestor) {
                actualItem = ancestor;
            } else {
                break;
            }
        }
        if (!actualItem) {
            return QPoint();
        }
    }

    //m_dialog->syncToGraphicsWidget();

    Plasma::Corona *corona = qobject_cast<Plasma::Corona *>(actualItem->scene());
    if (corona && item) {
        return corona->popupPosition(actualItem, m_dialog->size(), (Qt::AlignmentFlag)alignment);
    } else {

        if (!actualItem->scene()) {
            return QPoint();
        }

        QList<QGraphicsView*> views = actualItem->scene()->views();


        if (views.size() < 1) {
            return QPoint();
        }

        QGraphicsView *view = 0;
        if (views.size() == 1) {
            view = views[0];
        } else {
            QGraphicsView *found = 0;
            QGraphicsView *possibleFind = 0;

            foreach (QGraphicsView *v, views) {
                if (v->sceneRect().intersects(actualItem->sceneBoundingRect()) ||
                    v->sceneRect().contains(actualItem->scenePos())) {
                    if (v->isActiveWindow()) {
                        found = v;
                    } else {
                        possibleFind = v;
                    }
                }
            }
            view = found ? found : possibleFind;
        }

        if (!view) {
            return QPoint();
        }

        //if no item was explicitly specified, align the dialog in the center of the parent view
        if (!item) {
            return view->geometry().center() - QPoint(m_dialog->width()/2, m_dialog->height()/2);
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

        if (alignment == Qt::AlignCenter) {
            xOffset = actualItem->boundingRect().width()/2 - m_dialog->width()/2;
        } else if (alignment == Qt::AlignRight) {
            xOffset = actualItem->boundingRect().width() - m_dialog->width();
        }

        const QRect avail = QApplication::desktop()->availableGeometry(view);
        QPoint menuPos = view->mapToGlobal(view->mapFromScene(actualItem->scenePos()+QPoint(xOffset, actualItem->boundingRect().height())));

        if (menuPos.y() + m_dialog->height() > avail.bottom()) {
            menuPos = view->mapToGlobal(view->mapFromScene(actualItem->scenePos() - QPoint(-xOffset, m_dialog->height())));
        }
        return menuPos;
    }
    */
    return QPoint(200, 400);
}


int DialogProxy::x() const
{
    return geometry().topLeft().x();
}

void DialogProxy::setX(int x)
{
    //m_dialog->move(x, m_dialog->geometry().topLeft().y());
}

int DialogProxy::y() const
{
    return geometry().topLeft().y();
}

void DialogProxy::setY(int y)
{
    //m_dialog->move(m_dialog->geometry().topLeft().x(), y);
}

int DialogProxy::width() const
{
    return size().width();
}

int DialogProxy::height() const
{
    return size().height();
}

bool DialogProxy::isActiveWindow() const
{
    return m_activeWindow;
}

void DialogProxy::activateWindow()
{
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

int DialogProxy::location() const
{
    return (int)m_location;
}

void DialogProxy::setLocation(int location)
{
    if (m_location == location) {
        return;
    }
    m_location = (Plasma::Location)location;
    emit locationChanged();
}


QObject *DialogProxy::margins() const
{
    return m_margins;
}

void DialogProxy::resizeEvent(QResizeEvent *re)
{
    if (contentItem()) {
        contentItem()->setWidth(re->size().width());
        contentItem()->setHeight(re->size().height());
    }
    if (m_mainItem) {
        m_mainItem.data()->setWidth(re->size().width());
        m_mainItem.data()->setHeight(re->size().height());
    }
}

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
}

void DialogProxy::setAttribute(int attribute, bool on)
{
    //m_dialog->setAttribute((Qt::WidgetAttribute)attribute, on);

    if (attribute == Qt::WA_X11NetWmWindowTypeDock) {
        KWindowSystem::setOnAllDesktops(winId(), true);
    } else {
        KWindowSystem::setOnAllDesktops(winId(), false);
    }
}

#include "dialog.moc"

