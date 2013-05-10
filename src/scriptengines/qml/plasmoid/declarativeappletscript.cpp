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

#include <kdeclarative/qmlobject.h>
#include "declarative/packageaccessmanagerfactory.h"
#include <kdeclarative/configpropertymap.h>


K_EXPORT_PLASMA_APPLETSCRIPTENGINE(declarativeappletscript, DeclarativeAppletScript)


DeclarativeAppletScript::DeclarativeAppletScript(QObject *parent, const QVariantList &args)
    : Plasma::AppletScript(parent),
      m_interface(0)
{
    qmlRegisterType<AppletInterface>();
    qmlRegisterType<ConfigPropertyMap>();
    Q_UNUSED(args);
}

DeclarativeAppletScript::~DeclarativeAppletScript()
{
}

bool DeclarativeAppletScript::init()
{
    //FIXME: what replaced this?
    //KGlobal::locale()->insertCatalog("plasma_applet_" % description().pluginName());

    //make possible to import extensions from the package
    //FIXME: probably to be removed, would make possible to use native code from within the package :/
    //m_interface->qmlObject()->engine()->addImportPath(package()->path()+"/contents/imports");

    Plasma::Applet *a = applet();
    Plasma::Containment *pc = qobject_cast<Plasma::Containment *>(a);

    if (pc) {
        m_interface = new ContainmentInterface(this);

    //fail? so it's a normal Applet
    } else {
        m_interface = new AppletInterface(this);
    }

    m_interface->setParent(this);
    // set the graphicObject dynamic property on applet
    a->setProperty("graphicObject", QVariant::fromValue(m_interface));

    connect(applet(), SIGNAL(activate()),
            this, SLOT(activate()));

    return true;
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
    if (constraints & Plasma::Types::FormFactorConstraint) {
        emit formFactorChanged();
    }

    if (constraints & Plasma::Types::LocationConstraint) {
        emit locationChanged();
    }

    if (constraints & Plasma::Types::ContextConstraint) {
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
    if (m_interface->qmlObject()->rootObject()) {
         QMetaObject::invokeMethod(m_interface->qmlObject()->rootObject(), QString("action_" + name).toLatin1(), Qt::DirectConnection);
    }
}

bool DeclarativeAppletScript::include(const QString &path)
{
    /*TODO: probably include() doesn't make any sense anymore
    return m_env->include(path);
    */
    return false;
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


#include "declarativeappletscript.moc"

