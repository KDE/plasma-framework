/*
 *   Copyright (c) 2009 Aaron J. Seigo <aseigo@kde.org>
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
#include <QtGui/QPen>
#include "backportglobal.h"

Q_DECLARE_METATYPE(QPen*)

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    Q_UNUSED(ctx)
    return qScriptValueFromValue(eng, QPen());
}

static QScriptValue brush(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, brush);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setBrush(qscriptvalue_cast<QBrush>(arg));
    }

    return qScriptValueFromValue(eng, self->brush());
}

static QScriptValue color(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, color);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setColor(qscriptvalue_cast<QColor>(arg));
    }

    return qScriptValueFromValue(eng, self->color());
}

static QScriptValue style(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, style);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setStyle(Qt::PenStyle(arg.toInt32()));
    }

    return QScriptValue(eng, self->style());
}

static QScriptValue capStyle(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, capStyle);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setCapStyle(Qt::PenCapStyle(arg.toInt32()));
    }

    return QScriptValue(eng, self->capStyle());
}

static QScriptValue joinStyle(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, joinStyle);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setJoinStyle(Qt::PenJoinStyle(arg.toInt32()));
    }

    return QScriptValue(eng, self->joinStyle());
}

static QScriptValue dashOffset(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, dashOffset);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setDashOffset(arg.toInt32());
    }

    return QScriptValue(eng, self->dashOffset());
}

static QScriptValue miterLimit(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, miterLimit);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setMiterLimit(arg.toInt32());
    }

    return QScriptValue(eng, self->miterLimit());
}

static QScriptValue width(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, width);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setWidth(arg.toInt32());
    }

    return QScriptValue(eng, self->width());
}

static QScriptValue solid(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QPen, solid);
    return QScriptValue(eng, self->isSolid());
}

QScriptValue constructPenClass(QScriptEngine *eng)
{
    QScriptValue proto = qScriptValueFromValue(eng, QColor());
    QScriptValue::PropertyFlags getter = QScriptValue::PropertyGetter;
    QScriptValue::PropertyFlags setter = QScriptValue::PropertySetter;
    proto.setProperty("brush", eng->newFunction(brush), getter | setter);
    proto.setProperty("color", eng->newFunction(color), getter | setter);
    proto.setProperty("capStyle", eng->newFunction(capStyle), getter | setter);
    proto.setProperty("joinStyle", eng->newFunction(joinStyle), getter | setter);
    proto.setProperty("style", eng->newFunction(style), getter | setter);
    proto.setProperty("dashOffset", eng->newFunction(dashOffset), getter | setter);
    proto.setProperty("miterLimit", eng->newFunction(miterLimit), getter | setter);
    proto.setProperty("width", eng->newFunction(width), getter | setter);
    proto.setProperty("solid", eng->newFunction(solid), getter);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);
    ADD_ENUM_VALUE(ctorFun, Qt, FlatCap);
    ADD_ENUM_VALUE(ctorFun, Qt, SquareCap);
    ADD_ENUM_VALUE(ctorFun, Qt, RoundCap);
    ADD_ENUM_VALUE(ctorFun, Qt, RoundCap);
    ADD_ENUM_VALUE(ctorFun, Qt, BevelJoin);
    ADD_ENUM_VALUE(ctorFun, Qt, MiterJoin);
    ADD_ENUM_VALUE(ctorFun, Qt, RoundJoin);
    ADD_ENUM_VALUE(ctorFun, Qt, SolidLine);
    ADD_ENUM_VALUE(ctorFun, Qt, DashLine);
    ADD_ENUM_VALUE(ctorFun, Qt, DotLine);
    ADD_ENUM_VALUE(ctorFun, Qt, DashDotLine);
    ADD_ENUM_VALUE(ctorFun, Qt, DashDotDotLine);
    ADD_ENUM_VALUE(ctorFun, Qt, CustomDashLine);

    eng->setDefaultPrototype(qMetaTypeId<QPen>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QPen*>(), proto);

    return ctorFun;
}

