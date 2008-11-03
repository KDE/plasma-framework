/*  This file is part of the KDE project
    Copyright (C) 2007 John Tapsell <tapsell@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#include <QtTest>
#include <QtCore>

#include <klocale.h>
#include <kdebug.h>
#include <qtest_kde.h>

#include <processui/ksysguardprocesslist.h>

#include "guitest.h"

void testGuiProcess::testGUI() {
    KSysGuardProcessList processlist(NULL);

    QTime t;
    t.start();

    for(int i =0; i < 10; i++) {
      processlist.updateList();
    }
    kDebug() << "time taken: " << t.elapsed() << "ms";
}

QTEST_KDEMAIN(testGuiProcess, GUI)

#include "guitest.moc"

