/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>
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

#include <QScreen>
#include <plasma/theme.h>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include <KLocalizedString>

#include "dpitest.h"

int main(int argc, char **argv)
{
    QCommandLineParser *parser = new QCommandLineParser;
    Plasma::DPITest app(argc, argv, parser);

    const QString description = i18n("DPI test app");
    const QString version = QStringLiteral("2.0");

    app.setApplicationVersion(version);
    parser->addVersionOption();
    parser->setApplicationDescription(description);

    parser->addOption(QCommandLineOption(QStringList() << QStringLiteral("s") << QStringLiteral("show"), i18nc("Do not translate <name>", "Show icon sizes"), QStringLiteral("name")));

    return app.exec();
}

