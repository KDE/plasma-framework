/*
 *   Copyright 2008 by Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "abstracttoolbox.h"

#include "containment.h"

#include <kservicetypetrader.h>

namespace Plasma
{

class AbstractToolBoxPrivate
{
public:
    AbstractToolBoxPrivate(Containment *c)
      : containment(c)
    {}

    Containment *containment;
};

AbstractToolBox::AbstractToolBox(Containment *parent)
    : QGraphicsWidget(parent),
      d(new AbstractToolBoxPrivate(parent))
{
}

AbstractToolBox::AbstractToolBox(QObject *parent, const QVariantList &args)
   : QGraphicsWidget(dynamic_cast<QGraphicsItem *>(parent)),
     d(new AbstractToolBoxPrivate(qobject_cast<Containment *>(parent)))
{
    if (!parentItem()) {
        setParent(parent);
    }
}

AbstractToolBox::~AbstractToolBox()
{
    delete d;
}

AbstractToolBox *AbstractToolBox::load(const QString &name, const QVariantList &args, Plasma::Containment *containment)
{
    const QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(name);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/ToolBox", constraint);

    if (!offers.isEmpty()) {
        KService::Ptr offer = offers.first();

        KPluginLoader plugin(*offer);

        if (!Plasma::isPluginVersionCompatible(plugin.pluginVersion())) {
            return 0;
        }

        return offer->createInstance<AbstractToolBox>(containment, args);
    } else {
        return 0;
    }
}

Containment *AbstractToolBox::containment() const
{
    return d->containment;
}

} // plasma namespace

#include "abstracttoolbox.moc"

