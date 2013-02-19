/*
 *   Copyright 2009 by Alan Alpert <alan.alpert@nokia.com>
 *   Copyright 2010 by Ménard Alexis <menard@kde.org>

 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlExpression>
#include <QQmlProperty>
#include <QFile>
#include <QTimer>
#include <QUiLoader>
#include <QWidget>

#include <KConfigGroup>
#include <KDebug>
#include <KGlobal>
#include <KLocale>
#include <KGlobalSettings>
#include <klocalizedstring.h>

#include <Plasma/Applet>
#include <Plasma/Corona>
#include <Plasma/Package>
#include <Plasma/PluginLoader>
#include <Plasma/Service>

#include "plasmoid/declarativeappletscript.h"

#include "plasmoid/appletinterface.h"
#include "plasmoid/containmentinterface.h"

#include "declarative/qmlobject.h"
#include "declarative/packageaccessmanagerfactory.h"


K_EXPORT_PLASMA_APPLETSCRIPTENGINE(declarativeappletscript, DeclarativeAppletScript)


DeclarativeAppletScript::DeclarativeAppletScript(QObject *parent, const QVariantList &args)
    : Plasma::AppletScript(parent),
      m_qmlObject(0),
      m_interface(0)
{
    qmlRegisterType<AppletInterface>();
    Q_UNUSED(args);
}

DeclarativeAppletScript::~DeclarativeAppletScript()
{
}

bool DeclarativeAppletScript::init()
{
    Plasma::Containment *pc = qobject_cast<Plasma::Containment *>(applet());

    if (pc) {
        QString type = pc->pluginInfo().property("X-Plasma-ContainmentType").toString();

        if (type == "DesktopContainment") {
            pc->setContainmentType(Plasma::DesktopContainment);
        } else if (type == "PanelContainment") {
            pc->setContainmentType(Plasma::PanelContainment);
        } if (type == "CustomContainment") {
            pc->setContainmentType(Plasma::CustomContainment);
        } else if (type == "CustomPanelContainment") {
            pc->setContainmentType(Plasma::CustomPanelContainment);
        }
    }

    m_qmlObject = new QmlObject(applet());
    m_qmlObject->setInitializationDelayed(true);
    //FIXME: what replaced this?
    //KGlobal::locale()->insertCatalog("plasma_applet_" % description().pluginName());

    //make possible to import extensions from the package
    //FIXME: probably to be removed, would make possible to use native code from within the package :/
    //m_qmlObject->engine()->addImportPath(package()->path()+"/contents/imports");

    //use our own custom network access manager that will access Plasma packages and to manage security (i.e. deny access to remote stuff when the proper extension isn't enabled
    QQmlEngine *engine = m_qmlObject->engine();
    QQmlNetworkAccessManagerFactory *factory = engine->networkAccessManagerFactory();
    engine->setNetworkAccessManagerFactory(0);
    delete factory;
    engine->setNetworkAccessManagerFactory(new PackageAccessManagerFactory(package()));

    m_qmlObject->setQmlPath(mainScript());
    

    if (!m_qmlObject->engine() || !m_qmlObject->engine()->rootContext() || !m_qmlObject->engine()->rootContext()->isValid() || m_qmlObject->mainComponent()->isError()) {
        QString reason;
        foreach (QQmlError error, m_qmlObject->mainComponent()->errors()) {
            reason += error.toString()+'\n';
        }
        reason = i18n("Error loading QML file: %1", reason);

        m_qmlObject->setQmlPath(applet()->containment()->corona()->package().filePath("ui", "AppletError.qml"));
        m_qmlObject->completeInitialization();


        //even the error message QML may fail
        if (m_qmlObject->mainComponent()->isError()) {
            return false;
        } else {
            m_qmlObject->rootObject()->setProperty("reason", reason);
        }

        setLaunchErrorMessage(reason);
    }

    Plasma::Applet *a = applet();

    if (pc) {
        m_interface = new ContainmentInterface(this);
    //fail? so it's a normal Applet
    } else {
        m_interface = new AppletInterface(this);
    }

    m_interface->setParent(this);

    connect(applet(), SIGNAL(activate()),
            this, SLOT(activate()));

    setupObjects();

    m_qmlObject->completeInitialization();

    m_interface->setUiObject(m_qmlObject->rootObject());

    // set the graphicObject dynamic property on applet
    a->setProperty("graphicObject", QVariant::fromValue(m_interface));
    qDebug() << "Graphic object created:" << a << a->property("graphicObject");

    //Create the ToolBox
    if (pc) {
        Plasma::Package pkg = Plasma::PluginLoader::self()->loadPackage("Plasma/Generic");
        pkg.setPath("org.kde.toolbox");

        if (pkg.isValid()) {
            QQmlComponent *toolBoxComponent = new QQmlComponent(m_qmlObject->engine(), this);
            toolBoxComponent->loadUrl(QUrl::fromLocalFile(pkg.filePath("mainscript")));
            QObject *toolBoxObject = toolBoxComponent->create(engine->rootContext());

            QObject *containmentGraphicObject = m_interface->uiObject();

            if (containmentGraphicObject && toolBoxObject) {
                //memory management
                toolBoxComponent->setParent(toolBoxObject);
                toolBoxObject->setProperty("parent", QVariant::fromValue(containmentGraphicObject));

                containmentGraphicObject->setProperty("toolBox", QVariant::fromValue(toolBoxObject));
            } else {
                delete toolBoxComponent;
                delete toolBoxObject;
            }
        } else {
            kWarning() << "Could not load org.kde.toolbox package.";
        }
    }

    return !a->failedToLaunch();
}

QString DeclarativeAppletScript::filePath(const QString &type, const QString &file) const
{
    return package().filePath(type.toLocal8Bit().constData(), file);
}

void DeclarativeAppletScript::configChanged()
{
/*TODO: invent something that can replace event listeners
    if (!m_env) {
        return;
    }

    m_env->callEventListeners("configchanged");
*/
}

QObject *DeclarativeAppletScript::loadui(const QString &filename)
{
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly)) {
        kWarning() << i18n("Unable to open '%1'",filename);
        return 0;
    }

    QUiLoader loader;
    QWidget *w = loader.load(&f);
    f.close();

    return w;
}


void DeclarativeAppletScript::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        emit formFactorChanged();
    }

    if (constraints & Plasma::LocationConstraint) {
        emit locationChanged();
    }

    if (constraints & Plasma::ContextConstraint) {
        emit contextChanged();
    }
}

void DeclarativeAppletScript::activate()
{
#if 0
TODO: callEventListeners is broken without qscriptengine
    if (!m_env) {
        return;
    }

    m_env->callEventListeners("activate");
#endif
}

void DeclarativeAppletScript::executeAction(const QString &name)
{
    if (m_qmlObject->rootObject()) {
         QMetaObject::invokeMethod(m_qmlObject->rootObject(), QString("action_" + name).toLatin1(), Qt::DirectConnection);
    }
}

bool DeclarativeAppletScript::include(const QString &path)
{
    /*TODO: probably include() doesn't make any sense anymore
    return m_env->include(path);
    */
    return false;
}

void DeclarativeAppletScript::setupObjects()
{
    m_qmlObject->engine()->rootContext()->setContextProperty("plasmoid", m_interface);

#if 0
TODO: make this work with QQmlEngine
    m_engine = m_qmlObject->scriptEngine();
    if (!m_engine) {
        return;
    }

    connect(m_engine, SIGNAL(signalHandlerException(const QScriptValue &)),
            this, SLOT(signalHandlerException(const QScriptValue &)));

    delete m_env;
    m_env = new ScriptEnv(this, m_engine);

    QScriptValue global = m_engine->globalObject();

    QScriptValue v = m_engine->newVariant(QVariant::fromValue(*applet()->package()));
    global.setProperty("__plasma_package", v,
                       QScriptValue::ReadOnly | QScriptValue::Undeletable | QScriptValue::SkipInEnumeration);

    m_self = m_engine->newQObject(m_interface);
    m_self.setScope(global);
    global.setProperty("plasmoid", m_self);
    m_env->addMainObjectProperties(m_self);

    QScriptValue args = m_engine->newArray();
    int i = 0;
    foreach (const QVariant &arg, applet()->startupArguments()) {
        args.setProperty(i, m_engine->newVariant(arg));
        ++i;
    }
    global.setProperty("startupArguments", args);

    // Add a global loadui method for ui files
    QScriptValue fun = m_engine->newFunction(DeclarativeAppletScript::loadui);
    global.setProperty("loadui", fun);

    ScriptEnv::registerEnums(global, AppletInterface::staticMetaObject);
    //Make enum values accessible also as plasmoid.Planar etc
    ScriptEnv::registerEnums(m_self, AppletInterface::staticMetaObject);

    global.setProperty("service", m_engine->newFunction(DeclarativeAppletScript::service));
    global.setProperty("loadService", m_engine->newFunction(DeclarativeAppletScript::loadService));

    //Add stuff from Qt
    //TODO: move to libkdeclarative?
    ByteArrayClass *baClass = new ByteArrayClass(m_engine);
    global.setProperty("ByteArray", baClass->constructor());
    global.setProperty("QPoint", constructQPointClass(m_engine));

    // Add stuff from KDE libs
    qScriptRegisterSequenceMetaType<KUrl::List>(m_engine);
    global.setProperty("Url", constructKUrlClass(m_engine));

    // Add stuff from Plasma
    global.setProperty("Svg", m_engine->newFunction(DeclarativeAppletScript::newPlasmaSvg));
    global.setProperty("FrameSvg", m_engine->newFunction(DeclarativeAppletScript::newPlasmaFrameSvg));

    if (!m_env->importExtensions(description(), m_self, m_auth)) {
        return;
    }

    registerSimpleAppletMetaTypes(m_engine);
    QTimer::singleShot(0, this, SLOT(configChanged()));
#endif
}

QObject *DeclarativeAppletScript::loadService(const QString &pluginName)
{
    return Plasma::PluginLoader::self()->loadService(pluginName, QVariantList(), applet());
}

QList<QAction*> DeclarativeAppletScript::contextualActions()
{
    if (!m_interface) {
        return QList<QAction *>();
    }

    return m_interface->contextualActions();
}

QQmlEngine *DeclarativeAppletScript::engine() const
{
    return m_qmlObject->engine();
}


#include "declarativeappletscript.moc"

