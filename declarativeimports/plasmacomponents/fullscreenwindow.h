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
#ifndef FULLSCREENWINDOW_P
#define FULLSCREENWINDOW_P

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
class Background;

class FullScreenWindow : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(QDeclarativeListProperty<QGraphicsObject> title READ title DESIGNABLE false)
    Q_PROPERTY(QDeclarativeListProperty<QGraphicsObject> content READ content DESIGNABLE false)
    Q_PROPERTY(QDeclarativeListProperty<QGraphicsObject> buttons READ buttons DESIGNABLE false)
    Q_PROPERTY(DialogStatus::Status status READ status NOTIFY statusChanged)


public:
    FullScreenWindow(QDeclarativeItem *parent = 0);
    ~FullScreenWindow();

    QGraphicsObject *mainItem() const;
    void setMainItem(QGraphicsObject *mainItem);

    bool isVisible() const;
    void setVisible(const bool visible);

    QGraphicsView *view() const;

    //QML properties
    QDeclarativeListProperty<QGraphicsObject> title();
    QDeclarativeListProperty<QGraphicsObject> content();
    QDeclarativeListProperty<QGraphicsObject> buttons();
    DialogStatus::Status status() const;

    Q_INVOKABLE void open();
    Q_INVOKABLE void accept();
    Q_INVOKABLE void reject();
    Q_INVOKABLE void close();

Q_SIGNALS:
    void accepted();
    void rejected();
    void clickedOutside();
    void statusChanged();


private Q_SLOTS:
    void syncViewToMainItem();
    void syncMainItemToView();
    void statusHasChanged();

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void init(const QString &componentName);

private:
    QGraphicsView *m_view;
    QWeakPointer<QGraphicsObject> m_mainItem;
    DeclarativeItemContainer *m_declarativeItemContainer;
    QGraphicsScene *m_scene;
    QWeakPointer<QObject> m_rootObject;
    static uint s_numItems;
    Background *m_background;

    //those only used in case of error, to not make plasma crash
    QList<QGraphicsObject *> m_dummyTitleElements;
    QList<QGraphicsObject *> m_dummyContentElements;
    QList<QGraphicsObject *> m_dummyButtonsElements;
    friend class Background;
};

#endif
