/*
    SPDX-FileCopyrightText: 2016 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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

    // plasmapkg2 had some hardcoded types, kpackagetool5 requires the servicetype passed as the type parameter
    // convert between the two
    // user passed -t typeName
    int typeIndex = params.indexOf(QLatin1String("-t"));
    if (typeIndex > -1 && params.length() > typeIndex + 1) {
        params[typeIndex + 1] = typeFromLegacy(params.value(typeIndex + 1));
    } else {
        // user passed --type typeName
        typeIndex = params.indexOf(QLatin1String("--type"));
        if (typeIndex > -1 && params.length() > typeIndex + 1) {
            params[typeIndex + 1] = typeFromLegacy(params.value(typeIndex + 1));
        } else {
            // user passed --type=typeName
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
