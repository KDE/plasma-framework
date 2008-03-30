/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#ifndef PLASMA_TIMER_H
#define PLASMA_TIMER_H

#include <QtCore/QObject>

#include <plasma/plasma_export.h>

namespace Plasma
{

class PLASMA_EXPORT Timer : public QObject
{
    Q_OBJECT

public:
    explicit Timer(QObject *parent = 0);
    ~Timer();
    int interval() const;
    bool isActive() const;
    bool isSingleShot() const;
    void setInterval(int msec);
    void setSingleShot(bool singleShot);
    void activate() const;

public Q_SLOTS:
    void start(int msec);
    void start();
    void stop();

Q_SIGNALS:
    void timeout() const;

private:
    class Private;
    Private * const d;
};

} // namespace Plasma

#endif

