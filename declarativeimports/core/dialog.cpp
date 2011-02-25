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

#include <QDeclarativeItem>
#include <QGraphicsObject>
#include <QGraphicsWidget>
#include <QTimer>

#include <Plasma/Corona>
#include <Plasma/Dialog>

class DeclarativeItemContainer : public QGraphicsWidget
{
public:
    DeclarativeItemContainer(QGraphicsItem *parent = 0)
       : QGraphicsWidget(parent)
    {}

    ~DeclarativeItemContainer()
    {}

    void setDeclarativeItem(QDeclarativeItem *item)
    {
        m_declarativeItem = item;
        static_cast<QGraphicsItem *>(item)->setParentItem(this);
        setMinimumWidth(item->implicitWidth());
        setMinimumHeight(item->implicitHeight());
    }

    QDeclarativeItem *declarativeItem() const
    {
        return m_declarativeItem.data();
    }

protected:
    void resizeEvent(QGraphicsSceneResizeEvent *event)
    {
        if (m_declarativeItem) {
            m_declarativeItem.data()->setProperty("width", event->newSize().width());
            m_declarativeItem.data()->setProperty("height", event->newSize().height());
        }
    }

private:
    QWeakPointer<QDeclarativeItem> m_declarativeItem;
};

DialogProxy::DialogProxy(QObject *parent)
    : QObject(parent)
{
    m_dialog = new Plasma::Dialog();
}

DialogProxy::~DialogProxy()
{
    delete m_dialog;
    delete m_declarativeItemContainer;
}

QGraphicsObject *DialogProxy::mainItem() const
{
    return m_dialog->graphicsWidget();
}

void DialogProxy::setMainItem(QGraphicsObject *mainItem)
{
    if (m_mainItem.data() != mainItem) {
        m_mainItem = mainItem;

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
                scene->addItem(m_mainItem.data());
                break;
            }
        }
    }

    //the parent of the qobject never changed, only the parentitem, so put it back what it was
    m_mainItem.data()->setParentItem(qobject_cast<QGraphicsObject *>(m_mainItem.data()->parent()));

    QGraphicsWidget *widget = qobject_cast<QGraphicsWidget *>(m_mainItem.data());
    if (widget) {
        m_declarativeItemContainer->deleteLater();
        m_declarativeItemContainer = 0;
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
}

bool DialogProxy::isVisible() const
{
    return m_dialog->isVisible();
}

void DialogProxy::setVisible(const bool visible)
{
    if (m_dialog->isVisible() != visible) {
        m_dialog->setVisible(visible);
        emit visibleChanged();
    }
}

void DialogProxy::showPopup(QGraphicsObject *item)
{
    if (m_dialog->isVisible()) {
        m_dialog->hide();
    } else {
        Plasma::Corona *corona = qobject_cast<Plasma::Corona *>(item->scene());
        if (corona) {
            m_dialog->move(corona->popupPosition(item, m_dialog->size()));
        }
        m_dialog->show();
    }
    emit visibleChanged();
}

void DialogProxy::setAttribute(int attribute, bool on)
{
    m_dialog->setAttribute((Qt::WidgetAttribute)attribute, on);
}

#include "dialog.moc"

