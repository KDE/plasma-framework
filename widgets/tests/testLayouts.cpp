#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>

#include <KUniqueApplication>
#include <KCmdLineArgs>
#include <KCmdLineOptions>
#include <KAboutData>
#include <KIcon>

#include "plasma/layouts/boxlayout.h"
#include "plasma/widgets/pushbutton.h"
#include "plasma/widgets/lineedit.h"
#include "plasma/widgets/widget.h"
#include "plasma/widgets/label.h"

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

    Plasma::HBoxLayout *h1 = new Plasma::HBoxLayout( 0 );
    Plasma::HBoxLayout *h2 = new Plasma::HBoxLayout( 0 );
    widgetLayout->addItem( h1 );
    widgetLayout->addItem( h2 );

    // should be first row
    Plasma::PushButton *pushOne = new Plasma::PushButton("pushbutton one");
    h1->addItem(pushOne);
    scene.addItem(pushOne);

    Plasma::LineEdit *editOne = new Plasma::LineEdit;
    h1->addItem(editOne);
    scene.addItem(editOne);

    Plasma::Label *labelOne = new Plasma::Label( 0 );
    labelOne->setText( "hello world 1" );
    h1->addItem(labelOne);
    scene.addItem(labelOne);

    // should be second row
    Plasma::PushButton *pushTwo = new Plasma::PushButton("pushbutton two");
    h2->addItem(pushTwo);
    scene.addItem(pushTwo);

   Plasma::LineEdit *editTwo = new Plasma::LineEdit;
   h2->addItem(editTwo);
   scene.addItem(editTwo);

   Plasma::Label *labelTwo = new Plasma::Label( 0 );
   labelTwo->setText( "hello world 2" );
   h2->addItem(labelTwo);
   scene.addItem(labelTwo);

    widgetLayout->updateGeometry();

    view.show();
    return app.exec();
}

