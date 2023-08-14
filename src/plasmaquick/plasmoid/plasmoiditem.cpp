/*
    SPDX-FileCopyrightText: 2008-2013 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2010-2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plasmoiditem.h"
#include "appletcontext_p.h"
#include "sharedqmlengine.h"

#include <QAction>
#include <QActionGroup>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QTimer>

#include <KConfigLoader>
#include <KLocalizedString>
#include <QDebug>

#include <Plasma/ContainmentActions>
#include <Plasma/Corona>
#include <Plasma/Plasma>
#include <Plasma/PluginLoader>

#include "containmentitem.h"
#include "wallpaperitem.h"

#include <KConfigPropertyMap>

PlasmoidItem::PlasmoidItem(QQuickItem *parent)
    : AppletQuickItem(parent)
    , m_toolTipTextFormat(0)
    , m_toolTipItem(nullptr)
    , m_hideOnDeactivate(true)
    , m_oldKeyboardShortcut(0)
    , m_positionBeforeRemoval(QPointF(-1, -1))
{
    qmlRegisterAnonymousType<QAction>("org.kde.plasma.plasmoid", 1);
}

PlasmoidItem::~PlasmoidItem()
{
}

void PlasmoidItem::init()
{
    AppletQuickItem::init();

    auto *applet = PlasmoidItem::applet();

    connect(applet, &Plasma::Applet::contextualActionsAboutToShow, this, &PlasmoidItem::contextualActionsAboutToShow);

    connect(applet, &Plasma::Applet::titleChanged, this, [this]() {
        if (m_toolTipMainText.isNull()) {
            Q_EMIT toolTipMainTextChanged();
        }
    });

    if (applet->containment()) {
        connect(applet->containment(), &Plasma::Containment::screenChanged, this, &PlasmoidItem::screenChanged);

        // Screen change implies geo change for good measure.
        connect(applet->containment(), &Plasma::Containment::screenChanged, this, &PlasmoidItem::screenGeometryChanged);

        connect(applet->containment()->corona(), &Plasma::Corona::screenGeometryChanged, this, [this](int id) {
            if (id == AppletQuickItem::applet()->containment()->screen()) {
                Q_EMIT screenGeometryChanged();
            }
        });

        connect(applet->containment()->corona(), &Plasma::Corona::availableScreenRegionChanged, this, &ContainmentItem::availableScreenRegionChanged);
        connect(applet->containment()->corona(), &Plasma::Corona::availableScreenRectChanged, this, &ContainmentItem::availableScreenRectChanged);
    }

    connect(this, &PlasmoidItem::expandedChanged, [=](bool expanded) {
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

    geometryChange(QRectF(), QRectF(x(), y(), width(), height()));

    connect(applet, &Plasma::Applet::activated, this, [=]() {
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

    connect(applet, &Plasma::Applet::destroyedChanged, this, &PlasmoidItem::destroyedChanged);

    auto args = applet->startupArguments();

    if (args.count() == 1) {
        Q_EMIT externalData(QString(), args.first());
    } else if (!args.isEmpty()) {
        Q_EMIT externalData(QString(), args);
    }
}

void PlasmoidItem::destroyedChanged(bool destroyed)
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

QString PlasmoidItem::toolTipMainText() const
{
    if (m_toolTipMainText.isNull()) {
        return applet()->title();
    } else {
        return m_toolTipMainText;
    }
}

void PlasmoidItem::setToolTipMainText(const QString &text)
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

QString PlasmoidItem::toolTipSubText() const
{
    if (m_toolTipSubText.isNull() && applet()->pluginMetaData().isValid()) {
        return applet()->pluginMetaData().description();
    } else {
        return m_toolTipSubText;
    }
}

void PlasmoidItem::setToolTipSubText(const QString &text)
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

int PlasmoidItem::toolTipTextFormat() const
{
    return m_toolTipTextFormat;
}

void PlasmoidItem::setToolTipTextFormat(int format)
{
    if (m_toolTipTextFormat == format) {
        return;
    }

    m_toolTipTextFormat = format;
    Q_EMIT toolTipTextFormatChanged();
}

QQuickItem *PlasmoidItem::toolTipItem() const
{
    return m_toolTipItem.data();
}

void PlasmoidItem::setToolTipItem(QQuickItem *toolTipItem)
{
    if (m_toolTipItem.data() == toolTipItem) {
        return;
    }

    m_toolTipItem = toolTipItem;
    connect(m_toolTipItem.data(), &QObject::destroyed, this, &PlasmoidItem::toolTipItemChanged);

    Q_EMIT toolTipItemChanged();
}

int PlasmoidItem::screen() const
{
    if (Plasma::Containment *c = applet()->containment()) {
        return c->screen();
    }

    return -1;
}

void PlasmoidItem::setHideOnWindowDeactivate(bool hide)
{
    if (m_hideOnDeactivate != hide) {
        m_hideOnDeactivate = hide;
        Q_EMIT hideOnWindowDeactivateChanged();
    }
}

bool PlasmoidItem::hideOnWindowDeactivate() const
{
    return m_hideOnDeactivate;
}

QRect PlasmoidItem::screenGeometry() const
{
    if (!applet() || !applet()->containment() || !applet()->containment()->corona() || applet()->containment()->screen() < 0) {
        return QRect();
    }

    return applet()->containment()->corona()->screenGeometry(applet()->containment()->screen());
}

QVariantList PlasmoidItem::availableScreenRegion() const
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

QRect PlasmoidItem::availableScreenRect() const
{
    if (!applet()->containment() || !applet()->containment()->corona()) {
        return QRect();
    }

    QRect rect(0, 0, width(), height());

    int screenId = screen();

    // If corona returned an invalid screenId, try to use lastScreen value if it is valid
    if (screenId == -1 && applet()->containment()->lastScreen() > -1) {
        screenId = applet()->containment()->lastScreen();
        // Is this a screen not actually valid?
        if (screenId >= applet()->containment()->corona()->numScreens()) {
            screenId = -1;
        }
    }

    if (screenId > -1) {
        rect = applet()->containment()->corona()->availableScreenRect(screenId);
        // make it relative
        QRect geometry = applet()->containment()->corona()->screenGeometry(screenId);
        rect.moveTo(rect.topLeft() - geometry.topLeft());
    }

    return rect;
}

bool PlasmoidItem::event(QEvent *event)
{
    // QAction keyboard shortcuts cannot work with QML2 (and probably newver will
    // since in Qt qtquick and qwidgets cannot depend from each other in any way)
    // so do a simple keyboard shortcut matching here
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent *>(event);
        QKeySequence seq(ke->key() | ke->modifiers());

        QList<QAction *> actions = applet()->internalActions();
        actions.append(applet()->contextualActions());
        // find the wallpaper action if we are a containment
        ContainmentItem *ci = qobject_cast<ContainmentItem *>(this);
        if (ci) {
            WallpaperItem *wi = ci->wallpaperItem();
            if (wi) {
                actions << wi->contextualActions();
            }
        }

        // add any actions of the corona
        if (applet()->containment() && applet()->containment()->corona()) {
            actions << applet()->containment()->corona()->actions();
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

void PlasmoidItem::prepareContextualActions()
{
    Q_EMIT applet()->contextualActionsAboutToShow();
}

bool PlasmoidItem::eventFilter(QObject *watched, QEvent *event)
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

            ContainmentItem *ci = qobject_cast<ContainmentItem *>(AppletQuickItem::itemForApplet(c));

            if (!ci) {
                return false;
            }

            // the plugin can be a single action or a context menu
            // Don't have an action list? execute as single action
            // and set the event position as action data
            if (plugin->contextualActions().length() == 1) {
                // but first check whether we are not a popup
                // we don't want to randomly create applets without confirmation
                if (static_cast<QQuickItem *>(watched)->window() != ci->window()) {
                    return true;
                }

                QAction *action = plugin->contextualActions().at(0);
                action->setData(e->globalPosition().toPoint());
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
                desktopMenu->popup(e->globalPosition().toPoint());
                return true;
            }

            delete desktopMenu;
            return false;
        }
    }

    return AppletQuickItem::eventFilter(watched, event);
}

#include "moc_plasmoiditem.cpp"
