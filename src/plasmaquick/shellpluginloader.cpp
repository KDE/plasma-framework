/*
 *   Copyright 2013 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "shellpluginloader.h"

#include "private/packages.h"

#include <QDebug>

// when BC can be broken, these should become class members
static Plasma::PackageStructure *m_lnfPackage = 0;
static Plasma::PackageStructure *m_qmlPackage = 0;
static Plasma::PackageStructure *m_layoutPackage = 0;

ShellPluginLoader::ShellPluginLoader()
    : Plasma::PluginLoader()/*, when BC can be broken, these should become class members
      m_lnfPackage(0),
      m_qmlPackage(0),
      m_layoutPackage(0)*/
{
}

ShellPluginLoader::~ShellPluginLoader()
{
    delete m_lnfPackage;
    m_lnfPackage = 0;
    delete m_qmlPackage;
    m_qmlPackage = 0;
    delete m_layoutPackage;
    m_layoutPackage = 0;
}

Plasma::Package ShellPluginLoader::internalLoadPackage(const QString &packageFormat, const QString &specialization)
{
    Q_UNUSED(specialization)

    if (packageFormat == "Plasma/LookAndFeel") {
        if (!m_lnfPackage) {
            m_lnfPackage = new LookAndFeelPackage();
        }

        return Plasma::Package(m_lnfPackage);
    } else if (packageFormat == "Plasma/Wallpaper") {
        if (!m_qmlPackage) {
            m_qmlPackage = new QmlWallpaperPackage();
        }

        return Plasma::Package(m_qmlPackage);
    } else if (packageFormat == "Plasma/LayoutTemplate") {
        if (!m_layoutPackage) {
            m_layoutPackage = new LayoutTemplatePackage();
        }

        return Plasma::Package(m_layoutPackage);
    } else {
        return Plasma::Package();
    }
}

void ShellPluginLoader::init()
{
    Plasma::PluginLoader::setPluginLoader(new ShellPluginLoader);
}

