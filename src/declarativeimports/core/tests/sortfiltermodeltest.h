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
