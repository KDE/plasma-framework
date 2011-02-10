/*
 *   Copyright 2011 Marco Martin <mart@gmail.com>
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

#ifndef TESTOBJECT_P_H
#define TESTOBJECT_P_H

#include <QObject>

class TestObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString prop READ prop WRITE setProp NOTIFY propChanged)

public:
    void setProp(const QString &prop)
    {
        m_prop = prop;
        emit propChanged();
    }

    QString prop() const
    {
        return m_prop;
    }

Q_SIGNALS:
    void propChanged();

private:
    QString m_prop;
};

#endif
