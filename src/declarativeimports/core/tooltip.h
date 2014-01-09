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

#include <QQuickItem>
#include <QWeakPointer>
#include <QtCore/QVariant>

class QQuickItem;
class QGraphicsWidget;

/**
 * Exposed as `ToolTipProxy` in QML. This is used only internally for WIndow handling
 *
 * DO NOT USE THIS API, PlasmaCore.ToolTip is what you should use.
 */
class ToolTip : public QQuickItem
{
    Q_OBJECT

    /**
     * The item shown inside the tooltip.
     */
    Q_PROPERTY(QQuickItem *mainItem READ mainItem WRITE setMainItem NOTIFY mainItemChanged)

    /**
     * The main text of this tooltip
     */
    Q_PROPERTY(QString mainText READ mainText WRITE setMainText NOTIFY mainTextChanged)

    /**
     * The description of this tooltip
     */
    Q_PROPERTY(QString subText READ subText WRITE setSubText NOTIFY subTextChanged)

    /**
     * An icon for this tooltip, accepted values are an icon name, a QIcon, QImage or QPixmap
     */
    Q_PROPERTY(QVariant icon READ icon WRITE setIcon NOTIFY iconChanged)

    /**
     * An icon for this tooltip, accepted values are an icon name, a QIcon, QImage or QPixmap
     */
    Q_PROPERTY(bool m_containsMouse READ containsMouse NOTIFY containsMouseChanged)

    /**
     * TODO: single property for images?
     * An image for this tooltip, accepted values are an icon name, a QIcon, QImage or QPixmap
     */
    Q_PROPERTY(QVariant image READ image WRITE setImage NOTIFY imageChanged)

public:
    ToolTip(QQuickItem *parent = 0);
    ~ToolTip();

    QQuickItem *mainItem() const;
    void setMainItem(QQuickItem *mainItem);

    void showToolTip();

    QString mainText() const;
    void setMainText(const QString &mainText);

    QString subText() const;
    void setSubText(const QString &subText);

    QVariant icon() const;
    void setIcon(const QVariant &icon);

    QVariant image() const;
    void setImage(const QVariant &image);

    bool containsMouse() const;
    void setContainsMouse(bool contains);

protected:
    bool childMouseEventFilter(QQuickItem *item, QEvent *event);
    void hoverEnterEvent(QHoverEvent *event);
    void hoverLeaveEvent(QHoverEvent *event);

Q_SIGNALS:
    void mainItemChanged();
    void visibleChanged();
    void mainTextChanged();
    void subTextChanged();
    void iconChanged();
    void imageChanged();
    void containsMouseChanged();

private:
    bool m_containsMouse;
    QWeakPointer<QQuickItem> m_mainItem;
    QTimer *m_showTimer;
    QString m_mainText;
    QString m_subText;
    QVariant m_image;
    QVariant m_icon;
};

#endif
