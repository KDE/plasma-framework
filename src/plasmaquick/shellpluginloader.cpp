/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "shellpluginloader.h"

#if PLASMAQUICK_BUILD_DEPRECATED_SINCE(5, 25)
#include <KPackage/PackageLoader>
#include <QDebug>

ShellPluginLoader::ShellPluginLoader()
    : Plasma::PluginLoader() /*, when BC can be broken, these should become class members
       m_lnfPackage(0),
       m_qmlPackage(0),
       m_layoutPackage(0)*/
{
    Q_ASSERT(false);
}

ShellPluginLoader::~ShellPluginLoader()
{
}

Plasma::Package ShellPluginLoader::internalLoadPackage(const QString & /*packageFormat*/, const QString & /*specialization*/)
{
    Q_ASSERT(false);
    return Plasma::Package();
}

void ShellPluginLoader::init()
{
    Plasma::PluginLoader::setPluginLoader(new ShellPluginLoader);
}
#endif
