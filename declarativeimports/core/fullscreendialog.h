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
#include <QTimer>
#include <QWeakPointer>

class QGraphicsObject;
class QGraphicsView;
class QGraphicsScene;
class DeclarativeItemContainer;

class FullScreenDialog : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QGraphicsObject *mainItem READ mainItem WRITE setMainItem NOTIFY mainItemChanged)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)

public:
    FullScreenDialog(QObject *parent = 0);
    ~FullScreenDialog();

    QGraphicsObject *mainItem() const;
    void setMainItem(QGraphicsObject *mainItem);

    bool isVisible() const;
    void setVisible(const bool visible);

Q_SIGNALS:
    void visibleChanged();
    void mainItemChanged();


protected Q_SLOTS:
    void syncMainItem();

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    QGraphicsView *m_view;
    QWeakPointer<QGraphicsObject> m_mainItem;
    DeclarativeItemContainer *m_declarativeItemContainer;
    QGraphicsScene *m_scene;
    static uint s_numItems;
};

#endif
