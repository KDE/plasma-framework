/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef QUICKVIEWSHAREDENGINE_H
#define QUICKVIEWSHAREDENGINE_H

#include "plasmaquick_export.h"

#include <QQmlComponent>
#include <QQmlError>
#include <QQuickWindow>
#include <QUrl>
#include <memory>

class QQuickItem;
class QQmlEngine;

namespace PlasmaQuick
{
class QuickViewSharedEnginePrivate;

/**
 * @class KQuickAddons::QuickViewSharedEngine quickviewsharedengine.h KQuickAddons/QuickViewSharedEngine
 */
class PLASMAQUICK_EXPORT QuickViewSharedEngine : public QQuickWindow
{
    Q_OBJECT

    Q_PROPERTY(ResizeMode resizeMode READ resizeMode WRITE setResizeMode NOTIFY resizeModeChanged)
    Q_PROPERTY(QQmlComponent::Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)

public:
    enum ResizeMode {
        SizeViewToRootObject,
        SizeRootObjectToView,
    };
    Q_ENUM(ResizeMode)

    explicit QuickViewSharedEngine(QWindow *parent = nullptr);
    ~QuickViewSharedEngine() override;

    /**
     * Installs a translation domain for all
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
     * @since 5.25
     */
    void setTranslationDomain(const QString &translationDomain);

    /**
     * @return the translation domain for the i18n calls done in this QML engine
     * @since 5.25
     */
    QString translationDomain() const;

    std::shared_ptr<QQmlEngine> engine() const;
    QList<QQmlError> errors() const;
    QSize sizeHint() const;
    QSize initialSize() const;
    QQmlContext *rootContext() const;
    QQuickItem *rootObject() const;
    QUrl source() const;
    QQmlComponent::Status status() const;
    ResizeMode resizeMode() const;
    void setResizeMode(ResizeMode);

protected:
    void resizeEvent(QResizeEvent *e) override;

public Q_SLOTS:
    void setSource(const QUrl &url);

Q_SIGNALS:
    void statusChanged(QQmlComponent::Status status);
    void resizeModeChanged(QuickViewSharedEngine::ResizeMode resizeMode);
    void sourceChanged(const QUrl &source);

private:
    const std::unique_ptr<QuickViewSharedEnginePrivate> d;

    Q_PRIVATE_SLOT(d, void executionFinished())
    Q_PRIVATE_SLOT(d, void syncWidth())
    Q_PRIVATE_SLOT(d, void syncHeight())
};

}

#endif // QuickViewSharedEngine_H
