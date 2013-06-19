/*
 *   Copyright (C) 2013 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "inputdevice.h"
#include "sharedsingleton_p.h"
#include "connection.h"

#include <QDebug>

#include "inputdevice_p.h"

namespace backends {
namespace xlib {

InputDeviceListModel::InputDeviceListModel(QObject * parent)
    : QAbstractListModel(parent), d(Private::instance())
{
}

InputDeviceListModel::~InputDeviceListModel()
{
}

int InputDeviceListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;

    return 1;
}

QModelIndex InputDeviceListModel::sibling(int row, int column, const QModelIndex &idx) const
{
    if (!idx.isValid() || column != 0 || row >= 1)
        return QModelIndex();

    return createIndex(row, 0);
}

QVariant InputDeviceListModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= 1)
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole)
        return "asd";

    return QVariant();

}


} // namespace xlib
} // namespace backends
