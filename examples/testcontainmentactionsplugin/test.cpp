/*
    SPDX-FileCopyrightText: 2009 Chani Armitage <chani@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test.h"

#include <KActionCollection>
#include <QDebug>

#include <Plasma/Containment>

ContextTest::ContextTest(QObject *parent, const QVariantList &args)
    : Plasma::ContainmentActions(parent, args)
{
}

QList<QAction *> ContextTest::contextualActions()
{
    Plasma::Containment *c = containment();
    Q_ASSERT(c);
    QList<QAction *> actions;
    actions << c->actions()->action(QStringLiteral("configure"));

    return actions;
}

void ContextTest::performNextAction()
{
    qWarning() << "Next action requested";
}

void ContextTest::performPreviousAction()
{
    qWarning() << "Previous action requested";
}

void ContextTest::init(const KConfigGroup &config)
{
    m_text = config.readEntry("test-text", QString());
}

QWidget *ContextTest::createConfigurationInterface(QWidget *parent)
{
    // m_currentText = m_text;
    QWidget *widget = new QWidget(parent);
    m_ui.setupUi(widget);

    m_ui.text->setText(m_text);
    // FIXME this way or just get it on close?
    // connect(m_ui.text, SIGNAL(changed(QColor)), this, SLOT(setColor(QColor)));
    // connect(this, SIGNAL(settingsChanged(bool)), parent, SLOT(settingsChanged(bool)));
    return widget;
}

void ContextTest::configurationAccepted()
{
    m_text = m_ui.text->text();
}

void ContextTest::save(KConfigGroup &config)
{
    config.writeEntry("test-text", m_text);
}

K_PLUGIN_CLASS_WITH_JSON(ContextTest, "plasma-containmentactions-test.desktop")

#include "test.moc"
