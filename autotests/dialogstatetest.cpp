/********************************************************************************
*   Copyright 2016 Eike Hein <hein@kde.org>                                  *
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

#include "dialogstatetest.h"

#include <KWindowSystem>

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
    for (int i = 0; i <= 100; ++i) {
        m_dialog->show();

        QSignalSpy windowAddedSpy(KWindowSystem::self(), &KWindowSystem::windowAdded);
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

        QSignalSpy windowRemovedSpy(KWindowSystem::self(), &KWindowSystem::windowRemoved);
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
