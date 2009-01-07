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
#include <QtGui/QGraphicsLinearLayout>

#include <Plasma/Applet>

#include "../backportglobal.h"
#include "../appletinterface.h"

Q_DECLARE_METATYPE(QScript::Pointer<QGraphicsItem>::wrapped_pointer_type)
Q_DECLARE_METATYPE(QGraphicsWidget*)
Q_DECLARE_METATYPE(QGraphicsGridLayout*)
DECLARE_POINTER_METATYPE(QGraphicsLinearLayout)

DECLARE_VOID_NUMBER_METHOD(QGraphicsLinearLayout, removeAt)
DECLARE_VOID_NUMBER_METHOD(QGraphicsLinearLayout, addStretch)
DECLARE_VOID_NUMBER_NUMBER_METHOD(QGraphicsLinearLayout, insertStretch)
DECLARE_VOID_NUMBER_NUMBER_METHOD(QGraphicsLinearLayout, setItemSpacing)
DECLARE_VOID_QUAD_NUMBER_METHOD(QGraphicsLinearLayout, setContentsMargins)
DECLARE_NUMBER_GET_SET_METHODS(QGraphicsLinearLayout, spacing, setSpacing)

/////////////////////////////////////////////////////////////

QGraphicsLayoutItem *layoutItem(QScriptContext *ctx, int index)
{
    QGraphicsLayoutItem *item = qscriptvalue_cast<QGraphicsWidget*>(ctx->argument(0));

    if (!item) {
        item = qscriptvalue_cast<QGraphicsLinearLayout*>(ctx->argument(0));
    }

    if (!item) {
        item = qscriptvalue_cast<QGraphicsGridLayout*>(ctx->argument(0));
    }

    QObject *appletObject = ctx->argument(0).toQObject();
    if (appletObject) {
        AppletInterface *interface = qobject_cast<AppletInterface*>(appletObject);
        if (interface) {
            item = interface->applet();
        }
    }

    return item;
}

static QScriptValue ctor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() == 0) {
        return ctx->throwError(i18n("LinearLayout requires a parent"));
    }

    QGraphicsLayoutItem *parent = layoutItem(ctx, 0);

    if (!parent) {
        return ctx->throwError(i18n("The parent must be a QGraphicsLayoutItem"));
    }

    return qScriptValueFromValue(eng, new QGraphicsLinearLayout(parent));
}

BEGIN_DECLARE_METHOD(QGraphicsLinearLayout, orientation) {
    return QScriptValue(eng, static_cast<int>(self->orientation()));
} END_DECLARE_METHOD

BEGIN_DECLARE_METHOD(QGraphicsLinearLayout, setOrientation) {
    self->setOrientation(static_cast<Qt::Orientation>(ctx->argument(0).toInt32()));
    return eng->undefinedValue();
} END_DECLARE_METHOD

/*
void insertItem(int index, QGraphicsLayoutItem * item)
void removeItem(QGraphicsLayoutItem * item)

void setAlignment(QGraphicsLayoutItem * item, Qt::Alignment alignment)
void setStretchFactor(QGraphicsLayoutItem * item, int stretch)
*/

BEGIN_DECLARE_METHOD(QGraphicsLinearLayout, addItem) {
    QGraphicsLayoutItem *item = layoutItem(ctx, 0);
    if (!item) {
        return ctx->throwError(QScriptContext::TypeError,
                               "QGraphicsLinearLayout.prototype.addItem: argument is not a GraphicsLayoutItem");
    }

    self->addItem(item);
    return eng->undefinedValue();
} END_DECLARE_METHOD

BEGIN_DECLARE_METHOD(QGraphicsItem, toString) {
    return QScriptValue(eng, "QGraphicsLinearLayout");
} END_DECLARE_METHOD

/////////////////////////////////////////////////////////////

class PrototypeLinearLayout : public QGraphicsLinearLayout
{
public:
    PrototypeLinearLayout()
    { }
};

QScriptValue constructLinearLayoutClass(QScriptEngine *eng)
{
//    QScriptValue proto = QScript::wrapGVPointer<QGraphicsLinearLayout>(eng, new QGraphicsLinearLayout(), );
    QScriptValue proto = QScript::wrapPointer<QGraphicsLinearLayout>(eng, new QGraphicsLinearLayout(), QScript::UserOwnership);
    ADD_GET_SET_METHODS(proto, spacing, setSpacing);
    ADD_GET_SET_METHODS(proto, orientation, setOrientation);
    //ADD_GET_METHOD(proto, y);
    ADD_METHOD(proto, removeAt);
    ADD_METHOD(proto, addStretch);
    ADD_METHOD(proto, insertStretch);
    ADD_METHOD(proto, setItemSpacing);
    ADD_METHOD(proto, setContentsMargins);
    ADD_METHOD(proto, addItem);
    ADD_METHOD(proto, toString);

    QScript::registerPointerMetaType<QGraphicsLinearLayout>(eng, proto);

    QScriptValue ctorFun = eng->newFunction(ctor, proto);
    //ADD_ENUM_VALUE(ctorFun, QGraphicsItem, ItemIsMovable);

    return ctorFun;
}
