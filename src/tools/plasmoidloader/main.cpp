#include "appletviewer.h"

#include <QtQml/QQmlDebuggingEnabler>

#include <QApplication>
#include <QCommandLineParser>

//this app creates QQuickWindow with an applet inside it
//there are no containments or other shenanigans
//to make it simple to profile starting just applets

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    QQmlDebuggingEnabler debugEnabler(false);

    //TODO args:
//      - what to benchmark (errors, startime, framecounts, etc.)
//      - --size(width x height)
//      - --formfactor
//      - --location

    QCommandLineParser parser;

    parser.addPositionalArgument("plasmoid name", "The name of the plasmoid to load");

    parser.addOption(QCommandLineOption("mode", "The mode of the plasmoid: full, compact, preferred", "m", "full"));
    parser.process(app);

    if (parser.positionalArguments().size() == 0) {
        qFatal("Invalid arguments");
    }

    AppletViewer::Representation representationType;
    QString modeString = parser.value("mode");
    if (modeString == QLatin1String("full")) {
        representationType = AppletViewer::FullRepresentation;
    } else if (modeString == QLatin1String("compact")) {
        representationType = AppletViewer::CompactRepresentation;
    } else if (modeString == QLatin1String("preferred")) {
        representationType = AppletViewer::PreferredRepresentation;
    } else {
        qFatal("--mode not valid");
    }

    AppletViewer appletViewer(parser.positionalArguments()[0], representationType);
    appletViewer.show();

    app.exec();
}
