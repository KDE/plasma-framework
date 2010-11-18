/*
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
 *
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

#include "scriptengine.h"

#include <QFile>
#include <QFileInfo>
#include <QScriptValueIterator>

#include <KDebug>
#include <kdeversion.h>
#include <KServiceTypeTrader>
#include <KShell>
#include <KStandardDirs>

#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/Package>

#include "appinterface.h"
#include "containment.h"
#include "widget.h"
#include "layouttemplatepackagestructure.h"

QScriptValue constructQRectFClass(QScriptEngine *engine);

namespace WorkspaceScripting
{

ScriptEngine::ScriptEngine(Plasma::Corona *corona, QObject *parent)
    : QScriptEngine(parent),
      m_corona(corona)
{
    Q_ASSERT(m_corona);
    AppInterface *interface = new AppInterface(this);
    connect(interface, SIGNAL(print(QString)), this, SIGNAL(print(QString)));
    m_scriptSelf = newQObject(interface, QScriptEngine::QtOwnership,
                              QScriptEngine::ExcludeSuperClassProperties | QScriptEngine::ExcludeSuperClassMethods);
    setupEngine();
    connect(this, SIGNAL(signalHandlerException(QScriptValue)), this, SLOT(exception(QScriptValue)));
}

ScriptEngine::~ScriptEngine()
{
}

QScriptValue ScriptEngine::activityById(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0) {
        return context->throwError(i18n("activityById requires an id"));
    }

    const uint id = context->argument(0).toInt32();
    ScriptEngine *env = envFor(engine);
    foreach (Plasma::Containment *c, env->m_corona->containments()) {
        if (c->id() == id && !isPanel(c)) {
            return env->wrap(c);
        }
    }

    return engine->undefinedValue();
}

QScriptValue ScriptEngine::activityForScreen(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0) {
        return context->throwError(i18n("activityForScreen requires a screen id"));
    }

    const uint screen = context->argument(0).toInt32();
    const uint desktop = context->argumentCount() > 1 ? context->argument(1).toInt32() : -1;
    ScriptEngine *env = envFor(engine);
    return env->wrap(env->m_corona->containmentForScreen(screen, desktop));
}

QScriptValue ScriptEngine::newActivity(QScriptContext *context, QScriptEngine *engine)
{
    return createContainment("desktop", "desktop", context, engine);
}

QScriptValue ScriptEngine::newPanel(QScriptContext *context, QScriptEngine *engine)
{
    return createContainment("panel", "panel", context, engine);
}

QScriptValue ScriptEngine::createContainment(const QString &type, const QString &defaultPlugin,
                                             QScriptContext *context, QScriptEngine *engine)
{
    QString plugin = context->argumentCount() > 0 ? context->argument(0).toString() :
                                                    defaultPlugin;

    bool exists = false;
    const KPluginInfo::List list = Plasma::Containment::listContainmentsOfType(type);
    foreach (const KPluginInfo &info, list) {
        if (info.pluginName() == plugin) {
            exists = true;
            break;
        }
    }

    if (!exists) {
        return context->throwError(i18n("Could not find a plugin for %1 named %2.", type, plugin));
    }


    ScriptEngine *env = envFor(engine);
    Plasma::Containment *c = env->m_corona->addContainment(plugin);
    if (c) {
        if (type == "panel") {
            // some defaults
            c->setScreen(0);
            c->setLocation(Plasma::TopEdge);
        }
        c->updateConstraints(Plasma::AllConstraints | Plasma::StartupCompletedConstraint);
        c->flushPendingConstraintsEvents();
        emit env->createPendingPanelViews();
    }

    return env->wrap(c);
}

QScriptValue ScriptEngine::wrap(Plasma::Applet *w)
{
    Widget *wrapper = new Widget(w);
    QScriptValue v = newQObject(wrapper, QScriptEngine::ScriptOwnership,
                                QScriptEngine::ExcludeSuperClassProperties |
                                QScriptEngine::ExcludeSuperClassMethods);
    return v;
}

QScriptValue ScriptEngine::wrap(Plasma::Containment *c)
{
    Containment *wrapper = new Containment(c);
    return wrap(wrapper);
}

QScriptValue ScriptEngine::wrap(Containment *c)
{
    QScriptValue v = newQObject(c, QScriptEngine::ScriptOwnership,
                                QScriptEngine::ExcludeSuperClassProperties |
                                QScriptEngine::ExcludeSuperClassMethods);
    v.setProperty("widgetById", newFunction(Containment::widgetById));
    v.setProperty("addWidget", newFunction(Containment::addWidget));
    v.setProperty("widgets", newFunction(Containment::widgets));

    return v;
}

ScriptEngine *ScriptEngine::envFor(QScriptEngine *engine)
{
    QObject *object = engine->globalObject().toQObject();
    Q_ASSERT(object);

    AppInterface *interface = qobject_cast<AppInterface *>(object);
    Q_ASSERT(interface);

    ScriptEngine *env = qobject_cast<ScriptEngine *>(interface->parent());
    Q_ASSERT(env);

    return env;
}

QScriptValue ScriptEngine::panelById(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0) {
        return context->throwError(i18n("activityById requires an id"));
    }

    const uint id = context->argument(0).toInt32();
    ScriptEngine *env = envFor(engine);
    foreach (Plasma::Containment *c, env->m_corona->containments()) {
        if (c->id() == id && isPanel(c)) {
            return env->wrap(c);
        }
    }

    return engine->undefinedValue();
}

QScriptValue ScriptEngine::panels(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(context)

    QScriptValue panels = engine->newArray();
    ScriptEngine *env = envFor(engine);
    int count = 0;

    foreach (Plasma::Containment *c, env->m_corona->containments()) {
        if (isPanel(c)) {
            panels.setProperty(count, env->wrap(c));
            ++count;
        }
    }

    panels.setProperty("length", count);
    return panels;
}

QScriptValue ScriptEngine::fileExists(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(engine)
    if (context->argumentCount() == 0) {
        return false;
    }

    const QString path = context->argument(0).toString();
    if (path.isEmpty()) {
        return false;
    }

    QFile f(KShell::tildeExpand(path));
    return f.exists();
}

QScriptValue ScriptEngine::loadTemplate(QScriptContext *context, QScriptEngine *engine)
{
    kDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
    Q_UNUSED(engine)
    if (context->argumentCount() == 0) {
        kDebug() << "no arguments";
        return false;
    }

    const QString layout = context->argument(0).toString();
    if (layout.isEmpty() || layout.contains("'")) {
        kDebug() << "layout is empty";
        return false;
    }

    const QString constraint = QString("[X-Plasma-Shell] == '%1' and [X-KDE-PluginInfo-Name] == '%2'")
                                      .arg(KGlobal::mainComponent().componentName(),layout);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/LayoutTemplate", constraint);

    if (offers.isEmpty()) {
        kDebug() << "offers fail" << constraint;
        return false;
    }

    Plasma::PackageStructure::Ptr structure(new LayoutTemplatePackageStructure);
    KPluginInfo info(offers.first());
    const QString path = KStandardDirs::locate("data", structure->defaultPackageRoot() + '/' + info.pluginName() + '/');
    if (path.isEmpty()) {
        kDebug() << "script path is empty";
        return false;
    }

    Plasma::Package package(path, structure);
    const QString scriptFile = package.filePath("mainscript");
    if (scriptFile.isEmpty()) {
        kDebug() << "scriptfile is empty";
        return false;
    }

    QFile file(scriptFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        kWarning() << i18n("Unable to load script file: %1", path);
        return false;
    }

    QString script = file.readAll();
    if (script.isEmpty()) {
        kDebug() << "script is empty";
        return false;
    }

    ScriptEngine *env = envFor(engine);
    env->globalObject().setProperty("templateName", env->newVariant(info.name()), QScriptValue::ReadOnly | QScriptValue::Undeletable);
    env->globalObject().setProperty("templateComment", env->newVariant(info.comment()), QScriptValue::ReadOnly | QScriptValue::Undeletable);

    QScriptValue rv = env->newObject();
    QScriptContext *ctx = env->pushContext();
    ctx->setThisObject(rv);

    env->evaluateScript(script, path);

    env->popContext();
    return rv;
}

void ScriptEngine::setupEngine()
{
    QScriptValue v = globalObject();
    QScriptValueIterator it(v);
    while (it.hasNext()) {
        it.next();
        // we provide our own print implementation, but we want the rest
        if (it.name() != "print") {
            m_scriptSelf.setProperty(it.name(), it.value());
        }
    }

    m_scriptSelf.setProperty("QRectF", constructQRectFClass(this));
    m_scriptSelf.setProperty("Activity", newFunction(ScriptEngine::newActivity));
    m_scriptSelf.setProperty("Panel", newFunction(ScriptEngine::newPanel));
    m_scriptSelf.setProperty("activities", newFunction(ScriptEngine::activities));
    m_scriptSelf.setProperty("activityById", newFunction(ScriptEngine::activityById));
    m_scriptSelf.setProperty("activityForScreen", newFunction(ScriptEngine::activityForScreen));
    m_scriptSelf.setProperty("panelById", newFunction(ScriptEngine::panelById));
    m_scriptSelf.setProperty("panels", newFunction(ScriptEngine::panels));
    m_scriptSelf.setProperty("fileExists", newFunction(ScriptEngine::fileExists));
    m_scriptSelf.setProperty("loadTemplate", newFunction(ScriptEngine::loadTemplate));

    setGlobalObject(m_scriptSelf);
}

bool ScriptEngine::isPanel(const Plasma::Containment *c)
{
    if (!c) {
        return false;
    }

    return c->containmentType() == Plasma::Containment::PanelContainment ||
           c->containmentType() == Plasma::Containment::CustomPanelContainment;
}

QScriptValue ScriptEngine::activities(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(context)

    QScriptValue containments = engine->newArray();
    ScriptEngine *env = envFor(engine);
    int count = 0;

    foreach (Plasma::Containment *c, env->corona()->containments()) {
        if (!isPanel(c)) {
            containments.setProperty(count, env->wrap(c));
            ++count;
        }
    }

    containments.setProperty("length", count);
    return containments;
}

Plasma::Corona *ScriptEngine::corona() const
{
    return m_corona;
}

bool ScriptEngine::evaluateScript(const QString &script, const QString &path)
{
    //kDebug() << "evaluating" << m_editor->toPlainText();
    evaluate(script, path);
    if (hasUncaughtException()) {
        //kDebug() << "catch the exception!";
        QString error = i18n("Error: %1 at line %2\n\nBacktrace:\n%3",
                             uncaughtException().toString(),
                             QString::number(uncaughtExceptionLineNumber()),
                             uncaughtExceptionBacktrace().join("\n  "));
        emit printError(error);
        return false;
    }

    return true;
}

void ScriptEngine::exception(const QScriptValue &value)
{
    //kDebug() << "exception caught!" << value.toVariant();
    emit printError(value.toVariant().toString());
}

QStringList ScriptEngine::pendingUpdateScripts()
{
    const QString appName = KGlobal::activeComponent().aboutData()->appName();
    QStringList scripts = KGlobal::dirs()->findAllResources("data", appName + "/updates/*.js");
    QStringList scriptPaths;

    if (scripts.isEmpty()) {
        //kDebug() << "no update scripts";
        return scriptPaths;
    }

    KConfigGroup cg(KGlobal::config(), "Updates");
    QStringList performed = cg.readEntry("performed", QStringList());
    const QString localDir = KGlobal::dirs()->localkdedir();
    const QString localXdgDir = KGlobal::dirs()->localxdgdatadir();

    foreach (const QString &script, scripts) {
        if (performed.contains(script)) {
            continue;
        }

        if (script.startsWith(localDir) || script.startsWith(localXdgDir)) {
            kDebug() << "skipping user local script: " << script;
            continue;
        }

        scriptPaths.append(script);
        performed.append(script);
    }

    cg.writeEntry("performed", performed);
    KGlobal::config()->sync();
    return scriptPaths;
}

QStringList ScriptEngine::defaultLayoutScripts()
{
    const QString appName = KGlobal::activeComponent().aboutData()->appName();
    QStringList scripts = KGlobal::dirs()->findAllResources("data", appName + "/init/*.js");
    QStringList scriptPaths;

    if (scripts.isEmpty()) {
        //kDebug() << "no javascript based layouts";
        return scriptPaths;
    }

    const QString localDir = KGlobal::dirs()->localkdedir();
    const QString localXdgDir = KGlobal::dirs()->localxdgdatadir();

    QSet<QString> scriptNames;
    foreach (const QString &script, scripts) {
        if (script.startsWith(localDir) || script.startsWith(localXdgDir)) {
            kDebug() << "skipping user local script: " << script;
            continue;
        }

        QFileInfo f(script);
        QString filename = f.fileName();
        if (!scriptNames.contains(filename)) {
            scriptNames.insert(filename);
            scriptPaths.append(script);
        }
    }

    return scriptPaths;
}

}

#include "scriptengine.moc"

