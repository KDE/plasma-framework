/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SHELLPACKAGES_H
#define SHELLPACKAGES_H

#include <Plasma/PackageStructure>
#include <Plasma/Plasma>

class LookAndFeelPackage : public Plasma::PackageStructure
{
    Q_OBJECT
public:
    void initPackage(Plasma::Package *package) override;
};

class QmlWallpaperPackage : public Plasma::PackageStructure
{
    Q_OBJECT
public:
    void initPackage(Plasma::Package *package) override;
};

class LayoutTemplatePackage : public Plasma::PackageStructure
{
    Q_OBJECT
public:
    void initPackage(Plasma::Package *package) override;
};

#endif // LOOKANDFEELPACKAGE_H
