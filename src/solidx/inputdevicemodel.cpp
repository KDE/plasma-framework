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
#include "inputdevicemodel_p.h"

#include "utils/d_ptr_implementation.h"

#include "backends/xlib/xlibinputdevicebackend.h"
#include "backends/fake/fakeinputdevicebackend.h"

#include <QDebug>

namespace solidx {

InputDeviceModel::Private::Private(InputDeviceModel * parent)
    : q(parent)
{
    backends << new backends::xlib::XlibInputDeviceBackend(this);
    backends << new backends::fake::FakeInputDeviceBackend(this);

    foreach (auto backend, backends) {
        connect(backend, SIGNAL(addedDevice(QString)),
            this, SLOT(addedDevice(QString)));
        connect(backend, SIGNAL(removedDevice(QString)),
            this, SLOT(removedDevice(QString)));
    }
}

void InputDeviceModel::Private::addedDevice(const QString & id)
{
    if (devices.contains(id)) return;

    // TODO: Do this more efficient, and report
    // adding a specific model item, not resetting it
    q->beginResetModel();
    devices << id;
    q->endResetModel();

}

void InputDeviceModel::Private::removedDevice(const QString & id)
{
    if (!devices.contains(id)) return;

    // TODO: Do this more efficient, and report
    // erasing a specific model item, not resetting it
    q->beginResetModel();
    devices.removeAll(id);
    q->endResetModel();
}

void InputDeviceModel::Private::reload()
{
    q->beginResetModel();

    foreach (auto backend, backends) {
        foreach (auto device, backend->devices()) {
            addedDevice(device);
        }
    }

    q->endResetModel();
}

InputDeviceModel::InputDeviceModel(QObject * parent)
    : d(this)
{
    d->reload();
    qDebug() << "Gaga!" << d->devices;
}

InputDeviceModel::~InputDeviceModel()
{
}


int InputDeviceModel::rowCount(const QModelIndex & parent) const
{
    if (parent.isValid()) return 0;

    return d->devices.size();
}

QModelIndex InputDeviceModel::sibling(int row, int column, const QModelIndex & index) const
{
    if (!index.isValid() || column != 0 || row >= 1)
        return QModelIndex();

    return createIndex(row, 0);
}

QVariant InputDeviceModel::data(const QModelIndex & index, int role) const
{
    if (index.row() < 0 || index.row() >= d->devices.size())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole)
        return d->devices.at(index.row());

    return QVariant();
}

void InputDeviceModel::setType(InputDevice::Type type)
{
    d->type = type;
}

InputDevice::Type InputDeviceModel::type() const
{
    return d->type;
}

void InputDeviceModel::setSubtype(InputDevice::Subtype subtype)
{
    d->subtype = subtype;
}

InputDevice::Subtype InputDeviceModel::subtype() const
{
    return d->subtype;
}



} // namespace solidx


