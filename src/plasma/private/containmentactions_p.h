/*
    SPDX-FileCopyrightText: 2009 Chani Armitage <chani@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_CONTAINMENTACTIONSPRIVATE_H
#define PLASMA_CONTAINMENTACTIONSPRIVATE_H

#include <KPluginMetaData>

#ifndef KSERVICE_BUILD_DEPRECATED_SINCE
#define KSERVICE_BUILD_DEPRECATED_SINCE(a, b) 0
#endif

namespace Plasma
{
#if KSERVICE_BUILD_DEPRECATED_SINCE(5, 0)
static KPluginMetaData pluginMetaDatafromArg(const QVariant &arg)
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
    ContainmentActionsPrivate(const QVariant &arg, ContainmentActions * /*containmentActions*/)
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
#endif // PLASMA_CONTAINMENTACTIONSPRIVATE_H
