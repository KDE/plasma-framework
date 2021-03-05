/*
    SPDX-FileCopyrightText: 2012 Aurélien Gâteau <agateau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
#include <sortfiltermodeltest.h>

#include <declarativeimports/core/datamodel.h>

// KDE

// Qt
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QTest>

using namespace Plasma;

QTEST_MAIN(SortFilterModelTest)

void SortFilterModelTest::setModel()
{
    // TODO: Actually test model change
    QStandardItemModel model;

    SortFilterModel filterModel;
    QSignalSpy spy(&filterModel, SIGNAL(sourceModelChanged(QObject *)));

    filterModel.setModel(&model);
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).value<QObject *>(), static_cast<QObject *>(&model));

    filterModel.setModel(&model);
    QCOMPARE(spy.count(), 0);
}

void SortFilterModelTest::setEmptyModel()
{
    SortFilterModel filterModel;
    QStandardItemModel model;
    filterModel.setModel(&model);
    QCOMPARE(filterModel.sourceModel(), static_cast<QAbstractItemModel *>(&model));
    filterModel.setModel(nullptr);
    QCOMPARE(filterModel.sourceModel(), static_cast<QAbstractItemModel *>(nullptr));
}

void SortFilterModelTest::setFilterRegExp()
{
    // TODO: Actually test filtering
    SortFilterModel filterModel;
    QSignalSpy spy(&filterModel, SIGNAL(filterRegExpChanged(QString)));

    filterModel.setFilterRegExp(QStringLiteral("foo"));
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), QStringLiteral("foo"));

    filterModel.setFilterRegExp(QStringLiteral("foo"));
    QCOMPARE(spy.count(), 0);
}

void SortFilterModelTest::mapRowToSource()
{
    QStringList list = QStringList() << QStringLiteral("Foo") << QStringLiteral("Bar") << QStringLiteral("Baz");
    QStringListModel model(list);

    SortFilterModel filterModel;
    filterModel.setSourceModel(&model);

    QCOMPARE(filterModel.mapRowToSource(0), 0);
    QCOMPARE(filterModel.mapRowToSource(2), 2);
    QCOMPARE(filterModel.mapRowToSource(3), -1);
    QCOMPARE(filterModel.mapRowToSource(-1), -1);

    filterModel.setFilterRegExp(QStringLiteral("Ba"));
    // filterModel now contains "Bar" and "Baz"
    QCOMPARE(filterModel.mapRowToSource(0), 1);
    QCOMPARE(filterModel.mapRowToSource(1), 2);
    QCOMPARE(filterModel.mapRowToSource(2), -1);
    QCOMPARE(filterModel.mapRowToSource(-1), -1);
}

void SortFilterModelTest::mapRowFromSource()
{
    QStringList list = QStringList() << QStringLiteral("Foo") << QStringLiteral("Bar") << QStringLiteral("Baz");
    QStringListModel model(list);

    SortFilterModel filterModel;
    filterModel.setSourceModel(&model);

    QCOMPARE(filterModel.mapRowFromSource(0), 0);
    QCOMPARE(filterModel.mapRowFromSource(2), 2);
    QCOMPARE(filterModel.mapRowFromSource(3), -1);
    QCOMPARE(filterModel.mapRowFromSource(-1), -1);

    filterModel.setFilterRegExp(QStringLiteral("Ba"));
    // filterModel now contains "Bar" and "Baz"
    QCOMPARE(filterModel.mapRowFromSource(0), -1);
    QCOMPARE(filterModel.mapRowFromSource(1), 0);
    QCOMPARE(filterModel.mapRowFromSource(2), 1);
    QCOMPARE(filterModel.mapRowFromSource(-1), -1);
}
