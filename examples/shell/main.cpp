/*
    SPDX-FileCopyrightText: 2015 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>

#include <KDBusService>
#include <KLocalizedString>

#include "customcorona.h"

int main(int argc, char **argv)
{
    QQuickWindow::setDefaultAlphaBuffer(true);

    QApplication app(argc, argv);
    app.setApplicationVersion(QStringLiteral("1.0"));
    app.setOrganizationDomain(QStringLiteral("kde.org"));

    KDBusService service(KDBusService::Unique);

    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("Plasma Example shell"));
    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(app);

    CustomCorona *corona = new CustomCorona();

    const int ret = app.exec();
    delete corona;
    return ret;
}
