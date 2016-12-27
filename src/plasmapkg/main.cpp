/*
 *   Copyright 2016 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
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

/**
 * This binary is deprecated, please refer to kpackagetool5 within the KPackage framework
 */

#include <QCoreApplication>
#include <QProcess>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QProcess p;
    p.setProcessChannelMode(QProcess::ForwardedChannels);
    p.start("kpackagetool5", app.arguments().mid(1));
    p.waitForFinished();

    return p.exitCode();
}

