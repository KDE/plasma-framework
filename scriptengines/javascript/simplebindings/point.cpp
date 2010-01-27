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
#include <QtCore/QPoint>
#include "../backportglobal.h"

Q_DECLARE_METATYPE(QPoint*)
Q_DECLARE_METATYPE(QPoint)

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 2)
    {
        int x = ctx->argument(0).toInt32();
        int y = ctx->argument(1).toInt32();
        return qScriptValueFromValue(eng, QPoint(x, y));
    }

    return qScriptValueFromValue(eng, QPoint());
}

static QScriptValue null(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPoint, null);
    return QScriptValue(eng, self->isNull());
}

static QScriptValue manhattanLength(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPoint, manhattanLength);
    return QScriptValue(eng, self->manhattanLength());
}

static QScriptValue x(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPoint, x);

    if (ctx->argumentCount() > 0) {
        int x = ctx->argument(0).toInt32();
        self->setX(x);
    }

    return QScriptValue(eng, self->x());
}

static QScriptValue y(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPoint, y);

    if (ctx->argumentCount() > 0) {
        int y = ctx->argument(0).toInt32();
        self->setY(y);
    }

    return QScriptValue(eng, self->y());
}

QScriptValue constructQPointClass(QScriptEngine *eng)
{
    QScriptValue proto = qScriptValueFromValue(eng, QPoint());
    QScriptValue::PropertyFlags getter = QScriptValue::PropertyGetter;
    QScriptValue::PropertyFlags setter = QScriptValue::PropertySetter;

    proto.setProperty("null", eng->newFunction(null), getter);
    proto.setProperty("manhattanLength", eng->newFunction(manhattanLength), getter);
    proto.setProperty("x", eng->newFunction(x), getter | setter);
    proto.setProperty("y", eng->newFunction(y), getter | setter);

    eng->setDefaultPrototype(qMetaTypeId<QPoint>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QPoint*>(), proto);

    return eng->newFunction(ctor, proto);
}
