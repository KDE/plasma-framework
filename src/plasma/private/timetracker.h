/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TIMETRACKER_H
#define TIMETRACKER_H

#include <QDateTime>
#include <QObject>
#include <QVariantMap>
#include <QVector>

#include "plasma/plasma_export.h"

namespace Plasma
{
class Containment;
class Applet;

struct TimeEvent {
    QDateTime moment;
    QString comment;
};

struct ObjectHistory {
    QVariantMap initial;
    QVector<TimeEvent> events;
};

/**
 * This debugging class is meant to provide an overview of how the objects change
 * over time and hopefully provide the information required to detect buggy initialization.
 *
 * To use it, you'll pass the object you want to track to the constructor and the TimeTracker
 * will use Qt introspection to read the properties and check what values they have and how they
 * change.
 *
 * To analyze the results one can read the generated json file /tmp/debug-$USER, as soon
 * as the process has quit.
 */

class PLASMA_EXPORT TimeTracker : public QObject
{
    Q_OBJECT
public:
    explicit TimeTracker(QObject *applet);
    ~TimeTracker() override;

private Q_SLOTS:
    void init();
    void sync();
    void propertyChanged();

private:
    ObjectHistory m_history;
};

}

#endif // TIMETRACKER_H
