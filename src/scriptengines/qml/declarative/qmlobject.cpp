/*
 *   Copyright 2010 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
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

#include "qmlobject.h"


#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQmlIncubator>
#include <QTimer>

#include <kdebug.h>
#include <kdeclarative.h>


#include "packageaccessmanagerfactory.h"
//#include "private/declarative/dataenginebindings_p.h"


class QmlObjectPrivate
{
public:
    QmlObjectPrivate(QmlObject *parent)
        : q(parent),
          engine(0),
          component(0),
          delay(false)
    {
    }

    ~QmlObjectPrivate()
    {
        delete root.data();
    }

    void errorPrint();
    void execute(const QUrl &source);
    void scheduleExecutionEnd();
    void minimumWidthChanged();
    void minimumHeightChanged();
    void maximumWidthChanged();
    void maximumHeightChanged();
    void preferredWidthChanged();
    void preferredHeightChanged();


    QmlObject *q;

    QUrl source;
    QQmlEngine* engine;
    QQmlIncubator incubator;
    QQmlComponent* component;
    QWeakPointer<QObject> root;
    bool delay : 1;
};

void QmlObjectPrivate::errorPrint()
{
    QString errorStr = "Error loading QML file.\n";
    if(component->isError()){
        QList<QQmlError> errors = component->errors();
        foreach (const QQmlError &error, errors) {
            errorStr += (error.line()>0?QString(QString::number(error.line()) + QLatin1String(": ")):QLatin1String(""))
                + error.description() + '\n';
        }
    }
    kWarning() << component->url().toString() + '\n' + errorStr;
}

void QmlObjectPrivate::execute(const QUrl &source)
{
    if (source.isEmpty()) {
#ifndef NDEBUG
        kDebug() << "File name empty!";
#endif
        return;
    }

    delete component;
    component = new QQmlComponent(engine, q);
    delete root.data();

    KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(engine);
    kdeclarative.initialize();
    //binds things like kconfig and icons
    kdeclarative.setupBindings();

    component->loadUrl(source);

    if (delay) {
        QTimer::singleShot(0, q, SLOT(scheduleExecutionEnd()));
    } else {
        scheduleExecutionEnd();
    }
}

void QmlObjectPrivate::scheduleExecutionEnd()
{
    if (component->isReady() || component->isError()) {
        q->completeInitialization();
    } else {
        QObject::connect(component, SIGNAL(statusChanged(QQmlComponent::Status)), q, SLOT(completeInitialization()));
    }
}

QmlObject::QmlObject(QObject *parent)
    : QObject(parent),
      d(new QmlObjectPrivate(this))
{
    d->engine = new QQmlEngine(this);
    d->engine->setIncubationController(new QmlObjectIncubationController(0));
    //d->engine->setNetworkAccessManagerFactory(new PackageAccessManagerFactory());
}

QmlObject::~QmlObject()
{
//    QDeclarativeNetworkAccessManagerFactory *factory = d->engine->networkAccessManagerFactory();
//    d->engine->setNetworkAccessManagerFactory(0);
//    delete factory;
    delete d;
}

void QmlObject::setSource(const QUrl &source)
{
    qDebug() << "Opening" << source;
    d->source = source;
    d->execute(source);
}

QUrl QmlObject::source() const
{
    return d->source;
}

void QmlObject::setInitializationDelayed(const bool delay)
{
    d->delay = delay;
}

bool QmlObject::isInitializationDelayed() const
{
    return d->delay;
}

QQmlEngine* QmlObject::engine()
{
    return d->engine;
}

QObject *QmlObject::rootObject() const
{
    return d->root.data();
}

QQmlComponent *QmlObject::mainComponent() const
{
    return d->component;
}

void QmlObject::completeInitialization()
{
    if (d->root) {
        return;
    }
    if (d->component->status() != QQmlComponent::Ready || d->component->isError()) {
        d->errorPrint();
        return;
    }

    d->component->create(d->incubator);

    while (!d->incubator.isReady() && d->incubator.status() != QQmlIncubator::Error) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
    }

    d->root = d->incubator.object();
    //d->root = d->component->create();

    if (!d->root) {
        d->errorPrint();
    }

#ifndef NDEBUG
    kDebug() << "Execution of QML done!";
#endif

    emit finished();
}



#include "moc_qmlobject.cpp"
