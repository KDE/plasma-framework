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

#include <kdeversion.h>
#include <KServiceTypeTrader>
#include <KShell>
#include <KStandardDirs>

#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/Package>
#include <Plasma/Theme>

#include "widget.h"
#include "layouttemplatepackagestructure.h"

QScriptValue constructQRectFClass(QScriptEngine *engine);

namespace PlasmaKPartScripting
{

ScriptEngine::ScriptEngine(Plasma::Corona *corona, QObject *parent)
    : QScriptEngine(parent),
      m_corona(corona)
{
    Q_ASSERT(m_corona);

    m_containment = m_corona->containments().first();
    Q_ASSERT(m_containment);

    setupEngine();
    connect(this, SIGNAL(signalHandlerException(QScriptValue)), this, SLOT(exception(QScriptValue)));
}

ScriptEngine::~ScriptEngine()
{
}

QScriptValue ScriptEngine::wrap(Plasma::Applet *w)
{
    Widget *wrapper = new Widget(w);
    QScriptValue v = newQObject(wrapper, QScriptEngine::ScriptOwnership,
                                QScriptEngine::ExcludeSuperClassProperties |
                                QScriptEngine::ExcludeSuperClassMethods);
    return v;
}

ScriptEngine *ScriptEngine::envFor(QScriptEngine *engine)
{
    return static_cast<ScriptEngine *>(engine);
}

QScriptValue ScriptEngine::print(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() < 1) {
        return engine->undefinedValue();
    }

    const QString &msg = context->argument(0).toString();

    if (!msg.isEmpty()) {
        emit envFor(engine)->print(msg);
    }

    return engine->undefinedValue();
}

QScriptValue ScriptEngine::knownWidgetTypes(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(context)
    Q_UNUSED(engine)

    QStringList widgets;
    KPluginInfo::List infos = Plasma::Applet::listAppletInfo();

    foreach (const KPluginInfo &info, infos) {
        widgets.append(info.pluginName());
    }

    return qScriptValueFromValue(engine, widgets);
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
        kWarning() << QString("Unable to load script file: %1").arg(path);
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
    env->evaluateScript(script, path);
    return true;
}

void ScriptEngine::setupEngine()
{
    QScriptValue v = globalObject();
    v.setProperty("print", newFunction(ScriptEngine::print));
    v.setProperty("QRectF", constructQRectFClass(this));
    v.setProperty("knownWidgetTypes", newFunction(ScriptEngine::fileExists));
    v.setProperty("fileExists", newFunction(ScriptEngine::fileExists));
    v.setProperty("loadTemplate", newFunction(ScriptEngine::loadTemplate));
    v.setProperty("widgets", newFunction(ScriptEngine::widgets));
    v.setProperty("addWidget", newFunction(ScriptEngine::addWidget));
    v.setProperty("applicationVersion", KGlobal::mainComponent().aboutData()->version(),
                             QScriptValue::PropertyGetter | QScriptValue::ReadOnly | QScriptValue::Undeletable);
    v.setProperty("scriptingVersion", newVariant(PLASMA_KPART_SCRIPTING_VERSION),
                             QScriptValue::PropertyGetter | QScriptValue::ReadOnly | QScriptValue::Undeletable);
    v.setProperty("platformVersion", KDE::versionString(),
                             QScriptValue::PropertyGetter | QScriptValue::ReadOnly | QScriptValue::Undeletable);

    setGlobalObject(v);
}

bool ScriptEngine::evaluateScript(const QString &script, const QString &path)
{
    //kDebug() << "evaluating" << m_editor->toPlainText();
    evaluate(script, path);
    if (hasUncaughtException()) {
        //kDebug() << "catch the exception!";
        QString error = QString("Error: %1 at line %2\n\nBacktrace:\n%3").arg(
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
    QStringList scripts = KGlobal::dirs()->findAllResources("data", appName + "/plasma/layout/updates/*.js");
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
    QStringList scripts = KGlobal::dirs()->findAllResources("data", appName + "/plasma/layout/init/*.js");
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

QScriptValue ScriptEngine::widgets(QScriptContext *context, QScriptEngine *engine)
{
    ScriptEngine *env = ScriptEngine::envFor(engine);

    const QString widgetType = context->argumentCount() > 0 ? context->argument(0).toString() : QString();
    QScriptValue widgets = engine->newArray();
    int count = 0;

    foreach (Plasma::Applet *widget, env->m_containment->applets()) {
        if (widgetType.isEmpty() || widget->pluginName() == widgetType) {
            widgets.setProperty(count, env->wrap(widget));
            ++count;
        }
    }

    widgets.setProperty("length", count);
    return widgets;
}

QScriptValue ScriptEngine::addWidget(QScriptContext *context, QScriptEngine *engine)
{
    if (context->argumentCount() == 0) {
        return context->throwError("widgetById requires a name of a widget or a widget object");
    }
    quint16 row = 0;
    quint16 column = 0;

    ScriptEngine *env = ScriptEngine::envFor(engine);

    if (context->argumentCount() == 3) {
        QScriptValue v = context->argument(1);
        if (v.isNumber()) {
            row = v.toUInt16();
        }
        v = context->argument(2);
        if (v.isNumber()) {
            column = v.toUInt16();
        }
        kDebug() << "Calculated position as" << row << column;
    }

    QScriptValue v = context->argument(0);
    Plasma::Applet *applet = 0;
    if (v.isString()) {
        // FIXME: Using QMetaObject::invokeMethod until the newspaper's API is exported... Fuuuu
        // applet = env->m_containment->addApplet(v.toString(), row, column);
        QMetaObject::invokeMethod(env->m_containment, "addApplet", 
                                  Qt::DirectConnection, 
                                  Q_RETURN_ARG(Plasma::Applet*, applet), 
                                  Q_ARG(QString, v.toString()), 
                                  Q_ARG(int, row), Q_ARG(int, column));
        if (applet) {
            ScriptEngine *env = ScriptEngine::envFor(engine);
            return env->wrap(applet);
        }
    } else if (Widget *widget = qobject_cast<Widget*>(v.toQObject())) {
        applet = widget->applet();

        // FIXME: Using QMetaObject::invokeMethod until the newspaper's API is exported... Fuuuu
        // env->m_containment->addApplet(applet, row, column);
        QMetaObject::invokeMethod(env->m_containment, "addApplet", 
                                  Qt::DirectConnection, 
                                  Q_RETURN_ARG(Plasma::Applet*, applet), 
                                  Q_ARG(QString, v.toString()), 
                                  Q_ARG(int, row), Q_ARG(int, column));
        return v;
    }

    return engine->undefinedValue();
}

QScriptValue ScriptEngine::theme(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(engine)

    if (context->argumentCount() > 0) {
        const QString newTheme = context->argument(0).toString();
        Plasma::Theme::defaultTheme()->setThemeName(newTheme);
    }

    return Plasma::Theme::defaultTheme()->themeName();
}

}

#include "scriptengine.moc"

