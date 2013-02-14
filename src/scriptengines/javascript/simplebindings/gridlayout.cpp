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
#include <QGraphicsWidget>
#include <QGraphicsGridLayout>
#include <QGraphicsLayout>

#include <Plasma/Applet>

#include "backportglobal.h"

#define DECLARE_INT_NUMBER_GET_METHOD(Class, __get__) \
BEGIN_DECLARE_METHOD(Class, __get__) { \
    return QScriptValue(eng, self->__get__(ctx->argument(0).toInt32())); \
} END_DECLARE_METHOD

#define DECLARE_INT_NUMBER_SET_METHOD(Class, __set__) \
BEGIN_DECLARE_METHOD(Class, __set__) { \
    self->__set__(ctx->argument(0).toInt32(), ctx->argument(1).toNumber()); \
    return eng->undefinedValue(); \
} END_DECLARE_METHOD

#define DECLARE_INT_NUMBER_GET_SET_METHODS(Class, __get__, __set__) \
    DECLARE_INT_NUMBER_GET_METHOD(Class, __get__) \
    DECLARE_INT_NUMBER_SET_METHOD(Class, __set__)

Q_DECLARE_METATYPE(QScript::Pointer<QGraphicsItem>::wrapped_pointer_type)
Q_DECLARE_METATYPE(QGraphicsWidget*)
Q_DECLARE_METATYPE(QGraphicsLayout*)
Q_DECLARE_METATYPE(QGraphicsLayoutItem*)
DECLARE_POINTER_METATYPE(QGraphicsGridLayout)

DECLARE_VOID_NUMBER_METHOD(QGraphicsGridLayout, removeAt)
DECLARE_VOID_NUMBER_METHOD(QGraphicsGridLayout, setSpacing)
DECLARE_VOID_QUAD_NUMBER_METHOD(QGraphicsGridLayout, setContentsMargins)

DECLARE_NUMBER_GET_SET_METHODS(QGraphicsGridLayout, horizontalSpacing, setHorizontalSpacing)
DECLARE_NUMBER_GET_SET_METHODS(QGraphicsGridLayout, verticalSpacing, setVerticalSpacing)

DECLARE_INT_NUMBER_GET_SET_METHODS(QGraphicsGridLayout, rowSpacing, setRowSpacing)
DECLARE_INT_NUMBER_GET_SET_METHODS(QGraphicsGridLayout, columnSpacing, setColumnSpacing)

DECLARE_INT_NUMBER_GET_SET_METHODS(QGraphicsGridLayout, rowMinimumHeight, setRowMinimumHeight)
DECLARE_INT_NUMBER_GET_SET_METHODS(QGraphicsGridLayout, rowPreferredHeight, setRowPreferredHeight)
DECLARE_INT_NUMBER_GET_SET_METHODS(QGraphicsGridLayout, rowMaximumHeight, setRowMaximumHeight)
DECLARE_INT_NUMBER_SET_METHOD(QGraphicsGridLayout, setRowFixedHeight)

DECLARE_INT_NUMBER_GET_SET_METHODS(QGraphicsGridLayout, columnMinimumWidth, setColumnMinimumWidth)
DECLARE_INT_NUMBER_GET_SET_METHODS(QGraphicsGridLayout, columnPreferredWidth, setColumnPreferredWidth)
DECLARE_INT_NUMBER_GET_SET_METHODS(QGraphicsGridLayout, columnMaximumWidth, setColumnMaximumWidth)
DECLARE_INT_NUMBER_SET_METHOD(QGraphicsGridLayout, setColumnFixedWidth)


/////////////////////////////////////////////////////////////

QGraphicsLayoutItem *extractLayoutItem(QScriptContext *ctx, int index = 0, bool noExistingLayout = false);

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    QGraphicsLayoutItem *parent = extractLayoutItem(ctx, 0, true);
    //FIXME: don't leak memory when parent is 0
    return qScriptValueFromValue(eng, new QGraphicsGridLayout(parent));
}

BEGIN_DECLARE_METHOD(QGraphicsGridLayout, setAlignment) {
    QGraphicsLayoutItem *item = extractLayoutItem(ctx);

    if (!item) {
        return eng->undefinedValue();
    }

    self->setAlignment(item, static_cast<Qt::Alignment>(ctx->argument(1).toInt32()));
    return eng->undefinedValue();
} END_DECLARE_METHOD

BEGIN_DECLARE_METHOD(QGraphicsGridLayout, addItem) {
    QGraphicsLayoutItem *item = extractLayoutItem(ctx);

    if (!item) {
        return eng->undefinedValue();
    }

    int rowSpan = 1;
    int colSpan = 1;
    Qt::Alignment alignment = 0;
    const int argCount = ctx->argumentCount();
    if (argCount > 3) {
        rowSpan = ctx->argument(3).toInt32();
        if (argCount > 4) {
            colSpan = ctx->argument(4).toInt32();

            if (argCount > 5) {
                alignment = static_cast<Qt::Alignment>(ctx->argument(5).toInt32());
            }
        }
    }

    self->addItem(item, ctx->argument(1).toInt32(), ctx->argument(2).toInt32(),
                  rowSpan, colSpan, alignment);
    return eng->undefinedValue();
} END_DECLARE_METHOD

BEGIN_DECLARE_METHOD(QGraphicsGridLayout, toString) {
    return QScriptValue(eng, "QGraphicsGridLayout");
} END_DECLARE_METHOD

BEGIN_DECLARE_METHOD(QGraphicsGridLayout, activate) {
    self->activate();
    return eng->undefinedValue();
} END_DECLARE_METHOD

/////////////////////////////////////////////////////////////

class PrototypeGridLayout : public QGraphicsGridLayout
{
public:
    PrototypeGridLayout()
    { }
};

QScriptValue constructGridLayoutClass(QScriptEngine *eng)
{
    QScriptValue proto = QScript::wrapPointer<QGraphicsGridLayout>(eng, new QGraphicsGridLayout(), QScript::UserOwnership);
    const QScriptValue::PropertyFlags getter = QScriptValue::PropertyGetter;
    const QScriptValue::PropertyFlags setter = QScriptValue::PropertySetter;
    proto.setProperty("horizontalSpacing", eng->newFunction(horizontalSpacing), getter);
    proto.setProperty("horizontalSpacing", eng->newFunction(setHorizontalSpacing), setter);
    proto.setProperty("verticalSpacing", eng->newFunction(verticalSpacing), getter);
    proto.setProperty("verticalSpacing", eng->newFunction(setVerticalSpacing), setter);

    ADD_METHOD(proto, rowSpacing);
    ADD_METHOD(proto, setRowSpacing);
    ADD_METHOD(proto, columnSpacing);
    ADD_METHOD(proto, setColumnSpacing);

    ADD_METHOD(proto, rowMinimumHeight);
    ADD_METHOD(proto, setRowMinimumHeight);
    ADD_METHOD(proto, rowPreferredHeight);
    ADD_METHOD(proto, setRowPreferredHeight);
    ADD_METHOD(proto, rowMaximumHeight);
    ADD_METHOD(proto, setRowMaximumHeight);
    ADD_METHOD(proto, setRowFixedHeight);

    ADD_METHOD(proto, columnMinimumWidth);
    ADD_METHOD(proto, setColumnMinimumWidth);
    ADD_METHOD(proto, columnPreferredWidth);
    ADD_METHOD(proto, setColumnPreferredWidth);
    ADD_METHOD(proto, columnMaximumWidth);
    ADD_METHOD(proto, setColumnMaximumWidth);
    ADD_METHOD(proto, setColumnFixedWidth);

    ADD_METHOD(proto, removeAt);
    ADD_METHOD(proto, setAlignment);
    ADD_METHOD(proto, setSpacing);
    ADD_METHOD(proto, setContentsMargins);
    ADD_METHOD(proto, addItem);
    ADD_METHOD(proto, toString);
    ADD_METHOD(proto, activate);

    QScript::registerPointerMetaType<QGraphicsGridLayout>(eng, proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);
    return ctorFun;
}
