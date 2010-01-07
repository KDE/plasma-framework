/*
 *   Copyright 2007-2008 Richard J. Moore <rich@kde.org>
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
#include <QUiLoader>
#include <QGraphicsLayout>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QWidget>

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
#include <Plasma/Svg>
#include <Plasma/FrameSvg>
#include <Plasma/Package>
#include <Plasma/PopupApplet>
#include <Plasma/VideoWidget>

#include "appletauthorization.h"
#include "scriptenv.h"
#include "simplebindings/animationgroup.h"
#include "simplebindings/dataengine.h"
#include "simplebindings/i18n.h"
#include "simplebindings/appletinterface.h"
#include "simplebindings/bytearrayclass.h"
#include "simplebindings/variant.h"

using namespace Plasma;


Q_DECLARE_METATYPE(QPainter*)
Q_DECLARE_METATYPE(QStyleOptionGraphicsItem*)

QScriptValue constructColorClass(QScriptEngine *engine);
QScriptValue constructFontClass(QScriptEngine *engine);
QScriptValue constructGraphicsItemClass(QScriptEngine *engine);
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
QScriptValue constructTimerClass(QScriptEngine *engine);
void registerSimpleAppletMetaTypes(QScriptEngine *engine);

class DummyService : public Service
{
public:
    ServiceJob *createJob(const QString &operation, QMap<QString, QVariant> &parameters)
    {
        Q_UNUSED(operation)
        Q_UNUSED(parameters)
        return 0;
    }
};

KSharedPtr<UiLoader> SimpleJavaScriptApplet::s_widgetLoader;
QHash<QString, Plasma::Animator::Animation> SimpleJavaScriptApplet::s_animationDefs;

SimpleJavaScriptApplet::SimpleJavaScriptApplet(QObject *parent, const QVariantList &args)
    : Plasma::AppletScript(parent)
{
    Q_UNUSED(args)
//    kDebug() << "Script applet launched, args" << applet()->startupArguments();

    m_engine = new ScriptEnv(this);
    connect(m_engine, SIGNAL(reportError(ScriptEnv*,bool)), this, SLOT(engineReportsError(ScriptEnv*,bool)));
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

void SimpleJavaScriptApplet::reportError(QScriptEngine *engine, bool fatal)
{
    SimpleJavaScriptApplet *jsApplet = qobject_cast<SimpleJavaScriptApplet *>(engine->parent());
    const QString failureMsg = i18n("Script failure on line %1:\n%2",
                                    QString::number(engine->uncaughtExceptionLineNumber()),
                                    engine->uncaughtException().toString());
    if (jsApplet) {
        if (fatal) {
            jsApplet->setFailedToLaunch(true, failureMsg);
        } else {
            jsApplet->showMessage(KIcon("dialog-error"), failureMsg, Plasma::ButtonNone);
        }
    } else {
        kDebug() << failureMsg;
    }

    kDebug() << engine->uncaughtExceptionBacktrace();
}

void SimpleJavaScriptApplet::configChanged()
{
    QScriptValue fun = m_self.property("configChanged");
    if (!fun.isFunction()) {
        kDebug() << "Script: plasmoid.configChanged is not a function, " << fun.toString();
        return;
    }

    QScriptContext *ctx = m_engine->pushContext();
    ctx->setActivationObject(m_self);
    //kDebug() << "calling plasmoid";
    fun.call(m_self);
    m_engine->popContext();

    if (m_engine->hasUncaughtException()) {
        reportError(m_engine);
    }
}

void SimpleJavaScriptApplet::dataUpdated(const QString &name, const DataEngine::Data &data)
{
    QScriptValue fun = m_self.property("dataUpdate");
    if (!fun.isFunction()) {
        kDebug() << "Script: dataUpdate is not a function, " << fun.toString();
        return;
    }

    QScriptValueList args;
    args << m_engine->toScriptValue(name) << m_engine->toScriptValue(data);

    QScriptContext *ctx = m_engine->pushContext();
    ctx->setActivationObject(m_self);
    fun.call(m_self, args);
    m_engine->popContext();

    if (m_engine->hasUncaughtException()) {
        reportError(m_engine);
    }
}

void SimpleJavaScriptApplet::executeAction(const QString &name)
{
    //callFunction("action_" + name);
    QScriptValue fun = m_self.property("action_" + name);
    if (fun.isFunction()) {
        QScriptContext *ctx = m_engine->pushContext();
        ctx->setActivationObject(m_self);
        fun.call(m_self);
        m_engine->popContext();

        if (m_engine->hasUncaughtException()) {
            reportError(m_engine);
        }
    }
}

void SimpleJavaScriptApplet::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option)
    Q_UNUSED(contentsRect)

    //kDebug() << "paintInterface() (c++)";
    QScriptValue fun = m_self.property("paintInterface");
    if (!fun.isFunction()) {
        //kDebug() << "Script: paintInterface is not a function, " << fun.toString();
        AppletScript::paintInterface(p, option, contentsRect);
        return;
    }

    QScriptValueList args;
    args << m_engine->toScriptValue(p);
    args << m_engine->toScriptValue(const_cast<QStyleOptionGraphicsItem*>(option));
    args << m_engine->toScriptValue(contentsRect);

    QScriptContext *ctx = m_engine->pushContext();
    ctx->setActivationObject(m_self);
    fun.call(m_self, args);
    m_engine->popContext();

    if (m_engine->hasUncaughtException()) {
        reportError(m_engine);
    }
}

QList<QAction*> SimpleJavaScriptApplet::contextualActions()
{
    return m_interface->contextualActions();
}

void SimpleJavaScriptApplet::callFunction(const QString &functionName, const QScriptValueList &args)
{
    QScriptValue fun = m_self.property(functionName);
    if (fun.isFunction()) {
        QScriptContext *ctx = m_engine->pushContext();
        ctx->setActivationObject(m_self);
        fun.call(m_self, args);
        m_engine->popContext();

        if (m_engine->hasUncaughtException()) {
            reportError(m_engine);
        }
    }
}

void SimpleJavaScriptApplet::constraintsEvent(Plasma::Constraints constraints)
{
    QString functionName;

    if (constraints & Plasma::FormFactorConstraint) {
        callFunction("formFactorChanged");
    }

    if (constraints & Plasma::LocationConstraint) {
        callFunction("locationChanged");
    }

    if (constraints & Plasma::ContextConstraint) {
        callFunction("currentActivityChanged");
    }

    if (constraints & Plasma::SizeConstraint) {
        callFunction("sizeChanged");
    }

    if (constraints & Plasma::ImmutableConstraint) {
        callFunction("immutabilityChanged");
    }
}

bool SimpleJavaScriptApplet::include(const QString &path)
{
    return m_engine->include(path);
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
    setupObjects();

    AppletAuthorization auth(this);
    if (!m_engine->importExtensions(description(), m_self, auth)) {
        return false;
    }

    kDebug() << "ScriptName:" << applet()->name();
    kDebug() << "ScriptCategory:" << applet()->category();

    return m_engine->include(mainScript());
}

void SimpleJavaScriptApplet::setupObjects()
{
    QScriptValue global = m_engine->globalObject();

    // Bindings for animations
    global.setProperty("animation", m_engine->newFunction(SimpleJavaScriptApplet::animation));
    global.setProperty("AnimationGroup", m_engine->newFunction(SimpleJavaScriptApplet::animationGroup));
    global.setProperty("ParallelAnimationGroup", m_engine->newFunction(SimpleJavaScriptApplet::parallelAnimationGroup));

    // Bindings for data engine
    m_engine->setDefaultPrototype(qMetaTypeId<DataEngine*>(), m_engine->newQObject(new DataEngine(), QScriptEngine::ScriptOwnership));
    m_engine->setDefaultPrototype(qMetaTypeId<Service*>(), m_engine->newQObject(new DummyService(), QScriptEngine::ScriptOwnership));
    m_engine->setDefaultPrototype(qMetaTypeId<ServiceJob*>(),
                                  m_engine->newQObject(new ServiceJob(QString(), QString(), QMap<QString, QVariant>()),
                                                       QScriptEngine::ScriptOwnership ));

    bindI18N(m_engine);
    global.setProperty("dataEngine", m_engine->newFunction(SimpleJavaScriptApplet::dataEngine));
    global.setProperty("service", m_engine->newFunction(SimpleJavaScriptApplet::service));

    // Expose applet interface
    const bool isPopupApplet = qobject_cast<Plasma::PopupApplet *>(applet());
    m_interface = isPopupApplet ? new PopupAppletInterface(this) : new AppletInterface(this);
    m_self = m_engine->newQObject(m_interface);
    m_self.setScope(global);
    global.setProperty("plasmoid", m_self);

    QScriptValue args = m_engine->newArray();
    int i = 0;
    foreach (const QVariant &arg, applet()->startupArguments()) {
        args.setProperty(i, ::variantToScriptValue(m_engine, arg));
        ++i;
    }
    global.setProperty("startupArguments", args);

    m_engine->registerEnums(global, AppletInterface::staticMetaObject);


    // Add a global loadui method for ui files
    QScriptValue fun = m_engine->newFunction(SimpleJavaScriptApplet::loadui);
    global.setProperty("loadui", fun);

    // Work around bug in 4.3.0
    qMetaTypeId<QVariant>();

    // Add stuff from Qt
    global.setProperty("QPainter", constructPainterClass(m_engine));
    global.setProperty("QGraphicsItem", constructGraphicsItemClass(m_engine));
    global.setProperty("QTimer", constructTimerClass(m_engine));
    global.setProperty("QFont", constructFontClass(m_engine));
    global.setProperty("QColor", constructColorClass(m_engine));
    global.setProperty("QRectF", constructQRectFClass(m_engine));
    global.setProperty("QPen", constructPenClass(m_engine));
    global.setProperty("QPixmap", constructQPixmapClass(m_engine));
    global.setProperty("QSizeF", constructQSizeFClass(m_engine));
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

    registerSimpleAppletMetaTypes(m_engine);
    installWidgets(m_engine);
}

QSet<QString> SimpleJavaScriptApplet::loadedExtensions() const
{
    return m_engine->loadedExtensions();
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

    const QString dataEngine = context->argument(0).toString();
    DataEngine *data = interface->dataEngine(dataEngine);
    return engine->newQObject(data);
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
    QString animName = context->argument(0).toString().toLower();
    const bool isPause = animName == "pause";
    if (!isPause && !s_animationDefs.contains(animName)) {
        return context->throwError(i18n("%1 is not a known animation type", animName));
    }

    bool parentIsApplet = false;
    QGraphicsWidget *parent = extractParent(context, engine, 0, &parentIsApplet);
    if (isPause) {
        QPauseAnimation *pause = new QPauseAnimation(parent);
        return engine->newQObject(pause);
    } else {
        Plasma::Animation *anim = Plasma::Animator::create(s_animationDefs.value(animName), parent);
        if (anim) {
            if (!parentIsApplet) {
                anim->setTargetWidget(parent);
            }

            return engine->newQObject(anim);
        }
    }

    return context->throwError(i18n("%1 is not a known animation type", animName));
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
    return engine->newQObject(svg);
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
    return engine->newQObject(frameSvg);
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
    static_cast<ScriptEnv*>(engine)->registerEnums(fun, *w->metaObject());
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
#include "simplejavascriptapplet.moc"
