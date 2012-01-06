/***************************************************************************
 *   Copyright 2012 Marco Martin <mart@kde.org>                            *
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

#include "fullscreendialog.h"
#include "declarativeitemcontainer_p.h"

#include <QDeclarativeItem>
#include <QGraphicsObject>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QLayout>
#include <QTimer>

#include <KWindowSystem>

#include <Plasma/Corona>


uint FullScreenDialog::s_numItems = 0;

FullScreenDialog::FullScreenDialog(QObject *parent)
    : QObject(parent)
{
    m_view = new QGraphicsView();
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->installEventFilter(this);
    m_view->setAutoFillBackground(false);
    m_view->viewport()->setAutoFillBackground(false);
    m_view->setAttribute(Qt::WA_TranslucentBackground);
    m_view->setAttribute(Qt::WA_OpaquePaintEvent);
    m_view->setAttribute(Qt::WA_NoSystemBackground);
    m_view->viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
    m_view->viewport()->setAttribute(Qt::WA_NoSystemBackground);
    m_view->setWindowFlags(m_view->windowFlags() | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
    m_view->setFrameShape(QFrame::NoFrame);
    KWindowSystem::setOnAllDesktops(m_view->winId(), true);
    unsigned long state = NET::Sticky | NET::StaysOnTop | NET::KeepAbove | NET::SkipTaskbar | NET::SkipPager | NET::MaxVert | NET::MaxHoriz;
    KWindowSystem::setState(m_view->effectiveWinId(), state);
    KWindowSystem::setType(m_view->effectiveWinId(), NET::Dock);
}

FullScreenDialog::~FullScreenDialog()
{
    delete m_view;
}

QGraphicsObject *FullScreenDialog::mainItem() const
{
    return m_mainItem.data();
}

void FullScreenDialog::setMainItem(QGraphicsObject *mainItem)
{
    if (m_mainItem.data() != mainItem) {
        if (m_mainItem) {
            m_mainItem.data()->setParent(mainItem->parent());
            m_mainItem.data()->removeEventFilter(this);
            m_mainItem.data()->setY(0);
            m_scene = 0;
        }
        m_mainItem = mainItem;
        if (mainItem) {
            mainItem->setParentItem(0);
            mainItem->setParent(this);
            m_scene = mainItem->scene();
        }

        mainItem->installEventFilter(this);

        //if this is called in Compenent.onCompleted we have to wait a loop the item is added to a scene
        QTimer::singleShot(0, this, SLOT(syncMainItem()));
        emit mainItemChanged();
    }
}

void FullScreenDialog::syncMainItem()
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
                    ++s_numItems;
                    Plasma::Corona *corona = qobject_cast<Plasma::Corona *>(scene);
                    QDeclarativeItem *di = qobject_cast<QDeclarativeItem *>(m_mainItem.data());

                    if (corona && di) {
                        if (!m_declarativeItemContainer) {
                            m_declarativeItemContainer = new DeclarativeItemContainer();
                            scene->addItem(m_declarativeItemContainer);
                            corona->addOffscreenWidget(m_declarativeItemContainer);
                        }
                        m_declarativeItemContainer->setDeclarativeItem(di);
                    } else {
                        m_mainItem.data()->setY(-10000*s_numItems);
                        m_mainItem.data()->setY(10000*s_numItems);
                    }
                    break;
                }
            }
        }
    }

    if (!scene) {
        return;
    }

    m_view->setScene(scene);

    m_view->setMinimumSize(QSize(m_mainItem.data()->boundingRect().width(), m_mainItem.data()->boundingRect().height()));
    m_view->setMaximumSize(m_view->minimumSize());

    QRectF itemGeometry(QPointF(m_mainItem.data()->x(), m_mainItem.data()->y()),
                        QSizeF(m_mainItem.data()->boundingRect().size()));

    m_view->setSceneRect(itemGeometry);
}

bool FullScreenDialog::isVisible() const
{
    return m_view->isVisible();
}

void FullScreenDialog::setVisible(const bool visible)
{
    if (m_view->isVisible() != visible) {
        m_view->setVisible(visible);
        if (visible) {
            m_view->setVisible(visible);
            m_view->raise();
        }
        emit visibleChanged();
    }
}


bool FullScreenDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_view &&
        (event->type() == QEvent::Resize || event->type() == QEvent::Move)) {
        syncMainItem();
    }
    return false;
}



#include "fullscreendialog.moc"

