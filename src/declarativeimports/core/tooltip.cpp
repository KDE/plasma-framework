/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2011 Artur Duque de Souza <asouza@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "tooltip.h"
#include "tooltipdialog.h"

#include <QDebug>
#include <QQmlEngine>

#include "framesvgitem.h"
#include <KDirWatch>
#include <KWindowEffects>

ToolTipDialog *ToolTip::s_dialog = nullptr;
int ToolTip::s_dialogUsers = 0;

ToolTip::ToolTip(QQuickItem *parent)
    : QQuickItem(parent)
    , m_tooltipsEnabledGlobally(false)
    , m_containsMouse(false)
    , m_location(Plasma::Types::Floating)
    , m_textFormat(Qt::AutoText)
    , m_active(true)
    , m_interactive(false)
    , m_timeout(-1)
    , m_usingDialog(false)
{
    setAcceptHoverEvents(true);
    setFiltersChildMouseEvents(true);

    m_showTimer = new QTimer(this);
    m_showTimer->setSingleShot(true);
    connect(m_showTimer, &QTimer::timeout, this, &ToolTip::showToolTip);

    loadSettings();

    const QString configFile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/plasmarc");
    KDirWatch::self()->addFile(configFile);
    QObject::connect(KDirWatch::self(), &KDirWatch::created, this, &ToolTip::settingsChanged);
    QObject::connect(KDirWatch::self(), &KDirWatch::dirty, this, &ToolTip::settingsChanged);
}

ToolTip::~ToolTip()
{
    if (s_dialog && s_dialog->owner() == this) {
        s_dialog->setVisible(false);
    }

    if (m_usingDialog) {
        --s_dialogUsers;
    }

    if (s_dialogUsers == 0) {
        delete s_dialog;
        s_dialog = nullptr;
    }
}

void ToolTip::settingsChanged(const QString &file)
{
    if (!file.endsWith(QLatin1String("plasmarc"))) {
        return;
    }

    KSharedConfig::openConfig(QStringLiteral("plasmarc"))->reparseConfiguration();
    loadSettings();
}

void ToolTip::loadSettings()
{
    KConfigGroup cfg = KConfigGroup(KSharedConfig::openConfig(QStringLiteral("plasmarc")), "PlasmaToolTips");
    m_interval = cfg.readEntry("Delay", 700);
    m_tooltipsEnabledGlobally = (m_interval > 0);
}

QQuickItem *ToolTip::mainItem() const
{
    return m_mainItem.data();
}

ToolTipDialog *ToolTip::tooltipDialogInstance()
{
    if (!s_dialog) {
        s_dialog = new ToolTipDialog;
    }

    if (!m_usingDialog) {
        s_dialogUsers++;
        m_usingDialog = true;
    }

    return s_dialog;
}

void ToolTip::setMainItem(QQuickItem *mainItem)
{
    if (m_mainItem.data() != mainItem) {
        m_mainItem = mainItem;

        Q_EMIT mainItemChanged();

        if (!isValid() && s_dialog && s_dialog->owner() == this) {
            s_dialog->setVisible(false);
        }
    }
}

void ToolTip::showToolTip()
{
    if (!m_active) {
        return;
    }

    Q_EMIT aboutToShow();

    ToolTipDialog *dlg = tooltipDialogInstance();

    if (!mainItem()) {
        setMainItem(dlg->loadDefaultItem());
    }

    // Unset the dialog's old contents before reparenting the dialog.
    dlg->setMainItem(nullptr);

    Plasma::Types::Location location = m_location;
    if (m_location == Plasma::Types::Floating) {
        QQuickItem *p = parentItem();
        while (p) {
            if (p->property("location").isValid()) {
                location = (Plasma::Types::Location)p->property("location").toInt();
                break;
            }
            p = p->parentItem();
        }
    }

    if (mainItem()) {
        mainItem()->setProperty("toolTip", QVariant::fromValue(this));
        mainItem()->setVisible(true);
    }

    connect(dlg, &ToolTipDialog::visibleChanged, this, &ToolTip::toolTipVisibleChanged, Qt::UniqueConnection);

    dlg->setHideTimeout(m_timeout);
    dlg->setOwner(this);
    dlg->setLocation(location);
    dlg->setVisualParent(this);
    dlg->setMainItem(mainItem());
    dlg->setInteractive(m_interactive);
    dlg->setVisible(true);
    // In case the last owner triggered a dismiss but the dialog is still shown,
    // showEvent won't be reached and the old timeout will still be effective.
    // Call keepalive() to make it use the new timeout.
    dlg->keepalive();
}

QString ToolTip::mainText() const
{
    return m_mainText;
}

void ToolTip::setMainText(const QString &mainText)
{
    if (mainText == m_mainText) {
        return;
    }

    m_mainText = mainText;
    Q_EMIT mainTextChanged();

    if (!isValid() && s_dialog && s_dialog->owner() == this) {
        s_dialog->setVisible(false);
    }
}

QString ToolTip::subText() const
{
    return m_subText;
}

void ToolTip::setSubText(const QString &subText)
{
    if (subText == m_subText) {
        return;
    }

    m_subText = subText;
    Q_EMIT subTextChanged();

    if (!isValid() && s_dialog && s_dialog->owner() == this) {
        s_dialog->setVisible(false);
    }
}

int ToolTip::textFormat() const
{
    return m_textFormat;
}

void ToolTip::setTextFormat(int format)
{
    if (m_textFormat == format) {
        return;
    }

    m_textFormat = format;
    Q_EMIT textFormatChanged();
}

Plasma::Types::Location ToolTip::location() const
{
    return m_location;
}

void ToolTip::setLocation(Plasma::Types::Location location)
{
    if (m_location == location) {
        return;
    }
    m_location = location;
    Q_EMIT locationChanged();
}

void ToolTip::setActive(bool active)
{
    if (m_active == active) {
        return;
    }

    m_active = active;
    if (!active) {
        tooltipDialogInstance()->dismiss();
    }
    Q_EMIT activeChanged();
}

void ToolTip::setInteractive(bool interactive)
{
    if (m_interactive == interactive) {
        return;
    }

    m_interactive = interactive;

    Q_EMIT interactiveChanged();
}

void ToolTip::setTimeout(int timeout)
{
    m_timeout = timeout;
}

void ToolTip::hideToolTip()
{
    m_showTimer->stop();
    tooltipDialogInstance()->dismiss();
}

void ToolTip::hideImmediately()
{
    m_showTimer->stop();
    tooltipDialogInstance()->setVisible(false);
}

QVariant ToolTip::icon() const
{
    if (m_icon.isValid()) {
        return m_icon;
    } else {
        return QString();
    }
}

void ToolTip::setIcon(const QVariant &icon)
{
    if (icon == m_icon) {
        return;
    }

    m_icon = icon;
    Q_EMIT iconChanged();
}

QVariant ToolTip::image() const
{
    if (m_image.isValid()) {
        return m_image;
    } else {
        return QString();
    }
}

void ToolTip::setImage(const QVariant &image)
{
    if (image == m_image) {
        return;
    }

    m_image = image;
    Q_EMIT imageChanged();
}

bool ToolTip::containsMouse() const
{
    return m_containsMouse;
}

void ToolTip::setContainsMouse(bool contains)
{
    if (m_containsMouse != contains) {
        m_containsMouse = contains;
        Q_EMIT containsMouseChanged();
    }
    if (!contains) {
        tooltipDialogInstance()->dismiss();
    }
}

void ToolTip::hoverEnterEvent(QHoverEvent *event)
{
    Q_UNUSED(event)
    setContainsMouse(true);

    if (!m_tooltipsEnabledGlobally) {
        return;
    }

    if (!isValid()) {
        return;
    }

    if (tooltipDialogInstance()->isVisible()) {
        // We signal the tooltipmanager that we're "potentially interested,
        // and ask to keep it open for a bit, so other items get the chance
        // to update the content before the tooltip hides -- this avoids
        // flickering
        // It need to be considered only when other items can deal with tooltip area
        if (m_active) {
            tooltipDialogInstance()->keepalive();
            // FIXME: showToolTip needs to be renamed in sync or something like that
            showToolTip();
        }
    } else {
        m_showTimer->start(m_interval);
    }
}

void ToolTip::hoverLeaveEvent(QHoverEvent *event)
{
    Q_UNUSED(event)
    setContainsMouse(false);
    m_showTimer->stop();
}

bool ToolTip::childMouseEventFilter(QQuickItem *item, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        hideToolTip();
    }
    return QQuickItem::childMouseEventFilter(item, event);
}

bool ToolTip::isValid() const
{
    return m_mainItem || !mainText().isEmpty() || !subText().isEmpty();
}
