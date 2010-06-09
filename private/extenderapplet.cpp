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

#include "extenderapplet_p.h"

#include "plasma/extenders/extender.h"
#include "plasma/extenders/extenderitem.h"

#include <QGraphicsLinearLayout>

namespace Plasma
{

ExtenderApplet::ExtenderApplet(QObject *parent, const QVariantList &args)
    : PopupApplet(parent, args)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

ExtenderApplet::~ExtenderApplet()
{
    if (destroyed()) {
        disconnect(extender(), SIGNAL(itemDetached(Plasma::ExtenderItem*)),
                   this, SLOT(itemDetached(Plasma::ExtenderItem*)));
        foreach (ExtenderItem *item, extender()->attachedItems()) {
            item->returnToSource();
        }
    }
}

void ExtenderApplet::init()
{
    setPopupIcon("utilities-desktop-extra");

    extender()->setAppearance(Extender::NoBorders);

    connect(extender(), SIGNAL(itemDetached(Plasma::ExtenderItem*)),
            this, SLOT(itemDetached(Plasma::ExtenderItem*)));
    connect(extender(), SIGNAL(geometryChanged()),
            this, SLOT(extenderGeometryChanged()));
}

void ExtenderApplet::itemDetached(Plasma::ExtenderItem *)
{
    if (extender()->attachedItems().isEmpty()) {
        destroy();
    }
}

void ExtenderApplet::extenderGeometryChanged()
{
    if (formFactor() != Plasma::Horizontal &&
        formFactor() != Plasma::Vertical) {

        qreal left, top, right, bottom;
        getContentsMargins(&left, &top, &right, &bottom);
        QSizeF margins(left + right, top + bottom);

        setMinimumSize(extender()->minimumSize() + margins);
        setMaximumSize(extender()->maximumSize() + margins);
        setPreferredSize(extender()->preferredSize() + margins);

        updateGeometry();
    }
}

} // namespace Plasma

#include "extenderapplet_p.moc"

