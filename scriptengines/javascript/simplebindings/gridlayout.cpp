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
#include <QtGui/QGraphicsWidget>
#include <QtGui/QGraphicsGridLayout>
#include <QtGui/QGraphicsLayout>

#include <Plasma/Applet>

#include "../backportglobal.h"
#include "appletinterface.h"

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

DECLARE_INT_NUMBER_GET_SET_METHODS(QGraphicsGridLayout, rowSpacing, setRowSpacing);
DECLARE_INT_NUMBER_GET_SET_METHODS(QGraphicsGridLayout, columnSpacing, setColumnSpacing);

DECLARE_INT_NUMBER_GET_SET_METHODS(QGraphicsGridLayout, rowMinimumHeight, setRowMinimumHeight);
DECLARE_INT_NUMBER_GET_SET_METHODS(QGraphicsGridLayout, rowPreferredHeight, setRowPreferredHeight);
DECLARE_INT_NUMBER_GET_SET_METHODS(QGraphicsGridLayout, rowMaximumHeight, setRowMaximumHeight);
DECLARE_INT_NUMBER_SET_METHOD(QGraphicsGridLayout, setRowFixedHeight);

DECLARE_INT_NUMBER_GET_SET_METHODS(QGraphicsGridLayout, columnMinimumWidth, setColumnMinimumWidth);
DECLARE_INT_NUMBER_GET_SET_METHODS(QGraphicsGridLayout, columnPreferredWidth, setColumnPreferredWidth);
DECLARE_INT_NUMBER_GET_SET_METHODS(QGraphicsGridLayout, columnMaximumWidth, setColumnMaximumWidth);
DECLARE_INT_NUMBER_SET_METHOD(QGraphicsGridLayout, setColumnFixedWidth);


/////////////////////////////////////////////////////////////

QGraphicsLayoutItem *getLayoutItem(QScriptContext *ctx, int index = 0)
{
    QObject *object = ctx->argument(index).toQObject();
    QGraphicsLayoutItem *item = qobject_cast<QGraphicsWidget*>(object);

    if (!item) {
        item = qscriptvalue_cast<QGraphicsLayout*>(ctx->argument(index));
    }

    if (!item) {
        AppletInterface *interface = qobject_cast<AppletInterface*>(object);

        if (!interface) {
            interface = qobject_cast<AppletInterface*>(ctx->engine()->globalObject().property("plasmoid").toQObject());
        }

        if (interface) {
            item = interface->applet();
        }
    }

    return item;
}

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    QGraphicsLayoutItem *parent = getLayoutItem(ctx);

    if (!parent) {
        return ctx->throwError(i18n("The parent must be a QGraphicsLayoutItem"));
    }

    return qScriptValueFromValue(eng, new QGraphicsGridLayout(parent));
}

BEGIN_DECLARE_METHOD(QGraphicsGridLayout, setAlignment) {
    QGraphicsLayoutItem *item = getLayoutItem(ctx);

    if (!item) {
        return eng->undefinedValue();
    }

    self->setAlignment(item, static_cast<Qt::Alignment>(ctx->argument(1).toInt32()));
    return eng->undefinedValue();
} END_DECLARE_METHOD

BEGIN_DECLARE_METHOD(QGraphicsGridLayout, addItem) {
    QGraphicsLayoutItem *item = getLayoutItem(ctx);

    if (!item) {
        return eng->undefinedValue();
    }

    self->addItem(item, ctx->argument(1).toInt32(), ctx->argument(2).toInt32());
    return eng->undefinedValue();
} END_DECLARE_METHOD

BEGIN_DECLARE_METHOD(QGraphicsItem, toString) {
    return QScriptValue(eng, "QGraphicsGridLayout");
} END_DECLARE_METHOD

/////////////////////////////////////////////////////////////

class PrototypeGridLayout : public QGraphicsGridLayout
{
public:
    PrototypeGridLayout()
    { }
};

#define ADD_PROTO_PROPERTY(Property, __get__, __set__) \
    proto.setProperty(Property, eng->newFunction(__get__), QScriptValue::PropertyGetter); \
    proto.setProperty(Property, eng->newFunction(__set__), QScriptValue::PropertySetter);

QScriptValue constructGridLayoutClass(QScriptEngine *eng)
{
    QScriptValue proto = QScript::wrapPointer<QGraphicsGridLayout>(eng, new QGraphicsGridLayout(), QScript::UserOwnership);

    ADD_PROTO_PROPERTY("horizontalSpacing", horizontalSpacing, setHorizontalSpacing);
    ADD_PROTO_PROPERTY("verticalSpacing", verticalSpacing, setVerticalSpacing);

    ADD_PROTO_PROPERTY("rowSpacing", rowSpacing, setRowSpacing);
    ADD_PROTO_PROPERTY("columnSpacing", columnSpacing, setColumnSpacing);

    ADD_PROTO_PROPERTY("rowMinimumHeight", rowMinimumHeight, setRowMinimumHeight);
    ADD_PROTO_PROPERTY("rowPreferredHeight", rowPreferredHeight, setRowPreferredHeight);
    ADD_PROTO_PROPERTY("rowMaximumHeight", rowMaximumHeight, setRowMaximumHeight);
    ADD_METHOD(proto, setRowFixedHeight);

    ADD_PROTO_PROPERTY("columnMinimumWidth", columnMinimumWidth, setColumnMinimumWidth);
    ADD_PROTO_PROPERTY("columnPreferredWidth", columnPreferredWidth, setColumnPreferredWidth);
    ADD_PROTO_PROPERTY("columnMaximumWidth", columnMaximumWidth, setColumnMaximumWidth);
    ADD_METHOD(proto, setColumnFixedWidth);

    ADD_METHOD(proto, removeAt);
    ADD_METHOD(proto, setAlignment);
    ADD_METHOD(proto, setSpacing);
    ADD_METHOD(proto, setContentsMargins);
    ADD_METHOD(proto, addItem);
    ADD_METHOD(proto, toString);

    QScript::registerPointerMetaType<QGraphicsGridLayout>(eng, proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);
    return ctorFun;
}
