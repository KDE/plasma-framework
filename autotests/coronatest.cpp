/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "coronatest.h"
#include <KActionCollection>

#include <QAction>
#include <QApplication>
#include <QProcess>
#include <QRandomGenerator>
#include <QSignalSpy>
#include <QStandardPaths>

SimpleCorona::SimpleCorona(QObject *parent)
    : Plasma::Corona(parent)
{
}

SimpleCorona::~SimpleCorona()
{
}

QRect SimpleCorona::screenGeometry(int screen) const
{
    // completely arbitrary, still not tested
    return QRect(100 * screen, 100, 100, 100);
}

int SimpleCorona::screenForContainment(const Plasma::Containment *c) const
{
    if (qobject_cast<const SimpleNoScreenContainment *>(c)) {
        return -1;
    }
    return 0;
}

SimpleApplet::SimpleApplet(QObject *parentObject, const KPluginMetaData &data, const QVariantList &args)
    : Plasma::Applet(parentObject, data, args)
{
    // updateConstraints(Plasma::Types::UiReadyConstraint);
    m_timer.setSingleShot(true);
    m_timer.setInterval(QRandomGenerator::global()->bounded((500 + 1) - 100) + 100);
    m_timer.start();
    connect(&m_timer, &QTimer::timeout, [=]() {
        updateConstraints(Plasma::Types::UiReadyConstraint);
    });
}

SimpleContainment::SimpleContainment(QObject *parentObject, const KPluginMetaData &data, const QVariantList &args)
    : Plasma::Containment(parentObject, data, args)
{
    // updateConstraints(Plasma::Types::UiReadyConstraint);
    m_timer.setSingleShot(true);
    m_timer.setInterval(QRandomGenerator::global()->bounded((500 + 1) - 100) + 100);
    m_timer.start();
    connect(&m_timer, &QTimer::timeout, [=]() {
        updateConstraints(Plasma::Types::UiReadyConstraint);
    });
}

SimpleNoScreenContainment::SimpleNoScreenContainment(QObject *parentObject, const KPluginMetaData &data, const QVariantList &args)
    : Plasma::Containment(parentObject, data, args)
{
    // This containment will *never* be isUiReady()
}

void CoronaTest::initTestCase()
{
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

    const auto containments = m_corona->containments();
    for (auto cont : containments) {
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

    // check containments order
    QCOMPARE(m_corona->containments().at(0)->id(), (uint)1);
    QCOMPARE(m_corona->containments().at(1)->id(), (uint)4);
    QCOMPARE(m_corona->containments().at(2)->id(), (uint)5);

    // check applets order
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

    // remove action present
    QVERIFY(m_corona->containments().at(0)->applets().at(0)->actions()->action(QStringLiteral("remove")));
    // kill an applet
    m_corona->containments().at(0)->applets().at(0)->destroy();

    QSignalSpy spy(m_corona->containments().at(0)->applets().at(0), SIGNAL(destroyed()));
    QVERIFY(spy.wait(1000));
    QCOMPARE(m_corona->containments().at(0)->applets().count(), 2);
}

// this test has to be the last, since systemimmutability
// can't be programmatically unlocked
void CoronaTest::immutability()
{
    // immutability
    QCOMPARE(m_corona->immutability(), Plasma::Types::Mutable);
    m_corona->setImmutability(Plasma::Types::UserImmutable);
    QCOMPARE(m_corona->immutability(), Plasma::Types::UserImmutable);

    auto containments = m_corona->containments();
    for (Plasma::Containment *cont : std::as_const(containments)) {
        QCOMPARE(cont->immutability(), Plasma::Types::UserImmutable);
        const auto lstApplets = cont->applets();
        for (Plasma::Applet *app : lstApplets) {
            QCOMPARE(app->immutability(), Plasma::Types::UserImmutable);
        }
    }

    m_corona->setImmutability(Plasma::Types::Mutable);
    QCOMPARE(m_corona->immutability(), Plasma::Types::Mutable);

    containments = m_corona->containments();
    for (Plasma::Containment *cont : std::as_const(containments)) {
        QCOMPARE(cont->immutability(), Plasma::Types::Mutable);
        const auto lstApplets = cont->applets();
        for (Plasma::Applet *app : lstApplets) {
            QCOMPARE(app->immutability(), Plasma::Types::Mutable);
        }
    }

    m_corona->setImmutability(Plasma::Types::SystemImmutable);
    QCOMPARE(m_corona->immutability(), Plasma::Types::SystemImmutable);

    containments = m_corona->containments();
    for (Plasma::Containment *cont : std::as_const(containments)) {
        QCOMPARE(cont->immutability(), Plasma::Types::SystemImmutable);
        const auto lstApplets = cont->applets();
        for (Plasma::Applet *app : lstApplets) {
            QCOMPARE(app->immutability(), Plasma::Types::SystemImmutable);
        }
    }

    // can't unlock systemimmutable
    m_corona->setImmutability(Plasma::Types::Mutable);
    QCOMPARE(m_corona->immutability(), Plasma::Types::SystemImmutable);

    containments = m_corona->containments();
    for (Plasma::Containment *cont : std::as_const(containments)) {
        QCOMPARE(cont->immutability(), Plasma::Types::SystemImmutable);
        const auto lstApplets = cont->applets();
        for (Plasma::Applet *app : lstApplets) {
            QCOMPARE(app->immutability(), Plasma::Types::SystemImmutable);
        }
    }
}

QTEST_MAIN(CoronaTest)
