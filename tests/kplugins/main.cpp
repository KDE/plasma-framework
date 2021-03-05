/*
    SPDX-FileCopyrightText: 2008 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <KLocalizedString>
#include <QCommandLineOption>
#include <QCommandLineParser>

#include "plugintest.h"

int main(int argc, char **argv)
{
    QCommandLineParser *parser = new QCommandLineParser;
    Plasma::PluginTest app(argc, argv, parser);

    const QString description = i18n("Plugin test app");
    const QString version = QStringLiteral("2.0");

    app.setApplicationVersion(version);
    parser->addVersionOption();
    parser->setApplicationDescription(description);

    parser->addOption(QCommandLineOption(QStringList() << QStringLiteral("s") << QStringLiteral("show"),
                                         i18nc("Do not translate <name>", "Show plugins"),
                                         QStringLiteral("name")));

    return app.exec();
}
