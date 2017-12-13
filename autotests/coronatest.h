/******************************************************************************
*   Copyright 2014 Marco Martin <mart@kde.org>                                *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/
#ifndef CORONATEST_H
#define CORONATEST_H

#include <QtTest/QtTest>

#include <QTimer>

#include "plasma/corona.h"
#include "plasma/pluginloader.h"

class SimpleLoader : public Plasma::PluginLoader
{
protected:
    virtual Plasma::Applet *internalLoadApplet(const QString &name, uint appletId = 0,
                                       const QVariantList &args = QVariantList()) Q_DECL_OVERRIDE;
};


class SimpleCorona : public Plasma::Corona
{
    Q_OBJECT

public:
    explicit SimpleCorona(QObject * parent = nullptr);
    ~SimpleCorona();

    QRect screenGeometry(int) const Q_DECL_OVERRIDE;
    int screenForContainment(const Plasma::Containment *) const Q_DECL_OVERRIDE;
};

class SimpleApplet : public Plasma::Applet
{
    Q_OBJECT

public:
    explicit SimpleApplet(QObject *parent = nullptr, const QString &serviceId = QString(), uint appletId = 0);
private:
    QTimer m_timer;
};

class SimpleContainment : public Plasma::Containment
{
    Q_OBJECT

public:
    explicit SimpleContainment(QObject *parent = nullptr, const QString &serviceId = QString(), uint appletId = 0);
private:
    QTimer m_timer;
};

class SimpleNoScreenContainment : public Plasma::Containment
{
    Q_OBJECT

public:
    explicit SimpleNoScreenContainment(QObject *parent = nullptr, const QString &serviceId = QString(), uint appletId = 0);
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

