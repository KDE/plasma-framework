#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>

#include <KUniqueApplication>
#include <KCmdLineArgs>
#include <KCmdLineOptions>
#include <KAboutData>
#include <KIcon>

#include "../pushbutton.h"
#include "../lineedit.h"
#include "../vboxlayout.h"
#include "../hboxlayout.h"
#include "../widget.h"

int main(int argc, char **argv)
{
    KAboutData aboutData( QByteArray("test"), 0, ki18n("test"),
                          KDE_VERSION_STRING, ki18n("test"), KAboutData::License_BSD,
                          ki18n("test") );
    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;

    QGraphicsView view;
    QGraphicsScene scene;
    view.setScene(&scene);

    Plasma::VBoxLayout *widgetLayout = new Plasma::VBoxLayout(0);
    widgetLayout->setGeometry( QRectF(0.0, 0.0, 400.0, 700.0) );

    Plasma::PushButton *pushOne = new Plasma::PushButton("pushbutton one");
    Plasma::PushButton *pushTwo = new Plasma::PushButton("pushbutton two");
    Plasma::PushButton *pushThree = new Plasma::PushButton("pushbutton three");
    Plasma::PushButton *pushFour = new Plasma::PushButton("pushbutton four");

    widgetLayout->addItem(pushOne);
    widgetLayout->addItem(pushTwo);
    widgetLayout->addItem(pushThree);
    widgetLayout->addItem(pushFour);

    scene.addItem(pushOne);
    scene.addItem(pushTwo);
    scene.addItem(pushThree);
    scene.addItem(pushFour);

    Plasma::LineEdit *editOne = new Plasma::LineEdit;
    Plasma::LineEdit *editTwo = new Plasma::LineEdit;
    Plasma::LineEdit *editThree = new Plasma::LineEdit;
    Plasma::LineEdit *editFour = new Plasma::LineEdit;

    widgetLayout->addItem(editOne);
    widgetLayout->addItem(editTwo);
    widgetLayout->addItem(editThree);
    widgetLayout->addItem(editFour);

    scene.addItem(editOne);
    scene.addItem(editTwo);
    scene.addItem(editThree);
    scene.addItem(editFour);

    view.show();
    return app.exec();
}

