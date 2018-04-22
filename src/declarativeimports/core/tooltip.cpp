/***************************************************************************
 *   Copyright 2011 Marco Martin <mart@kde.org>                            *
 *   Copyright 2011 Artur Duque de Souza <asouza@kde.org>                  *
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "tooltip.h"
#include "tooltipdialog.h"

#include <QQmlEngine>
#include <QQuickItem>
#include <QDebug>

#include "framesvgitem.h"
#include <kwindoweffects.h>
#include <KDirWatch>

ToolTipDialog *ToolTip::s_dialog = nullptr;
int ToolTip::s_dialogUsers  = 0;

ToolTip::ToolTip(QQuickItem *parent)
    : QQuickItem(parent),
      m_tooltipsEnabledGlobally(false),
      m_containsMouse(false),
      m_location(Plasma::Types::Floating),
      m_textFormat(Qt::AutoText),
      m_active(true),
      m_interactive(false),
      m_usingDialog(false)
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
        s_dialogUsers = 1;
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

        emit mainItemChanged();

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

    emit aboutToShow();

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

    dlg->setOwner(this);
    dlg->setLocation(location);
    dlg->setVisualParent(this);
    dlg->setMainItem(mainItem());
    dlg->setInteractive(m_interactive);
    dlg->setVisible(true);
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
    emit mainTextChanged();

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
    emit subTextChanged();

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
    emit textFormatChanged();
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
    emit locationChanged();
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
    emit activeChanged();
}

void ToolTip::setInteractive(bool interactive)
{
    if (m_interactive == interactive) {
        return;
    }

    m_interactive = interactive;

    emit interactiveChanged();
}

void ToolTip::hideToolTip()
{
    m_showTimer->stop();
    tooltipDialogInstance()->dismiss();
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
    emit iconChanged();
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
    emit imageChanged();
}

bool ToolTip::containsMouse() const
{
    return m_containsMouse;
}

void ToolTip::setContainsMouse(bool contains)
{
    if (m_containsMouse != contains) {
        m_containsMouse = contains;
        emit containsMouseChanged();
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
            //FIXME: showToolTip needs to be renamed in sync or something like that
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
