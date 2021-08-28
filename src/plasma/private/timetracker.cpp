/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "timetracker.h"
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMetaObject>
#include <QMetaProperty>
#include <QTimer>
#include <containment.h>

using namespace Plasma;

Q_GLOBAL_STATIC_WITH_ARGS(const qint64, s_beginning, (QDateTime::currentDateTime().toMSecsSinceEpoch()))

struct TimeTrackerWriter : QObject {
    Q_OBJECT
public:
    TimeTrackerWriter()
    {
        QObject::connect(QCoreApplication::instance(),
                         &QCoreApplication::aboutToQuit,
                         this,
                         static_cast<void (TimeTrackerWriter::*)()>(&TimeTrackerWriter::print));
    }

    void print()
    {
        QJsonArray array;

        for (const ObjectHistory &history : std::as_const(m_data)) {
            array.append(QJsonObject{{QStringLiteral("events"), serializeEvents(history.events)},
                                     {QStringLiteral("initial"), QJsonValue::fromVariant(history.initial)}});
        }
        Q_ASSERT(array.count() == m_data.count());
        QJsonDocument doc;
        doc.setArray(array);

        QFile f(QStringLiteral("/tmp/debug-") + QString::fromUtf8(qgetenv("USER")) + QStringLiteral(".json"));
        bool b = f.open(QFile::WriteOnly);
        Q_ASSERT(b);
        f.write(doc.toJson());
    }

    void feed(QObject *obj, const ObjectHistory &tracker)
    {
        m_data[obj] = tracker;
    }

    QHash<QObject *, ObjectHistory> m_data;

private:
    QJsonArray serializeEvents(const QVector<TimeEvent> &events) const
    {
        QJsonArray ret;
        Q_ASSERT(!events.isEmpty());
        for (const TimeEvent &ev : events) {
            ret.append(QJsonObject{{QStringLiteral("comment"), ev.comment}, {QStringLiteral("time"), ev.moment.toMSecsSinceEpoch() - *s_beginning}});
        }
        Q_ASSERT(ret.count() == events.count());
        return ret;
    }
};
Q_GLOBAL_STATIC(TimeTrackerWriter, s_writer)

TimeTracker::TimeTracker(QObject *o)
    : QObject(o)
{
    *s_beginning * 1; // ensure it's initialized

    QTimer *t = new QTimer(this);
    t->setInterval(2000);
    t->setSingleShot(false);
    connect(t, &QTimer::timeout, this, &TimeTracker::sync);
    t->start();

    QMetaObject::invokeMethod(this, "init", Qt::QueuedConnection);
}

void TimeTracker::init()
{
    m_history.events.append(TimeEvent{QDateTime::currentDateTime(),
                                      QStringLiteral("constructed %1 %2").arg(QString::fromUtf8(parent()->metaObject()->className()), parent()->objectName())});

    QMetaMethod propChange = metaObject()->method(metaObject()->indexOfSlot("propertyChanged()"));
    Q_ASSERT(propChange.isValid() && metaObject()->indexOfSlot("propertyChanged()") >= 0);

    QObject *o = parent();
    for (int i = 0, pc = o->metaObject()->propertyCount(); i < pc; ++i) {
        QMetaProperty prop = o->metaObject()->property(i);
        m_history.initial[QString::fromUtf8(prop.name())] = prop.read(o);

        if (prop.hasNotifySignal()) {
            connect(o, prop.notifySignal(), this, propChange);
        }
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

    const QMetaObject *mo = parent()->metaObject();

    for (int i = 0, pc = mo->propertyCount(); i < pc; ++i) {
        const QMetaProperty prop = mo->property(i);
        if (prop.notifySignalIndex() == senderSignalIndex()) {
            QString val;
            QDebug d(&val);
            d << prop.read(parent());
            m_history.events.append(
                TimeEvent{QDateTime::currentDateTime(), QStringLiteral("property %1 changed to %2").arg(QString::fromUtf8(prop.name()), val.trimmed())});
        }
    }
}

#include "timetracker.moc"
