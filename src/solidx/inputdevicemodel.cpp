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

#include "inputdevicemodel.h"

#include "utils/d_ptr_implementation.h"

namespace solidx {

class InputDeviceModel::Private {
public:

};

InputDeviceModel::InputDeviceModel(QObject * parent)
{
}

InputDeviceModel::InputDeviceModel(InputDevice::Type type,
        InputDevice::Subtype subtype,
        QObject * parent
    )
{
}

InputDeviceModel::~InputDeviceModel()
{
}


int InputDeviceModel::rowCount(const QModelIndex & parent) const
{
    if (parent.isValid()) return 0;

    return 1;
}

QModelIndex InputDeviceModel::sibling(int row, int column, const QModelIndex & index) const
{
    if (!index.isValid() || column != 0 || row >= 1)
        return QModelIndex();

    return createIndex(row, 0);
}

QVariant InputDeviceModel::data(const QModelIndex & index, int role) const
{
    if (index.row() < 0 || index.row() >= 1)
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole)
        return "asd";

    return QVariant();
}


} // namespace solidx


