/*
    SPDX-FileCopyrightText: 2008-2013 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2010-2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "appletinterface.h"

#include <QAction>
#include <QActionGroup>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QTimer>

#include <KActionCollection>
#include <KConfigLoader>
#include <KLocalizedString>
#include <KService>
#include <QDebug>

#include <Plasma/ContainmentActions>
#include <Plasma/Corona>
#include <Plasma/Package>
#include <Plasma/Plasma>
#include <Plasma/PluginLoader>

#include "containmentinterface.h"
#include "wallpaperinterface.h"
#include <kdeclarative/configpropertymap.h>
#include <kdeclarative/qmlobject.h>

AppletInterface::AppletInterface(DeclarativeAppletScript *script, const QVariantList &args, QQuickItem *parent)
    : AppletQuickItem(script->applet(), parent)
    , m_configuration(nullptr)
    , m_appletScriptEngine(script)
    , m_toolTipTextFormat(0)
    , m_toolTipItem(nullptr)
    , m_args(args)
    , m_hideOnDeactivate(true)
    , m_oldKeyboardShortcut(0)
    , m_dummyNativeInterface(nullptr)
    , m_positionBeforeRemoval(QPointF(-1, -1))
{
    qmlRegisterAnonymousType<QAction>("org.kde.plasma.plasmoid", 1);

    connect(this, &AppletInterface::configNeedsSaving, applet(), &Plasma::Applet::configNeedsSaving);
    connect(applet(), &Plasma::Applet::immutabilityChanged, this, &AppletInterface::immutabilityChanged);
    connect(applet(), &Plasma::Applet::userConfiguringChanged, this, &AppletInterface::userConfiguringChanged);

    connect(applet(), &Plasma::Applet::contextualActionsAboutToShow, this, &AppletInterface::contextualActionsAboutToShow);

    connect(applet(), &Plasma::Applet::statusChanged, this, &AppletInterface::statusChanged);

    connect(applet(), &Plasma::Applet::destroyedChanged, this, &AppletInterface::destroyedChanged);

    connect(applet(), &Plasma::Applet::titleChanged, this, &AppletInterface::titleChanged);

    connect(applet(), &Plasma::Applet::titleChanged, this, [this]() {
        if (m_toolTipMainText.isNull()) {
            Q_EMIT toolTipMainTextChanged();
        }
    });

    connect(applet(), &Plasma::Applet::iconChanged, this, &AppletInterface::iconChanged);

    connect(applet(), &Plasma::Applet::busyChanged, this, &AppletInterface::busyChanged);

    connect(applet(), &Plasma::Applet::backgroundHintsChanged, this, &AppletInterface::backgroundHintsChanged);
    connect(applet(), &Plasma::Applet::effectiveBackgroundHintsChanged, this, &AppletInterface::effectiveBackgroundHintsChanged);
    connect(applet(), &Plasma::Applet::userBackgroundHintsChanged, this, &AppletInterface::userBackgroundHintsChanged);

    connect(applet(), &Plasma::Applet::configurationRequiredChanged, this, [this](bool configurationRequired, const QString &reason) {
        Q_UNUSED(configurationRequired);
        Q_UNUSED(reason);
        Q_EMIT configurationRequiredChanged();
        Q_EMIT configurationRequiredReasonChanged();
    });

    connect(applet(), &Plasma::Applet::activated, this, &AppletInterface::activated);
    connect(applet(), &Plasma::Applet::containmentDisplayHintsChanged, this, &AppletInterface::containmentDisplayHintsChanged);

    connect(appletScript(), &DeclarativeAppletScript::formFactorChanged, this, &AppletInterface::formFactorChanged);
    connect(appletScript(), &DeclarativeAppletScript::locationChanged, this, &AppletInterface::locationChanged);
    connect(appletScript(), &DeclarativeAppletScript::contextChanged, this, &AppletInterface::contextChanged);

    if (applet()->containment()) {
        connect(applet()->containment(), &Plasma::Containment::screenChanged, this, &AppletInterface::screenChanged);

        // Screen change implies geo change for good measure.
        connect(applet()->containment(), &Plasma::Containment::screenChanged, this, &AppletInterface::screenGeometryChanged);

        connect(applet()->containment()->corona(), &Plasma::Corona::screenGeometryChanged, this, [this](int id) {
            if (id == applet()->containment()->screen()) {
                Q_EMIT screenGeometryChanged();
            }
        });

        connect(applet()->containment()->corona(), &Plasma::Corona::availableScreenRegionChanged, this, &ContainmentInterface::availableScreenRegionChanged);
        connect(applet()->containment()->corona(), &Plasma::Corona::availableScreenRectChanged, this, &ContainmentInterface::availableScreenRectChanged);
    }

    connect(this, &AppletInterface::expandedChanged, [=](bool expanded) {
        // if both compactRepresentationItem and fullRepresentationItem exist,
        // the applet is in a popup
        if (expanded) {
            /* clang-format off */
            if (compactRepresentationItem()
                && fullRepresentationItem()
                && fullRepresentationItem()->window()
                && compactRepresentationItem()->window()
                && fullRepresentationItem()->window() != compactRepresentationItem()->window()
                && fullRepresentationItem()->parentItem()) {
                /* clang-format on */
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
    Q_EMIT busyChanged();

    updateUiReadyConstraint();

    connect(this, &AppletInterface::isLoadingChanged, this, &AppletInterface::updateUiReadyConstraint);

    connect(applet(), &Plasma::Applet::activated, this, [=]() {
        // in case the applet doesn't want to get shrunk on reactivation,
        // we always expand it again (only in order to conform with legacy behaviour)
        bool activate = !(isExpanded() && isActivationTogglesExpanded());

        setExpanded(activate);
        if (activate) {
            if (QQuickItem *i = qobject_cast<QQuickItem *>(fullRepresentationItem())) {
                // Bug 372476: never pull focus away from it, only setFocus(true)
                i->setFocus(true, Qt::ShortcutFocusReason);
            }
        }
    });

    if (m_args.count() == 1) {
        Q_EMIT externalData(QString(), m_args.first());
    } else if (!m_args.isEmpty()) {
        Q_EMIT externalData(QString(), m_args);
    }
}

void AppletInterface::destroyedChanged(bool destroyed)
{
    // if an item loses its scene before losing the focus, will never
    // be able to gain focus again
    if (destroyed && window() && window()->activeFocusItem()) {
        QQuickItem *focus = window()->activeFocusItem();
        QQuickItem *candidate = focus;
        bool isAncestor = false;

        // search if the current focus item is a child or grandchild of the applet
        while (candidate) {
            if (candidate == this) {
                isAncestor = true;
                break;
            }
            candidate = candidate->parentItem();
        }

        if (isAncestor) {
            // Found? remove focus for the whole hierarchy
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

Plasma::Types::ContainmentDisplayHints AppletInterface::containmentDisplayHints() const
{
    return applet()->containmentDisplayHints();
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
    // Here we are abusing the difference between a null and an empty string.
    // by default is null so falls back to the name
    // the fist time it gets set, an empty non null one is set, and won't fallback anymore
    if (!m_toolTipMainText.isNull() && m_toolTipMainText == text) {
        return;
    }

    if (text.isEmpty()) {
        m_toolTipMainText = QStringLiteral(""); // this "" makes it non-null
    } else {
        m_toolTipMainText = text;
    }

    Q_EMIT toolTipMainTextChanged();
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
    // Also there the difference between null and empty gets exploited
    if (!m_toolTipSubText.isNull() && m_toolTipSubText == text) {
        return;
    }

    if (text.isEmpty()) {
        m_toolTipSubText = QStringLiteral(""); // this "" makes it non-null
    } else {
        m_toolTipSubText = text;
    }

    Q_EMIT toolTipSubTextChanged();
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
    Q_EMIT toolTipTextFormatChanged();
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
    connect(m_toolTipItem.data(), &QObject::destroyed, this, &AppletInterface::toolTipItemChanged);

    Q_EMIT toolTipItemChanged();
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
    return applet()->backgroundHints();
}

void AppletInterface::setBackgroundHints(Plasma::Types::BackgroundHints hint)
{
    applet()->setBackgroundHints(hint);
}

Plasma::Types::BackgroundHints AppletInterface::effectiveBackgroundHints() const
{
    return applet()->effectiveBackgroundHints();
}

Plasma::Types::BackgroundHints AppletInterface::userBackgroundHints() const
{
    return applet()->userBackgroundHints();
}

void AppletInterface::setUserBackgroundHints(Plasma::Types::BackgroundHints hint)
{
    applet()->setUserBackgroundHints(hint);
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

QList<QObject *> AppletInterface::contextualActionsObjects() const
{
    QList<QObject *> actions;
    Plasma::Applet *a = applet();
    if (a->failedToLaunch()) {
        return actions;
    }

    for (const QString &name : std::as_const(m_actions)) {
        QAction *action = a->actions()->action(name);

        if (action) {
            actions << action;
        }
    }

    return actions;
}

QList<QAction *> AppletInterface::contextualActions() const
{
    QList<QAction *> actions;
    Plasma::Applet *a = applet();
    if (a->failedToLaunch()) {
        return actions;
    }

    for (const QString &name : std::as_const(m_actions)) {
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
        Q_EMIT contextualActionsChanged();
    }
}

void AppletInterface::setActionGroup(const QString &actionName, const QString &group)
{
    Plasma::Applet *a = applet();
    QAction *action = a->actions()->action(actionName);

    if (!action) {
        return;
    }

    if (!m_actionGroups.contains(group)) {
        m_actionGroups[group] = new QActionGroup(this);
    }

    action->setActionGroup(m_actionGroups[group]);
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
        Q_EMIT contextualActionsChanged();

        connect(action, &QAction::triggered, this, [this, name] {
            executeAction(name);
        });
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
    delete action;
    m_actions.removeAll(name);
}

void AppletInterface::clearActions()
{
    const auto oldActionsList = m_actions;
    for (const QString &action : oldActionsList) {
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
    auto filter = [](const KPluginMetaData &md) -> bool {
        return md.value(QStringLiteral("X-Plasma-API")) == QLatin1String("declarativeappletscript")
            && md.value(QStringLiteral("X-Plasma-ComponentTypes")).contains(QLatin1String("Applet"));
    };
    QVector<KPluginMetaData> plugins = KPluginMetaData::findPlugins(QStringLiteral("plasma/scriptengines"), filter);
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
    Q_EMIT associatedApplicationChanged();
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
    Q_EMIT associatedApplicationUrlsChanged();
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
    if (Plasma::Containment *c = applet()->containment()) {
        return c->screen();
    }

    return -1;
}

QRect AppletInterface::screenGeometry() const
{
    if (!applet() || !applet()->containment() || !applet()->containment()->corona()) {
        return QRect();
    }
    return applet()->containment()->corona()->screenGeometry(applet()->containment()->screen());
}

void AppletInterface::setHideOnWindowDeactivate(bool hide)
{
    if (m_hideOnDeactivate != hide) {
        m_hideOnDeactivate = hide;
        Q_EMIT hideOnWindowDeactivateChanged();
    }
}

bool AppletInterface::hideOnWindowDeactivate() const
{
    return m_hideOnDeactivate;
}

void AppletInterface::setConstraintHints(Plasma::Types::ConstraintHints hints)
{
    if (m_constraintHints == hints) {
        return;
    }

    m_constraintHints = hints;
    Q_EMIT constraintHintsChanged();
}

Plasma::Types::ConstraintHints AppletInterface::constraintHints() const
{
    return m_constraintHints;
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
    if (qstrcmp(applet()->metaObject()->className(), "Plasma::Applet") != 0) {
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
    const QString downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + QStringLiteral("/Plasma/")
        + applet()->pluginMetaData().pluginId() + QLatin1Char('/');

    if (!QFile::exists(downloadDir)) {
        QDir dir({QLatin1Char('/')});
        dir.mkpath(downloadDir);
    }

    return downloadDir;
}

QStringList AppletInterface::downloadedFiles() const
{
    const QString downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + QStringLiteral("/Plasma/")
        + applet()->pluginMetaData().pluginId() + QLatin1Char('/');
    QDir dir(downloadDir);
    return dir.entryList(QDir::Files | QDir::NoSymLinks | QDir::Readable);
}

void AppletInterface::executeAction(const QString &name)
{
    if (qmlObject()->rootObject()) {
        const QMetaObject *metaObj = qmlObject()->rootObject()->metaObject();
        const QByteArray actionMethodName = "action_" + name.toUtf8();
        const QByteArray actionFunctionName = actionMethodName + QByteArray("()");
        if (metaObj->indexOfMethod(QMetaObject::normalizedSignature(actionFunctionName.constData()).constData()) != -1) {
            QMetaObject::invokeMethod(qmlObject()->rootObject(), actionMethodName.constData(), Qt::DirectConnection);
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

    auto it = reg.begin();
    const auto itEnd = reg.end();
    for (; it != itEnd; ++it) {
        QRect rect = *it;
        // make it relative
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

    // If corona returned an invalid screenId, try to use lastScreen value if it is valid
    if (screenId == -1 && applet()->containment()->lastScreen() > -1) {
        screenId = applet()->containment()->lastScreen();
    }

    if (screenId > -1) {
        rect = applet()->containment()->corona()->availableScreenRect(screenId);
        // make it relative
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
        QKeySequence seq(ke->key() | ke->modifiers());

        QList<QAction *> actions = applet()->actions()->actions();
        // find the wallpaper action if we are a containment
        ContainmentInterface *ci = qobject_cast<ContainmentInterface *>(this);
        if (ci) {
            WallpaperInterface *wi = ci->wallpaperInterface();
            if (wi) {
                actions << wi->contextualActions();
            }
        }

        // add any actions of the corona
        if (applet()->containment() && applet()->containment()->corona()) {
            actions << applet()->containment()->corona()->actions()->actions();
        }

        bool keySequenceUsed = false;
        for (auto a : std::as_const(actions)) {
            if (a->shortcut().isEmpty()) {
                continue;
            }

            if (!a->isEnabled()) {
                continue;
            }

            // this will happen on a normal, non emacs shortcut
            if (seq.matches(a->shortcut()) == QKeySequence::ExactMatch) {
                event->accept();
                a->trigger();
                m_oldKeyboardShortcut = 0;
                return true;

                // first part of an emacs style shortcut?
            } else if (seq.matches(a->shortcut()) == QKeySequence::PartialMatch) {
                keySequenceUsed = true;
                m_oldKeyboardShortcut = ke->key() | ke->modifiers();

                // no match at all, but it can be the second part of an emacs style shortcut
            } else {
                QKeySequence seq(m_oldKeyboardShortcut, ke->key() | ke->modifiers());

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

void AppletInterface::prepareContextualActions()
{
    Q_EMIT applet()->contextualActionsAboutToShow();
}

bool AppletInterface::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *e = static_cast<QMouseEvent *>(event);

        // pass it up to the applet
        // well, actually we have to pass it to the *containment*
        // because all the code for showing an applet's contextmenu is actually in Containment.
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

            // the plugin can be a single action or a context menu
            // Don't have an action list? execute as single action
            // and set the event position as action data
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
            prepareContextualActions();
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

void AppletInterface::updateUiReadyConstraint()
{
    if (!isLoading()) {
        applet()->updateConstraints(Plasma::Types::UiReadyConstraint);
    }
}

bool AppletInterface::isLoading() const
{
    return m_loading;
}

KPluginMetaData AppletInterface::metaData() const
{
    return applet()->pluginMetaData();
}

#include "moc_appletinterface.cpp"
