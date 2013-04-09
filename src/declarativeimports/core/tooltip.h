/***************************************************************************
 *   Copyright 2011 Marco Martin <mart@kde.org>                            *
 *   Copyright 2011 Artur Duque de Souza <asouza@kde.org>                  *
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

#ifndef TOOLTIPOBJECT_H
#define TOOLTIPOBJECT_H

#include <QTimer>
#include <QQuickWindow>
#include <QQmlComponent>
#include <QWeakPointer>
#include <QtCore/QVariant>

class QQuickItem;
class QGraphicsWidget;

/**
 * Exposed as `ToolTipProxy` in QML.
 */
class ToolTip : public QObject
{
    Q_OBJECT

    /**
     * The item that will display this tooltip on mouse over 
     */
    Q_PROPERTY(QQuickItem *target READ target WRITE setTarget NOTIFY targetChanged)

    /**
     * T
     */
    Q_PROPERTY(QQuickItem *mainItem READ mainItem WRITE setMainItem NOTIFY mainItemChanged)


    /**
     * The main QML item that will be displayed in the Dialog
     */
    Q_PROPERTY(QQuickItem *visualParent READ visualParent WRITE setVisualParent NOTIFY visualParentChanged)

    /**
     * Visibility of the Dialog window. Doesn't have anything to do with the visibility of the mainItem.
     */
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)


public:
    ToolTip(QObject *parent = 0);
    ~ToolTip();

    QQuickItem *target() const;
    void setTarget(QQuickItem *target);

    QQmlComponent* mainComponent() const;
    void setMainComponent(QQmlComponent *mainComponent);

    QQuickItem *mainItem() const;
    void setMainItem(QQuickItem *mainItem);

    QQuickItem *visualParent() const;
    void setVisualParent(QQuickItem *visualParent);

    bool isVisible() const;
    void setVisible(const bool visible);

    QPoint popupPosition(QQuickItem *item = 0, Qt::AlignmentFlag alignment=Qt::AlignCenter) ;

Q_SIGNALS:
    void targetChanged();
    void mainComponentChanged();
    void mainItemChanged();
    void visualParentChanged();
    void visibleChanged();

private:
    QTimer *m_syncTimer;
    QWeakPointer<QQmlComponent> m_mainComponent;
    QWeakPointer<QQuickItem> m_mainItem;
    QWeakPointer<QQuickItem> m_visualParent;
    QWeakPointer<QQuickItem> m_declarativeItemContainer;
    QWeakPointer<QQuickItem> m_target;
};

#endif
