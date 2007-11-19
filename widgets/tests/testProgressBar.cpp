#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>

#include <KUniqueApplication>
#include <KCmdLineArgs>
#include <KCmdLineOptions>
#include <KAboutData>
#include <KIcon>


#include "plasma/layouts/boxlayout.h"
#include "plasma/widgets/progressbar.h"

class Counter : QObject
{
    Q_OBJECT

public:
    Counter(Plasma::ProgressBar *p);
    ~Counter();

    void start();

private Q_SLOTS:
    void updateValue();

private:

    QTimer timer;
    Plasma::ProgressBar *pbar;
    int seconds;

};


Counter::Counter(Plasma::ProgressBar *p)
        : QObject(),  timer(0)
{
    seconds = 10;
    pbar = p;

}

Counter::~Counter()
{
}

void Counter::start()
{
    connect(&timer, SIGNAL(timeout()),
            this, SLOT(updateValue()));

    timer.start(100);
}

void Counter::updateValue()
{
    int inc = 1;

    pbar->setValue(pbar->value() + inc);

    if (pbar->value() >= 100) {
        timer.stop();
    }
}

int main(int argc, char **argv)
{
    KAboutData aboutData(QByteArray("test"), 0, ki18n("test"),
                         KDE_VERSION_STRING, ki18n("test"), KAboutData::License_BSD,
                         ki18n("test"));
    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;

    QGraphicsView view;
    QGraphicsScene scene;
    view.setScene(&scene);

    Plasma::VBoxLayout *widgetLayout = new Plasma::VBoxLayout(0);
    widgetLayout->setGeometry(QRectF(0.0, 0.0, 350.0, 30.0));

    Plasma::HBoxLayout *h1 = new Plasma::HBoxLayout(0);
    widgetLayout->addItem(h1);

    Plasma::ProgressBar *progressBar = new Plasma::ProgressBar(0);
    h1->addItem(progressBar);
    scene.addItem(progressBar);

    widgetLayout->updateGeometry();

    view.show();

    Counter *c = new Counter(progressBar);
    c->start();

//   Uncomment for hide the text.
//    progressBar->setTextVisible( false );
//
//    Uncomment for change the progressbar alignment
//    progressBar->setAlignment( Qt::AlignRight );
//
//    Uncomment in order to invert the progress, from right to left.
//    progressBar->setInvertedAppearance( true );;

    progressBar->setGeometry(QRectF(0, 0, 200, 30));


    return app.exec();
}

#include "testLayouts.moc"
