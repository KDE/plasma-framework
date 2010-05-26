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

#include "context.h"

namespace Plasma
{

class ContextPrivate
{
public:
    QString activityId;
    QString activityName;
};

Context::Context(QObject *parent)
    : QObject(parent),
      d(new ContextPrivate)
{
    //TODO: look up activity in Nepomuk
    //except we can't, because that code is in kdebase.
}

Context::~Context()
{
      delete d;
}

void Context::createActivity(const QString &name)
{
    Q_UNUSED(name);
}

QStringList Context::listActivities() const
{
    return QStringList();
}

void Context::setCurrentActivity(const QString &name)
{
    if (d->activityName == name || name.isEmpty()) {
        return;
    }

    d->activityName = name;
    emit activityChanged(this);
    emit changed(this);
}

QString Context::currentActivity() const
{
    return d->activityName;
}

void Context::setCurrentActivityId(const QString &id)
{
    if (d->activityId == id) {
        return;
    }

    d->activityId = id;
    emit changed(this);
}

QString Context::currentActivityId() const
{
    return d->activityId;
}

} // namespace Plasma

#include "context.moc"

