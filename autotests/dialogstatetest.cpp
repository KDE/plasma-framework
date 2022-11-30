/*
    SPDX-FileCopyrightText: 2016 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dialogstatetest.h"
#include <KWindowInfo>
#include <KX11Extras>

#include <QSignalSpy>

void DialogStateTest::initTestCase()
{
    m_dialog = new PlasmaQuick::Dialog;
    m_dialog->setLocation(Plasma::Types::TopEdge);
    m_dialog->setGeometry(0, 0, 50, 50);

    qRegisterMetaType<WId>("WId");
}

void DialogStateTest::cleanupTestCase()
{
    delete m_dialog;
}

void DialogStateTest::windowState()
{
    if (QGuiApplication::platformName() == "wayland") {
        QEXPECT_FAIL("windowState", "KX11Extras::windowAdded doesn't work on wayland", Continue);
        return;
    }

    for (int i = 0; i <= 100; ++i) {
        m_dialog->show();

        QSignalSpy windowAddedSpy(KX11Extras::self(), &KX11Extras::windowAdded);
        QVERIFY(windowAddedSpy.isValid());
        QVERIFY(windowAddedSpy.wait());

        bool windowAdded = false;

        while (windowAddedSpy.count()) {
            const QVariantList &arguments = windowAddedSpy.takeFirst();

            if (arguments.at(0).value<WId>() == m_dialog->winId()) {
                windowAdded = true;
                break;
            }
            if (windowAddedSpy.isEmpty()) {
                QVERIFY(windowAddedSpy.wait());
            }
        }

        QVERIFY(windowAdded);

        QVERIFY(verifyState(m_dialog));

        m_dialog->hide();

        QSignalSpy windowRemovedSpy(KX11Extras::self(), &KX11Extras::windowRemoved);
        QVERIFY(windowRemovedSpy.isValid());
        QVERIFY(windowRemovedSpy.wait());

        bool windowRemoved = false;

        while (windowRemovedSpy.count()) {
            const QVariantList &arguments = windowRemovedSpy.takeFirst();

            if (arguments.at(0).value<WId>() == m_dialog->winId()) {
                windowRemoved = true;
                break;
            }
        }

        QVERIFY(windowRemoved);
    }
}

bool DialogStateTest::verifyState(PlasmaQuick::Dialog *dialog) const
{
    KWindowInfo info(dialog->winId(), NET::WMState);

    if (!(info.state() & NET::SkipTaskbar)) {
        return false;
    }

    if (!info.hasState(NET::SkipPager)) {
        return false;
    }

    return true;
}

QTEST_MAIN(DialogStateTest)
