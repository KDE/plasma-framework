/***************************************************************************
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

#include "tooltipdialog.h"

#include <QQmlEngine>
#include <QQuickItem>
#include <QDebug>
#include <QPropertyAnimation>

#include "framesvgitem.h"

#include <kdeclarative/qmlobject.h>

ToolTipDialog::ToolTipDialog(QQuickItem  *parent)
    : DialogProxy(parent),
      m_qmlObject(0),
      m_animation(0),
      m_hideTimeout(4000),
      m_direction(Plasma::Types::Up),
      m_interactive(false)
{
    setFlags(Qt::ToolTip);
    setLocation(Plasma::Types::Floating);
    setDirection(Plasma::Types::Up);
    frameSvgItem()->setImagePath("widgets/tooltip");

    m_animation = new QPropertyAnimation(this);
    connect(m_animation, SIGNAL(valueChanged(QVariant)),
            this, SLOT(valueChanged(QVariant)));
    m_animation->setTargetObject(this);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    m_animation->setDuration(250);

    m_showTimer = new QTimer(this);
    m_showTimer->setSingleShot(true);
    connect(m_showTimer, &QTimer::timeout, [=]() {
        setVisible(false);
    });
}

ToolTipDialog::~ToolTipDialog()
{
}

QQuickItem *ToolTipDialog::loadDefaultItem()
{
    if (!m_qmlObject) {
        m_qmlObject = new KDeclarative::QmlObject(this);
    }

    if (!m_qmlObject->rootObject()) {
        //HACK: search our own import
        foreach (const QString &path, m_qmlObject->engine()->importPathList()) {
            if (QFile::exists(path + "/org/kde/plasma/core")) {
                m_qmlObject->setSource(QUrl::fromLocalFile(path + "/org/kde/plasma/core/private/DefaultToolTip.qml"));
                break;
            }
        }
    }

    return qobject_cast<QQuickItem *>(m_qmlObject->rootObject());
}

Plasma::Types::Direction ToolTipDialog::direction() const
{
    return m_direction;
}

void ToolTipDialog::setDirection(Plasma::Types::Direction dir)
{
    m_direction = dir;
}

void ToolTipDialog::showEvent(QShowEvent *event)
{
    m_showTimer->start(m_hideTimeout);

    m_animation->stop();
    DialogProxy::showEvent(event);
    setFlags(Qt::ToolTip);
}

void ToolTipDialog::hideEvent(QHideEvent *event)
{
    m_showTimer->stop();
    m_animation->stop();

    DialogProxy::hideEvent(event);
}

void ToolTipDialog::resizeEvent(QResizeEvent *re)
{
    DialogProxy::resizeEvent(re);
}

bool ToolTipDialog::event(QEvent *e)
{
    if (e->type() == QEvent::Enter) {
        if (m_interactive) {
            m_showTimer->stop();
        }
    } else if (e->type() == QEvent::Leave) {
        dismiss();
    }
    return DialogProxy::event(e);
}

void ToolTipDialog::adjustGeometry(const QRect &geom)
{
    switch (m_direction) {
    case Plasma::Types::Right:
        setX(x() + (size().width() - geom.size().width()));
        break;
    case Plasma::Types::Up:
        setY(y() + (size().height() - geom.size().height()));
        break;
    default:
        break;
    }

    if (isVisible()) {

        resize(geom.size());
        m_animation->setStartValue(position());
        m_animation->setEndValue(geom.topLeft());
        m_animation->start();
    } else {
        setGeometry(geom);
    }
}

void ToolTipDialog::dismiss()
{
    m_showTimer->start(m_hideTimeout / 20); // pretty short: 200ms
}

void ToolTipDialog::keepalive()
{
    m_showTimer->start(m_hideTimeout);
}

bool ToolTipDialog::interactive()
{
    return m_interactive;
}

void ToolTipDialog::setInteractive(bool interactive)
{
    m_interactive = interactive;
}

void ToolTipDialog::valueChanged(const QVariant &value)
{
    setPosition(value.value<QPoint>());
}

#include "moc_tooltipdialog.cpp"
