/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
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

#include <KSharedConfig>
#include <KStandardDirs>

#include "theme.h"

namespace Plasma
{

class Theme::Private
{
    public:
        Private()
            : themeName("default")
        {
        }

    QString themeName;
};

Theme::Theme(QObject* parent)
    : QObject(parent),
      d(new Private)
{
    KConfig config("plasma");
    KConfigGroup group(&config, "Theme");
    d->themeName = group.readEntry("name", d->themeName);
}

Theme::~Theme()
{
}

QString Theme::themeName() const
{
    return d->themeName;
}

QString Theme::imagePath(const QString& name) const
{
    return KStandardDirs::locate("data", "desktoptheme/" + d->themeName
                                          + "/" + name + ".svg");
}

}

#include <theme.moc>
