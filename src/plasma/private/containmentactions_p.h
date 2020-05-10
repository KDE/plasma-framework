/*
 *   Copyright (c) 2009 Chani Armitage <chani@kde.org>
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

#ifndef PLASMA_CONTAINMENTACTIONSPRIVATE_H
#define PLASMA_CONTAINMENTACTIONSPRIVATE_H

#include <KPluginMetaData>

namespace Plasma
{

#if KSERVICE_BUILD_DEPRECATED_SINCE(5, 0)
static KPluginMetaData pluginMetaDatafromArg(const QVariant& arg)
{
    if (arg.canConvert<KPluginMetaData>()) {
        return arg.value<KPluginMetaData>();
    }

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wdeprecated-declarations")
QT_WARNING_DISABLE_GCC("-Wdeprecated-declarations")
    return KPluginInfo(KService::serviceByStorageId(arg.toString())).toMetaData();
QT_WARNING_POP
}
#endif

class ContainmentActionsPrivate
{
public:
    ContainmentActionsPrivate(const QVariant& arg, ContainmentActions */*containmentActions*/)
#if KSERVICE_BUILD_DEPRECATED_SINCE(5, 0)
        : containmentActionsDescription(pluginMetaDatafromArg(arg))
#else
        : containmentActionsDescription(arg.value<KPluginMetaData>())
#endif
    {
    }

    const KPluginMetaData containmentActionsDescription;
    Containment *containment = nullptr;
};

} // namespace Plasma
#endif //PLASMA_CONTAINMENTACTIONSPRIVATE_H

