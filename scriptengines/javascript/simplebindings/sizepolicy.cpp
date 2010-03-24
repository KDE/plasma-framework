/*
 *   Copyright (c) 2010 Aaron J. Seigo <aseigo@kde.org>
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
#include <QtGui/QSizePolicy>
#include "../backportglobal.h"

Q_DECLARE_METATYPE(QSizePolicy*)

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    QSizePolicy::Policy h(QSizePolicy::Fixed);
    QSizePolicy::Policy v(QSizePolicy::Fixed);
    if (ctx->argumentCount() > 1) {
        h = static_cast<QSizePolicy::Policy>(ctx->argument(0).toInt32());
        v = static_cast<QSizePolicy::Policy>(ctx->argument(1).toInt32());
    }

    QScriptValue value = qScriptValueFromValue(eng, QSizePolicy(h, v));
    return value;
}

static QScriptValue horizontalPolicy(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QSizePolicy, horizontalPolicy);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setHorizontalPolicy(static_cast<QSizePolicy::Policy>(arg.toInt32()));
    }

    return QScriptValue(eng, self->horizontalPolicy());
}

static QScriptValue verticalPolicy(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QSizePolicy, vertialPolicy);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setVerticalPolicy(static_cast<QSizePolicy::Policy>(arg.toInt32()));
    }

    return QScriptValue(eng, self->verticalPolicy());
}

static QScriptValue horizontalStretch(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QSizePolicy, horizontalStretch);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setHorizontalStretch(arg.toInt32());
    }

    return QScriptValue(eng, self->horizontalStretch());
}

static QScriptValue verticalStretch(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QSizePolicy, verticalStretch);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setVerticalStretch(arg.toInt32());
    }

    return QScriptValue(eng, self->verticalStretch());
}

QScriptValue constructQSizePolicyClass(QScriptEngine *eng)
{
    QScriptValue proto = qScriptValueFromValue(eng, QSizePolicy());
    QScriptValue::PropertyFlags getter = QScriptValue::PropertyGetter;
    QScriptValue::PropertyFlags setter = QScriptValue::PropertySetter;
    proto.setProperty("horizontalPolicy", eng->newFunction(horizontalPolicy), getter | setter);
    proto.setProperty("verticalPolicy", eng->newFunction(verticalPolicy), getter | setter);
    proto.setProperty("horizontalStretch", eng->newFunction(horizontalStretch), getter | setter);
    proto.setProperty("verticalStretch", eng->newFunction(verticalStretch), getter | setter);

    eng->setDefaultPrototype(qMetaTypeId<QSizePolicy>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QSizePolicy*>(), proto);

    return eng->newFunction(ctor, proto);
}
