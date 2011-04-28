/********************************************************************************
*   Copyright 2010 by Martin Blumenstingl <darklight.xdarklight@googlemail.com> *
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

#include "storagetest.h"

#include "../private/storage_p.h"

void StorageTest::init()
{
    m_data.insert("String 1", "Fork");
    m_data.insert("String 2", "Spoon");
    m_data.insert("String 3", "Knife");
    m_data.insert("Int 1", 3141);
    m_data.insert("Int 2", 60);
    QByteArray bytes("yadda yadda yadda");
    m_data.insert("Binary Data", bytes);
}

void StorageTest::store()
{
    Storage storage;
    KConfigGroup op = storage.operationDescription("save");
    op.writeEntry("group", "Test");
    Plasma::ServiceJob *job = storage.startOperationCall(op);
    StorageJob *storageJob = qobject_cast<StorageJob *>(job);

    QVERIFY(storageJob);
    if (storageJob) {
        storageJob->setData(m_data);
        QVERIFY(storageJob->exec());
        QVERIFY(storageJob->result().toBool());
    }
}

void StorageTest::retrieve()
{
    Storage storage;
    KConfigGroup op = storage.operationDescription("retrieve");
    op.writeEntry("group", "Test");
    Plasma::ServiceJob *job = storage.startOperationCall(op);
    StorageJob *storageJob = qobject_cast<StorageJob *>(job);

    QVERIFY(storageJob);
    if (storageJob) {
        QVERIFY(storageJob->exec());
        QVERIFY(storageJob->result().type() != QVariant::Bool);
        QCOMPARE(storageJob->data(), m_data);
    }
}

QTEST_KDEMAIN(StorageTest, NoGUI)

