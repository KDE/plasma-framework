/*
 *   Copyright 2012 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef ICONITEM_H
#define ICONITEM_H

#include <QDeclarativeItem>
#include <QPixmap>
#include <QVariant>

class QPropertyAnimation;

namespace Plasma {
    class Svg;
}

class IconItem : public QDeclarativeItem
{
    Q_OBJECT

    Q_PROPERTY(QVariant source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(bool smooth READ smooth WRITE setSmooth NOTIFY smoothChanged)
    Q_PROPERTY(qreal implicitWidth READ implicitWidth WRITE setImplicitWidth NOTIFY implicitWidthChanged)
    Q_PROPERTY(qreal implicitHeight READ implicitHeight WRITE setImplicitHeight NOTIFY implicitHeightChanged)
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(bool valid READ isValid NOTIFY validChanged)

public:

    IconItem(QDeclarativeItem *parent=0);
    ~IconItem();

    void setSource(const QVariant &source);
    QVariant source() const;

    bool isActive() const;
    void setActive(bool active);

    void setImplicitWidth(qreal width);
    qreal implicitWidth() const;

    void setImplicitHeight(qreal height);
    qreal implicitHeight() const;

    void setSmooth(const bool smooth);
    bool smooth() const;

    bool isValid() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void geometryChanged(const QRectF &newGeometry,
                         const QRectF &oldGeometry);

Q_SIGNALS:
    void activeChanged();
    void sourceChanged();
    void smoothChanged();
    void validChanged();
    void implicitWidthChanged();
    void implicitHeightChanged();

private Q_SLOTS:
    void loadPixmap();
    void animationFinished();
    void valueChanged(const QVariant &value);

private:
    //all the ways we can set an source. Only one of them will be valid
    QIcon m_icon;
    Plasma::Svg *m_svgIcon;
    QPixmap m_pixmapIcon;
    QImage m_imageIcon;
    //this contains the raw variant it was passed
    QVariant m_source;

    QSizeF m_implicitSize;

    bool m_smooth;
    bool m_active;

    //This list contains at most 2 sources, when a pixmap transition is due,
    //a new pixmap is queued, the old one is removed when the animation finishes
    QList<QPixmap> m_iconPixmaps;

    //animation on pixmap change
    QPropertyAnimation *m_animation;
    qreal m_animValue;
};

#endif
