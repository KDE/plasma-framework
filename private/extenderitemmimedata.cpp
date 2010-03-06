/*
 * Copyright 2008 by Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include "extenderitemmimedata_p.h"

#include "plasma/extenders/extenderitem.h"

#include <QMimeData>

namespace Plasma
{

ExtenderItemMimeData::ExtenderItemMimeData() : QMimeData()
{
}

ExtenderItemMimeData::~ExtenderItemMimeData()
{
}

QStringList ExtenderItemMimeData::formats() const
{
    QStringList list;
    list.append(mimeType());
    return list;
}

bool ExtenderItemMimeData::hasFormat(const QString &type) const
{
    return (type == mimeType());
}

void ExtenderItemMimeData::setExtenderItem(ExtenderItem *item)
{
    m_extenderItem = item;
}

void ExtenderItemMimeData::setPointerOffset(const QPoint &p)
{
    m_offset = p;
}

QPoint ExtenderItemMimeData::pointerOffset() const
{
    return m_offset;
}

ExtenderItem *ExtenderItemMimeData::extenderItem() const
{
    return m_extenderItem;
}

QString ExtenderItemMimeData::mimeType()
{
    return "plasma/extenderitem";
}

} // namespace Plasma

