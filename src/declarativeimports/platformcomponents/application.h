/*
 *   Copyright (C) 2013 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <QString>

#include "utils/d_ptr.h"

class Application: public QObject {
    Q_OBJECT

    Q_PROPERTY(QString application READ application WRITE setApplication NOTIFY applicationChanged);
    Q_PROPERTY(bool running READ running WRITE setRunning NOTIFY runningChanged);

public:
    Application(QObject * parent = Q_NULLPTR);
    ~Application();

    QString application() const;
    bool running() const;

public Q_SLOTS:
    void setApplication(const QString & application);

    void start();
    void terminate();

    void setRunning(bool run);

Q_SIGNALS:
    void applicationChanged(const QString & application);
    void runningChanged(bool running);

private:
    D_PTR;
};

#endif /* APPLICATION_H */

