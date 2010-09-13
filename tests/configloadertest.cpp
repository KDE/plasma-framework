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

#include "configloadertest.h"

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kconfigskeleton.h>

#include "plasma/configloader.h"

Q_DECLARE_METATYPE(QList<int>);

#define TEST_NAME QString::fromLatin1("configloadertest")

#define GET_CONFIG_ITEM_VALUE(type, configName) \
    KConfigSkeletonItem* item = cl->findItem(TEST_NAME, configName); \
    /* Check if we got back a valid item. */ \
    QVERIFY(item != 0); \
    /* Cast the item to the given type. */ \
    type typeItem = dynamic_cast<type>(item); \
    /* Make sure the cast was successful. */ \
    QVERIFY(typeItem != 0);

void ConfigLoaderTest::init()
{
    QString fileName = TEST_NAME + QString::fromLatin1(".xml");
    configFile = new QFile(QString::fromLatin1(KDESRCDIR) + QString::fromLatin1("/") + fileName);
    cl = new Plasma::ConfigLoader(configFile->fileName(), configFile);
}

void ConfigLoaderTest::cleanup()
{
    delete cl;
    delete configFile;
}

void ConfigLoaderTest::boolDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemBool*, "DefaultBoolItem");

    QVERIFY(typeItem->isEqual(true));
}

void ConfigLoaderTest::colorDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KConfigSkeleton::ItemColor*, "DefaultColorItem");

    QVERIFY(typeItem->isEqual(QColor("#00FF00")));
}

void ConfigLoaderTest::dateTimeDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemDateTime*, "DefaultDateTimeItem");

    QVERIFY(typeItem->isEqual(QDateTime::fromString("Thu Sep 09 2010")));
}

void ConfigLoaderTest::enumDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KConfigSkeleton::ItemEnum*, "DefaultEnumItem");

    QVERIFY(typeItem->isEqual(3));
}

void ConfigLoaderTest::fontDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KConfigSkeleton::ItemFont*, "DefaultFontItem");

    QVERIFY(typeItem->isEqual(QFont("DejaVu Sans")));
}

void ConfigLoaderTest::intDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemInt*, "DefaultIntItem");

    QVERIFY(typeItem->isEqual(27));
}

void ConfigLoaderTest::passwordDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KConfigSkeleton::ItemPassword*, "DefaultPasswordItem");

    QVERIFY(typeItem->isEqual(QString::fromLatin1("h4x.")));
}

void ConfigLoaderTest::pathDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KConfigSkeleton::ItemPath*, "DefaultPathItem");

    QVERIFY(typeItem->isEqual(QString::fromLatin1("/dev/null")));
}

void ConfigLoaderTest::stringDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KConfigSkeleton::ItemString*, "DefaultStringItem");

    QVERIFY(typeItem->isEqual(QString::fromLatin1("TestString")));
}

void ConfigLoaderTest::stringListDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KConfigSkeleton::ItemStringList*, "DefaultStringListItem");

    // Create a string list with the expected values.
    QStringList expected;
    expected.append("One");
    expected.append("Two");
    expected.append("Three");
    expected.append("Four");
    expected.append("Five");

    QVERIFY(typeItem->isEqual(expected));
}

void ConfigLoaderTest::uintDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemUInt*, "DefaultUIntItem");

    QVERIFY(typeItem->isEqual(7U));
}

void ConfigLoaderTest::urlDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemUrl*, "DefaultUrlItem");

    QVERIFY(typeItem->isEqual(KUrl("http://kde.org")));
}

void ConfigLoaderTest::doubleDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemDouble*, "DefaultDoubleItem");

    QVERIFY(typeItem->isEqual(13.37));
}

void ConfigLoaderTest::intListDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemIntList*, "DefaultIntListItem");

    // Create a int list with the expected values.
    QList<int> expected;
    expected.append(1);
    expected.append(1);
    expected.append(2);
    expected.append(3);
    expected.append(5);
    expected.append(8);

    QVERIFY(typeItem->isEqual(qVariantFromValue(expected)));
}

void ConfigLoaderTest::longLongDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemLongLong*, "DefaultLongLongItem");

    QVERIFY(typeItem->isEqual(Q_INT64_C(-9211372036854775808)));
}

void ConfigLoaderTest::pointDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemPoint*, "DefaultPointItem");

    QVERIFY(typeItem->isEqual(QPoint(185, 857)));
}

void ConfigLoaderTest::rectDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemRect*, "DefaultRectItem");

    // Create a new QRect with the expected value.
    QRect expected;
    expected.setCoords(3, 7, 951, 358);

    QVERIFY(typeItem->isEqual(expected));
}

void ConfigLoaderTest::sizeDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemSize*, "DefaultSizeItem");

    QVERIFY(typeItem->isEqual(QSize(640, 480)));
}

void ConfigLoaderTest::ulongLongDefaultValue()
{
    GET_CONFIG_ITEM_VALUE(KCoreConfigSkeleton::ItemULongLong*, "DefaultULongLongItem");

    QVERIFY(typeItem->isEqual(Q_UINT64_C(9223372036854775806)));
}


QTEST_KDEMAIN(ConfigLoaderTest, NoGUI)

//#include "configloadertest.moc"

