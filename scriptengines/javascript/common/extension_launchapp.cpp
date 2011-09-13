/*
 *   Copyright 2011 Aaron Seigo <aseigo@kde.org>
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

#include "scriptenv.h"

#include <KEMailSettings>
#include <KMimeTypeTrader>
#include <KStandardDirs>
#include <KRun>
#include <KServiceTypeTrader>
#include <KShell>

QScriptValue ScriptEnv::runApplication(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(engine)
    if (context->argumentCount() == 0) {
        return false;
    }

    KUrl::List urls;
    if (context->argumentCount() > 1) {
        urls = qscriptvalue_cast<KUrl::List>(context->argument(1));
    }

    const QString app = context->argument(0).toString();

    const QString exec = KGlobal::dirs()->findExe(app);
    if (!exec.isEmpty()) {
        return KRun::run(exec, urls, 0);
    }

    KService::Ptr service = KService::serviceByStorageId(app);
    if (service) {
        return KRun::run(*service, urls, 0);
    }

    return false;
}

QScriptValue ScriptEnv::runCommand(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(engine);

    if (context->argumentCount() == 0) {
        return false;
    }

    const QString exec = KGlobal::dirs()->findExe(context->argument(0).toString());
    if (!exec.isEmpty()) {
        QString args;
        if (context->argumentCount() > 1) {
            const QStringList argList = qscriptvalue_cast<QStringList>(context->argument(1));
            if (!argList.isEmpty()) {
                args = ' ' + KShell::joinArgs(argList);
            }
        }

        return KRun::runCommand(exec + args, 0);
    }

    return false;
}

QScriptValue ScriptEnv::applicationExists(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(engine)
    if (context->argumentCount() == 0) {
        return false;
    }

    const QString application = context->argument(0).toString();
    if (application.isEmpty()) {
        return false;
    }

    // first, check for it in $PATH
    if (!KStandardDirs::findExe(application).isEmpty()) {
        return true;
    }

    if (KService::serviceByStorageId(application)) {
        return true;
    }

    if (application.contains("'")) {
        // apostrophes just screw up the trader lookups below, so check for it
        return false;
    }

    // next, consult ksycoca for an app by that name
    if (!KServiceTypeTrader::self()->query("Application", QString("Name =~ '%1'").arg(application)).isEmpty()) {
        return true;
    }

    // next, consult ksycoca for an app by that generic name
    if (!KServiceTypeTrader::self()->query("Application", QString("GenericName =~ '%1'").arg(application)).isEmpty()) {
        return true;
    }

    return false;
}

QScriptValue ScriptEnv::defaultApplication(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(engine)
    if (context->argumentCount() == 0) {
        return false;
    }

    const QString application = context->argument(0).toString();
    if (application.isEmpty()) {
        return false;
    }

    const bool storageId = context->argumentCount() < 2 ? false : context->argument(1).toBool();

    // FIXME: there are some pretty horrible hacks below, in the sense that they assume a very
    // specific implementation system. there is much room for improvement here. see
    // kdebase-runtime/kcontrol/componentchooser/ for all the gory details ;)
    if (application.compare("mailer", Qt::CaseInsensitive) == 0) {
        KEMailSettings settings;

        // in KToolInvocation, the default is kmail; but let's be friendlier :)
        QString command = settings.getSetting(KEMailSettings::ClientProgram);
        if (command.isEmpty()) {
            if (KService::Ptr kontact = KService::serviceByStorageId("kontact")) {
                return storageId ? kontact->storageId() : kontact->exec();
            } else if (KService::Ptr kmail = KService::serviceByStorageId("kmail")) {
                return storageId ? kmail->storageId() : kmail->exec();
            }
        }

        if (!command.isEmpty()) {
            if (settings.getSetting(KEMailSettings::ClientTerminal) == "true") {
                KConfigGroup confGroup(KGlobal::config(), "General");
                const QString preferredTerminal = confGroup.readPathEntry("TerminalApplication",
                        QString::fromLatin1("konsole"));
                command = preferredTerminal + QString::fromLatin1(" -e ") + command;
            }

            return command;
        }
    } else if (application.compare("browser", Qt::CaseInsensitive) == 0) {
        KConfigGroup config(KGlobal::config(), "General");
        QString browserApp = config.readPathEntry("BrowserApplication", QString());
        if (browserApp.isEmpty()) {
            const KService::Ptr htmlApp = KMimeTypeTrader::self()->preferredService(QLatin1String("text/html"));
            if (htmlApp) {
                browserApp = storageId ? htmlApp->storageId() : htmlApp->exec();
            }
        } else if (browserApp.startsWith('!')) {
            browserApp = browserApp.mid(1);
        }

        return browserApp;
    } else if (application.compare("terminal", Qt::CaseInsensitive) == 0) {
        KConfigGroup confGroup(KGlobal::config(), "General");
        return confGroup.readPathEntry("TerminalApplication", QString::fromLatin1("konsole"));
    } else if (application.compare("filemanager", Qt::CaseInsensitive) == 0) {
        KService::Ptr service = KMimeTypeTrader::self()->preferredService("inode/directory");
        if (service) {
            return storageId ? service->storageId() : service->exec();
        }
    } else if (application.compare("windowmanager", Qt::CaseInsensitive) == 0) {
        KConfig cfg("ksmserverrc", KConfig::NoGlobals);
        KConfigGroup confGroup(&cfg, "General");
        return confGroup.readEntry("windowManager", QString::fromLatin1("konsole"));
    } else if (KService::Ptr service = KMimeTypeTrader::self()->preferredService(application)) {
        return storageId ? service->storageId() : service->exec();
    } else {
        // try the files in share/apps/kcm_componentchooser/
        const QStringList services = KGlobal::dirs()->findAllResources("data","kcm_componentchooser/*.desktop", KStandardDirs::NoDuplicates);
        //kDebug() << "ok, trying in" << services.count();
        foreach (const QString &service, services) {
            KConfig config(service, KConfig::SimpleConfig);
            KConfigGroup cg = config.group(QByteArray());
            const QString type = cg.readEntry("valueName", QString());
            //kDebug() << "    checking" << service << type << application;
            if (type.compare(application, Qt::CaseInsensitive) == 0) {
                KConfig store(cg.readPathEntry("storeInFile", "null"));
                KConfigGroup storeCg(&store, cg.readEntry("valueSection", QString()));
                const QString exec = storeCg.readPathEntry(cg.readEntry("valueName", "kcm_componenchooser_null"),
                                                           cg.readEntry("defaultImplementation", QString()));
                if (!exec.isEmpty()) {
                    return exec;
                }

                break;
            }
        }
    }

    return false;
}

QScriptValue ScriptEnv::applicationPath(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(engine)
    if (context->argumentCount() == 0) {
        return false;
    }

    const QString application = context->argument(0).toString();
    if (application.isEmpty()) {
        return false;
    }

    // first, check for it in $PATH
    const QString path = KStandardDirs::findExe(application);
    if (!path.isEmpty()) {
        return path;
    }

    if (KService::Ptr service = KService::serviceByStorageId(application)) {
        return KStandardDirs::locate("apps", service->entryPath());
    }

    if (application.contains("'")) {
        // apostrophes just screw up the trader lookups below, so check for it
        return QString();
    }

    // next, consult ksycoca for an app by that name
    KService::List offers = KServiceTypeTrader::self()->query("Application", QString("Name =~ '%1'").arg(application));
    if (offers.isEmpty()) {
        // next, consult ksycoca for an app by that generic name
        offers = KServiceTypeTrader::self()->query("Application", QString("GenericName =~ '%1'").arg(application));
    }

    if (!offers.isEmpty()) {
        KService::Ptr offer = offers.first();
        return KStandardDirs::locate("apps", offer->entryPath());
    }

    return QString();
}

