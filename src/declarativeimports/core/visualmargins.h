/*
 *   Copyright 2013 Marco Martin <mart@kde.org>
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
#include < *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef VISUALMARGINS_H
#define VISUALMARGINS_H

#include <QObject>
#include <QQuickItem>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QPointer>

class VisualMargins : public QObject
{
    Q_OBJECT

    /**
     * Width in pixels of the left margin.
     */
    Q_PROPERTY(qreal left READ left WRITE setLeft NOTIFY marginsChanged)

    /**
     * Height in pixels of the top margin.
     */
    Q_PROPERTY(qreal top READ top WRITE setTop NOTIFY marginsChanged)

    /**
     * Width in pixels of the right margin.
     */
    Q_PROPERTY(qreal right READ right WRITE setRight NOTIFY marginsChanged)

    /**
     * Height in pixels of the bottom margin.
     */
    Q_PROPERTY(qreal bottom READ bottom WRITE setBottom NOTIFY marginsChanged)

    Q_PROPERTY(QObject *other READ binded WRITE bindTo)

public:
    VisualMargins(QObject *parent = 0);
    ~VisualMargins();

    qreal left() const;
    void setLeft(qreal left);
    qreal top() const;
    void setTop(qreal top);
    qreal right() const;
    void setRight(qreal right);
    qreal bottom() const;
    void setBottom(qreal bottom);

    Q_INVOKABLE void bindTo(QObject *other);
    QObject *binded() const {return m_other.data();}

    static VisualMargins *qmlAttachedProperties(QObject *object);

Q_SIGNALS:
    void marginsChanged();

private Q_SLOTS:
    void updateMarginsFromOther();

private:
    qreal m_left;
    qreal m_top;
    qreal m_right;
    qreal m_bottom;

    QPointer<VisualMargins> m_other;
};

QML_DECLARE_TYPEINFO(VisualMargins, QML_HAS_ATTACHED_PROPERTIES)

#endif
