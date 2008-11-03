/*
 *   Copyright 2008 Marco Martin <notmart@gmail.com>
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#include "toolbutton.h"

#include <QAction>
#include <QPainter>
#include <QPaintEvent>
#include <QStyle>
#include <QStyleOptionToolButton>

#include <plasma/paintutils.h>
#include <plasma/theme.h>
#include <plasma/framesvg.h>

ToolButton::ToolButton()
    : QToolButton(),
      m_action(0)
{
    setAttribute(Qt::WA_NoSystemBackground);

    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/button");
    m_background->setCacheAllRenderedFrames(true);
    m_background->setElementPrefix("plain");
}


ToolButton::ToolButton(QWidget *parent)
    : QToolButton(parent),
      m_action(0)
{
    m_background = new Plasma::FrameSvg(this);
    m_background->setImagePath("widgets/button");
    m_background->setCacheAllRenderedFrames(true);
    m_background->setElementPrefix("plain");
}

void ToolButton::setAction(QAction *action)
{
    if (!action) {
        return;
    }

    if (m_action) {
        disconnect(m_action, SIGNAL(changed()), this, SLOT(syncToAction()));
        disconnect(this, SIGNAL(clicked()), m_action, SLOT(trigger()));
    }

    m_action = action;
    connect(m_action, SIGNAL(changed()), this, SLOT(syncToAction()));
    connect(this, SIGNAL(clicked()), m_action, SLOT(trigger()));
    connect(m_action, SIGNAL(destroyed(QObject*)), this, SLOT(actionDestroyed(QObject*)));
    syncToAction();
}

void ToolButton::syncToAction()
{
    if (!m_action) {
        return;
    }

    setIcon(m_action->icon());
    setText(m_action->text());

    if (toolButtonStyle() == Qt::ToolButtonIconOnly) {
        setToolTip(m_action->text());
    }

    setCheckable(m_action->isCheckable());
    if (m_action->actionGroup()) {
        setAutoExclusive(m_action->actionGroup()->isExclusive());
    }

    setEnabled(m_action->isEnabled());
}

void ToolButton::actionDestroyed(QObject *)
{
    m_action = 0;
}

void ToolButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);

    QStyleOptionToolButton buttonOpt;
    initStyleOption(&buttonOpt);

    if ((buttonOpt.state & QStyle::State_MouseOver) || (buttonOpt.state & QStyle::State_On)) {
        if (buttonOpt.state & QStyle::State_Sunken || (buttonOpt.state & QStyle::State_On)) {
            m_background->setElementPrefix("pressed");
        } else {
            m_background->setElementPrefix("normal");
        }
        m_background->resizeFrame(size());
        m_background->paintFrame(&painter);

        buttonOpt.palette.setColor(QPalette::ButtonText, Plasma::Theme::defaultTheme()->color(Plasma::Theme::ButtonTextColor));
    } else {
        buttonOpt.palette.setColor(QPalette::ButtonText, Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    }

    style()->drawControl(QStyle::CE_ToolButtonLabel, &buttonOpt, &painter, this);
}

#include "toolbutton.moc"

