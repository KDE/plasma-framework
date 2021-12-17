/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "containmentview.h"
#include "configview.h"

#include <QDebug>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QScreen>
#include <QTimer>

#include "plasma/pluginloader.h"
#include <kdeclarative/kdeclarative.h>
#include <packageurlinterceptor.h>

namespace PlasmaQuick
{
class ContainmentViewPrivate
{
public:
    ContainmentViewPrivate(Plasma::Corona *corona, ContainmentView *view);
    ~ContainmentViewPrivate();

    void setContainment(Plasma::Containment *cont);
    Plasma::Types::FormFactor formFactor() const;
    Plasma::Types::Location location() const;
    void showConfigurationInterface(Plasma::Applet *applet);
    void updateDestroyed(bool destroyed);

    ContainmentView *q;
    friend class ContainmentView;
    Plasma::Corona *corona;
    QPointer<Plasma::Containment> containment;
    QPointer<ConfigView> configContainmentView;
};

ContainmentViewPrivate::ContainmentViewPrivate(Plasma::Corona *cor, ContainmentView *view)
    : q(view)
    , corona(cor)
{
}

ContainmentViewPrivate::~ContainmentViewPrivate()
{
}

void ContainmentViewPrivate::setContainment(Plasma::Containment *cont)
{
    if (containment == cont) {
        return;
    }

    Plasma::Types::Location oldLoc = location();
    Plasma::Types::FormFactor oldForm = formFactor();

    if (containment) {
        QObject::disconnect(containment, nullptr, q, nullptr);
        QObject *oldGraphicObject = containment->property("_plasma_graphicObject").value<QObject *>();
        if (auto item = qobject_cast<QQuickItem *>(oldGraphicObject)) {
            item->setVisible(false);
        }
        containment->reactToScreenChange();
    }

    containment = cont;

    if (oldLoc != location()) {
        Q_EMIT q->locationChanged(location());
    }
    if (oldForm != formFactor()) {
        Q_EMIT q->formFactorChanged(formFactor());
    }

    Q_EMIT q->containmentChanged();

    // we are QuickViewSharedEngine::SizeRootObjectToView, but that's not enough, as
    // the root object isn't immediately resized (done at the resizeEvent handler).
    // by resizing it just before restoring the containment, it removes a chain of resizes at startup
    if (q->rootObject()) {
        q->rootObject()->setSize(q->size());
    }
    if (cont) {
        cont->reactToScreenChange();
        QObject::connect(cont, &Plasma::Containment::locationChanged, q, &ContainmentView::locationChanged);
        QObject::connect(cont, &Plasma::Containment::formFactorChanged, q, &ContainmentView::formFactorChanged);
        QObject::connect(cont, &Plasma::Containment::configureRequested, q, &ContainmentView::showConfigurationInterface);
        QObject::connect(cont, SIGNAL(destroyedChanged(bool)), q, SLOT(updateDestroyed(bool)));

        // Panels are created invisible and the code below ensures they are only
        // shown once their contents have settled to avoid visual glitches on startup
        if (cont->containmentType() == Plasma::Types::PanelContainment || cont->containmentType() == Plasma::Types::CustomPanelContainment) {
            QObject::connect(
                cont,
                &Plasma::Containment::uiReadyChanged,
                q,
                [this, cont](bool ready) {
                    if (ready && !cont->destroyed()) {
                        q->setVisible(true);
                    }
                },
                Qt::QueuedConnection);

            q->setVisible(!cont->destroyed() && cont->isUiReady());
        }
    } else {
        return;
    }

    QQuickItem *graphicObject = qobject_cast<QQuickItem *>(containment->property("_plasma_graphicObject").value<QObject *>());

    if (graphicObject) {
        //         qDebug() << "using as graphic containment" << graphicObject << containment.data();

        graphicObject->setFocus(true);
        // by resizing before adding, it will avoid some resizes in most cases
        graphicObject->setSize(q->size());
        graphicObject->setParentItem(q->rootObject());
        if (q->rootObject()) {
            q->rootObject()->setProperty("containment", QVariant::fromValue(graphicObject));
            QObject *wpGraphicObject = containment->property("wallpaperGraphicsObject").value<QObject *>();
            if (wpGraphicObject) {
                q->rootObject()->setProperty("wallpaper", QVariant::fromValue(wpGraphicObject));
            }
        } else {
            qWarning() << "Could not set containment property on rootObject";
        }
    } else {
        qWarning() << "Containment graphic object not valid";
    }
}

Plasma::Types::Location ContainmentViewPrivate::location() const
{
    if (!containment) {
        return Plasma::Types::Desktop;
    }
    return containment->location();
}

Plasma::Types::FormFactor ContainmentViewPrivate::formFactor() const
{
    if (!containment) {
        return Plasma::Types::Planar;
    }
    return containment->formFactor();
}

void ContainmentViewPrivate::showConfigurationInterface(Plasma::Applet *applet)
{
    if (configContainmentView) {
        if (configContainmentView->applet() != applet) {
            configContainmentView->hide();
            configContainmentView->deleteLater();
        } else {
            configContainmentView->raise();
            configContainmentView->requestActivate();
            return;
        }
    }

    if (!applet || !applet->containment()) {
        return;
    }

    configContainmentView = new ConfigView(applet);

    configContainmentView->init();
    configContainmentView->show();
}

void ContainmentViewPrivate::updateDestroyed(bool destroyed)
{
    q->setVisible(!destroyed);
}

ContainmentView::ContainmentView(Plasma::Corona *corona, QWindow *parent)
    : KQuickAddons::QuickViewSharedEngine(parent)
    , d(new ContainmentViewPrivate(corona, this))
{
    setColor(Qt::transparent);

    QObject::connect(screen(), &QScreen::geometryChanged, this, &ContainmentView::screenGeometryChanged);

    if (corona->kPackage().isValid()) {
        const auto info = corona->kPackage().metadata();
        if (info.isValid()) {
            setTranslationDomain(QStringLiteral("plasma_shell_") + info.pluginId());
        } else {
            qWarning() << "Invalid corona package metadata";
        }
    } else {
        qWarning() << "Invalid home screen package";
    }

    // Force QtQuickControls to use the "Plasma" style for this engine.
    // this way is possible to mix QtQuickControls and plasma components in applets
    // while still having the desktop style in configuration dialogs
    QQmlComponent c(engine());
    c.setData(
        "import QtQuick 2.1\n\
        import QtQuick.Controls 1.0\n\
        import QtQuick.Controls.Private 1.0\n \
        Item {\
          Component.onCompleted: {\
            Settings.styleName = \"Plasma\";\
          }\
        }",
        QUrl());
    QObject *o = c.create();
    o->deleteLater();

    setResizeMode(ContainmentView::SizeRootObjectToView);
}

ContainmentView::~ContainmentView()
{
    delete d;
}

void ContainmentView::destroy()
{
    QObject *graphicObject = d->containment->property("_plasma_graphicObject").value<QObject *>();
    if (auto item = qobject_cast<QQuickItem *>(graphicObject)) {
        item->setVisible(false);
        item->setParentItem(nullptr); // First, remove the item from the view
    }
    deleteLater(); // delete the view
}

Plasma::Corona *ContainmentView::corona() const
{
    return d->corona;
}

KConfigGroup ContainmentView::config() const
{
    if (!containment()) {
        return KConfigGroup();
    }
    KConfigGroup views(KSharedConfig::openConfig(), "PlasmaContainmentViews");
    return KConfigGroup(&views, QString::number(containment()->screen()));
}

void ContainmentView::setContainment(Plasma::Containment *cont)
{
    d->setContainment(cont);
}

Plasma::Containment *ContainmentView::containment() const
{
    return d->containment;
}

void ContainmentView::setLocation(Plasma::Types::Location location)
{
    d->containment->setLocation(location);
}

Plasma::Types::Location ContainmentView::location() const
{
    return d->location();
}

Plasma::Types::FormFactor ContainmentView::formFactor() const
{
    return d->formFactor();
}

QRectF ContainmentView::screenGeometry()
{
    return screen()->geometry();
}

void ContainmentView::showConfigurationInterface(Plasma::Applet *applet)
{
    d->showConfigurationInterface(applet);
}

}

#include "moc_containmentview.cpp"
