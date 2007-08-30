/*
 *   Copyright 2007 by Matias Valdenegro T. <mvaldenegro@informatica.utem.cl>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "vboxlayout.h"

namespace Plasma
{

VBoxLayout::VBoxLayout(LayoutItem *parent)
    : BoxLayout(Qt::Vertical, parent),
      d(0)
{
}

VBoxLayout::~VBoxLayout()
{
}

bool VBoxLayout::hasHeightForWidth() const
{
    return true;
}

qreal VBoxLayout::heightForWidth(qreal w) const
{
    Q_UNUSED(w);
    return qreal();
}

}
