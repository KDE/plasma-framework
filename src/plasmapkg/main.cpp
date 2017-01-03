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
#include <QRegularExpression>

QString typeFromLegacy(const QString &type)
{
    if (type == QStringLiteral("plasmoid")) {
        return QStringLiteral("Plasma/Applet");
    } else if (type == QStringLiteral("package")) {
        return QStringLiteral("Plasma/Generic");
    } else if (type == QStringLiteral("theme")) {
        return QStringLiteral("Plasma/Theme");
    } else if (type == QStringLiteral("wallpaper")) {
        return QStringLiteral("Plasma/ImageWallpaper");
    } else if (type == QStringLiteral("dataengine")) {
        return QStringLiteral("Plasma/DataEngine");
    } else if (type == QStringLiteral("runner")) {
        return QStringLiteral("Plasma/Runner");
    } else if (type == QStringLiteral("wallpaperplugin")) {
        return QStringLiteral("Plasma/Wallpaper");
    } else if (type == QStringLiteral("lookandfeel")) {
        return QStringLiteral("Plasma/LookAndFeel");
    } else if (type == QStringLiteral("shell")) {
        return QStringLiteral("Plasma/Shell");
    } else if (type == QStringLiteral("layout-template")) {
        return QStringLiteral("Plasma/LayoutTemplate");
    } else if (type == QStringLiteral("kwineffect")) {
        return QStringLiteral("KWin/Effect");
    } else if (type == QStringLiteral("windowswitcher")) {
        return QStringLiteral("KWin/WindowSwitcher");
    } else if (type == QStringLiteral("kwinscript")) {
        return QStringLiteral("KWin/Script");
    } else {
        return type;
    }
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QStringList params = app.arguments().mid(1);

    //plasmapkg2 had some hardcoded types, kpackagetool5 requires the servicetype passed as the type parameter
    //convert between the two
    //user passed -t typeName
    int typeIndex = params.indexOf(QStringLiteral("-t"));
    if (typeIndex > -1 && params.length() > typeIndex + 1) {
        params[typeIndex + 1] = typeFromLegacy(params.value(typeIndex + 1));
    } else {
        //user passed --type typeName
        typeIndex = params.indexOf(QStringLiteral("--type"));
        if (typeIndex > -1 && params.length() > typeIndex + 1) {
            params[typeIndex + 1] = typeFromLegacy(params.value(typeIndex + 1));
        } else {
            //user passed --type=typeName
            typeIndex = params.indexOf(QRegularExpression("--type=.*"));
            if (typeIndex > -1) {
                params[typeIndex] = QStringLiteral("--type=") + typeFromLegacy(params.value(typeIndex).replace(QStringLiteral("--type="), QString()));
            }
        }
    }

    QProcess p;
    p.setProcessChannelMode(QProcess::ForwardedChannels);
    p.start(QLatin1String("kpackagetool5"), params);
    p.waitForFinished();

    return p.exitCode();
}

