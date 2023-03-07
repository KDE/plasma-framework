/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText:

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_SHAREDQMLENGINE_H
#define PLASMA_SHAREDQMLENGINE_H

#include <plasmaquick/plasmaquick_export.h>

#include <QObject>
#include <QQmlComponent>
#include <QQmlContext>

#include <memory>

class QQmlComponent;
class QQmlEngine;
class KLocalizedContext;

namespace PlasmaQuick
{
class SharedQmlEnginePrivate;

/**
 * @class PlasmaQuick::SharedQmlEngine PlasmaQuick/sharedqmlengine.h PlasmaQuick/SharedQmlEngine
 *
 * @short An object that instantiates an entire QML context, with its own declarative engine
 *
 * PlasmaQuick::SharedQmlEngine provides a class to conveniently use QML based
 * declarative user interfaces.
 * A SharedQmlEngine corresponds to one QML file (which can include others).
 * It will a shared QQmlEngine with a single root object, described in the QML file.
 *
 * @since 6.0
 */
class PLASMAQUICK_EXPORT SharedQmlEngine : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QUrl source READ source WRITE setSource)
    Q_PROPERTY(QString translationDomain READ translationDomain WRITE setTranslationDomain)
    Q_PROPERTY(bool initializationDelayed READ isInitializationDelayed WRITE setInitializationDelayed)
    Q_PROPERTY(QObject *rootObject READ rootObject)
    Q_PROPERTY(QQmlComponent::Status status READ status NOTIFY statusChanged)

public:
    /**
     * Construct a new PlasmaQuick::SharedQmlEngine
     *
     * @param parent The QObject parent for this object.
     */
    explicit SharedQmlEngine(QObject *parent = nullptr);

    ~SharedQmlEngine() override;

    /**
     * Call this method before calling setupBindings to install a translation domain for all
     * i18n global functions. If a translation domain is set all i18n calls delegate to the
     * matching i18nd calls with the provided translation domain.
     *
     * The translationDomain affects all i18n calls including those from imports. Because of
     * that modules intended to be used as imports should prefer the i18nd variants and set
     * the translation domain explicitly in each call.
     *
     * This method is only required if your declarative usage is inside a library. If it's
     * in an application there is no need to set the translation domain as the application's
     * domain can be used.
     *
     * @param translationDomain The translation domain to be used for i18n calls.
     */
    void setTranslationDomain(const QString &translationDomain);

    /**
     * @return the translation domain for the i18n calls done in this QML engine
     */
    QString translationDomain() const;

    /**
     * Sets the path of the QML file to parse and execute
     *
     * @param path the absolute path of a QML file
     */
    void setSource(const QUrl &source);

    /**
     * @return the absolute path of the current QML file
     */
    QUrl source() const;

    /**
     * Sets whether the execution of the QML file has to be delayed later in the event loop. It has to be called before setQmlPath().
     * In this case it will be possible to assign new objects in the main engine context
     * before the main component gets initialized.
     * In that case it will be possible to access it immediately from the QML code.
     * The initialization will either be completed automatically asynchronously
     * or explicitly by calling completeInitialization()
     *
     * @param delay if true the initialization of the QML file will be delayed
     *              at the end of the event loop
     */
    void setInitializationDelayed(const bool delay);

    /**
     * @return true if the initialization of the QML file will be delayed
     *              at the end of the event loop
     */
    bool isInitializationDelayed() const;

    /**
     * @return the declarative engine that runs the qml file assigned to this widget.
     */
    std::shared_ptr<QQmlEngine> engine();

    /**
     * @return the root object of the declarative object tree
     */
    QObject *rootObject() const;

    /**
     * @return the main QQmlComponent of the engine
     */
    QQmlComponent *mainComponent() const;

    /**
     * The components's creation context.
     */
    QQmlContext *rootContext() const;

    /**
     * The component's current status.
     */
    QQmlComponent::Status status() const;

    /**
     * Creates and returns an object based on the provided url to a Qml file
     * with the same QQmlEngine and the same root context as the main object,
     * that will be the parent of the newly created object
     * @param source url where the QML file is located
     * @param context The QQmlContext in which we will create the object,
     *             if 0 it will use the engine's root context
     * @param initialProperties optional properties that will be set on
     *             the object when created (and before Component.onCompleted
     *             gets emitted
     */
    QObject *createObjectFromSource(const QUrl &source, QQmlContext *context = nullptr, const QVariantHash &initialProperties = QVariantHash());

    /**
     * Creates and returns an object based on the provided QQmlComponent
     * with the same QQmlEngine and the same root context as the admin object,
     * that will be the parent of the newly created object
     * @param component the component we want to instantiate
     * @param context The QQmlContext in which we will create the object,
     *             if 0 it will use the engine's root context
     * @param initialProperties optional properties that will be set on
     *             the object when created (and before Component.onCompleted
     *             gets emitted
     */
    QObject *createObjectFromComponent(QQmlComponent *component, QQmlContext *context = nullptr, const QVariantHash &initialProperties = QVariantHash());

public Q_SLOTS:
    /**
     * Finishes the process of initialization.
     * If isInitializationDelayed() is false, calling this will have no effect.
     * @param initialProperties optional properties that will be set on
     *             the object when created (and before Component.onCompleted
     *             gets emitted
     */
    void completeInitialization(const QVariantHash &initialProperties = QVariantHash());

Q_SIGNALS:
    /**
     * Emitted when the parsing and execution of the QML file is terminated
     */
    void finished();

    void statusChanged(QQmlComponent::Status);

private:
    const std::unique_ptr<SharedQmlEnginePrivate> d;

    Q_PRIVATE_SLOT(d, void scheduleExecutionEnd())
    Q_PRIVATE_SLOT(d, void checkInitializationCompleted())
};

}

#endif // multiple inclusion guard
