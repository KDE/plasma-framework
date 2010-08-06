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
#include <QtGui/QFont>
#include "backportglobal.h"

Q_DECLARE_METATYPE(QFont*)

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0)
        return qScriptValueFromValue(eng, QFont());
    QString family = ctx->argument(0).toString();
    if (ctx->argumentCount() == 1) {
        QFont *other = qscriptvalue_cast<QFont*>(ctx->argument(0));
        if (other)
            return qScriptValueFromValue(eng, QFont(*other));
        return qScriptValueFromValue(eng, QFont(family));
    }
    int pointSize = ctx->argument(1).toInt32();
    if (ctx->argumentCount() == 2)
        return qScriptValueFromValue(eng, QFont(family, pointSize));
    int weight = ctx->argument(2).toInt32();
    if (ctx->argumentCount() == 3)
        return qScriptValueFromValue(eng, QFont(family, pointSize, weight));
    bool italic = ctx->argument(3).toBoolean();
    return qScriptValueFromValue(eng, QFont(family, pointSize, weight, italic));
}

static QScriptValue bold(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFont, bold);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setBold(arg.toBoolean());
    }

    return QScriptValue(eng, self->bold());
}

static QScriptValue defaultFamily(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFont, defaultFamily);
    return QScriptValue(eng, self->defaultFamily());
}

static QScriptValue exactMatch(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFont, exactMatch);
    return QScriptValue(eng, self->exactMatch());
}

static QScriptValue family(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFont, family);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setFamily(arg.toString());
    }

    return QScriptValue(eng, self->family());
}

static QScriptValue fixedPitch(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFont, fixedPitch);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setFixedPitch(arg.toBoolean());
    }

    return QScriptValue(eng, self->fixedPitch());
}

static QScriptValue fromString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFont, fromString);
    return QScriptValue(eng, self->fromString(ctx->argument(0).toString()));
}

static QScriptValue isCopyOf(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFont, isCopyOf);
    QFont *other = qscriptvalue_cast<QFont*>(ctx->argument(0));
    if (!other) {
        return ctx->throwError(QScriptContext::TypeError,
                               "QFont.prototype.isCopyOf: argument is not a Font");
    }
    return QScriptValue(eng, self->isCopyOf(*other));
}

static QScriptValue italic(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFont, italic);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setItalic(arg.toBoolean());
    }

    return QScriptValue(eng, self->italic());
}

static QScriptValue kerning(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFont, kerning);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setKerning(arg.toBoolean());
    }

    return QScriptValue(eng, self->kerning());
}

static QScriptValue key(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFont, key);
    return QScriptValue(eng, self->key());
}

static QScriptValue lastResortFamily(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFont, lastResortFamily);
    return QScriptValue(eng, self->lastResortFamily());
}

static QScriptValue lastResortFont(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFont, lastResortFont);
    return QScriptValue(eng, self->lastResortFont());
}

static QScriptValue overline(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFont, overline);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setOverline(arg.toBoolean());
    }

    return QScriptValue(eng, self->overline());
}

static QScriptValue pixelSize(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFont, pixelSize);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setPixelSize(arg.toInt32());
    }

    return QScriptValue(eng, self->pixelSize());
}

static QScriptValue pointSize(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFont, pointSize);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setPointSize(arg.toInt32());
    }

    return QScriptValue(eng, self->pointSize());
}

static QScriptValue pointSizeF(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFont, pointSizeF);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setPointSizeF(arg.toNumber());
    }

    return QScriptValue(eng, self->pointSizeF());
}

static QScriptValue resolve(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFont, resolve);
    QFont *other = qscriptvalue_cast<QFont*>(ctx->argument(0));
    if (!other) {
        return ctx->throwError(QScriptContext::TypeError,
                               "QFont.prototype.isCopyOf: argument is not a Font");
    }
    return qScriptValueFromValue(eng, self->resolve(*other));
}

static QScriptValue stretch(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFont, stretch);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setStretch(arg.toInt32());
    }

    return QScriptValue(eng, self->stretch());
}

static QScriptValue strikeOut(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFont, strikeOut);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setStrikeOut(arg.toBoolean());
    }

    return QScriptValue(eng, self->strikeOut());
}

static QScriptValue toString(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFont, toString);
    return QScriptValue(eng, self->toString());
}

static QScriptValue underline(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFont, underline);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setUnderline(arg.toBoolean());
    }

    return QScriptValue(eng, self->underline());
}

static QScriptValue weight(QScriptContext *ctx, QScriptEngine *eng)
{
    DECLARE_SELF(QFont, weight);

    if (ctx->argumentCount() > 0) {
        QScriptValue arg = ctx->argument(0);
        self->setWeight(arg.toInt32());
    }

    return QScriptValue(eng, self->weight());
}

QScriptValue constructFontClass(QScriptEngine *eng)
{
    QScriptValue proto = qScriptValueFromValue(eng, QFont());
    QScriptValue::PropertyFlags getter = QScriptValue::PropertyGetter;
    QScriptValue::PropertyFlags setter = QScriptValue::PropertySetter;

    proto.setProperty("key", eng->newFunction(key), getter);
    proto.setProperty("lastResortFamily", eng->newFunction(lastResortFamily), getter);
    proto.setProperty("lastResortFont", eng->newFunction(lastResortFont), getter);
    proto.setProperty("defaultFamily", eng->newFunction(defaultFamily), getter);
    proto.setProperty("exactMatch", eng->newFunction(exactMatch), getter);
    proto.setProperty("toString", eng->newFunction(toString), getter);

    proto.setProperty("bold", eng->newFunction(bold), getter | setter);
    proto.setProperty("family", eng->newFunction(family), getter|setter);
    proto.setProperty("fixedPitch", eng->newFunction(fixedPitch), getter);
    proto.setProperty("fromString", eng->newFunction(fromString), setter);
    proto.setProperty("italic", eng->newFunction(italic), getter | setter);
    proto.setProperty("kerning", eng->newFunction(kerning), getter | setter);
    proto.setProperty("overline", eng->newFunction(overline), getter | setter);
    proto.setProperty("pixelSize", eng->newFunction(pixelSize), getter | setter);
    proto.setProperty("pointSize", eng->newFunction(pointSize), getter | setter);
    proto.setProperty("pointSizeF", eng->newFunction(pointSizeF), getter | setter);
    proto.setProperty("strikeOut", eng->newFunction(strikeOut), getter | setter);
    proto.setProperty("stretch", eng->newFunction(stretch), getter | setter);
    proto.setProperty("underline", eng->newFunction(underline), getter | setter);
    proto.setProperty("weight", eng->newFunction(weight), getter | setter);

    proto.setProperty("isCopyOf", eng->newFunction(isCopyOf));
    proto.setProperty("resolve", eng->newFunction(resolve));

    eng->setDefaultPrototype(qMetaTypeId<QFont>(), proto);
    eng->setDefaultPrototype(qMetaTypeId<QFont*>(), proto);

    return eng->newFunction(ctor, proto);
}
