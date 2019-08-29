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
    if (type == QLatin1String("plasmoid")) {
        return QStringLiteral("Plasma/Applet");
    } else if (type == QLatin1String("package")) {
        return QStringLiteral("Plasma/Generic");
    } else if (type == QLatin1String("theme")) {
        return QStringLiteral("Plasma/Theme");
    } else if (type == QLatin1String("wallpaper")) {
        return QStringLiteral("Plasma/ImageWallpaper");
    } else if (type == QLatin1String("dataengine")) {
        return QStringLiteral("Plasma/DataEngine");
    } else if (type == QLatin1String("runner")) {
        return QStringLiteral("Plasma/Runner");
    } else if (type == QLatin1String("wallpaperplugin")) {
        return QStringLiteral("Plasma/Wallpaper");
    } else if (type == QLatin1String("lookandfeel")) {
        return QStringLiteral("Plasma/LookAndFeel");
    } else if (type == QLatin1String("shell")) {
        return QStringLiteral("Plasma/Shell");
    } else if (type == QLatin1String("layout-template")) {
        return QStringLiteral("Plasma/LayoutTemplate");
    } else if (type == QLatin1String("kwineffect")) {
        return QStringLiteral("KWin/Effect");
    } else if (type == QLatin1String("windowswitcher")) {
        return QStringLiteral("KWin/WindowSwitcher");
    } else if (type == QLatin1String("kwinscript")) {
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
    int typeIndex = params.indexOf(QLatin1String("-t"));
    if (typeIndex > -1 && params.length() > typeIndex + 1) {
        params[typeIndex + 1] = typeFromLegacy(params.value(typeIndex + 1));
    } else {
        //user passed --type typeName
        typeIndex = params.indexOf(QLatin1String("--type"));
        if (typeIndex > -1 && params.length() > typeIndex + 1) {
            params[typeIndex + 1] = typeFromLegacy(params.value(typeIndex + 1));
        } else {
            //user passed --type=typeName
            typeIndex = params.indexOf(QRegularExpression(QStringLiteral("--type=.*")));
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

