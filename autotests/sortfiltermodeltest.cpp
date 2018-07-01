/*
 * Copyright 2012 Aurélien Gâteau <agateau@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#include <sortfiltermodeltest.h>

#include <declarativeimports/core/datamodel.h>

// KDE

// Qt
#include <QStandardItemModel>
#include <QStringListModel>
#include <QSignalSpy>
#include <QtTest>

using namespace Plasma;

QTEST_MAIN(SortFilterModelTest)

void SortFilterModelTest::setModel()
{
    // TODO: Actually test model change
    QStandardItemModel model;

    SortFilterModel filterModel;
    QSignalSpy spy(&filterModel, SIGNAL(sourceModelChanged(QObject*)));

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

