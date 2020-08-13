/*
    SPDX-FileCopyrightText: 2012 Aurélien Gâteau <agateau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
#ifndef SORTFILTERMODELTEST_H
#define SORTFILTERMODELTEST_H

// Qt
#include <QObject>

class SortFilterModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void setFilterRegExp();
    void setModel();
    void setEmptyModel();
    void mapRowToSource();
    void mapRowFromSource();
};

#endif /* SORTFILTERMODELTEST_H */
