/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef CORONATEST_H
#define CORONATEST_H

#include <QTest>

#include <QTimer>

#include "plasma/corona.h"
#include "plasma/pluginloader.h"

class SimpleCorona : public Plasma::Corona
{
    Q_OBJECT

public:
    explicit SimpleCorona(QObject *parent = nullptr);
    ~SimpleCorona() override;

    QRect screenGeometry(int) const override;
    int screenForContainment(const Plasma::Containment *) const override;
};

class SimpleApplet : public Plasma::Applet
{
    Q_OBJECT

public:
    explicit SimpleApplet(QObject *parentObject, const KPluginMetaData &data, const QVariantList &args);

private:
    QTimer m_timer;
};

class SimpleContainment : public Plasma::Containment
{
    Q_OBJECT

public:
    explicit SimpleContainment(QObject *parentObject, const KPluginMetaData &data, const QVariantList &args);

private:
    QTimer m_timer;
};

class SimpleNoScreenContainment : public Plasma::Containment
{
    Q_OBJECT

public:
    explicit SimpleNoScreenContainment(QObject *parentObject, const KPluginMetaData &data, const QVariantList &args);
};

class CoronaTest : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

private Q_SLOTS:
    void restore();
    void checkOrder();
    void startupCompletion();
    void addRemoveApplets();
    void immutability();

private:
    SimpleCorona *m_corona;
    QDir m_configDir;
};

#endif
