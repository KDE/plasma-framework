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

#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>
#include <QtScript/QScriptable>
#include <QtCore/QTimer>
#include "../backportglobal.h"

Q_DECLARE_METATYPE(QTimer*)

static QScriptValue newTimer(QScriptEngine *eng, QTimer *timer)
{
    return eng->newQObject(timer, QScriptEngine::AutoOwnership);
}

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    return newTimer(eng, new QTimer(qscriptvalue_cast<QObject*>(ctx->argument(0))));
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTimer, toString);
    return QScriptValue(eng, QString::fromLatin1("QTimer(interval=%0)")
                        .arg(self->interval()));
}

static QScriptValue singleShot(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTimer, start);
    if (ctx->argumentCount()) {
        self->setSingleShot(ctx->argument(0).toBool());
    }
    return QScriptValue(eng, self->isSingleShot());
}

static QScriptValue interval(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTimer, interval);
    if (ctx->argumentCount()) {
        self->setInterval(ctx->argument(0).toInt32());
    }
    return QScriptValue(eng, self->interval());
}

static QScriptValue isActive(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QTimer, isActive);
    return QScriptValue(eng, self->isActive());
}

QScriptValue constructTimerClass(QScriptEngine *eng)
{
    QScriptValue proto = newTimer(eng, new QTimer());
    ADD_METHOD(proto, toString);
    eng->setDefaultPrototype(qMetaTypeId<QTimer*>(), proto);
    QScriptValue::PropertyFlags getter = QScriptValue::PropertyGetter;
    QScriptValue::PropertyFlags setter = QScriptValue::PropertySetter;
    proto.setProperty("singleShot", eng->newFunction(singleShot), getter | setter);
    proto.setProperty("interval", eng->newFunction(interval), getter | setter);
    proto.setProperty("isActive", eng->newFunction(isActive), getter);

    return eng->newFunction(ctor, proto);
}
