/********************************************************************************
*   Copyright 2014 Marco Martin <mart@kde.org>                                  *
*                                                                               *
*   This library is free software; you can redistribute it and/or               *
*   modify it under the terms of the GNU Library General Public                 *
*   License as published by the Free Software Foundation; either                *
*   version 2 of the License, or (at your option) any later version.            *
*                                                                               *
*   This library is distributed in the hope that it will be useful,             *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of              *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU            *
*   Library General Public License for more details.                            *
*                                                                               *
*   You should have received a copy of the GNU Library General Public License   *
*   along with this library; see the file COPYING.LIB.  If not, write to        *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,        *
*   Boston, MA 02110-1301, USA.                                                 *
*********************************************************************************/

#include "coronatest.h"
#include <ksycoca.h>
#include <kactioncollection.h>
#include <QStandardPaths>
#include <QAction>
#include <QApplication>
#include <QSignalSpy>
#include <QRandomGenerator>
#include <QProcess>
Plasma::Applet *SimpleLoader::internalLoadApplet(const QString &name, uint appletId,
                                   const QVariantList &args)
{
    Q_UNUSED(args)
    if (name == QLatin1String("simpleapplet")) {
        return new SimpleApplet(nullptr, QString(), appletId);
    } else if (name == QLatin1String("simplecontainment")) {
        return new SimpleContainment(nullptr, QString(), appletId);
    } else if (name == QLatin1String("simplenoscreencontainment")) {
        return new SimpleNoScreenContainment(nullptr, QString(), appletId);
    } else {
        return nullptr;
    }
}

SimpleCorona::SimpleCorona(QObject *parent)
    : Plasma::Corona(parent)
{
    Plasma::PluginLoader::setPluginLoader(new SimpleLoader);
}

SimpleCorona::~SimpleCorona()
{}


QRect SimpleCorona::screenGeometry(int screen) const
{
    //completely arbitrary, still not tested
    return QRect(100*screen, 100, 100, 100);
}

int SimpleCorona::screenForContainment(const Plasma::Containment *c) const
{
    if (qobject_cast<const SimpleNoScreenContainment *>(c)) {
        return -1;
    }
    return 0;
}

SimpleApplet::SimpleApplet(QObject *parent , const QString &serviceId, uint appletId)
    : Plasma::Applet(parent, serviceId, appletId)
{
    //updateConstraints(Plasma::Types::UiReadyConstraint);
    m_timer.setSingleShot(true);
    m_timer.setInterval(QRandomGenerator::global()->bounded((500 + 1) - 100) + 100);
    m_timer.start();
    connect(&m_timer, &QTimer::timeout, [=]() {
        updateConstraints(Plasma::Types::UiReadyConstraint);
    });
}


SimpleContainment::SimpleContainment(QObject *parent , const QString &serviceId, uint appletId)
    : Plasma::Containment(parent, serviceId, appletId)
{
    //updateConstraints(Plasma::Types::UiReadyConstraint);
    m_timer.setSingleShot(true);
    m_timer.setInterval(QRandomGenerator::global()->bounded((500 + 1) - 100) + 100);
    m_timer.start();
    connect(&m_timer, &QTimer::timeout, [=]() {
        updateConstraints(Plasma::Types::UiReadyConstraint);
    });
}

SimpleNoScreenContainment::SimpleNoScreenContainment(QObject *parent , const QString &serviceId, uint appletId)
    : Plasma::Containment(parent, serviceId, appletId)
{
    //This containment will *never* be isUiReady()
}

static void runKBuildSycoca()
{
    QProcess proc;
    const QString kbuildsycoca = QStandardPaths::findExecutable(QStringLiteral(KBUILDSYCOCA_EXENAME));
    QVERIFY(!kbuildsycoca.isEmpty());
    QStringList args;
    args << QStringLiteral("--testmode");
    proc.setProcessChannelMode(QProcess::MergedChannels); // silence kbuildsycoca output
    proc.start(kbuildsycoca, args);

    QSignalSpy spy(KSycoca::self(), SIGNAL(databaseChanged(QStringList)));
    QVERIFY(spy.wait(10000));

    proc.waitForFinished();
    QCOMPARE(proc.exitStatus(), QProcess::NormalExit);
}


void CoronaTest::initTestCase()
{
    if (!KSycoca::isAvailable()) {
        runKBuildSycoca();
    }

    QStandardPaths::setTestModeEnabled(true);
    m_corona = new SimpleCorona;

    m_configDir = QDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
    m_configDir.removeRecursively();

    QVERIFY(m_configDir.mkpath(QStringLiteral(".")));

    QVERIFY(QFile::copy(QStringLiteral(":/plasma-test-appletsrc"), m_configDir.filePath(QStringLiteral("plasma-test-appletsrc"))));
}

void CoronaTest::cleanupTestCase()
{
    m_configDir.removeRecursively();
    delete m_corona;
}

void CoronaTest::restore()
{
    m_corona->loadLayout(QStringLiteral("plasma-test-appletsrc"));
    QCOMPARE(m_corona->containments().count(), 3);

    foreach (auto cont, m_corona->containments()) {
        switch (cont->id()) {
        case 1:
            QCOMPARE(cont->applets().count(), 2);
            break;
        default:
            QCOMPARE(cont->applets().count(), 0);
            break;
        }
    }


}


void CoronaTest::checkOrder()
{
    QCOMPARE(m_corona->containments().count(), 3);

    //check containments order
    QCOMPARE(m_corona->containments().at(0)->id(), (uint)1);
    QCOMPARE(m_corona->containments().at(1)->id(), (uint)4);
    QCOMPARE(m_corona->containments().at(2)->id(), (uint)5);

    //check applets order
    QCOMPARE(m_corona->containments().at(0)->applets().count(), 2);
    QCOMPARE(m_corona->containments().at(0)->applets().at(0)->id(), (uint)2);
    QCOMPARE(m_corona->containments().at(0)->applets().at(1)->id(), (uint)3);
}

void CoronaTest::startupCompletion()
{
    QVERIFY(!m_corona->isStartupCompleted());
    QVERIFY(!m_corona->containments().at(0)->isUiReady());

    QSignalSpy spy(m_corona, SIGNAL(startupCompleted()));
    QVERIFY(spy.wait(1000));

    QVERIFY(m_corona->isStartupCompleted());
    QVERIFY(m_corona->containments().at(0)->isUiReady());
}

void CoronaTest::addRemoveApplets()
{
    m_corona->containments().at(0)->createApplet(QStringLiteral("invalid"));
    QCOMPARE(m_corona->containments().at(0)->applets().count(), 3);

    //remove action present
    QVERIFY(m_corona->containments().at(0)->applets().at(0)->actions()->action(QStringLiteral("remove")));
    //kill an applet
    m_corona->containments().at(0)->applets().at(0)->destroy();

    QSignalSpy spy(m_corona->containments().at(0)->applets().at(0), SIGNAL(destroyed()));
    QVERIFY(spy.wait(1000));
    QCOMPARE(m_corona->containments().at(0)->applets().count(), 2);
}

//this test has to be the last, since systemimmutability
//can't be programmatically unlocked
void CoronaTest::immutability()
{
    //immutability
    QCOMPARE(m_corona->immutability(), Plasma::Types::Mutable);
    m_corona->setImmutability(Plasma::Types::UserImmutable);
    QCOMPARE(m_corona->immutability(), Plasma::Types::UserImmutable);

    foreach (Plasma::Containment *cont, m_corona->containments()) {
        QCOMPARE(cont->immutability(), Plasma::Types::UserImmutable);
        foreach (Plasma::Applet *app, cont->applets()) {
            QCOMPARE(app->immutability(), Plasma::Types::UserImmutable);
        }
    }

    m_corona->setImmutability(Plasma::Types::Mutable);
    QCOMPARE(m_corona->immutability(), Plasma::Types::Mutable);

    foreach (Plasma::Containment *cont, m_corona->containments()) {
        QCOMPARE(cont->immutability(), Plasma::Types::Mutable);
        foreach (Plasma::Applet *app, cont->applets()) {
            QCOMPARE(app->immutability(), Plasma::Types::Mutable);
        }
    }

    m_corona->setImmutability(Plasma::Types::SystemImmutable);
    QCOMPARE(m_corona->immutability(), Plasma::Types::SystemImmutable);

    foreach (Plasma::Containment *cont, m_corona->containments()) {
        QCOMPARE(cont->immutability(), Plasma::Types::SystemImmutable);
        foreach (Plasma::Applet *app, cont->applets()) {
            QCOMPARE(app->immutability(), Plasma::Types::SystemImmutable);
        }
    }

    //can't unlock systemimmutable
    m_corona->setImmutability(Plasma::Types::Mutable);
    QCOMPARE(m_corona->immutability(), Plasma::Types::SystemImmutable);

    foreach (Plasma::Containment *cont, m_corona->containments()) {
        QCOMPARE(cont->immutability(), Plasma::Types::SystemImmutable);
        foreach (Plasma::Applet *app, cont->applets()) {
            QCOMPARE(app->immutability(), Plasma::Types::SystemImmutable);
        }
    }
}

QTEST_MAIN(CoronaTest)

