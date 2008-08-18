/*
 *   Copyright 2008 by Aaron Seigo <aseigo@kde.org>
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

#ifndef PLASMA_CONTEXT_H
#define PLASMA_CONTEXT_H

#include <QtCore/QObject>
#include <QtCore/QStringList>

#include "plasma_export.h"

namespace Plasma
{

class ContextPrivate;

class PLASMA_EXPORT Context : QObject
{
    Q_OBJECT

public:
    explicit Context(QObject *parent = 0);
    ~Context();

    void createContext(const QString &name);
    QStringList contexts() const;

    void setCurrentContext(const QString &name);
    QString currentContext() const;

Q_SIGNALS:
    void currentChanged(const QString &);

private:
    ContextPrivate * const d;
};

} // namespace Plasma

#endif // multiple inclusion guard

