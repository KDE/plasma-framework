/*
 *   Copyright 2008-2013 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2010-2013 Marco Martin <mart@kde.org>
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

#include "appletinterface.h"

#include <QAction>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlExpression>
#include <QQmlProperty>
#include <QQmlComponent>
#include <QSignalMapper>
#include <QTimer>

#include <kactioncollection.h>
#include <QDebug>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <klocalizedstring.h>
#include <KConfigLoader>

#include <Plasma/Plasma>
#include <Plasma/Applet>
#include <Plasma/Corona>
#include <Plasma/Package>
#include <Plasma/PluginLoader>
#include <Plasma/ContainmentActions>

#include "containmentinterface.h"
#include <kdeclarative/configpropertymap.h>
#include <kdeclarative/qmlobject.h>

Q_DECLARE_METATYPE(AppletInterface *)

AppletInterface::AppletInterface(DeclarativeAppletScript *script, const QVariantList &args, QQuickItem *parent)
    : AppletQuickItem(script->applet(), parent),
      m_args(args),
      m_actionSignals(0),
      m_appletScriptEngine(script),
      m_backgroundHints(Plasma::Types::StandardBackground),
      m_busy(false),
      m_hideOnDeactivate(true),
      m_positionBeforeRemoval(QPointF(-1, -1))
{
    qmlRegisterType<QAction>();

    connect(this, &AppletInterface::configNeedsSaving,
            applet(), &Plasma::Applet::configNeedsSaving);
    connect(applet(), &Plasma::Applet::immutabilityChanged,
            this, &AppletInterface::immutableChanged);
    connect(applet(), &Plasma::Applet::userConfiguringChanged,
            this, &AppletInterface::userConfiguringChanged);

    connect(applet(), &Plasma::Applet::statusChanged,
            this, &AppletInterface::statusChanged);

    connect(applet(), &Plasma::Applet::destroyedChanged,
            this, &AppletInterface::destroyedChanged);

    connect(applet(), &Plasma::Applet::activated,
            this, &AppletInterface::activated);

    connect(appletScript(), &DeclarativeAppletScript::formFactorChanged,
            this, &AppletInterface::formFactorChanged);
    connect(appletScript(), &DeclarativeAppletScript::locationChanged,
            this, &AppletInterface::locationChanged);
    connect(appletScript(), &DeclarativeAppletScript::contextChanged,
            this, &AppletInterface::contextChanged);

    if (applet()->containment()) {
        connect(applet()->containment(), &Plasma::Containment::screenChanged,
                this, &ContainmentInterface::screenChanged);
    }

    connect(this, &AppletInterface::expandedChanged, [=](bool expanded) {
        //if both compactRepresentationItem and fullRepresentationItem exist,
        //the applet is in a popup
        if (expanded) {
            if (compactRepresentationItem() && fullRepresentationItem() &&
                fullRepresentationItem()->window() && compactRepresentationItem()->window() &&
                fullRepresentationItem()->window() != compactRepresentationItem()->window() &&
                fullRepresentationItem()->parentItem()) {
                fullRepresentationItem()->parentItem()->installEventFilter(this);
            } else if (fullRepresentationItem() && fullRepresentationItem()->parentItem()) {
                fullRepresentationItem()->parentItem()->removeEventFilter(this);
            }
        }
    });
}

AppletInterface::AppletInterface(Plasma::Applet *a, const QVariantList &args, QQuickItem *parent)
    : AppletQuickItem(a, parent),
      m_args(args),
      m_actionSignals(0),
      m_appletScriptEngine(0),
      m_backgroundHints(Plasma::Types::StandardBackground),
      m_busy(false),
      m_hideOnDeactivate(true)
{
    qmlRegisterType<QAction>();

    connect(this, &AppletInterface::configNeedsSaving,
            applet(), &Plasma::Applet::configNeedsSaving);
    connect(applet(), &Plasma::Applet::immutabilityChanged,
            this, &AppletInterface::immutableChanged);
    connect(applet(), &Plasma::Applet::userConfiguringChanged,
            this, &AppletInterface::userConfiguringChanged);

    connect(applet(), &Plasma::Applet::statusChanged,
            this, &AppletInterface::statusChanged);

    connect(applet(), &Plasma::Applet::destroyedChanged,
            this, &AppletInterface::destroyedChanged);

    connect(appletScript(), &DeclarativeAppletScript::formFactorChanged,
            this, &AppletInterface::formFactorChanged);
    connect(appletScript(), &DeclarativeAppletScript::locationChanged,
            this, &AppletInterface::locationChanged);
    connect(appletScript(), &DeclarativeAppletScript::contextChanged,
            this, &AppletInterface::contextChanged);

    if (applet()->containment()) {
        connect(applet()->containment(), &Plasma::Containment::screenChanged,
                this, &ContainmentInterface::screenChanged);
    }
}

AppletInterface::~AppletInterface()
{
}

DeclarativeAppletScript *AppletInterface::appletScript() const
{
    return m_appletScriptEngine;
}

void AppletInterface::init()
{
    if (qmlObject()->rootObject() && m_configuration) {
        return;
    }

    m_configuration = new KDeclarative::ConfigPropertyMap(applet()->configScheme(), this);

    AppletQuickItem::init();

    geometryChanged(QRectF(), QRectF(x(), y(), width(), height()));
    emit busyChanged();

    applet()->updateConstraints(Plasma::Types::UiReadyConstraint);
    connect(applet(), &Plasma::Applet::activated,
    [ = ]() {
        setExpanded(true);
        if (QQuickItem *i = qobject_cast<QQuickItem *>(fullRepresentationItem())) {
            i->setFocus(true, Qt::ShortcutFocusReason);
        }
    });

    if (m_args.count() == 1) {
        emit externalData(QString(), m_args.first());
    } else if (m_args.count() > 0) {
        emit externalData(QString(), m_args);
    }
}

void AppletInterface::destroyedChanged(bool destroyed)
{
    //if an item loses its scene before losing the focus, will never
    //be able to gain focus again
    if (destroyed && window() && window()->activeFocusItem()) {
        QQuickItem *focus = window()->activeFocusItem();
        QQuickItem *candidate = focus;
        bool isAncestor = false;

        //search if the current focus item is a child or granchild of the applet
        while (candidate) {
            if (candidate == this) {
                isAncestor = true;
                break;
            }
            candidate = candidate->parentItem();
        }

        if (isAncestor) {
            //Found? remove focus for the whole hyerarchy
            candidate = focus;

            while (candidate && candidate != this) {
                candidate->setFocus(false);
                candidate = candidate->parentItem();
            }
        }
    }

    setVisible(!destroyed);
}

Plasma::Types::FormFactor AppletInterface::formFactor() const
{
    return applet()->formFactor();
}

Plasma::Types::Location AppletInterface::location() const
{
    return applet()->location();
}

QString AppletInterface::currentActivity() const
{
    if (applet()->containment()) {
        return applet()->containment()->activity();
    } else {
        return QString();
    }
}

QObject *AppletInterface::configuration() const
{
    return m_configuration;
}

uint AppletInterface::id() const
{
    return applet()->id();
}

QString AppletInterface::pluginName() const
{
    return applet()->pluginInfo().isValid() ? applet()->pluginInfo().pluginName() : QString();
}

QString AppletInterface::icon() const
{
    return applet()->icon();
}

void AppletInterface::setIcon(const QString &icon)
{
    if (applet()->icon() == icon) {
        return;
    }

    applet()->setIcon(icon);
    emit iconChanged();
}

QString AppletInterface::title() const
{
    return applet()->title();
}

void AppletInterface::setTitle(const QString &title)
{
    if (applet()->title() == title) {
        return;
    }

    applet()->setTitle(title);
    emit titleChanged();
}

QString AppletInterface::toolTipMainText() const
{
    if (m_toolTipMainText.isNull()) {
        return title();
    } else {
        return m_toolTipMainText;
    }
}

void AppletInterface::setToolTipMainText(const QString &text)
{
    //Here we are abusing the differentce between a null and and empty string.
    //by default is null so fallsback to the name
    //the fist time it gets set, an empty non null one is set, and won't fallback anymore
    if (!m_toolTipMainText.isNull() && m_toolTipMainText == text) {
        return;
    }

    if (text.isEmpty()) {
        m_toolTipMainText = QStringLiteral("");//this "" makes it non-null
    } else {
        m_toolTipMainText = text;
    }

    emit toolTipMainTextChanged();
}

QString AppletInterface::toolTipSubText() const
{
    if (m_toolTipSubText.isNull() && applet()->pluginInfo().isValid()) {
        return applet()->pluginInfo().comment();
    } else {
        return m_toolTipSubText;
    }
}

void AppletInterface::setToolTipSubText(const QString &text)
{
    //Also there the difference between null and empty gets exploited
    if (!m_toolTipSubText.isNull() && m_toolTipSubText == text) {
        return;
    }

    if (text.isEmpty()) {
        m_toolTipSubText = QStringLiteral("");//this "" makes it non-null
    } else {
        m_toolTipSubText = text;
    }

    emit toolTipSubTextChanged();
}

bool AppletInterface::isBusy() const
{
    return m_busy;
}

void AppletInterface::setBusy(bool busy)
{
    if (m_busy == busy) {
        return;
    }

    m_busy = busy;
    emit busyChanged();
}

Plasma::Types::BackgroundHints AppletInterface::backgroundHints() const
{
    return m_backgroundHints;
}

void AppletInterface::setBackgroundHints(Plasma::Types::BackgroundHints hint)
{
    if (m_backgroundHints == hint) {
        return;
    }

    m_backgroundHints = hint;
    emit backgroundHintsChanged();
}

void AppletInterface::setConfigurationRequired(bool needsConfiguring, const QString &reason)
{
    appletScript()->setConfigurationRequired(needsConfiguring, reason);
}

QString AppletInterface::file(const QString &fileType)
{
    return appletScript()->filePath(fileType, QString());
}

QString AppletInterface::file(const QString &fileType, const QString &filePath)
{
    return appletScript()->filePath(fileType, filePath);
}

QList<QAction *> AppletInterface::contextualActions() const
{
    QList<QAction *> actions;
    Plasma::Applet *a = applet();
    if (a->failedToLaunch()) {
        return actions;
    }

    foreach (const QString &name, m_actions) {
        QAction *action = a->actions()->action(name);

        if (action) {
            actions << action;
        }
    }

    return actions;
}

void AppletInterface::setActionSeparator(const QString &name)
{
    Plasma::Applet *a = applet();
    QAction *action = a->actions()->action(name);

    if (action) {
        action->setSeparator(true);
    } else {
        action = new QAction(this);
        action->setSeparator(true);
        a->actions()->addAction(name, action);
        m_actions.append(name);
    }
}

void AppletInterface::setAction(const QString &name, const QString &text, const QString &icon, const QString &shortcut)
{
    Plasma::Applet *a = applet();
    QAction *action = a->actions()->action(name);

    if (action) {
        action->setText(text);
    } else {
        action = new QAction(text, this);
        a->actions()->addAction(name, action);

        Q_ASSERT(!m_actions.contains(name));
        m_actions.append(name);

        if (!m_actionSignals) {
            m_actionSignals = new QSignalMapper(this);
            connect(m_actionSignals, SIGNAL(mapped(QString)),
                    appletScript(), SLOT(executeAction(QString)));
        }

        connect(action, SIGNAL(triggered()), m_actionSignals, SLOT(map()));
        m_actionSignals->setMapping(action, name);
    }

    if (!icon.isEmpty()) {
        action->setIcon(QIcon::fromTheme(icon));
    }

    if (!shortcut.isEmpty()) {
        action->setShortcut(shortcut);
    }

    action->setObjectName(name);
}

void AppletInterface::removeAction(const QString &name)
{
    Plasma::Applet *a = applet();
    QAction *action = a->actions()->action(name);

    if (action) {
        if (m_actionSignals) {
            m_actionSignals->removeMappings(action);
        }

        delete action;
    }

    m_actions.removeAll(name);
}

QAction *AppletInterface::action(QString name) const
{
    return applet()->actions()->action(name);
}

bool AppletInterface::immutable() const
{
    return applet()->immutability() != Plasma::Types::Mutable;
}

bool AppletInterface::userConfiguring() const
{
    return applet()->isUserConfiguring();
}

int AppletInterface::apiVersion() const
{
    const QString constraint("[X-Plasma-API] == 'declarative' and 'Applet' in [X-Plasma-ComponentTypes]");
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/ScriptEngine", constraint);
    if (offers.isEmpty()) {
        return -1;
    }

    return offers.first()->property("X-KDE-PluginInfo-Version", QVariant::Int).toInt();
}

void AppletInterface::setAssociatedApplication(const QString &string)
{
    if (applet()->associatedApplication() == string) {
        return;
    }

    applet()->setAssociatedApplication(string);
    emit associatedApplicationChanged();
}

QString AppletInterface::associatedApplication() const
{
    return applet()->associatedApplication();
}

void AppletInterface::setAssociatedApplicationUrls(const QList<QUrl> &urls)
{
    if (applet()->associatedApplicationUrls() == urls) {
        return;
    }

    applet()->setAssociatedApplicationUrls(urls);
    emit associatedApplicationUrlsChanged();
}

QList<QUrl> AppletInterface::associatedApplicationUrls() const
{
    return applet()->associatedApplicationUrls();
}

void AppletInterface::setStatus(const Plasma::Types::ItemStatus &status)
{
    applet()->setStatus(status);
}

Plasma::Types::ItemStatus AppletInterface::status() const
{
    return applet()->status();
}

int AppletInterface::screen() const
{
    if (Plasma::Containment* c = applet()->containment()) {
        return c->screen();
    }

    return -1;
}

QRect AppletInterface::screenGeometry() const
{
    if (!applet() || !applet()->containment()) {
        return QRect();
    }
    return applet()->containment()->corona()->screenGeometry(applet()->containment()->screen());
}

void AppletInterface::setHideOnWindowDeactivate(bool hide)
{
    if (m_hideOnDeactivate != hide) {
        m_hideOnDeactivate = hide;
        emit hideOnWindowDeactivateChanged();
    }
}

bool AppletInterface::hideOnWindowDeactivate() const
{
    return m_hideOnDeactivate;
}

QKeySequence AppletInterface::globalShortcut() const
{
    return applet()->globalShortcut();
}

void AppletInterface::setGlobalShortcut(const QKeySequence &sequence)
{
    applet()->setGlobalShortcut(sequence);
}

QString AppletInterface::downloadPath(const QString &file)
{
    const QString downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/Plasma/" + applet()->pluginInfo().pluginName() + '/';

    if (!QFile::exists(downloadDir)) {
        QDir dir(QChar('/'));
        dir.mkpath(downloadDir);
    }

    return downloadDir;
}

QStringList AppletInterface::downloadedFiles() const
{
    const QString downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/Plasma/" + applet()->pluginInfo().pluginName() + '/';
    QDir dir(downloadDir);
    return dir.entryList(QDir::Files | QDir::NoSymLinks | QDir::Readable);
}

void AppletInterface::executeAction(const QString &name)
{
    if (qmlObject()->rootObject()) {
        const QMetaObject *metaObj = qmlObject()->rootObject()->metaObject();
        QString actionMethodName = QString("action_" + name);
        if (metaObj->indexOfMethod(QMetaObject::normalizedSignature((actionMethodName + "()").toLatin1())) != -1) {
            QMetaObject::invokeMethod(qmlObject()->rootObject(), actionMethodName.toLatin1(), Qt::DirectConnection);
        } else {
            QMetaObject::invokeMethod(qmlObject()->rootObject(), "actionTriggered", Qt::DirectConnection, Q_ARG(QVariant, name));
        }
    }
}

bool AppletInterface::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        //pass it up to the applet
        //well, actually we have to pass it to the *containment*
        //because all the code for showing an applet's contextmenu is actually in Containment.
        Plasma::Containment *c = applet()->containment();
        if (c) {
            const QString trigger = Plasma::ContainmentActions::eventToString(event);
            Plasma::ContainmentActions *plugin = c->containmentActions().value(trigger);
            if (!plugin) {
                return false;
            }
            ContainmentInterface *ci = c->property("_plasma_graphicObject").value<ContainmentInterface *>();

            QMenu desktopMenu;
            ci->addAppletActions(desktopMenu, applet(), event);

            if (!desktopMenu.isEmpty()) {
                desktopMenu.exec(static_cast<QMouseEvent*>(event)->globalPos());
                return true;
            }

            return false;
        }
    }

    return AppletQuickItem::eventFilter(watched, event);
}


#include "moc_appletinterface.cpp"
