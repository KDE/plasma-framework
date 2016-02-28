/*
 *   Copyright 2014 by Aleix Pol Gonzalez <aleixpol@blue-systems.com>
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

#include "timetracker.h"
#include <containment.h>
#include <QDateTime>
#include <qmetaobject.h>
#include <QFile>
#include <qjsondocument.h>
#include <qjsonarray.h>
#include <QTimer>

using namespace Plasma;

struct TimeTrackerWriter : QObject {
    Q_OBJECT
public:
    TimeTrackerWriter() {
        QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, static_cast<void (TimeTrackerWriter::*)()>(&TimeTrackerWriter::print));
    }

    void print() {
        QJsonArray array;

        Q_FOREACH(const ObjectHistory& history, m_data) {
            QVariantMap map;
            map[QStringLiteral("events")] = serializeEvents(history.events);
            map[QStringLiteral("initial")] = history.initial;

            array.append(QJsonValue::fromVariant(map));
        }
        Q_ASSERT(array.count() == m_data.count());
        QJsonDocument doc;
        doc.setArray(array);

        QFile f(QStringLiteral("/tmp/debug-")+qgetenv("USER")+".json");
        bool b = f.open(QFile::WriteOnly);
        Q_ASSERT(b);
        f.write(doc.toJson());
    }

    void feed(QObject* obj, const ObjectHistory& tracker)
    {
        m_data[obj] = tracker;
    }

    QHash<QObject*, ObjectHistory> m_data;

private:
    QVariantList serializeEvents(const QVector<TimeEvent>& events) const {
        QVariantList ret;
        Q_ASSERT(!events.isEmpty());
        foreach(const TimeEvent& ev, events) {
            QVariantMap map;
            map[QStringLiteral("comment")] = ev.comment;
            map[QStringLiteral("time")] = ev.moment.toMSecsSinceEpoch();
            ret.append(map);
        }
        Q_ASSERT(ret.count() == events.count());
        return ret;
    }
};
Q_GLOBAL_STATIC(TimeTrackerWriter, s_writer);

TimeTracker::TimeTracker(QObject* o)
    : QObject(o)
{
    m_history.events.append(TimeEvent { QDateTime::currentDateTime(), QStringLiteral("constructed %1 %2").arg(o->metaObject()->className(), o->objectName()) });

    QTimer* t = new QTimer(this);
    t->setInterval(2000);
    t->setSingleShot(false);
    connect(t, SIGNAL(timeout()), this, SLOT(sync()));
    t->start();

    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

void TimeTracker::init()
{
    QMetaMethod propChange = metaObject()->method(metaObject()->indexOfSlot("propertyChanged()"));
    Q_ASSERT(propChange.isValid() && metaObject()->indexOfSlot("propertyChanged()")>=0);

    QObject* o = parent();
    for (int i = 0, pc = o->metaObject()->propertyCount(); i<pc; ++i) {
        QMetaProperty prop = o->metaObject()->property(i);
        if (prop.isFinal() || prop.isConstant())
            m_history.initial[prop.name()] = prop.read(o);

        if (prop.hasNotifySignal())
            connect(o, prop.notifySignal(), this, propChange);
    }
}

TimeTracker::~TimeTracker()
{
    sync();
}

void TimeTracker::sync()
{
    s_writer->feed(parent(), m_history);
}

void TimeTracker::propertyChanged()
{
    Q_ASSERT(sender() == parent());

    const QMetaObject* mo = parent()->metaObject();
    for (int i = 0, pc = mo->propertyCount(); i<pc; ++i) {
        QMetaProperty prop = mo->property(i);
        if (prop.notifySignalIndex() == senderSignalIndex()) {
            QString val;
            if (prop.type() < QVariant::UserType) {
                QVariant var = prop.read(parent());
                if(var.canConvert<QString>()) {
                    val = var.toString();
                }
            }

            if (val.isEmpty()) {
                val = QStringLiteral("<unknown %1>").arg(prop.typeName());
            }
            m_history.events.append(TimeEvent { QDateTime::currentDateTime(), QStringLiteral("property %1 changed to %2").arg(prop.name(), val)});
            break;
        }
    }
}

#include "timetracker.moc"
