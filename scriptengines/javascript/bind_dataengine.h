/*
 *   Copyright 2007 Richard J. Moore <rich@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#ifndef BIND_DATAENGINE_H
#define BIND_DATAENGINE_H

#include <QtScript/QtScript>
#include <KDebug>

#include <Plasma/DataEngine>
#include <Plasma/Service>
#include <Plasma/ServiceJob>

using namespace Plasma;

Q_DECLARE_METATYPE(DataEngine*)
Q_DECLARE_METATYPE(Service*)
Q_DECLARE_METATYPE(ServiceJob*)
Q_DECLARE_METATYPE(QVariant)
Q_DECLARE_METATYPE(DataEngine::Dict)
Q_DECLARE_METATYPE(DataEngine::Data)


template <class M>
QScriptValue qScriptValueFromMap(QScriptEngine *eng, const M &map)
{
    kDebug() << "qScriptValueFromMap called";

    QScriptValue obj = eng->newObject();
    typename M::const_iterator begin = map.constBegin();
    typename M::const_iterator end = map.constEnd();
    typename M::const_iterator it;
    for (it = begin; it != end; ++it)
        obj.setProperty(it.key(), qScriptValueFromValue(eng, it.value()));
    return obj;
}

template <class M>
void qScriptValueToMap(const QScriptValue &value, M &map)
{
    QScriptValueIterator it(value);
    while (it.hasNext()) {
        it.next();
        map[it.name()] = qscriptvalue_cast<typename M::mapped_type>(it.value());
    }
}

template<typename T>
int qScriptRegisterMapMetaType(
    QScriptEngine *engine,
    const QScriptValue &prototype = QScriptValue()
#ifndef qdoc
    , T * /* dummy */ = 0
#endif
)
{
    return qScriptRegisterMetaType<T>(engine, qScriptValueFromMap,
                                      qScriptValueToMap, prototype);
}

/*
 * Workaround the fact that QtScripts handling of variants seems a bit broken.
 */
QScriptValue variantToScriptValue(QScriptEngine *engine, QVariant var)
{
    if (var.isNull()) {
        return engine->nullValue();
    }

    switch(var.type())
    {
        case QVariant::Invalid:
            return engine->nullValue();
        case QVariant::Bool:
            return QScriptValue(engine, var.toBool());
        case QVariant::Date:
            return engine->newDate(var.toDateTime());
        case QVariant::DateTime:
            return engine->newDate(var.toDateTime());
        case QVariant::Double:
            return QScriptValue(engine, var.toDouble());
        case QVariant::Int:
        case QVariant::LongLong:
            return QScriptValue(engine, var.toInt());
        case QVariant::String:
            return QScriptValue(engine, var.toString());
        case QVariant::Time: {
            QDateTime t(QDate::currentDate(), var.toTime());
            return engine->newDate(t);
        }
        case QVariant::UInt:
            return QScriptValue(engine, var.toUInt());
        default:
            if (var.typeName() == QLatin1String("KUrl")) {
                return QScriptValue(engine, var.value<KUrl>().prettyUrl());
            } else if (var.typeName() == QLatin1String("QColor")) {
                return QScriptValue(engine, var.value<QColor>().name());
            } else if (var.typeName() == QLatin1String("QUrl")) {
                return QScriptValue(engine, var.value<QUrl>().toString());
            }
            break;
    }

    return qScriptValueFromValue(engine, var);
}
QScriptValue qScriptValueFromData(QScriptEngine *engine, const DataEngine::Data &data)
{
    DataEngine::Data::const_iterator begin = data.begin();
    DataEngine::Data::const_iterator end = data.end();
    DataEngine::Data::const_iterator it;

    QScriptValue obj = engine->newObject();

    for (it = begin; it != end; ++it) {
        //kDebug() << "setting" << it.key() << "to" << it.value();
        QString prop = it.key();
        prop.replace(' ', '_');
        obj.setProperty(prop, variantToScriptValue(engine, it.value()));
    }

    return obj;
}

#endif // BIND_DATAENGINE_H

