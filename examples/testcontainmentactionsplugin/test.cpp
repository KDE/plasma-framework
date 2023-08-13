/*
    SPDX-FileCopyrightText: 2009 Chani Armitage <chani@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "test.h"

#include <QDebug>

#include <Plasma/Containment>

ContextTest::ContextTest(QObject *parent, const QVariantList &args)
    : Plasma::ContainmentActions(parent, args)
{
}

QList<QAction *> ContextTest::contextualActions()
{
    return {};
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
    Q_UNUSED(config)
}

QWidget *ContextTest::createConfigurationInterface(QWidget *parent)
{
    QWidget *widget = new QWidget(parent);
    m_ui.setupUi(widget);

    m_ui.text->setText(m_text);
    return widget;
}

void ContextTest::configurationAccepted()
{
    m_text = m_ui.text->text();
}

void ContextTest::restore(const KConfigGroup &config)
{
    m_text = config.readEntry("test-text", QString());
}

void ContextTest::save(KConfigGroup &config)
{
    config.writeEntry("test-text", m_text);
}

K_PLUGIN_CLASS_WITH_JSON(ContextTest, "plasma-containmentactions-test.json")

#include "test.moc"

#include "moc_test.cpp"
