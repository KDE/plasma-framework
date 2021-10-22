/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "appletrepresentation.h"
#include "debug_p.h"
#include "private/appletrepresentation_p.h"
#include "appletcontext_p.h"
#include "plasmoid.h"

#include <QJsonArray>
#include <QQmlContext>
#include <QQmlExpression>
#include <QQmlProperty>
#include <QQuickWindow>
#include <QRandomGenerator>

#include <QDebug>

#include <KLocalizedString>

#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/Corona>
#include <kdeclarative/qmlobjectsharedengine.h>

#include <packageurlinterceptor.h>
#include <qloggingcategory.h>

namespace PlasmaQuick
{

AppletRepresentationPrivate::AppletRepresentationPrivate(AppletRepresentation *item)
    : q(item)
    , switchWidth(-1)
    , switchHeight(-1)
    , expanded(false)
    , activationTogglesExpanded(true)
{
    //qmlRegisterType<PlasmaQuick::Plasmoid>();
}

QQuickItem *AppletRepresentationPrivate::createCompactRepresentationItem()
{
    if (!compactRepresentation) {
        
        QQmlComponent *component = new QQmlComponent(qmlEngine(q), coronaPackage.fileUrl("defaultcompactrepresentation"), applet);
        if (component->isError()) {
            qWarning() << "No compact representation available";
            return nullptr;
        } else {
            compactRepresentation = component;
        }
    }

    if (compactRepresentationItem) {
        return compactRepresentationItem;
    }

    QVariantMap initialProperties;
    initialProperties[QStringLiteral("parent")] = QVariant::fromValue(q);

    compactRepresentationItem = qobject_cast<QQuickItem *>(compactRepresentation->createWithInitialProperties(initialProperties, rootContext));

    Q_EMIT q->compactRepresentationItemChanged(compactRepresentationItem);

    return compactRepresentationItem;
}

QQuickItem *AppletRepresentationPrivate::createFullRepresentationItem()
{
    //FULL REP should be a required property
    if (fullRepresentationItem) {
        return fullRepresentationItem;
    }

    QVariantMap initialProperties;
    initialProperties[QStringLiteral("parent")] = QVariant();
    fullRepresentationItem = qobject_cast<QQuickItem *>(fullRepresentation->createWithInitialProperties(initialProperties, rootContext));

    Q_EMIT q->fullRepresentationItemChanged(fullRepresentationItem);

    return fullRepresentationItem;
}

AppletRepresentation::AppletRepresentation(QQuickItem *parent)
    : QObject(parent)
    , d(new AppletRepresentationPrivate(this))
{
    
}

AppletRepresentation::~AppletRepresentation()
{
    // Here the order is important
    delete d->compactRepresentationItem;
    delete d->fullRepresentationItem;

    delete d;
}


void AppletRepresentation::init(Plasma::Applet *applet, QQmlContext *rootContext)
{
    d->applet = applet;
    d->rootContext = rootContext;

    if (d->applet) {
        d->appletPackage = d->applet->kPackage();

        if (d->applet->containment()) {
            if (d->applet->containment()->corona()) {
                d->coronaPackage = d->applet->containment()->corona()->kPackage();
            }

            d->containmentPackage = d->applet->containment()->kPackage();
        }

        if (d->applet->pluginMetaData().isValid()) {
            const QString rootPath = d->applet->pluginMetaData().value(QStringLiteral("X-Plasma-RootPath"));
            if (!rootPath.isEmpty()) {//FIXME: how to do translationdomain?
          //      d->qmlObject->setTranslationDomain(QLatin1String("plasma_applet_") + rootPath);
            } else {
          //      d->qmlObject->setTranslationDomain(QLatin1String("plasma_applet_") + d->applet->pluginMetaData().pluginId());
            }
        }
    }
}

//FIXME: remove engine param
AppletRepresentation *AppletRepresentation::representationForApplet(Plasma::Applet *applet, QQmlEngine *engine)
{
    if (applet->representation()) {
        return qobject_cast<AppletRepresentation*>(applet->representation());
    }

    AppletRepresentation *appletItem = nullptr;

    QJsonObject errorData;
    errorData[QStringLiteral("appletName")] = i18n("Unknown Applet");
    errorData[QStringLiteral("isDebugMode")] = qEnvironmentVariableIntValue("PLASMA_ENABLE_QML_DEBUG") != 0;

    if (applet->kPackage().isValid()) {
        applet->setLaunchErrorMessage(i18n("Error loading Applet: package inexistent"));
    }
    
    const QUrl url(applet->kPackage().fileUrl("mainscript"));
    if (url.isEmpty()) {
        applet->setLaunchErrorMessage(i18n("Error loading Applet: missing main QML file"));
    }

   // QQmlEngine *engine = nullptr;//qmlEngine(m_view); TODO: singleton?
    Q_ASSERT(engine);

    QQmlComponent *component = new QQmlComponent(engine, url, applet);
    
    if (component->status() == QQmlComponent::Error) {
        const auto errors = component->errors();
        QStringList errorList;
        QString reason;
        for (const QQmlError &error : errors) {
            reason += error.toString() + QLatin1Char('\n');
            errorList << error.toString();
        }
        errorData[QStringLiteral("errors")] = QJsonArray::fromStringList(errorList);
        errorData[QStringLiteral("appletName")] = applet->kPackage().metadata().name();
        reason = i18n("Error loading QML file: %1 %2", component->url().toString(), reason);
        qWarning()<<reason;
        applet->setLaunchErrorMessage(reason);
        //TODO: load an AppletRepresentation with the error message

    // Only allow sync qml loading?
    } else {
        AppletContext *context = new AppletContext(engine, applet);//TODO: this context will have to have the translation stuff installed
        QObject *guiObject = component->beginCreate(context);
        appletItem = qobject_cast<AppletRepresentation *>(guiObject);
        if (appletItem) {
            appletItem->init(applet, qmlContext(guiObject));
            component->completeCreate();
        } else {
            applet->setLaunchErrorMessage(i18n("QML root object not an AppletRepresentation instance"));
            if (guiObject) {
                guiObject->deleteLater();
            }
            qWarning() << i18n("QML root object not an AppletRepresentation instance");
        }
    }

    return appletItem;
}


Plasma::Applet *AppletRepresentation::applet() const
{
    return d->applet;
}

int AppletRepresentation::switchWidth() const
{
    return d->switchWidth;
}

void AppletRepresentation::setSwitchWidth(int width)
{
    if (d->switchWidth == width) {
        return;
    }

    d->switchWidth = width;
    Q_EMIT switchWidthChanged(width);
}

int AppletRepresentation::switchHeight() const
{
    return d->switchHeight;
}

void AppletRepresentation::setSwitchHeight(int height)
{
    if (d->switchHeight == height) {
        return;
    }

    d->switchHeight = height;
    Q_EMIT switchHeightChanged(height);
}

QQmlComponent *AppletRepresentation::compactRepresentation()
{
    return d->compactRepresentation;
}

void AppletRepresentation::setCompactRepresentation(QQmlComponent *component)
{
    if (d->compactRepresentation == component) {
        return;
    }

    d->compactRepresentation = component;
    Q_EMIT compactRepresentationChanged(component);
}

QQmlComponent *AppletRepresentation::fullRepresentation()
{
    return d->fullRepresentation;
}

QObject *AppletRepresentation::testItem()
{
    if (!d->testItem) {
        const QUrl url(d->appletPackage.fileUrl("test"));
        if (url.isEmpty()) {
            return nullptr;
        }

        //TODO: creation
       // d->testItem = d->qmlObject->createObjectFromSource(url, QtQml::qmlContext(rootItem()));
        if (d->testItem) {
            d->testItem->setProperty("plasmoidItem", QVariant::fromValue<QObject *>(this));
        }
    }

    return d->testItem;
}

void AppletRepresentation::setFullRepresentation(QQmlComponent *component)
{
    if (d->fullRepresentation == component) {
        return;
    }

    d->fullRepresentation = component;
    Q_EMIT fullRepresentationChanged(component);
}

QQmlComponent *AppletRepresentation::preferredRepresentation()
{
    return d->preferredRepresentation;
}

void AppletRepresentation::setPreferredRepresentation(QQmlComponent *component)
{
    if (d->preferredRepresentation == component) {
        return;
    }

    d->preferredRepresentation = component;
    Q_EMIT preferredRepresentationChanged(component);
}

bool AppletRepresentation::isExpanded() const
{
    return d->expanded;
}

//TODO: remove
void AppletRepresentation::setExpanded(bool expanded)
{
    if (d->expanded == expanded) {
        return;
    }

    d->expanded = expanded;
    Q_EMIT expandedChanged(expanded);
}

bool AppletRepresentation::isActivationTogglesExpanded() const
{
    return d->activationTogglesExpanded;
}

void AppletRepresentation::setActivationTogglesExpanded(bool activationTogglesExpanded)
{
    if (d->activationTogglesExpanded == activationTogglesExpanded) {
        return;
    }
    d->activationTogglesExpanded = activationTogglesExpanded;
    Q_EMIT activationTogglesExpandedChanged(activationTogglesExpanded);
}

////////////Internals

QQuickItem *AppletRepresentation::compactRepresentationItem()
{
    d->createCompactRepresentationItem();
    return d->compactRepresentationItem;
}

QQuickItem *AppletRepresentation::fullRepresentationItem()
{
    d->createFullRepresentationItem();
    return d->fullRepresentationItem;
}

}

#include "moc_appletrepresentation.cpp"
