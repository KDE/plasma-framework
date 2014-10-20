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
    Q_PROPERTY(qreal left READ left NOTIFY marginsChanged)

    /**
     * Height in pixels of the top margin.
     */
    Q_PROPERTY(qreal top READ top NOTIFY marginsChanged)

    /**
     * Width in pixels of the right margin.
     */
    Q_PROPERTY(qreal right READ right NOTIFY marginsChanged)

    /**
     * Height in pixels of the bottom margin.
     */
    Q_PROPERTY(qreal bottom READ bottom NOTIFY marginsChanged)

    qreal marginProperty(const QString &prop) const;

public:
    VisualMargins(QObject *parent = 0);
    ~VisualMargins();

    qreal left() const;
    qreal top() const;
    qreal right() const;
    qreal bottom() const;
    qreal horizontal() const;
    qreal vertical() const;

    static VisualMargins *qmlAttachedProperties(QObject *object)
    {
        return new VisualMargins(object);
    }


Q_SIGNALS:
    void marginsChanged();

private Q_SLOTS:
    void connectMarginObject(QQuickWindow *window);

private:

    QPointer<QObject> m_marginsObject;
};

QML_DECLARE_TYPEINFO(VisualMargins, QML_HAS_ATTACHED_PROPERTIES)

#endif
