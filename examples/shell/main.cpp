/*
 * Copyright 2015 Marco Martin <notmart@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include <QApplication>
#include <qcommandlineparser.h>
#include <qcommandlineoption.h>

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
