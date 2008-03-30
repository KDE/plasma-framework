/*
 * Copyright 2008 Aaron J. Seigo
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
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

#include "sharedtimertest.h"

#include <QCoreApplication>
#include <QTime>
#include <QTimer>

#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
#include <KComponentData>
#include <KConfig>
#include <KDebug>

#include "plasma/timer.h"

static const char description[] = I18N_NOOP("Applet browser dialog");
static const char version[] = "1.0";

Tester::Tester(int rounds)
    : QObject(0),
      m_count(0),
      m_round(0),
      m_targetRounds(rounds)
{
    Plasma::Timer *t50 = new Plasma::Timer(this);
    connect(t50, SIGNAL(timeout()), this, SLOT(timeout()));
    t50->start(50);

    Plasma::Timer *t70 = new Plasma::Timer(this);
    connect(t70, SIGNAL(timeout()), this, SLOT(timeout()));
    t70->start(70);

    Plasma::Timer *t210 = new Plasma::Timer(this);
    connect(t210, SIGNAL(timeout()), this, SLOT(timeout()));
    t210->start(210);

    Plasma::Timer *t500 = new Plasma::Timer(this);
    connect(t500, SIGNAL(timeout()), this, SLOT(timeout()));
    t500->start(500);
//    QObject::connect(t, SIGNAL(timeout()), &app, SLOT(quit()));

    Plasma::Timer *t1000 = new Plasma::Timer(this);
    t1000->start(1000);
    delete t1000;

    Plasma::Timer *t95 = new Plasma::Timer(this);
    connect(t95, SIGNAL(timeout()), this, SLOT(timeout()));
    t95->start(95);

    Plasma::Timer *t200 = new Plasma::Timer(this);
    connect(t200, SIGNAL(timeout()), this, SLOT(timeout()));
    t200->start(200);


    m_order << t50 << t70 << t95
            << t50 << t70 << t95
            << t200 << t210
            << t50 << t70 << t95
            << t50 << t70 << t95
            << t200 << t210
            << t50 << t70
            << t500;

}

void Tester::timeout()
{
    Plasma::Timer *t = qobject_cast<Plasma::Timer*>(sender());
    if (m_order[m_count] != t) {
        kFatal() << "round" << m_round << "call" << m_count << "expected"
                 << m_order[m_count]->interval() << "but got" << t->interval();
    }

    //kDebug() << "round" << m_round << "call" << m_count << "got" << t->interval();
    ++m_count;

    if (m_count == m_order.count()) {
        m_count = 0;
        ++m_round;
        //kDebug() << "starting round" << m_round;
    }

    if (m_round == m_targetRounds) {
        qApp->quit();
    }
}

int main(int argc, char *argv[])
{
    KAboutData aboutData("sharedtimertest", 0, ki18n("Plasma Shared Timer Test"),
                         version, ki18n( description ), KAboutData::License_GPL,
                         ki18n("(C) 2008, Aaron Seigo"));
    aboutData.addAuthor(ki18n("Aaron Seigo"), ki18n("Original author"), "aseigo@kde.org");
    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;

    Tester t(2);
    //QTimer::singleShot(2000, &app, SLOT(quit()));

    QTime elapsed;
    elapsed.start();

    int rv = app.exec();

    int totalTime = elapsed.elapsed();
    kDebug() << "successful run, took" << totalTime;
    return rv;
}

#include "sharedtimertest.moc"

