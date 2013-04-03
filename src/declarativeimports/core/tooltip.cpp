/***************************************************************************
 *   Copyright 2011 Marco Martin <mart@kde.org>                            *
 *   Copyright 2011 Artur Duque de Souza <asouza@kde.org>                  *
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

#include "tooltip.h"

#include <QQuickItem>
// #include <QGraphicsWidget>
// #include <QGraphicsScene>
#include <QDebug>
#include <QTimer>

//#include <KIcon>
// #include <KIconLoader>
// #include <Plasma/ToolTipContent>
// #include <Plasma/ToolTipManager>


ToolTipWindow::ToolTipWindow(QWindow *parent)
    : QQuickWindow(parent), m_mainText(""), m_subText(""), m_widget(0)
{
    QSurfaceFormat format;
    format.setAlphaBufferSize(8);
    setFormat(format);
    setClearBeforeRendering(true);
    setColor(QColor(Qt::transparent));
    setFlags(Qt::FramelessWindowHint);
//             tooltipDialog.setAttribute(Qt.WA_X11NetWmWindowTypeToolTip, true)
//             tooltipDialog.windowFlags = Qt.Window|Qt.WindowStaysOnTopHint|Qt.X11BypassWindowManagerHint

    //m_flags = flags();

    m_syncTimer = new QTimer(this);
    m_syncTimer->setSingleShot(true);
    m_syncTimer->setInterval(250);
//    connect(m_syncTimer, &QTimer::timeout, this,  &DialogProxy::syncToMainItemSize);

    connect(this, SIGNAL(targetChanged()), this, SLOT(updateToolTip()));
    connect(this, SIGNAL(mainTextChanged()), this, SLOT(updateToolTip()));
    connect(this, SIGNAL(subTextChanged()), this, SLOT(updateToolTip()));
    connect(this, SIGNAL(imageChanged()), this, SLOT(updateToolTip()));
}

ToolTipWindow::~ToolTipWindow()
{
}

QQuickItem *ToolTipWindow::target() const
{
    return m_target.data();
}

void ToolTipWindow::setTarget(QQuickItem *target)
{
    if (m_target.data() != target) {
        m_target = target;
/*
        m_widget = qobject_cast<QGraphicsWidget*>(m_target.data());
        if (!m_widget) {
            // if this is called in Compenent.onCompleted we have to
            // wait a loop for the item to be added to a scene
            QTimer::singleShot(0, this, SLOT(syncTarget()));
            return;
        }*/
        emit targetChanged();
    }
}

void ToolTipWindow::syncTarget()
{
    /*
    if (!m_target) {
        return;
    }
    // find the scene
    QGraphicsScene *scene = m_target.data()->scene();
    if (!scene) {
        QObject *parent = m_target.data();
        while ((parent = parent->parent())) {
            QQuickItem *qo = qobject_cast<QQuickItem*>(parent);
            if (qo && qo->scene()) {
                scene = qo->scene();
                scene->addItem(m_target.data());
                break;
            }
        }
    }

    QQuickItem *item = qobject_cast<QQuickItem*>(m_target.data());
    if (!item) {
        return;
    }

    if (!m_declarativeItemContainer && scene) {
        m_declarativeItemContainer = QWeakPointer<QQuickItem>(new QQuickItem());
        m_declarativeItemContainer.data()->setObjectName("DIContainer");
        scene->addItem(m_declarativeItemContainer.data());
    }

    if (m_declarativeItemContainer) {
        m_target.data()->setObjectName("Original Item");
        m_declarativeItemContainer.data()->setDeclarativeItem(item, false);
        m_declarativeItemContainer.data()->setAcceptHoverEvents(true);
        m_declarativeItemContainer.data()->setParentItem(m_target.data());
        m_widget = m_declarativeItemContainer.data();
        emit targetChanged();
    }
    */
}

QString ToolTipWindow::mainText() const
{
    return m_mainText;
}

void ToolTipWindow::setMainText(const QString &text)
{
    if (text == m_mainText) {
        return;
    }

    m_mainText = text;
    emit mainTextChanged();
}

QString ToolTipWindow::subText() const
{
    return m_subText;
}

void ToolTipWindow::setSubText(const QString &text)
{
    if (text == m_subText) {
        return;
    }

    m_subText = text;
    emit subTextChanged();
}

QVariant ToolTipWindow::image() const
{
    return m_image;
}

void ToolTipWindow::setImage(QVariant name)
{
    if (name == m_image) {
        return;
    }

    m_image = name;
    emit imageChanged();
}

void ToolTipWindow::updateToolTip()
{
    if (!m_widget) {
        return;
    }

//     Plasma::ToolTipContent data;
//     data.setMainText(m_mainText);
//     data.setSubText(m_subText);

    // set image
    switch (m_image.type()) {
        case QVariant::String: {
            QString name = m_image.toString();
            if (!name.isEmpty()) {
                QIcon icon = QIcon::fromTheme(name);
                if (!icon.isNull()) {
//                     data.setImage(icon.pixmap(IconSize(KIconLoader::Desktop)));
                }
            }
            break;
        }

        case QVariant::Icon: {
            QIcon icon = m_image.value<QIcon>();
//             data.setImage(icon);
            break;
        }

        case QVariant::Pixmap: {
            QPixmap pixmap = m_image.value<QPixmap>();
//             data.setImage(pixmap);
            break;
        }

        default:
            break;
    }
    //Plasma::ToolTipManager::self()->setContent(m_widget, data);
}

QQuickItem *ToolTipWindow::mainItem() const
{
    return m_mainItem.data();
}

void ToolTipWindow::setMainItem(QQuickItem *mainItem)
{
    qDebug() << "mainitem changed: " << mainItem->width() << mainItem->height();

    //resize(mainItem->width(), mainItem->height());

    resize(200, 200);
    if (m_mainItem.data() != mainItem) {
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
        }

        //if this is called in Compenent.onCompleted we have to wait a loop the item is added to a scene
        emit mainItemChanged();
    }
}

QQuickItem *ToolTipWindow::visualParent() const
{
    return m_visualParent.data();
}

void ToolTipWindow::setVisualParent(QQuickItem *visualParent)
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


bool ToolTipWindow::isVisible() const
{
    return QQuickWindow::isVisible();

}

void ToolTipWindow::setVisible(const bool visible)
{
    qDebug() << visible;
    if (visible) {
        setPosition(popupPosition());
        raise();
    }
    QQuickWindow::setVisible(visible);
}

QPoint ToolTipWindow::popupPosition(QQuickItem *item, Qt::AlignmentFlag alignment)
{
    // FIXME :: Item
    QQuickItem *parentItem = qobject_cast<QQuickItem *>(parent());
    if (parentItem && parentItem->window()) {
        qDebug() << "NO visual parent ... Centering at " << (parentItem->window()->geometry().center() - QPoint(width()/2, height()/2));
        qDebug() << parentItem->window()->geometry().center() - QPoint(width()/2, height()/2);
        return parentItem->window()->geometry().center() - QPoint(width()/2, height()/2);
    } else {
        qDebug() << "No QQuickItem as parent found";
        return QPoint();
    }
}
//#include "tooltip.moc"

