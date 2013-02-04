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
          root(0),
          delay(false)
    {
    }

    ~QmlObjectPrivate()
    {
    }

    void errorPrint();
    void execute(const QString &fileName);
    void finishExecute();
    void scheduleExecutionEnd();
    void minimumWidthChanged();
    void minimumHeightChanged();
    void maximumWidthChanged();
    void maximumHeightChanged();
    void preferredWidthChanged();
    void preferredHeightChanged();


    QmlObject *q;

    QString qmlPath;
    QQmlEngine* engine;
    QQmlComponent* component;
    QObject *root;
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

void QmlObjectPrivate::execute(const QString &fileName)
{
    if (fileName.isEmpty()) {
#ifndef NDEBUG
        kDebug() << "File name empty!";
#endif
        return;
    }

    KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(engine);
    kdeclarative.initialize();
    //binds things like kconfig and icons
    kdeclarative.setupBindings();

    component->loadUrl(QUrl::fromLocalFile(fileName));

    if (delay) {
        QTimer::singleShot(0, q, SLOT(scheduleExecutionEnd()));
    } else {
        scheduleExecutionEnd();
    }
}

void QmlObjectPrivate::scheduleExecutionEnd()
{
    if (component->isReady() || component->isError()) {
        finishExecute();
    } else {
        QObject::connect(component, SIGNAL(statusChanged(QQmlComponent::Status)), q, SLOT(finishExecute()));
    }
}

void QmlObjectPrivate::finishExecute()
{
    if (component->isError()) {
        errorPrint();
    }

    root = component->create();

    if (!root) {
        errorPrint();
    }

#ifndef NDEBUG
    kDebug() << "Execution of QML done!";
#endif

    emit q->finished();
}

QmlObject::QmlObject(QObject *parent)
    : QObject(parent),
      d(new QmlObjectPrivate(this))
{
    d->engine = new QQmlEngine(this);
    //d->engine->setNetworkAccessManagerFactory(new PackageAccessManagerFactory());

    d->component = new QQmlComponent(d->engine, this);
}

QmlObject::~QmlObject()
{
//    QDeclarativeNetworkAccessManagerFactory *factory = d->engine->networkAccessManagerFactory();
//    d->engine->setNetworkAccessManagerFactory(0);
//    delete factory;
    delete d;
}

void QmlObject::setQmlPath(const QString &path)
{
    qDebug() << "Opening" << path;
    d->qmlPath = path;
    d->execute(path);
}

QString QmlObject::qmlPath() const
{
    return d->qmlPath;
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
    return d->root;
}

QQmlComponent *QmlObject::mainComponent() const
{
    return d->component;
}





#include "moc_qmlobject.cpp"
