/*  This file is part of the KDE project
    Copyright (C) 2006 Will Stephenson <wstephenson@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

// KDE-QTestLib include
#include <qtest_kde.h>

#include "fakenetworkmanager.h"

#include "fakenetworktest.h"

QTEST_KDEMAIN_CORE(FakeNetworking_Test)

void FakeNetworking_Test::testFakeNetworking()
{
    FakeNetworkManager *fnm = new FakeNetworkManager(0, QStringList(), TEST_DATA);
    QVERIFY(!fnm->networkInterfaces().isEmpty());
    QVERIFY(fnm->createNetworkInterface("/org/kde/solid/fakenet/eth0") != 0);
    QVERIFY(fnm->createNetworkInterface("/org/kde/wobbly/jellynet/eth0") == 0);
    delete fnm;
}

#include "fakenetworktest.moc"
