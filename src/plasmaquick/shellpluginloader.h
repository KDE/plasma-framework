/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SHELLPLUGINLOADER_H
#define SHELLPLUGINLOADER_H

#include <Plasma/PluginLoader>

#include <plasmaquick/plasmaquick_export.h>

#if PLASMAQUICK_ENABLE_DEPRECATED_SINCE(5, 25)

/**
 * @deprecated Since 5.25, use Plasma::PluginLoader
 */
class PLASMAQUICK_EXPORT ShellPluginLoader : public Plasma::PluginLoader
{
public:
    PLASMAQUICK_DEPRECATED_VERSION(5, 25, "Use PluginLoader")
    ShellPluginLoader();
    ~ShellPluginLoader() override;

    static void init();

protected:
    Plasma::Package internalLoadPackage(const QString &packageFormat, const QString &specialization) override;
};

#endif // PLASMAQUICK_ENABLE_DEPRECATED_SINCE(5, 25)

#endif
