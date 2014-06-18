/*
 *   Copyright 2012 Marco Martin <mart@kde.org>
 *   Copyright 2014 David Edmundson <davidedmudnson@kde.org>
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

#include <QIcon>
#include <QQuickItem>
#include <QPixmap>
#include <QVariant>
#include <QTimer>

#include <plasma/svg.h>

class QPropertyAnimation;

class IconItem : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QVariant source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(Plasma::Theme::ColorGroup colorGroup READ colorGroup WRITE setColorGroup NOTIFY colorGroupChanged)
    Q_PROPERTY(bool smooth READ smooth WRITE setSmooth NOTIFY smoothChanged)
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(bool valid READ isValid NOTIFY validChanged)

public:

    IconItem(QQuickItem *parent = 0);
    ~IconItem();

    void setSource(const QVariant &source);
    QVariant source() const;

    void setColorGroup(Plasma::Theme::ColorGroup group);
    Plasma::Theme::ColorGroup colorGroup() const;

    bool isActive() const;
    void setActive(bool active);

    void setSmooth(const bool smooth);
    bool smooth() const;

    bool isValid() const;

    QSGNode* updatePaintNode(QSGNode * oldNode, UpdatePaintNodeData * updatePaintNodeData);

    void geometryChanged(const QRectF &newGeometry,
                         const QRectF &oldGeometry);

Q_SIGNALS:
    void activeChanged();
    void sourceChanged();
    void smoothChanged();
    void validChanged();
    void colorGroupChanged();

private Q_SLOTS:
    void loadPixmap();
    void animationFinished();
    void valueChanged(const QVariant &value);

private:
    int adjustedSize(int size);
    //all the ways we can set an source. Only one of them will be valid
    QIcon m_icon;
    Plasma::Svg *m_svgIcon;
    QPixmap m_pixmapIcon;
    QImage m_imageIcon;
    //this contains the raw variant it was passed
    QVariant m_source;
    QTimer m_loadPixmapTimer;

    QSizeF m_implicitSize;

    bool m_smooth;
    bool m_active;

    bool m_textureChanged;
    bool m_sizeChanged;

    QPixmap m_iconPixmap;
    QPixmap m_oldIconPixmap;

    Plasma::Theme::ColorGroup m_colorGroup;

    //animation on pixmap change
    QPropertyAnimation *m_animation;
    qreal m_animValue;
};

#endif
