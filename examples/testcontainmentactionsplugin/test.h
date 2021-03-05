/*
    SPDX-FileCopyrightText: 2009 Chani Armitage <chani@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CONTEXTTEST_HEADER
#define CONTEXTTEST_HEADER

#include "ui_config.h"
#include <QString>
#include <plasma/containmentactions.h>

class ContextTest : public Plasma::ContainmentActions
{
    Q_OBJECT
public:
    ContextTest(QObject *parent, const QVariantList &args);

    void init(const KConfigGroup &config);

    QList<QAction *> contextualActions() override;

    void performNextAction() override;
    void performPreviousAction() override;

    QWidget *createConfigurationInterface(QWidget *parent) override;
    void configurationAccepted() override;
    void save(KConfigGroup &config) override;

private:
    Ui::Config m_ui;
    QString m_text;
};

#endif
