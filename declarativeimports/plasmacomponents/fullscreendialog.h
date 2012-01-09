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
#ifndef FULLSCREENDIALOG_P
#define FULLSCREENDIALOG_P

#include <QObject>
#include <QPoint>
#include <QRect>
#include <QWeakPointer>
#include <QDeclarativeListProperty>
#include <QDeclarativeItem>

#include "enums.h"

class QGraphicsObject;
class QGraphicsView;
class QGraphicsScene;
class DeclarativeItemContainer;

class FullScreenDialog : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QDeclarativeListProperty<QDeclarativeItem> title READ title)
    Q_PROPERTY(QDeclarativeListProperty<QDeclarativeItem> content READ content)
    Q_PROPERTY(QDeclarativeListProperty<QDeclarativeItem> buttons READ buttons)
    Q_PROPERTY(DialogStatus::Status status READ status NOTIFY statusChanged)
    /*property alias content: contentItem.children
    property alias buttons: buttonItem.children
    property int status: DialogStatus.Closed*/


public:
    FullScreenDialog(QObject *parent = 0);
    ~FullScreenDialog();

    QGraphicsObject *mainItem() const;
    void setMainItem(QGraphicsObject *mainItem);

    bool isVisible() const;
    void setVisible(const bool visible);

    //QML properties
    QDeclarativeListProperty<QDeclarativeItem> title()const ;
    QDeclarativeListProperty<QDeclarativeItem> content() const;
    QDeclarativeListProperty<QDeclarativeItem> buttons() const;
    DialogStatus::Status status() const;

Q_SIGNALS:
    void accepted();
    void rejected();
    void clickedOutside();
    void statusChanged();


protected Q_SLOTS:
    void syncMainItem();

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    QGraphicsView *m_view;
    QWeakPointer<QGraphicsObject> m_mainItem;
    DeclarativeItemContainer *m_declarativeItemContainer;
    QGraphicsScene *m_scene;
    QWeakPointer<QObject> m_rootObject;
    static uint s_numItems;
};

#endif
