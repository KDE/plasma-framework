// SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include <QTest>
#include <plasmaquick/sharedqmlengine.h>

using namespace PlasmaQuick;

class SharedQmlEngineTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testSettingTranslationDomain()
    {
        std::unique_ptr<SharedQmlEngine> obj(new SharedQmlEngine());

        const QString testDomain = QStringLiteral("testme");
        QVERIFY(obj->translationDomain().isEmpty());
        obj->setTranslationDomain(testDomain);
        QCOMPARE(obj->translationDomain(), testDomain);
        obj.reset(new SharedQmlEngine());
        QVERIFY(obj->translationDomain().isEmpty());
    }

    void testUsingSameEngine()
    {
        std::unique_ptr<SharedQmlEngine> obj1(new SharedQmlEngine());
        std::unique_ptr<SharedQmlEngine> obj2(new SharedQmlEngine());

        QVERIFY(obj1->engine() == obj2->engine());
        QVERIFY(obj1->rootContext() != obj2->rootContext());
    }

    void testDeletingEngine()
    {
        std::unique_ptr<SharedQmlEngine> obj1(new SharedQmlEngine());
        std::weak_ptr<QQmlEngine> weakPtr(obj1->engine());
        QVERIFY(weakPtr.lock());

        // The static shared_ptr and the one in obj1
        QCOMPARE(weakPtr.use_count(), 2);

        {
            std::unique_ptr<SharedQmlEngine> obj2(new SharedQmlEngine());
            // The static shared_ptr, the one in obj1 and in obj2
            QCOMPARE(weakPtr.use_count(), 3);
        }

        obj1.reset(nullptr);
        // Our object is deleted, the static pointer should be reset
        QVERIFY(!weakPtr.lock());
        QCOMPARE(weakPtr.use_count(), 0);
    }
};

QTEST_MAIN(SharedQmlEngineTest)

#include "sharedqmlenginetest.moc"
