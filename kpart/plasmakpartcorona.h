/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2010 Ryan Rix <ry@n.rix.si>
 *   Copyright 2010 Siddharth Sharma <siddharth.kde@gmail.com>
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

#ifndef PLASMAKPARTCORONA_H
#define PLASMAKPARTCORONA_H

#include <Plasma/Corona>

namespace Plasma
{
    class Applet;
} // namespace Plasma

/**
 * @short A Corona with dashboard-y considerations
 */
class PlasmaKPartCorona : public Plasma::Corona
{
Q_OBJECT
public:
    PlasmaKPartCorona(QObject* parent);

    Plasma::Containment *containment();

    void loadDefaultLayout();

private:
    void evaluateScripts(const QStringList &scripts);
    void init();

private Q_SLOTS:
    void printScriptError(const QString &error);
    void printScriptMessage(const QString &error);

private:
    Plasma::Applet *loadDefaultApplet(const QString &pluginName, Plasma::Containment *c);
};

#endif
