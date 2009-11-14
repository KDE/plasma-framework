/*
 *   Copyright 2007-2008 Richard J. Moore <rich@kde.org>
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

#include <QScriptEngine>
#include <QFile>
#include <QUiLoader>
#include <QGraphicsLayout>
#include <QWidget>

#include <KDebug>
#include <KFileDialog>
#include <KIcon>
#include <KLocale>
#include <KStandardDirs>
#include <KConfigGroup>

#include <Plasma/AbstractAnimation>
#include <Plasma/AnimationGroup>
#include <Plasma/Applet>
#include <Plasma/Svg>
#include <Plasma/FrameSvg>
#include <Plasma/Package>
#include <Plasma/VideoWidget>

#include "appletinterface.h"
#include "qtgui/filedialogproxy.h"

using namespace Plasma;

#include "bind_dataengine.h"
#include "bind_i18n.h"

Q_DECLARE_METATYPE(QPainter*)
Q_DECLARE_METATYPE(QStyleOptionGraphicsItem*)
Q_DECLARE_METATYPE(SimpleJavaScriptApplet*)
Q_DECLARE_METATYPE(AppletInterface*)
Q_DECLARE_METATYPE(Applet*)
Q_DECLARE_METATYPE(QGraphicsWidget*)
Q_DECLARE_METATYPE(QGraphicsLayout*)
Q_DECLARE_METATYPE(KConfigGroup)
Q_DECLARE_METATYPE(Plasma::AbstractAnimation *)
Q_DECLARE_METATYPE(Plasma::AnimationGroup *)

Q_SCRIPT_DECLARE_QMETAOBJECT(AppletInterface, SimpleJavaScriptApplet*)

QScriptValue constructColorClass(QScriptEngine *engine);
QScriptValue constructFontClass(QScriptEngine *engine);
QScriptValue constructGraphicsItemClass(QScriptEngine *engine);
QScriptValue constructKUrlClass(QScriptEngine *engine);
QScriptValue constructLinearLayoutClass(QScriptEngine *engine);
QScriptValue constructAnchorLayoutClass(QScriptEngine *engine);
QScriptValue constructPainterClass(QScriptEngine *engine);
QScriptValue constructQPixmapClass(QScriptEngine *engine);
QScriptValue constructQPointClass(QScriptEngine *engine);
QScriptValue constructQRectFClass(QScriptEngine *engine);
QScriptValue constructQSizeFClass(QScriptEngine *engine);
QScriptValue constructTimerClass(QScriptEngine *engine);

//typedef VideoWidget::Control Control;
Q_DECLARE_FLAGS(Controls, VideoWidget::Control)
Q_DECLARE_METATYPE(Controls)

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

QScriptValue qScriptValueFromControls(QScriptEngine *engine, const Controls &controls)
{
    return QScriptValue(engine, controls);
}

void controlsFromScriptValue(const QScriptValue& obj, Controls &controls)
{
    int flagValue = obj.toInteger();
    //FIXME: it has to be a less ugly way to do that :)
    if (flagValue&VideoWidget::Play) {
        controls |= VideoWidget::Play;
    }
    if (flagValue&VideoWidget::Pause) {
        controls |= VideoWidget::Pause;
    }
    if (flagValue&VideoWidget::Stop) {
        controls |= VideoWidget::Stop;
    }
    if (flagValue&VideoWidget::PlayPause) {
        controls |= VideoWidget::PlayPause;
    }
    if (flagValue&VideoWidget::Progress) {
        controls |= VideoWidget::Progress;
    }
    if (flagValue&VideoWidget::Volume) {
        controls |= VideoWidget::Volume;
    }
    if (flagValue&VideoWidget::OpenFile) {
        controls |= VideoWidget::OpenFile;
    }
}

QScriptValue qScriptValueFromKConfigGroup(QScriptEngine *engine, const KConfigGroup &config)
{
    QScriptValue obj = engine->newObject();

    if (!config.isValid()) {
        return obj;
    }

    QMap<QString, QString> entryMap = config.entryMap();
    QMap<QString, QString>::const_iterator it = entryMap.constBegin();
    QMap<QString, QString>::const_iterator begin = it;
    QMap<QString, QString>::const_iterator end = entryMap.constEnd();

    //setting the group name
    obj.setProperty("__name", QScriptValue(engine, config.name()));

    //setting the key/value pairs
    for (it = begin; it != end; ++it) {
        //kDebug() << "setting" << it.key() << "to" << it.value();
        QString prop = it.key();
        prop.replace(' ', '_');
        obj.setProperty(prop, variantToScriptValue(engine, it.value()));
    }

    return obj;
}

void kConfigGroupFromScriptValue(const QScriptValue& obj, KConfigGroup &config)
{
    KConfigSkeleton *skel = new KConfigSkeleton();
    config = KConfigGroup(skel->config(), obj.property("__name").toString());

    QScriptValueIterator it(obj);

    while (it.hasNext()) {
        it.next();
        //kDebug() << it.name() << "is" << it.value().toString();
        if (it.name() != "__name") {
            config.writeEntry(it.name(), it.value().toString());
        }
    }
}

void registerEnums(QScriptEngine *engine, QScriptValue &scriptValue, const QMetaObject &meta)
{
    //manually create enum values. ugh
    for (int i = 0; i < meta.enumeratorCount(); ++i) {
        QMetaEnum e = meta.enumerator(i);
        //kDebug() << e.name();
        for (int i=0; i < e.keyCount(); ++i) {
            //kDebug() << e.key(i) << e.value(i);
            scriptValue.setProperty(e.key(i), QScriptValue(engine, e.value(i)));
        }
    }
}

KSharedPtr<UiLoader> SimpleJavaScriptApplet::s_widgetLoader;
QHash<QString, Plasma::Animator::Animation> SimpleJavaScriptApplet::s_animationDefs;

SimpleJavaScriptApplet::SimpleJavaScriptApplet(QObject *parent, const QVariantList &args)
    : Plasma::AppletScript(parent)
{
    Q_UNUSED(args)
//    kDebug() << "Script applet launched, args" << applet()->startupArguments();

    m_engine = new QScriptEngine(this);
}

SimpleJavaScriptApplet::~SimpleJavaScriptApplet()
{
    if (s_widgetLoader.count() == 1) {
        s_widgetLoader.clear();
    }
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
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        kWarning() << i18n("Unable to load script file: %1", path);
        return false;
    }

    QString script = file.readAll();
    //kDebug() << "Script says" << script;

    m_engine->evaluate(script);
    if (m_engine->hasUncaughtException()) {
        reportError(m_engine, true);
        return false;
    }

    return true;
}

void SimpleJavaScriptApplet::populateAnimationsHash()
{
    if (s_animationDefs.isEmpty()) {
        s_animationDefs.insert("fade", Plasma::Animator::FadeAnimation);
        s_animationDefs.insert("grow", Plasma::Animator::GrowAnimation);
        s_animationDefs.insert("expand", Plasma::Animator::ExpandAnimation);
        s_animationDefs.insert("pause", Plasma::Animator::PauseAnimation);
        s_animationDefs.insert("pulse", Plasma::Animator::PulseAnimation);
        s_animationDefs.insert("rotate", Plasma::Animator::RotationAnimation);
        s_animationDefs.insert("rotateStacked", Plasma::Animator::RotationStackedAnimation);
        s_animationDefs.insert("slide", Plasma::Animator::SlideAnimation);
    }
}

bool SimpleJavaScriptApplet::init()
{
    setupObjects();
    populateAnimationsHash();

    if (!importExtensions()) {
        return false;
    }

    kDebug() << "ScriptName:" << applet()->name();
    kDebug() << "ScriptCategory:" << applet()->category();

    return include(mainScript());
}

bool SimpleJavaScriptApplet::importBuiltinExtesion(const QString &extension)
{
    if ("filedialog" == extension) {
        //qScriptRegisterMetaType<KFileDialog*>(m_engine, qScriptValueFromKFileDialog, kFileDialogFromQScriptValue);
        QScriptValue global = m_engine->globalObject();
        global.setProperty("OpenFileDialog", m_engine->newFunction(SimpleJavaScriptApplet::fileDialogOpen));
        global.setProperty("SaveFileDialog", m_engine->newFunction(SimpleJavaScriptApplet::fileDialogSave));
        return true;
    } else if ("launchapp" == extension) {
        return true;
    } else if ("http" == extension) {
        return true;
    } else if ("networkio" == extension) {
        return true;
    } else if ("localio" == extension) {
        return true;
    }

    return false;
}

bool SimpleJavaScriptApplet::importExtensions()
{
    KPluginInfo info = description();
    QStringList requiredExtensions = info.property("X-Plasma-RequiredExtensions").toStringList();
    kDebug() << "required extensions are" << requiredExtensions;
    foreach (const QString &ext, requiredExtensions) {
        QString extension = ext.toLower();
        if (m_extensions.contains(extension)) {
            continue;
        }

        if (!applet()->hasAuthorization(extension)) {
            setFailedToLaunch(true,
                              i18n("Authorization for required extension '%1' was denied.",
                                   extension));
            return false;
        }

        if (!importBuiltinExtesion(extension)) {
            m_engine->importExtension(extension);
        }

        if (m_engine->hasUncaughtException()) {
            reportError(m_engine, true);
            return false;
        } else {
            m_extensions << extension;
        }
    }

    QStringList optionalExtensions = info.property("X-Plasma-OptionalExtensions").toStringList();
    kDebug() << "extensions are" << optionalExtensions;
    foreach (const QString &ext, requiredExtensions) {
        QString extension = ext.toLower();

        if (m_extensions.contains(extension)) {
            continue;
        }

        if (!applet()->hasAuthorization(extension)) {
            setFailedToLaunch(true,
                              i18n("Authorization for required extension '%1' was denied.",
                                   extension));
            continue;
        }

        if (!importBuiltinExtesion(extension)) {
            m_engine->importExtension(extension);
        }

        m_engine->importExtension(extension);
        if (m_engine->hasUncaughtException()) {
            reportError(m_engine);
        } else {
            m_extensions << extension;
        }
    }

    return true;
}

typedef AbstractAnimation* AbstractAnimationPtr;
QScriptValue qScriptValueFromAbstractAnimation(QScriptEngine *engine, const AbstractAnimationPtr &anim)
{
    return engine->newQObject(const_cast<AbstractAnimation *>(anim));
}

void abstractAnimationFromQScriptValue(const QScriptValue &scriptValue, AbstractAnimationPtr &anim)
{
    QObject *obj = scriptValue.toQObject();
    anim = static_cast<AbstractAnimation *>(obj);
}

typedef QGraphicsWidget * QGraphicsWidgetPtr;
QScriptValue qScriptValueFromQGraphicsWidget(QScriptEngine *engine, const QGraphicsWidgetPtr &anim)
{
    return engine->newQObject(const_cast<QGraphicsWidget *>(anim));
}

void qGraphicsWidgetFromQScriptValue(const QScriptValue &scriptValue, QGraphicsWidgetPtr &anim)
{
    QObject *obj = scriptValue.toQObject();
    anim = static_cast<QGraphicsWidget *>(obj);
}

void SimpleJavaScriptApplet::setupObjects()
{
    QScriptValue global = m_engine->globalObject();

    // Bindings for animations
    global.setProperty("animation", m_engine->newFunction(SimpleJavaScriptApplet::animation));
    qScriptRegisterMetaType<AbstractAnimation*>(m_engine, qScriptValueFromAbstractAnimation, abstractAnimationFromQScriptValue);
    global.setProperty("AnimationGroup", m_engine->newFunction(SimpleJavaScriptApplet::animationGroup));

    // Bindings for data engine
    m_engine->setDefaultPrototype(qMetaTypeId<DataEngine*>(), m_engine->newQObject(new DataEngine()));
    m_engine->setDefaultPrototype(qMetaTypeId<Service*>(), m_engine->newQObject(new DummyService()));
    m_engine->setDefaultPrototype(qMetaTypeId<ServiceJob*>(), m_engine->newQObject(new ServiceJob(QString(), QString(), QMap<QString, QVariant>())));

    bindI18N(m_engine);
    global.setProperty("dataEngine", m_engine->newFunction(SimpleJavaScriptApplet::dataEngine));
    global.setProperty("service", m_engine->newFunction(SimpleJavaScriptApplet::service));
    qScriptRegisterMetaType<DataEngine::Data>(m_engine, qScriptValueFromData, 0, QScriptValue());
    qScriptRegisterMetaType<KConfigGroup>(m_engine, qScriptValueFromKConfigGroup, kConfigGroupFromScriptValue, QScriptValue());

    // Expose applet interface
    m_interface = new AppletInterface(this);
    m_self = m_engine->newQObject(m_interface);
    m_self.setScope(global);
    global.setProperty("plasmoid", m_self);

    QScriptValue args = m_engine->newArray();
    int i = 0;
    foreach (const QVariant &arg, applet()->startupArguments()) {
        args.setProperty(i, variantToScriptValue(arg));
        ++i;
    }
    global.setProperty("startupArguments", args);

    registerEnums(m_engine, global, AppletInterface::staticMetaObject);


    // Add a global loadui method for ui files
    qScriptRegisterMetaType<QGraphicsWidget*>(m_engine, qScriptValueFromQGraphicsWidget, qGraphicsWidgetFromQScriptValue);
    QScriptValue fun = m_engine->newFunction(SimpleJavaScriptApplet::loadui);
    global.setProperty("loadui", fun);

    fun = m_engine->newFunction(SimpleJavaScriptApplet::print);
    global.setProperty("print", fun);


    // Work around bug in 4.3.0
    qMetaTypeId<QVariant>();

    // Add stuff from Qt
    global.setProperty("QPainter", constructPainterClass(m_engine));
    global.setProperty("QGraphicsItem", constructGraphicsItemClass(m_engine));
    global.setProperty("QTimer", constructTimerClass(m_engine));
    global.setProperty("QFont", constructFontClass(m_engine));
    global.setProperty("QColor", constructColorClass(m_engine));
    global.setProperty("QRectF", constructQRectFClass(m_engine));
    global.setProperty("QPixmap", constructQPixmapClass(m_engine));
    global.setProperty("QSizeF", constructQSizeFClass(m_engine));
    global.setProperty("QPoint", constructQPointClass(m_engine));
    global.setProperty("LinearLayout", constructLinearLayoutClass(m_engine));
    global.setProperty("AnchorLayout", constructAnchorLayoutClass(m_engine));

    // Add stuff from KDE libs
    global.setProperty("Url", constructKUrlClass(m_engine));

    // Add stuff from Plasma
    global.setProperty("PlasmaSvg", m_engine->newFunction(SimpleJavaScriptApplet::newPlasmaSvg));
    global.setProperty("PlasmaFrameSvg", m_engine->newFunction(SimpleJavaScriptApplet::newPlasmaFrameSvg));

    installWidgets(m_engine);
}

QSet<QString> SimpleJavaScriptApplet::loadedExtensions() const
{
    return m_extensions;
}

QScriptValue SimpleJavaScriptApplet::dataEngine(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) {
        return context->throwError(i18n("dataEngine() takes one argument"));
    }

    QString dataEngine = context->argument(0).toString();

    QScriptValue appletValue = engine->globalObject().property("plasmoid");
    //kDebug() << "appletValue is " << appletValue.toString();

    QObject *appletObject = appletValue.toQObject();
    if (!appletObject) {
        return context->throwError(i18n("Could not extract the AppletObject"));
    }

    AppletInterface *interface = qobject_cast<AppletInterface*>(appletObject);
    if (!interface) {
        return context->throwError(i18n("Could not extract the Applet"));
    }

    DataEngine *data = interface->dataEngine(dataEngine);
    return engine->newQObject(data);
}

QScriptValue SimpleJavaScriptApplet::service(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 2) {
        return context->throwError(i18n("service() takes two arguments"));
    }

    QString dataEngine = context->argument(0).toString();

    QScriptValue appletValue = engine->globalObject().property("plasmoid");
    //kDebug() << "appletValue is " << appletValue.toString();

    QObject *appletObject = appletValue.toQObject();
    if (!appletObject) {
        return context->throwError(i18n("Could not extract the AppletObject"));
    }

    AppletInterface *interface = qobject_cast<AppletInterface*>(appletObject);
    if (!interface) {
        return context->throwError(i18n("Could not extract the Applet"));
    }

    DataEngine *data = interface->dataEngine(dataEngine);
    QString source = context->argument(1).toString();
    Service *service = data->serviceForSource(source);
    //kDebug( )<< "lets try to get" << source << "from" << dataEngine;
    return engine->newQObject(service);
}

#include <Plasma/Animation>
QScriptValue SimpleJavaScriptApplet::animation(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) {
        return context->throwError(i18n("animation() takes one argument"));
    }

    QString animName = context->argument(0).toString().toLower();
    if (!s_animationDefs.contains(animName)) {
        return context->throwError(i18n("%1 is not a known animation type", animName));
    }

    QScriptValue appletValue = engine->globalObject().property("plasmoid");
    //kDebug() << "appletValue is " << appletValue.toString();

    QObject *appletObject = appletValue.toQObject();
    if (!appletObject) {
        return context->throwError(i18n("Could not extract the AppletObject"));
    }

    AppletInterface *interface = qobject_cast<AppletInterface*>(appletObject);
    if (!interface) {
        return context->throwError(i18n("Could not extract the Applet"));
    }

    Plasma::AbstractAnimation *anim = Plasma::Animator::create(s_animationDefs.value(animName), interface->applet());
    anim->setWidgetToAnimate(interface->applet());
    return engine->newQObject(anim);
}

QScriptValue SimpleJavaScriptApplet::animationGroup(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue appletValue = engine->globalObject().property("plasmoid");
    //kDebug() << "appletValue is " << appletValue.toString();

    QObject *appletObject = appletValue.toQObject();
    if (!appletObject) {
        return context->throwError(i18n("Could not extract the AppletObject"));
    }

    AppletInterface *interface = qobject_cast<AppletInterface*>(appletObject);
    if (!interface) {
        return context->throwError(i18n("Could not extract the Applet"));
    }

    Plasma::AnimationGroup *group = new Plasma::AnimationGroup(interface->applet());
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

    return engine->newQObject(w);
}

QString SimpleJavaScriptApplet::findImageFile(QScriptEngine *engine, const QString &file)
{
    QScriptValue appletValue = engine->globalObject().property("plasmoid");
    //kDebug() << "appletValue is " << appletValue.toString();

    QObject *appletObject = appletValue.toQObject();
    if (!appletObject) {
        return QString();
    }

    AppletInterface *interface = qobject_cast<AppletInterface*>(appletObject);
    if (!interface) {
        return QString();
    }

    return interface->package()->filePath("images", file);
}

QString SimpleJavaScriptApplet::findSvg(QScriptEngine *engine, const QString &file)
{
    QScriptValue appletValue = engine->globalObject().property("plasmoid");
    //kDebug() << "appletValue is " << appletValue.toString();

    QObject *appletObject = appletValue.toQObject();
    if (!appletObject) {
        return file;
    }

    AppletInterface *interface = qobject_cast<AppletInterface*>(appletObject);
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

    QString filename = context->argument(0).toString();
    QObject *parent = 0;

    QScriptValue appletValue = engine->globalObject().property("plasmoid");
    QObject *appletObject = appletValue.toQObject();
    AppletInterface *interface = 0;
    if (appletObject) {
        interface = qobject_cast<AppletInterface*>(appletObject);
    }

    bool parentedToApplet = false;
    if (context->argumentCount() == 2) {
        parent = context->argument(1).toQObject();
        parentedToApplet = parent == interface;
    }

    if (!parent && interface) {
        parentedToApplet = true;
        parent = interface->applet();
    }

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
    QObject *parent = 0;

    if (context->argumentCount() == 2) {
        parent = context->argument(1).toQObject();
    }

    bool parentedToApplet = false;
    if (!parent) {
        QScriptValue appletValue = engine->globalObject().property("plasmoid");
        //kDebug() << "appletValue is " << appletValue.toString();

        QObject *appletObject = appletValue.toQObject();
        if (appletObject) {
            AppletInterface *interface = qobject_cast<AppletInterface*>(appletObject);
            if (interface) {
                parentedToApplet = true;
                parent = interface->applet();
            }
        }
    }

    FrameSvg *frameSvg = new FrameSvg(parent);
    frameSvg->setImagePath(parentedToApplet ? filename : findSvg(engine, filename));
    return engine->newQObject(frameSvg);
}

QScriptValue SimpleJavaScriptApplet::fileDialogSave(QScriptContext *context, QScriptEngine *engine)
{
    FileDialogProxy *fd = new FileDialogProxy(KFileDialog::Saving);

    return engine->newQObject(fd);
}

QScriptValue SimpleJavaScriptApplet::fileDialogOpen(QScriptContext *context, QScriptEngine *engine)
{
    FileDialogProxy *fd = new FileDialogProxy(KFileDialog::Opening);

    return engine->newQObject(fd);
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
        fun.setProperty(QString("prototype"), createPrototype(engine, name.toString()));

        globalObject.setProperty(widget, fun);
    }
}

QScriptValue SimpleJavaScriptApplet::createWidget(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() > 1) {
        return context->throwError(i18n("CreateWidget takes one argument"));
    }

    QGraphicsWidget *parent = 0;
    if (context->argumentCount()) {
        parent = qobject_cast<QGraphicsWidget*>(context->argument(0).toQObject());

        if (!parent) {
            context->throwError(i18n("The parent must be a QGraphicsWidget"));
        }
    }

    if (!parent) {
        QScriptValue appletValue = engine->globalObject().property("plasmoid");
        //kDebug() << "appletValue is " << appletValue.toString();

        QObject *appletObject = appletValue.toQObject();
        if (!appletObject) {
            return context->throwError(i18n("Could not extract the AppletObject"));
        }

        AppletInterface *interface = qobject_cast<AppletInterface*>(appletObject);
        if (!interface) {
            return context->throwError(i18n("Could not extract the Applet"));
        }

        //kDebug() << "got the applet!";
        parent = interface->applet();
    }

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

    //register enums will be accessed for instance as frame.Sunken for Frame shadow...
    registerEnums(engine, fun, *w->metaObject());

    //FIXME: still don't have a better approach than try to cast for every widget that could have flags..
    if (qobject_cast<VideoWidget *>(w)) {
        qScriptRegisterMetaType<Controls>(engine, qScriptValueFromControls, controlsFromScriptValue, QScriptValue());
    }

    return fun;
}

QScriptValue SimpleJavaScriptApplet::notSupported(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(engine)
    QString message = context->callee().property("message").toString();
    return context->throwError(i18n("This operation was not supported, %1", message) );
}


QScriptValue SimpleJavaScriptApplet::print(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() != 1) {
        return context->throwError(i18n("print() takes one argument"));
    }

    //TODO: a GUI console? :)
    std::cout << context->argument(0).toString().toLocal8Bit().constData() << std::endl;
    return engine->undefinedValue();
}

QScriptValue SimpleJavaScriptApplet::createPrototype(QScriptEngine *engine, const QString &name)
{
    Q_UNUSED(name)
    QScriptValue proto = engine->newObject();

    // Hook for adding extra properties/methods
    return proto;
}

QScriptValue SimpleJavaScriptApplet::variantToScriptValue(QVariant var)
{
    return ::variantToScriptValue(m_engine, var);
}

K_EXPORT_PLASMA_APPLETSCRIPTENGINE(qscriptapplet, SimpleJavaScriptApplet)

#include "simplejavascriptapplet.moc"


