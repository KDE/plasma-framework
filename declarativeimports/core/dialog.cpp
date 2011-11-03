/***************************************************************************
 *   Copyright 2011 Marco Martin <mart@kde.org>                            *
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
#include "declarativeitemcontainer_p.h"

#include <QApplication>
#include <QDeclarativeItem>
#include <QDesktopWidget>
#include <QGraphicsObject>
#include <QGraphicsWidget>
#include <QTimer>
#include <QLayout>

#include <Plasma/Corona>
#include <Plasma/Dialog>
#include <Plasma/WindowEffects>


int DialogProxy::offscreenX = 0;
int DialogProxy::offscreenY = 0;

DialogMargins::DialogMargins(Plasma::Dialog *dialog, QObject *parent)
    : QObject(parent),
      m_dialog(dialog)
{
    checkMargins();
}

void DialogMargins::checkMargins()
{
    int left, top, right, bottom;
    m_dialog->getContentsMargins(&left, &top, &right, &bottom);

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

DialogProxy::DialogProxy(QObject *parent)
    : QObject(parent),
      m_declarativeItemContainer(0),
      m_activeWindow(false),
      m_location(Plasma::Floating)
{
    m_dialog = new Plasma::Dialog();
    m_margins = new DialogMargins(m_dialog, this);
    m_dialog->installEventFilter(this);
    m_flags = m_dialog->windowFlags();
}

DialogProxy::~DialogProxy()
{
    delete m_dialog;
    delete m_declarativeItemContainer;
}

QGraphicsObject *DialogProxy::mainItem() const
{
    return m_mainItem.data();
}

void DialogProxy::setMainItem(QGraphicsObject *mainItem)
{
    if (m_mainItem.data() != mainItem) {
        if (m_mainItem) {
            m_mainItem.data()->setParent(mainItem ? mainItem->parent() : 0);
        }

        m_mainItem = mainItem;

        if (mainItem) {
            mainItem->setParentItem(0);
            mainItem->setParent(this);
        }

        //if this is called in Compenent.onCompleted we have to wait a loop the item is added to a scene
        QTimer::singleShot(0, this, SLOT(syncMainItem()));
        emit mainItemChanged();
    }
}

void DialogProxy::syncMainItem()
{
    if (!m_mainItem) {
        return;
    }

    //not have a scene? go up in the hyerarchy until we find something with a scene
    QGraphicsScene *scene = m_mainItem.data()->scene();
    if (!scene) {
        QObject *parent = m_mainItem.data();
        while ((parent = parent->parent())) {
            QGraphicsObject *qo = qobject_cast<QGraphicsObject *>(parent);
            if (qo) {
                scene = qo->scene();
                if (scene) {
                    scene->addItem(m_mainItem.data());
                    break;
                }
            }
        }
    }

    if (!scene) {
        return;
    }

    //the parent of the qobject never changed, only the parentitem, so put it back what it was
    m_mainItem.data()->setParentItem(qobject_cast<QGraphicsObject *>(m_mainItem.data()->parent()));

    QGraphicsWidget *widget = qobject_cast<QGraphicsWidget *>(m_mainItem.data());
    if (widget) {
        if (m_declarativeItemContainer) {
            m_declarativeItemContainer->deleteLater();
            m_declarativeItemContainer = 0;
        }
    } else {
        QDeclarativeItem *di = qobject_cast<QDeclarativeItem *>(m_mainItem.data());
        if (di) {
            if (!m_declarativeItemContainer) {
                m_declarativeItemContainer = new DeclarativeItemContainer();
                scene->addItem(m_declarativeItemContainer);
            }
            m_declarativeItemContainer->setDeclarativeItem(di);
            widget = m_declarativeItemContainer;
        }
    }
    m_dialog->setGraphicsWidget(widget);

    if (!qobject_cast<Plasma::Corona *>(scene)) {
        offscreenX -= 1024;
        offscreenY -= 1024;
        widget->setPos(offscreenX, offscreenY);
    }
}

bool DialogProxy::isVisible() const
{
    return m_dialog->isVisible();
}

void DialogProxy::setVisible(const bool visible)
{
    if (m_dialog->isVisible() != visible) {
        m_dialog->setVisible(visible);
        if (visible) {
            m_dialog->setWindowFlags(m_flags);
            m_dialog->setVisible(visible);
            m_dialog->raise();
        }
    }
}

QPoint DialogProxy::popupPosition(QGraphicsObject *item, int alignment)
{
    QGraphicsObject *actualItem = item;

    //if no item is passed search the root item in order to figure out the view
    if (!actualItem) {
        actualItem = qobject_cast<QGraphicsObject *>(parent());

        //search the root object
        while (true) {
            QGraphicsObject *ancestor = qobject_cast<QGraphicsObject *>(actualItem->parent());

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

    //ensure the dialog has the proper size
    syncMainItem();
    m_dialog->syncToGraphicsWidget();

    Plasma::Corona *corona = qobject_cast<Plasma::Corona *>(actualItem->scene());
    if (corona && item) {
        return corona->popupPosition(actualItem, m_dialog->size(), (Qt::AlignmentFlag)alignment);
    } else {

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
}


int DialogProxy::x() const
{
    return m_dialog->pos().x();
}

void DialogProxy::setX(int x)
{
    m_dialog->move(x, m_dialog->pos().y());
}

int DialogProxy::y() const
{
    return m_dialog->pos().y();
}

void DialogProxy::setY(int y)
{
    m_dialog->move(m_dialog->pos().x(), y);
}

int DialogProxy::width() const
{
    return m_dialog->size().width();
}

int DialogProxy::height() const
{
    return m_dialog->size().height();
}

bool DialogProxy::isActiveWindow() const
{
    return m_activeWindow;
}

int DialogProxy::windowFlags() const
{
    return (int)m_dialog->windowFlags();
}

void DialogProxy::setWindowFlags(const int flags)
{
    m_flags = (Qt::WindowFlags)flags;
    m_dialog->setWindowFlags((Qt::WindowFlags)flags);
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

bool DialogProxy::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_dialog && event->type() == QEvent::Move) {
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
    } else if (watched == m_dialog && event->type() == QEvent::Resize) {
        QResizeEvent *re = static_cast<QResizeEvent *>(event);
        if (re->oldSize().width() != re->size().width()) {
            emit widthChanged();
        }
        if (re->oldSize().height() != re->size().height()) {
            emit heightChanged();
        }
    } else if (watched == m_dialog && event->type() == QEvent::Show) {
        Plasma::WindowEffects::slideWindow(m_dialog, m_location);
        emit visibleChanged();
    } else if (watched == m_dialog && event->type() == QEvent::Hide) {
        Plasma::WindowEffects::slideWindow(m_dialog, m_location);
        emit visibleChanged();
    } else if (watched == m_dialog && event->type() == QEvent::WindowActivate) {
        m_activeWindow = true;
        emit activeWindowChanged();
    } else if (watched == m_dialog && event->type() == QEvent::WindowDeactivate) {
        m_activeWindow = false;
        emit activeWindowChanged();
    }
    return false;
}

void DialogProxy::setAttribute(int attribute, bool on)
{
    m_dialog->setAttribute((Qt::WidgetAttribute)attribute, on);
}

#include "dialog.moc"

