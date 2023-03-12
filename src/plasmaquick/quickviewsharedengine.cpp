/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "quickviewsharedengine.h"
#include "sharedqmlengine.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>

#include <KLocalizedString>

namespace PlasmaQuick
{
class QuickViewSharedEnginePrivate
{
public:
    QuickViewSharedEnginePrivate(QuickViewSharedEngine *module)
        : q(module)
        , resizeMode(QuickViewSharedEngine::ResizeMode::SizeRootObjectToView)
        , initialSize(0, 0)
    {
        qmlObject = new SharedQmlEngine(q);
        QObject::connect(qmlObject, &SharedQmlEngine::statusChanged, q, &QuickViewSharedEngine::statusChanged);
        QObject::connect(qmlObject, &SharedQmlEngine::finished, q, [this]() {
            executionFinished();
        });
    }

    void executionFinished();
    void syncResizeMode();
    void syncWidth();
    void syncHeight();

    QuickViewSharedEngine *q;
    SharedQmlEngine *qmlObject;
    QuickViewSharedEngine::ResizeMode resizeMode;
    QSize initialSize;
};

void QuickViewSharedEnginePrivate::executionFinished()
{
    if (!qmlObject->rootObject()) {
        return;
    }

    QQuickItem *item = qobject_cast<QQuickItem *>(qmlObject->rootObject());

    if (!item) {
        return;
    }

    item->setParentItem(q->contentItem());
    initialSize = QSize(item->width(), item->height());

    if (q->size().isEmpty()) {
        q->resize(initialSize);
        q->contentItem()->setSize(initialSize);
    }

    syncResizeMode();
}

void QuickViewSharedEnginePrivate::syncResizeMode()
{
    QQuickItem *item = qobject_cast<QQuickItem *>(qmlObject->rootObject());

    if (!item) {
        return;
    }

    if (resizeMode == QuickViewSharedEngine::SizeRootObjectToView) {
        item->setSize(QSize(q->width(), q->height()));

        QObject::disconnect(item, &QQuickItem::widthChanged, q, nullptr);
        QObject::disconnect(item, &QQuickItem::heightChanged, q, nullptr);

    } else {
        QObject::connect(item, &QQuickItem::widthChanged, q, [this]() {
            syncWidth();
        });
        QObject::connect(item, &QQuickItem::heightChanged, q, [this]() {
            syncHeight();
        });

        syncWidth();
        syncHeight();
    }
}

void QuickViewSharedEnginePrivate::syncWidth()
{
    QQuickItem *item = qobject_cast<QQuickItem *>(qmlObject->rootObject());

    if (!item) {
        return;
    }

    q->setWidth(item->width());
}

void QuickViewSharedEnginePrivate::syncHeight()
{
    QQuickItem *item = qobject_cast<QQuickItem *>(qmlObject->rootObject());

    if (!item) {
        return;
    }

    q->setHeight(item->height());
}

QuickViewSharedEngine::QuickViewSharedEngine(QWindow *parent)
    : QQuickWindow(parent)
    , d(new QuickViewSharedEnginePrivate(this))
{
}

QuickViewSharedEngine::~QuickViewSharedEngine()
{
    delete d->qmlObject;
}

void QuickViewSharedEngine::setTranslationDomain(const QString &translationDomain)
{
    d->qmlObject->setTranslationDomain(translationDomain);
}

QString QuickViewSharedEngine::translationDomain() const
{
    return d->qmlObject->translationDomain();
}

std::shared_ptr<QQmlEngine> QuickViewSharedEngine::engine() const
{
    return d->qmlObject->engine();
}

QList<QQmlError> QuickViewSharedEngine::errors() const
{
    QList<QQmlError> errs;

    if (d->qmlObject->mainComponent()) {
        errs = d->qmlObject->mainComponent()->errors();
    }

    return errs;
}

QSize QuickViewSharedEngine::sizeHint() const
{
    QQuickItem *item = qobject_cast<QQuickItem *>(d->qmlObject->rootObject());
    if (!item) {
        return QSize();
    }

    const QSizeF implicitSize(item->implicitWidth(), item->implicitHeight());

    if (!implicitSize.isEmpty()) {
        return implicitSize.toSize();
    }

    return QSize(item->width(), item->height());
}

QSize QuickViewSharedEngine::initialSize() const
{
    return d->initialSize;
}

QuickViewSharedEngine::ResizeMode QuickViewSharedEngine::resizeMode() const
{
    return d->resizeMode;
}

QQmlContext *QuickViewSharedEngine::rootContext() const
{
    return d->qmlObject->rootContext();
}

QQuickItem *QuickViewSharedEngine::rootObject() const
{
    return qobject_cast<QQuickItem *>(d->qmlObject->rootObject());
}

void QuickViewSharedEngine::setResizeMode(ResizeMode mode)
{
    if (d->resizeMode == mode) {
        return;
    }

    d->resizeMode = mode;

    Q_EMIT resizeModeChanged(mode);

    QQuickItem *item = qobject_cast<QQuickItem *>(d->qmlObject->rootObject());
    if (!item) {
        return;
    }

    d->syncResizeMode();
}

void QuickViewSharedEngine::setSource(const QUrl &url)
{
    if (d->qmlObject->source() == url) {
        return;
    }

    d->qmlObject->setSource(url);
    Q_EMIT sourceChanged(url);
}

QUrl QuickViewSharedEngine::source() const
{
    return d->qmlObject->source();
}

QQmlComponent::Status QuickViewSharedEngine::status() const
{
    if (!d->qmlObject->mainComponent()) {
        return QQmlComponent::Null;
    }

    return QQmlComponent::Status(d->qmlObject->status());
}

void QuickViewSharedEngine::resizeEvent(QResizeEvent *e)
{
    QQuickItem *item = qobject_cast<QQuickItem *>(d->qmlObject->rootObject());
    if (item && d->resizeMode == SizeRootObjectToView) {
        item->setSize(e->size());
    }

    QQuickWindow::resizeEvent(e);
}

}

#include "moc_quickviewsharedengine.cpp"
