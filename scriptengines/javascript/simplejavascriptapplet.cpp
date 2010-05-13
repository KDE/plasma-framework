/*
 *   Copyright 2007-2008,2010 Richard J. Moore <rich@kde.org>
 *   Copyright 2009 Aaron J. Seigo <aseigo@kde.org>
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

#include "simplejavascriptapplet.h"

#include <iostream>

#include <QFile>
#include <QKeyEvent>
#include <QGraphicsLayout>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QParallelAnimationGroup>
#include <QPauseAnimation>
#include <QSequentialAnimationGroup>
#include <QUiLoader>
#include <QWidget>
#include <QScriptEngine>

#include <KConfigGroup>
#include <KDebug>
#include <KFileDialog>
#include <KIcon>
#include <KIO/Job>
#include <KMimeType>
#include <KShell>
#include <KStandardDirs>
#include <KLocale>
#include <KRun>

#include <Plasma/Animation>
#include <Plasma/Applet>
#include <Plasma/Extender>
#include <Plasma/ExtenderItem>
#include <Plasma/FrameSvg>
#include <Plasma/Package>
#include <Plasma/PopupApplet>
#include <Plasma/Svg>
#include <Plasma/VideoWidget>

#include "appletauthorization.h"
#include "scriptenv.h"
#include "simplebindings/animationgroup.h"
#include "simplebindings/dataengine.h"
#include "simplebindings/dataenginereceiver.h"
#include "simplebindings/i18n.h"
#include "simplebindings/appletinterface.h"
#include "simplebindings/bytearrayclass.h"
#include "simplebindings/variant.h"

using namespace Plasma;

Q_DECLARE_METATYPE(QKeyEvent*)
Q_DECLARE_METATYPE(QPainter*)
Q_DECLARE_METATYPE(QStyleOptionGraphicsItem*)
Q_DECLARE_METATYPE(QGraphicsSceneHoverEvent *)
Q_DECLARE_METATYPE(QGraphicsSceneMouseEvent *)
Q_DECLARE_METATYPE(QGraphicsSceneWheelEvent *)

QScriptValue constructColorClass(QScriptEngine *engine);
QScriptValue constructEasingCurveClass(QScriptEngine *engine);
QScriptValue constructFontClass(QScriptEngine *engine);
QScriptValue constructGraphicsItemClass(QScriptEngine *engine);
QScriptValue constructIconClass(QScriptEngine *engine);
QScriptValue constructKUrlClass(QScriptEngine *engine);
QScriptValue constructLinearLayoutClass(QScriptEngine *engine);
QScriptValue constructGridLayoutClass(QScriptEngine *engine);
QScriptValue constructAnchorLayoutClass(QScriptEngine *engine);
QScriptValue constructPainterClass(QScriptEngine *engine);
QScriptValue constructPenClass(QScriptEngine *engine);
QScriptValue constructQPixmapClass(QScriptEngine *engine);
QScriptValue constructQPointClass(QScriptEngine *engine);
QScriptValue constructQRectFClass(QScriptEngine *engine);
QScriptValue constructQSizeFClass(QScriptEngine *engine);
QScriptValue constructQSizePolicyClass(QScriptEngine *engine);
QScriptValue constructTimerClass(QScriptEngine *engine);
void registerSimpleAppletMetaTypes(QScriptEngine *engine);

KSharedPtr<UiLoader> SimpleJavaScriptApplet::s_widgetLoader;
QHash<QString, Plasma::Animator::Animation> SimpleJavaScriptApplet::s_animationDefs;

SimpleJavaScriptApplet::SimpleJavaScriptApplet(QObject *parent, const QVariantList &args)
    : Plasma::AppletScript(parent)
{
    Q_UNUSED(args);
//    kDebug() << "Script applet launched, args" << applet()->startupArguments();

    // TODO this will be set to the engine we get from QML
    m_engine = new QScriptEngine(this);
    m_env = new ScriptEnv(this, m_engine);
    connect(m_env, SIGNAL(reportError(ScriptEnv*,bool)), this, SLOT(engineReportsError(ScriptEnv*,bool)));
}

SimpleJavaScriptApplet::~SimpleJavaScriptApplet()
{
    if (s_widgetLoader.count() == 1) {
        s_widgetLoader.clear();
    }
}

void SimpleJavaScriptApplet::engineReportsError(ScriptEnv *engine, bool fatal)
{
    reportError(engine, fatal);
}

void SimpleJavaScriptApplet::reportError(ScriptEnv *env, bool fatal)
{
    SimpleJavaScriptApplet *jsApplet = qobject_cast<SimpleJavaScriptApplet *>(env->parent());
    AppletInterface *interface = extractAppletInterface(env->engine());
    const QScriptValue error = env->engine()->uncaughtException();
    QString file = error.property("fileName").toString();
    if (interface) {
        file.remove(interface->package()->path());
    }

    const QString failureMsg = i18n("Error in %1 on line %2.<br><br>%3",
                                    file, error.property("lineNumber").toString(),
                                    error.toString());
    if (jsApplet) {
        if (fatal) {
            jsApplet->setFailedToLaunch(true, failureMsg);
        } else {
            jsApplet->showMessage(KIcon("dialog-error"), failureMsg, Plasma::ButtonNone);
        }
    } else {
        kDebug() << failureMsg;
    }

    kDebug() << env->engine()->uncaughtExceptionBacktrace();
}

void SimpleJavaScriptApplet::configChanged()
{
    if (m_eventListeners.contains("configchanged")) {
        callEventListeners("configchanged");
    } else {
        callPlasmoidFunction("configChanged");
    }
}

void SimpleJavaScriptApplet::dataUpdated(const QString &name, const DataEngine::Data &data)
{
    QScriptValueList args;
    args << m_engine->toScriptValue(name) << m_engine->toScriptValue(data);
    if (m_eventListeners.contains("dataupdated")) {
        callEventListeners("dataupdated");
    } else {
        callPlasmoidFunction("dataUpdated", args);
    }
}

void SimpleJavaScriptApplet::extenderItemRestored(Plasma::ExtenderItem* item)
{
    QScriptValueList args;
    args << m_engine->newQObject(item, QScriptEngine::AutoOwnership, QScriptEngine::PreferExistingWrapperObject);
    if (m_eventListeners.contains("initextenderitem")) {
        callEventListeners("initextenderitem");
    } else {
        callPlasmoidFunction("initExtenderItem", args);
    }
}

void SimpleJavaScriptApplet::activate()
{
    if (m_eventListeners.contains("activate")) {
        callEventListeners("activate");
    } else {
        callPlasmoidFunction("activate");
    }
}

void SimpleJavaScriptApplet::popupEvent(bool popped)
{
    QScriptValueList args;
    args << popped;
    if (m_eventListeners.contains("popupevent")) {
        callEventListeners("popupevent");
    } else {
        callPlasmoidFunction("popupEvent", args);
    }
}

void SimpleJavaScriptApplet::executeAction(const QString &name)
{
    if (m_eventListeners.contains("action_" + name)) {
        callEventListeners("action_" + name);
    } else {
        callPlasmoidFunction("action_" + name);
    }
}

void SimpleJavaScriptApplet::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    QScriptValueList args;
    args << m_engine->toScriptValue(p);
    args << m_engine->toScriptValue(const_cast<QStyleOptionGraphicsItem*>(option));
    args << m_engine->toScriptValue(QRectF(contentsRect));

    if (m_eventListeners.contains("paintinterface")) {
        callEventListeners("paintinterface", args);
    } else {
        callPlasmoidFunction("paintInterface", args);
    }
}

QList<QAction*> SimpleJavaScriptApplet::contextualActions()
{
    return m_interface->contextualActions();
}

void SimpleJavaScriptApplet::callPlasmoidFunction(const QString &functionName, const QScriptValueList &args)
{
    QScriptValue func = m_self.property(functionName);
    callFunction(func, args, m_self);
}

void SimpleJavaScriptApplet::callFunction(QScriptValue &func, const QScriptValueList &args, const QScriptValue &activator)
{
    if (!func.isFunction()) {
        return;
    }

    QScriptContext *ctx = m_engine->pushContext();
    ctx->setActivationObject(activator);
    func.call(activator, args);
    m_engine->popContext();

    if (m_engine->hasUncaughtException()) {
        reportError(m_env);
        m_engine->clearExceptions();
    }
}

void SimpleJavaScriptApplet::callEventListeners(const QString &event, const QScriptValueList &args)
{
    QScriptValueList funcs = m_eventListeners.value(event);
    QMutableListIterator<QScriptValue> it(funcs);
    while (it.hasNext()) {
        callFunction(it.next(), args);
    }
}

void SimpleJavaScriptApplet::constraintsEvent(Plasma::Constraints constraints)
{
    QString functionName;

    if (constraints & Plasma::FormFactorConstraint) {
        if (m_eventListeners.contains("formfactorchanged")) {
            callEventListeners("formfactorchanged");
        } else {
            callPlasmoidFunction("formFactorChanged");
        }
    }

    if (constraints & Plasma::LocationConstraint) {
        if (m_eventListeners.contains("locationchanged")) {
            callEventListeners("locationchanged");
        } else {
            callPlasmoidFunction("locationChanged");
        }
    }

    if (constraints & Plasma::ContextConstraint) {
        if (m_eventListeners.contains("currentactivitychanged")) {
            callEventListeners("currentactivitychanged");
        } else {
            callPlasmoidFunction("currentActivityChanged");
        }
    }

    if (constraints & Plasma::SizeConstraint) {
        if (m_eventListeners.contains("sizechanged")) {
            callEventListeners("sizechanged");
        } else {
            callPlasmoidFunction("sizeChanged");
        }
    }

    if (constraints & Plasma::ImmutableConstraint) {
        if (m_eventListeners.contains("immutabilitychanged")) {
            callEventListeners("immutabilitychanged");
        } else {
            callPlasmoidFunction("immutabilityChanged");
        }
    }
}

bool SimpleJavaScriptApplet::include(const QString &path)
{
    return m_env->include(path);
}

void SimpleJavaScriptApplet::populateAnimationsHash()
{
    if (s_animationDefs.isEmpty()) {
        s_animationDefs.insert("fade", Plasma::Animator::FadeAnimation);
        s_animationDefs.insert("geometry", Plasma::Animator::GeometryAnimation);
        s_animationDefs.insert("grow", Plasma::Animator::GrowAnimation);
        s_animationDefs.insert("pulse", Plasma::Animator::PulseAnimation);
        s_animationDefs.insert("rotate", Plasma::Animator::RotationAnimation);
        s_animationDefs.insert("rotateStacked", Plasma::Animator::RotationStackedAnimation);
        s_animationDefs.insert("slide", Plasma::Animator::SlideAnimation);
        s_animationDefs.insert("zoom", Plasma::Animator::ZoomAnimation);
    }
}

bool SimpleJavaScriptApplet::init()
{
    connect(applet(), SIGNAL(extenderItemRestored(Plasma::ExtenderItem*)),
            this, SLOT(extenderItemRestored(Plasma::ExtenderItem*)));
    connect(applet(), SIGNAL(activate()),
            this, SLOT(activate()));
    setupObjects();

    AppletAuthorization auth(this);
    if (!m_env->importExtensions(description(), m_self, auth)) {
        return false;
    }

    kDebug() << "ScriptName:" << applet()->name();
    kDebug() << "ScriptCategory:" << applet()->category();
    applet()->installEventFilter(this);
    return m_env->include(mainScript());
}

QScriptValue SimpleJavaScriptApplet::createKeyEventObject(QKeyEvent *event)
{
    QScriptValue v = m_env->engine()->newObject();
    v.setProperty("count", event->count());
    v.setProperty("key", event->key());
    v.setProperty("modifiers", static_cast<int>(event->modifiers()));
    v.setProperty("text", event->text());
    return v;
}

QScriptValue SimpleJavaScriptApplet::createHoverEventObject(QGraphicsSceneHoverEvent *event)
{
    QScriptEngine *engine = m_env->engine();
    QScriptValue v = engine->newObject();
    v.setProperty("pos", engine->toScriptValue<QPoint>(event->pos().toPoint()));
    v.setProperty("scenePos", engine->toScriptValue<QPoint>(event->scenePos().toPoint()));
    v.setProperty("screenPos", engine->toScriptValue<QPoint>(event->screenPos()));
    v.setProperty("lastPos", engine->toScriptValue<QPoint>(event->lastPos().toPoint()));
    v.setProperty("lastScenePos", engine->toScriptValue<QPoint>(event->lastScenePos().toPoint()));
    v.setProperty("lastScreenPos", engine->toScriptValue<QPoint>(event->lastScreenPos()));
    v.setProperty("modifiers", static_cast<int>(event->modifiers()));
    return v;
}

QScriptValue SimpleJavaScriptApplet::createMouseEventObject(QGraphicsSceneMouseEvent *event)
{
    QScriptEngine *engine = m_env->engine();
    QScriptValue v = engine->newObject();
    v.setProperty("button", static_cast<int>(event->button()));
    v.setProperty("buttons", static_cast<int>(event->buttons()));
    v.setProperty("modifiers", static_cast<int>(event->modifiers()));
    v.setProperty("pos", engine->toScriptValue<QPoint>(event->pos().toPoint()));
    v.setProperty("scenePos", engine->toScriptValue<QPoint>(event->scenePos().toPoint()));
    v.setProperty("screenPos", engine->toScriptValue<QPoint>(event->screenPos()));
    v.setProperty("lastPos", engine->toScriptValue<QPoint>(event->lastPos().toPoint()));
    v.setProperty("lastScenePos", engine->toScriptValue<QPoint>(event->lastScenePos().toPoint()));
    v.setProperty("lastScreenPos", engine->toScriptValue<QPoint>(event->lastScreenPos()));
    return v;
}

QScriptValue SimpleJavaScriptApplet::createWheelEventObject(QGraphicsSceneWheelEvent *event)
{
    QScriptEngine *engine = m_env->engine();
    QScriptValue v = engine->newObject();
    v.setProperty("delta", event->delta());
    v.setProperty("buttons", static_cast<int>(event->buttons()));
    v.setProperty("modifiers", static_cast<int>(event->modifiers()));
    v.setProperty("orientation", static_cast<int>(event->orientation()));
    v.setProperty("pos", engine->toScriptValue<QPoint>(event->pos().toPoint()));
    v.setProperty("scenePos", engine->toScriptValue<QPoint>(event->scenePos().toPoint()));
    v.setProperty("screenPos", engine->toScriptValue<QPoint>(event->screenPos()));
    return v;
}

bool SimpleJavaScriptApplet::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type()) {
        case QEvent::KeyPress: {
            if (m_eventListeners.contains("keypress")) {
                QScriptValueList args;
                args << createKeyEventObject(static_cast<QKeyEvent *>(event));
                callEventListeners("keypress", args);
                return true;
            }
        }

        case QEvent::KeyRelease: {
            if (m_eventListeners.contains("keyrelease")) {
                QScriptValueList args;
                args << createKeyEventObject(static_cast<QKeyEvent *>(event));
                callEventListeners("keyrelease", args);
                return true;
            }
        }
        break;

        case QEvent::GraphicsSceneHoverEnter: {
            if (m_eventListeners.contains("hoverenter")) {
                QScriptValueList args;
                args << createHoverEventObject(static_cast<QGraphicsSceneHoverEvent *>(event));
                callEventListeners("hoverenter", args);
                return true;
            }
        }
        break;

        case QEvent::GraphicsSceneHoverLeave: {
            if (m_eventListeners.contains("hoverleave")) {
                QScriptValueList args;
                args << createHoverEventObject(static_cast<QGraphicsSceneHoverEvent *>(event));
                callEventListeners("hoverleave", args);
                return true;
            }
        }
        break;

        case QEvent::GraphicsSceneHoverMove: {
            if (m_eventListeners.contains("hovermove")) {
                QScriptValueList args;
                args << createHoverEventObject(static_cast<QGraphicsSceneHoverEvent *>(event));
                callEventListeners("hovermove", args);
                return true;
            }
        }
        break;

        case QEvent::GraphicsSceneMousePress: {
            if (m_eventListeners.contains("mousepress")) {
                QScriptValueList args;
                args << createMouseEventObject(static_cast<QGraphicsSceneMouseEvent *>(event));
                callEventListeners("mousepress", args);
                return true;
            }
        }
        break;

        case QEvent::GraphicsSceneMouseRelease: {
            if (m_eventListeners.contains("mouserelease")) {
                QScriptValueList args;
                args << createMouseEventObject(static_cast<QGraphicsSceneMouseEvent *>(event));
                callEventListeners("mouserelease", args);
                return true;
            }
        }
        break;

        case QEvent::GraphicsSceneMouseMove: {
            if (m_eventListeners.contains("mousemove")) {
                QScriptValueList args;
                args << createMouseEventObject(static_cast<QGraphicsSceneMouseEvent *>(event));
                callEventListeners("mousemove", args);
                return true;
            }
        }
        break;

        case QEvent::GraphicsSceneMouseDoubleClick: {
            if (m_eventListeners.contains("mousedoubleclick")) {
                QScriptValueList args;
                args << createMouseEventObject(static_cast<QGraphicsSceneMouseEvent *>(event));
                callEventListeners("mousedoubleclick", args);
                return true;
            }
        }
        break;

        case QEvent::GraphicsSceneWheel: {
            if (m_eventListeners.contains("wheel")) {
                QScriptValueList args;
                args << createWheelEventObject(static_cast<QGraphicsSceneWheelEvent *>(event));
                callEventListeners("wheel", args);
                return true;
            }
        }
        break;

        default:
        break;
    }

    return Plasma::AppletScript::eventFilter(watched, event);
}

void SimpleJavaScriptApplet::addEventListener(const QString &event, const QScriptValue &func)
{
    if (func.isFunction()) {
        m_eventListeners[event.toLower()].append(func);
    }
}

void SimpleJavaScriptApplet::removeEventListener(const QString &event, const QScriptValue &func)
{
    if (func.isFunction()) {
        QScriptValueList funcs = m_eventListeners.value("mousepress");
        QMutableListIterator<QScriptValue> it(funcs);//m_eventListeners.value("mousepress"));
        while (it.hasNext()) {
            if (it.next().equals(func)) {
                it.remove();
            }
        }

        if (funcs.isEmpty()) {
            m_eventListeners.remove(event.toLower());
        } else {
            m_eventListeners.insert(event.toLower(), funcs);
        }
    }
}

void SimpleJavaScriptApplet::setupObjects()
{
    QScriptValue global = m_engine->globalObject();

    // Bindings for animations
    global.setProperty("animation", m_engine->newFunction(SimpleJavaScriptApplet::animation));
    global.setProperty("AnimationGroup", m_engine->newFunction(SimpleJavaScriptApplet::animationGroup));
    global.setProperty("ParallelAnimationGroup", m_engine->newFunction(SimpleJavaScriptApplet::parallelAnimationGroup));

    // Bindings for data engine

    bindI18N(m_engine);
    global.setProperty("dataEngine", m_engine->newFunction(SimpleJavaScriptApplet::dataEngine));
    global.setProperty("service", m_engine->newFunction(SimpleJavaScriptApplet::service));

    // Expose applet interface
    const bool isPopupApplet = qobject_cast<Plasma::PopupApplet *>(applet());
    m_interface = isPopupApplet ? new PopupAppletInterface(this) : new AppletInterface(this);
    m_self = m_engine->newQObject(m_interface);
    m_self.setScope(global);
    global.setProperty("plasmoid", m_self);

    if (isPopupApplet) {
        connect(applet(), SIGNAL(popupEvent(bool)), this, SLOT(popupEvent(bool)));
    }

    QScriptValue args = m_engine->newArray();
    int i = 0;
    foreach (const QVariant &arg, applet()->startupArguments()) {
        args.setProperty(i, ::variantToScriptValue(m_engine, arg));
        ++i;
    }
    global.setProperty("startupArguments", args);

    ScriptEnv::registerEnums(global, AppletInterface::staticMetaObject);

    // Add a global loadui method for ui files
    QScriptValue fun = m_engine->newFunction(SimpleJavaScriptApplet::loadui);
    global.setProperty("loadui", fun);

    // Work around bug in 4.3.0
    qMetaTypeId<QVariant>();

    // Add stuff from Qt
    global.setProperty("QPainter", constructPainterClass(m_engine));
    global.setProperty("QGraphicsItem", constructGraphicsItemClass(m_engine));
    global.setProperty("QIcon", constructIconClass(m_engine));
    global.setProperty("QTimer", constructTimerClass(m_engine));
    global.setProperty("QFont", constructFontClass(m_engine));
    global.setProperty("QColor", constructColorClass(m_engine));
    global.setProperty("QEasingCurve", constructEasingCurveClass(m_engine));
    global.setProperty("QRectF", constructQRectFClass(m_engine));
    global.setProperty("QPen", constructPenClass(m_engine));
    global.setProperty("QPixmap", constructQPixmapClass(m_engine));
    global.setProperty("QSizeF", constructQSizeFClass(m_engine));
    global.setProperty("QSizePolicy", constructQSizePolicyClass(m_engine));
    global.setProperty("QPoint", constructQPointClass(m_engine));
    global.setProperty("LinearLayout", constructLinearLayoutClass(m_engine));
    global.setProperty("GridLayout", constructGridLayoutClass(m_engine));
    global.setProperty("AnchorLayout", constructAnchorLayoutClass(m_engine));
    ByteArrayClass *baClass = new ByteArrayClass(m_engine);
    global.setProperty("ByteArray", baClass->constructor());

    // Add stuff from KDE libs
    qScriptRegisterSequenceMetaType<KUrl::List>(m_engine);
    global.setProperty("Url", constructKUrlClass(m_engine));

    // Add stuff from Plasma
    global.setProperty("PlasmaSvg", m_engine->newFunction(SimpleJavaScriptApplet::newPlasmaSvg));
    global.setProperty("PlasmaFrameSvg", m_engine->newFunction(SimpleJavaScriptApplet::newPlasmaFrameSvg));
    global.setProperty("Svg", m_engine->newFunction(SimpleJavaScriptApplet::newPlasmaSvg));
    global.setProperty("FrameSvg", m_engine->newFunction(SimpleJavaScriptApplet::newPlasmaFrameSvg));
    global.setProperty("ExtenderItem", m_engine->newFunction(SimpleJavaScriptApplet::newPlasmaExtenderItem));

    registerSimpleAppletMetaTypes(m_engine);
    installWidgets(m_engine);
}

QSet<QString> SimpleJavaScriptApplet::loadedExtensions() const
{
    return m_env->loadedExtensions();
}

AppletInterface *SimpleJavaScriptApplet::extractAppletInterface(QScriptEngine *engine)
{
    QScriptValue appletValue = engine->globalObject().property("plasmoid");
    return qobject_cast<AppletInterface*>(appletValue.toQObject());
}

QScriptValue SimpleJavaScriptApplet::dataEngine(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) {
        return context->throwError(i18n("dataEngine() takes one argument"));
    }

    AppletInterface *interface = extractAppletInterface(engine);
    if (!interface) {
        return context->throwError(i18n("Could not extract the Applet"));
    }

    const QString dataEngineName = context->argument(0).toString();
    DataEngine *dataEngine = interface->dataEngine(dataEngineName);
    QScriptValue v = engine->newQObject(dataEngine, QScriptEngine::QtOwnership, QScriptEngine::PreferExistingWrapperObject);
    v.setProperty("connectSource", engine->newFunction(DataEngineReceiver::connectSource));
    v.setProperty("disconnectSource", engine->newFunction(DataEngineReceiver::disconnectSource));
    return v;
}

QScriptValue SimpleJavaScriptApplet::service(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) {
        return context->throwError(i18n("service() takes two arguments"));
    }

    QString dataEngine = context->argument(0).toString();

    AppletInterface *interface = extractAppletInterface(engine);
    if (!interface) {
        return context->throwError(i18n("Could not extract the Applet"));
    }

    DataEngine *data = interface->dataEngine(dataEngine);
    QString source = context->argument(1).toString();
    Service *service = data->serviceForSource(source);
    //kDebug( )<< "lets try to get" << source << "from" << dataEngine;
    return engine->newQObject(service, QScriptEngine::AutoOwnership);
}

QScriptValue SimpleJavaScriptApplet::animation(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) {
        return context->throwError(i18n("animation() takes one argument"));
    }

    populateAnimationsHash();
    QString name = context->argument(0).toString();
    QString animName = name.toLower();
    const bool isPause = animName == "pause";
    const bool isProperty = animName == "property";

    bool parentIsApplet = false;
    QGraphicsWidget *parent = extractParent(context, engine, 0, &parentIsApplet);
    QAbstractAnimation *anim = 0;
    Plasma::Animation *plasmaAnim = 0;
    if (isPause) {
        anim = new QPauseAnimation(parent);
    } else if (isProperty) {
        anim = new QPropertyAnimation(parent);
    } else if (s_animationDefs.contains(animName)) {
        plasmaAnim = Plasma::Animator::create(s_animationDefs.value(animName), parent);
    } else {
        SimpleJavaScriptApplet *jsApplet = qobject_cast<SimpleJavaScriptApplet *>(engine->parent());
        if (jsApplet) {
            //kDebug() << "trying to load it from the package";
            plasmaAnim = jsApplet->loadAnimationFromPackage(name, parent);
        }

        if (!plasmaAnim) {
            plasmaAnim = Plasma::Animator::create(animName, parent);
        }
    }

    if (plasmaAnim) {
        if (!parentIsApplet) {
            plasmaAnim->setTargetWidget(parent);
        }
        anim = plasmaAnim;
    }

    if (anim) {
        QScriptValue value = engine->newQObject(anim);
        ScriptEnv::registerEnums(value, *anim->metaObject());
        return value;
    }

    context->throwError(i18n("%1 is not a known animation type", animName));

    ScriptEnv *env = ScriptEnv::findScriptEnv(engine);
    if (env) {
        env->checkForErrors(false);
    }
    return engine->undefinedValue();
}

QScriptValue SimpleJavaScriptApplet::animationGroup(QScriptContext *context, QScriptEngine *engine)
{
    QGraphicsWidget *parent = extractParent(context, engine);
    SequentialAnimationGroup *group = new SequentialAnimationGroup(parent);
    return engine->newQObject(group);
}

QScriptValue SimpleJavaScriptApplet::parallelAnimationGroup(QScriptContext *context, QScriptEngine *engine)
{
    QGraphicsWidget *parent = extractParent(context, engine);
    ParallelAnimationGroup *group = new ParallelAnimationGroup(parent);
    return engine->newQObject(group);
}

QScriptValue SimpleJavaScriptApplet::loadui(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) {
        return context->throwError(i18n("loadui() takes one argument"));
    }

    QString filename = context->argument(0).toString();
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly)) {
        return context->throwError(i18n("Unable to open '%1'",filename));
    }

    QUiLoader loader;
    QWidget *w = loader.load(&f);
    f.close();

    return engine->newQObject(w, QScriptEngine::AutoOwnership);
}

QString SimpleJavaScriptApplet::findImageFile(QScriptEngine *engine, const QString &file)
{
    AppletInterface *interface = extractAppletInterface(engine);
    if (!interface) {
        return QString();
    }

    return interface->package()->filePath("images", file);
}

QString SimpleJavaScriptApplet::findSvg(QScriptEngine *engine, const QString &file)
{
    AppletInterface *interface = extractAppletInterface(engine);
    if (!interface) {
        return file;
    }

    QString path = interface->package()->filePath("images", file + ".svg");
    if (path.isEmpty()) {
        path = interface->package()->filePath("images", file + ".svgz");

        if (path.isEmpty()) {
            return file;
        }
    }

    return path;
}

QScriptValue SimpleJavaScriptApplet::newPlasmaSvg(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0) {
        return context->throwError(i18n("Constructor takes at least 1 argument"));
    }

    const QString filename = context->argument(0).toString();
    bool parentedToApplet = false;
    QGraphicsWidget *parent = extractParent(context, engine, 1, &parentedToApplet);
    Svg *svg = new Svg(parent);
    svg->setImagePath(parentedToApplet ? findSvg(engine, filename) : filename);
    QScriptValue fun = engine->newQObject(svg);
    ScriptEnv::registerEnums(fun, *svg->metaObject());
    return fun;
}

QScriptValue SimpleJavaScriptApplet::newPlasmaFrameSvg(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0) {
        return context->throwError(i18n("Constructor takes at least 1 argument"));
    }

    QString filename = context->argument(0).toString();

    bool parentedToApplet = false;
    QGraphicsWidget *parent = extractParent(context, engine, 1, &parentedToApplet);
    FrameSvg *frameSvg = new FrameSvg(parent);
    frameSvg->setImagePath(parentedToApplet ? filename : findSvg(engine, filename));

    QScriptValue fun = engine->newQObject(frameSvg);
    ScriptEnv::registerEnums(fun, *frameSvg->metaObject());
    return fun;
}

QScriptValue SimpleJavaScriptApplet::newPlasmaExtenderItem(QScriptContext *context, QScriptEngine *engine)
{
    Plasma::Extender *extender = 0;
    if (context->argumentCount() > 0) {
        extender = qobject_cast<Plasma::Extender *>(context->argument(0).toQObject());
    }

    if (!extender) {
        AppletInterface *interface = extractAppletInterface(engine);
        if (!interface) {
            engine->undefinedValue();
        }

        extender = interface->extender();
    }

    Plasma::ExtenderItem *extenderItem = new Plasma::ExtenderItem(extender);
    QScriptValue fun = engine->newQObject(extenderItem);
    ScriptEnv::registerEnums(fun, *extenderItem->metaObject());
    return fun;
}

QScriptValue SimpleJavaScriptApplet::widgetAdjustSize(QScriptContext *context, QScriptEngine *engine)
{
    QGraphicsWidget *widget = qobject_cast<QGraphicsWidget*>(context->thisObject().toQObject());
    if (widget) {
        widget->adjustSize();
    }
    return engine->undefinedValue();
}

void SimpleJavaScriptApplet::installWidgets(QScriptEngine *engine)
{
    QScriptValue globalObject = engine->globalObject();
    if (!s_widgetLoader) {
        s_widgetLoader = new UiLoader;
    }

    foreach (const QString &widget, s_widgetLoader->availableWidgets()) {
        QScriptValue fun = engine->newFunction(createWidget);
        QScriptValue name = engine->toScriptValue(widget);
        fun.setProperty(QString("functionName"), name,
                         QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);
        fun.setProperty(QString("prototype"), engine->newObject());
        globalObject.setProperty(widget, fun);
    }
}

QGraphicsWidget *SimpleJavaScriptApplet::extractParent(QScriptContext *context, QScriptEngine *engine,
                                                       int argIndex, bool *parentedToApplet)
{
    if (parentedToApplet) {
        *parentedToApplet = false;
    }

    QGraphicsWidget *parent = 0;
    if (context->argumentCount() >= argIndex) {
        parent = qobject_cast<QGraphicsWidget*>(context->argument(argIndex).toQObject());
    }

    if (!parent) {
        AppletInterface *interface = extractAppletInterface(engine);
        if (!interface) {
            return 0;
        }

        //kDebug() << "got the applet!";
        parent = interface->applet();

        if (parentedToApplet) {
            *parentedToApplet = true;
        }
    }

    return parent;
}

QScriptValue SimpleJavaScriptApplet::createWidget(QScriptContext *context, QScriptEngine *engine)
{
    QGraphicsWidget *parent = extractParent(context, engine);
    QString self = context->callee().property("functionName").toString();
    if (!s_widgetLoader) {
        s_widgetLoader = new UiLoader;
    }

    QGraphicsWidget *w = s_widgetLoader->createWidget(self, parent);

    if (!w) {
        return QScriptValue();
    }

    QScriptValue fun = engine->newQObject(w);
    fun.setPrototype(context->callee().property("prototype"));
    fun.setProperty("adjustSize", engine->newFunction(widgetAdjustSize));

    //register enums will be accessed for instance as frame.Sunken for Frame shadow...
    ScriptEnv::registerEnums(fun, *w->metaObject());
    return fun;
}

void SimpleJavaScriptApplet::collectGarbage()
{
    m_engine->collectGarbage();
}

/*
 * Workaround the fact that QtScripts handling of variants seems a bit broken.
 */
QScriptValue SimpleJavaScriptApplet::variantToScriptValue(QVariant var)
{
    return ::variantToScriptValue(m_engine, var);
}

K_EXPORT_PLASMA_APPLETSCRIPTENGINE(qscriptapplet, SimpleJavaScriptApplet)

