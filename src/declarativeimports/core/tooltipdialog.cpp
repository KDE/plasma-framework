/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "tooltipdialog.h"

#include <QDebug>
#include <QFile>
#include <QPlatformSurfaceEvent>
#include <QQmlEngine>
#include <QQuickItem>

#include <KWindowSystem>
#include <kdeclarative/qmlobjectsharedengine.h>

ToolTipDialog::ToolTipDialog(QQuickItem *parent)
    : Dialog(parent)
    , m_qmlObject(nullptr)
    , m_hideTimeout(-1)
    , m_interactive(false)
    , m_owner(nullptr)
{
    setLocation(Plasma::Types::Floating);
    setType(Dialog::WindowType::Tooltip);

    m_showTimer = new QTimer(this);
    m_showTimer->setSingleShot(true);
    connect(m_showTimer, &QTimer::timeout, this, [this]() {
        setVisible(false);
    });
}

ToolTipDialog::~ToolTipDialog()
{
}

QQuickItem *ToolTipDialog::loadDefaultItem()
{
    if (!m_qmlObject) {
        m_qmlObject = new KDeclarative::QmlObjectSharedEngine(this);
    }

    if (!m_qmlObject->rootObject()) {
        // HACK: search our own import
        const auto paths = m_qmlObject->engine()->importPathList();
        for (const QString &path : paths) {
            if (QFile::exists(path + QStringLiteral("/org/kde/plasma/core"))) {
                m_qmlObject->setSource(QUrl::fromLocalFile(path + QStringLiteral("/org/kde/plasma/core/private/DefaultToolTip.qml")));
                break;
            }
        }
    }

    return qobject_cast<QQuickItem *>(m_qmlObject->rootObject());
}

void ToolTipDialog::showEvent(QShowEvent *event)
{
    keepalive();

    Dialog::showEvent(event);
}

void ToolTipDialog::hideEvent(QHideEvent *event)
{
    m_showTimer->stop();

    Dialog::hideEvent(event);
}

void ToolTipDialog::resizeEvent(QResizeEvent *re)
{
    Dialog::resizeEvent(re);
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

    bool ret = Dialog::event(e);
    Qt::WindowFlags flags = Qt::ToolTip | Qt::WindowDoesNotAcceptFocus | Qt::WindowStaysOnTopHint;
    if (KWindowSystem::isPlatformX11()) {
        flags = flags | Qt::BypassWindowManagerHint;
    }
    setFlags(flags);
    return ret;
}

QObject *ToolTipDialog::owner() const
{
    return m_owner;
}

void ToolTipDialog::setOwner(QObject *owner)
{
    m_owner = owner;
}

void ToolTipDialog::dismiss()
{
    m_showTimer->start(200);
}

void ToolTipDialog::keepalive()
{
    if (m_hideTimeout > 0) {
        m_showTimer->start(m_hideTimeout);
    } else {
        m_showTimer->stop();
    }
}

bool ToolTipDialog::interactive()
{
    return m_interactive;
}

void ToolTipDialog::setInteractive(bool interactive)
{
    m_interactive = interactive;
    setOutputOnly(!interactive);
}

void ToolTipDialog::valueChanged(const QVariant &value)
{
    setPosition(value.toPoint());
}

void ToolTipDialog::setHideTimeout(int timeout)
{
    m_hideTimeout = timeout;
}

int ToolTipDialog::hideTimeout() const
{
    return m_hideTimeout;
}

#include "moc_tooltipdialog.cpp"
