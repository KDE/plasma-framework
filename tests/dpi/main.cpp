/*
    SPDX-FileCopyrightText: 2008 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QScreen>
#include <plasma/theme.h>

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

    parser->addOption(QCommandLineOption(QStringList() << QStringLiteral("s") << QStringLiteral("show"),
                                         i18nc("Do not translate <name>", "Show icon sizes"),
                                         QStringLiteral("name")));

    return app.exec();
}
