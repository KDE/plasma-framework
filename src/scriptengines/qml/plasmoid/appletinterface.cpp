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
#include <QSignalMapper>
#include <QTimer>

#include <kactioncollection.h>
#include <QDebug>
#include <kservice.h>
#include <klocalizedstring.h>
#include <KConfigLoader>

#include <Plasma/Plasma>
#include <Plasma/Applet>
#include <Plasma/Corona>
#include <Plasma/Package>
#include <Plasma/PluginLoader>
#include <Plasma/ContainmentActions>

#include "containmentinterface.h"
#include "wallpaperinterface.h"
#include <kdeclarative/configpropertymap.h>
#include <kdeclarative/qmlobject.h>

Q_DECLARE_METATYPE(AppletInterface *)

AppletInterface::AppletInterface(DeclarativeAppletScript *script, const QVariantList &args, QQuickItem *parent)
    : AppletQuickItem(script->applet(), parent),
      m_actionSignals(0),
      m_configuration(0),
      m_appletScriptEngine(script),
      m_toolTipTextFormat(0),
      m_toolTipItem(0),
      m_args(args),
      m_backgroundHints(Plasma::Types::StandardBackground),
      m_hideOnDeactivate(true),
      m_oldKeyboardShortcut(0),
      m_dummyNativeInterface(0),
      m_positionBeforeRemoval(QPointF(-1, -1))
{
    qmlRegisterType<QAction>();

    connect(this, &AppletInterface::configNeedsSaving,
            applet(), &Plasma::Applet::configNeedsSaving);
    connect(applet(), &Plasma::Applet::immutabilityChanged,
            this, &AppletInterface::immutabilityChanged);
    connect(applet(), &Plasma::Applet::userConfiguringChanged,
            this, &AppletInterface::userConfiguringChanged);

    connect(applet(), &Plasma::Applet::contextualActionsAboutToShow,
            this, &AppletInterface::contextualActionsAboutToShow);

    connect(applet(), &Plasma::Applet::statusChanged,
            this, &AppletInterface::statusChanged);

    connect(applet(), &Plasma::Applet::destroyedChanged,
            this, &AppletInterface::destroyedChanged);

    connect(applet(), &Plasma::Applet::titleChanged,
            this, &AppletInterface::titleChanged);

    connect(applet(), &Plasma::Applet::titleChanged,
            this, [this]() {
                if (m_toolTipMainText.isNull()) {
                    emit toolTipMainTextChanged();
                }
            });

    connect(applet(), &Plasma::Applet::iconChanged,
            this, &AppletInterface::iconChanged);

    connect(applet(), &Plasma::Applet::busyChanged,
            this, &AppletInterface::busyChanged);

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
                this, &AppletInterface::screenChanged);

        // Screen change implies geo change for good measure.
        connect(applet()->containment(), &Plasma::Containment::screenChanged,
                this, &AppletInterface::screenGeometryChanged);

        connect(applet()->containment()->corona(), &Plasma::Corona::screenGeometryChanged, this, [this](int id) {
            if (id == applet()->containment()->screen()) {
                emit screenGeometryChanged();
            }
        });

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
        // in case the applet doesn't want to get shrinked on reactivation,
        // we always expand it again (only in order to conform with legacy behaviour)
        bool activate = !( isExpanded() && isActivationTogglesExpanded() );

        setExpanded(activate);
        if (activate) {
            if (QQuickItem *i = qobject_cast<QQuickItem *>(fullRepresentationItem())) {
                // Bug 372476: never pull focus away from it, only setFocus(true)
                i->setFocus(true, Qt::ShortcutFocusReason);
            }
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

        //search if the current focus item is a child or grandchild of the applet
        while (candidate) {
            if (candidate == this) {
                isAncestor = true;
                break;
            }
            candidate = candidate->parentItem();
        }

        if (isAncestor) {
            //Found? remove focus for the whole hierachy
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
    return applet()->pluginMetaData().isValid() ? applet()->pluginMetaData().pluginId() : QString();
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
    if (m_toolTipSubText.isNull() && applet()->pluginMetaData().isValid()) {
        return applet()->pluginMetaData().description();
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

int AppletInterface::toolTipTextFormat() const
{
    return m_toolTipTextFormat;
}

void AppletInterface::setToolTipTextFormat(int format)
{
    if (m_toolTipTextFormat == format) {
        return;
    }

    m_toolTipTextFormat = format;
    emit toolTipTextFormatChanged();
}

QQuickItem *AppletInterface::toolTipItem() const
{
    return m_toolTipItem.data();
}

void AppletInterface::setToolTipItem(QQuickItem *toolTipItem)
{
    if (m_toolTipItem.data() == toolTipItem) {
        return;
    }

    m_toolTipItem = toolTipItem;
    connect(m_toolTipItem.data(), &QObject::destroyed,
            this, &AppletInterface::toolTipItemChanged);

    emit toolTipItemChanged();
}

bool AppletInterface::isBusy() const
{
    return applet()->isBusy();
}

void AppletInterface::setBusy(bool busy)
{
    applet()->setBusy(busy);
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

void AppletInterface::clearActions()
{
    Q_FOREACH (const QString &action, m_actions) {
        removeAction(action);
    }
}

QAction *AppletInterface::action(QString name) const
{
    return applet()->actions()->action(name);
}

bool AppletInterface::immutable() const
{
    return applet()->immutability() != Plasma::Types::Mutable;
}

Plasma::Types::ImmutabilityType AppletInterface::immutability() const
{
    return applet()->immutability();
}

bool AppletInterface::userConfiguring() const
{
    return applet()->isUserConfiguring();
}

int AppletInterface::apiVersion() const
{
    // Look for C++ plugins first
    auto filter = [](const KPluginMetaData &md) -> bool
    {
        return md.value(QStringLiteral("X-Plasma-API")) == QLatin1String("declarativeappletscript")
            && md.value(QStringLiteral("X-Plasma-ComponentTypes")).contains(QLatin1String("Applet"));
    };
    QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(QStringLiteral("plasma/scriptengines"), filter);
    if (plugins.isEmpty()) {
        return -1;
    }

    return plugins.first().value(QStringLiteral("X-KDE-PluginInfo-Version")).toInt();
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

QObject *AppletInterface::nativeInterface()
{
    if (qstrcmp(applet()->metaObject()->className(),"Plasma::Applet") != 0) {
        return applet();
    } else {
        if (!m_dummyNativeInterface) {
            m_dummyNativeInterface = new QObject(this);
        }
        return m_dummyNativeInterface;
    }
}

bool AppletInterface::configurationRequired() const
{
    return applet()->configurationRequired();
}

void AppletInterface::setConfigurationRequiredProperty(bool needsConfiguring)
{
    appletScript()->setConfigurationRequired(needsConfiguring, applet()->configurationRequiredReason());
}

QString AppletInterface::configurationRequiredReason() const
{
    return applet()->configurationRequiredReason();
}

void AppletInterface::setConfigurationRequiredReason(const QString &reason)
{
    appletScript()->setConfigurationRequired(applet()->configurationRequired(), reason);
}

QString AppletInterface::downloadPath(const QString &file)
{
    Q_UNUSED(file);
    return downloadPath();
}

QString AppletInterface::downloadPath() const
{
    const QString downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/Plasma/" + applet()->pluginMetaData().pluginId() + '/';

    if (!QFile::exists(downloadDir)) {
        QDir dir(QChar('/'));
        dir.mkpath(downloadDir);
    }

    return downloadDir;
}

QStringList AppletInterface::downloadedFiles() const
{
    const QString downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/Plasma/" + applet()->pluginMetaData().pluginId() + '/';
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

QVariantList AppletInterface::availableScreenRegion() const
{
    QVariantList regVal;

    if (!applet()->containment() || !applet()->containment()->corona()) {
        return regVal;
    }

    QRegion reg = QRect(0, 0, width(), height());
    int screenId = screen();
    if (screenId > -1) {
        reg = applet()->containment()->corona()->availableScreenRegion(screenId);
    }

    foreach (QRect rect, reg.rects()) {
        //make it relative
        QRect geometry = applet()->containment()->corona()->screenGeometry(screenId);
        rect.moveTo(rect.topLeft() - geometry.topLeft());
        regVal << QVariant::fromValue(QRectF(rect));
    }
    return regVal;
}

QRect AppletInterface::availableScreenRect() const
{
    if (!applet()->containment() || !applet()->containment()->corona()) {
        return QRect();
    }

    QRect rect(0, 0, width(), height());

    int screenId = screen();

    if (screenId > -1) {
        rect = applet()->containment()->corona()->availableScreenRect(screenId);
        //make it relative
        QRect geometry = applet()->containment()->corona()->screenGeometry(screenId);
        rect.moveTo(rect.topLeft() - geometry.topLeft());
    }

    return rect;
}

bool AppletInterface::event(QEvent *event)
{
    // QAction keyboard shortcuts cannot work with QML2 (and probably newver will
    // since in Qt qtquick and qwidgets cannot depend from each other in any way)
    // so do a simple keyboard shortcut matching here
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        QKeySequence seq(ke->key()|ke->modifiers());

        QList <QAction *> actions = applet()->actions()->actions();
        //find the wallpaper action if we are a containment
        ContainmentInterface *ci = qobject_cast<ContainmentInterface *>(this);
        if (ci) {
            WallpaperInterface *wi = ci->wallpaperInterface();
            if (wi) {
                actions << wi->contextualActions();
            }
        }

        //add any actions of the corona
        if (applet()->containment() && applet()->containment()->corona()) {
            actions << applet()->containment()->corona()->actions()->actions();
        }

        bool keySequenceUsed = false;
        foreach (auto a, actions) {

            if (a->shortcut().isEmpty()) {
                continue;
            }

            //this will happen on a normal, non emacs shortcut
            if (seq.matches(a->shortcut()) == QKeySequence::ExactMatch) {
                event->accept();
                a->trigger();
                m_oldKeyboardShortcut = 0;
                return true;

            //first part of an emacs style shortcut?
            } else if (seq.matches(a->shortcut()) == QKeySequence::PartialMatch) {
                keySequenceUsed = true;
                m_oldKeyboardShortcut = ke->key()|ke->modifiers();

            //no match at all, but it can be the second part of an emacs style shortcut
            } else {
                QKeySequence seq(m_oldKeyboardShortcut, ke->key()|ke->modifiers());

                if (seq.matches(a->shortcut()) == QKeySequence::ExactMatch) {
                    event->accept();
                    a->trigger();

                    return true;
                }
            }
        }

        if (!keySequenceUsed) {
            m_oldKeyboardShortcut = 0;
        }
    }

    return AppletQuickItem::event(event);
}

bool AppletInterface::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *e = static_cast<QMouseEvent *>(event);

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
            if (!ci) {
                return false;
            }

            //the plugin can be a single action or a context menu
            //Don't have an action list? execute as single action
            //and set the event position as action data
            if (plugin->contextualActions().length() == 1) {
                // but first check whether we are not a popup
                // we don't want to randomly creates applets without confirmation
                if (static_cast<QQuickItem *>(watched)->window() != ci->window()) {
                    return true;
                }

                QAction *action = plugin->contextualActions().at(0);
                action->setData(e->globalPos());
                action->trigger();
                return true;
            }

            QMenu *desktopMenu = new QMenu;
            if (desktopMenu->winId()) {
                desktopMenu->windowHandle()->setTransientParent(window());
            }
            emit applet()->contextualActionsAboutToShow();
            ci->addAppletActions(desktopMenu, applet(), event);

            if (!desktopMenu->isEmpty()) {
                desktopMenu->setAttribute(Qt::WA_DeleteOnClose);
                desktopMenu->popup(e->globalPos());
                return true;
            }

            delete desktopMenu;
            return false;
        }
    }

    return AppletQuickItem::eventFilter(watched, event);
}


#include "moc_appletinterface.cpp"
