/*
 *  Copyright 2013 Marco Martin <mart@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "view.h"
#include "configview.h"

#include <QDebug>
#include <QQuickItem>
#include <QQmlContext>
#include <QTimer>
#include <QScreen>
#include <QQmlEngine>

#include "plasma/pluginloader.h"
#include <packageurlinterceptor.h>
#include <kdeclarative/kdeclarative.h>

namespace PlasmaQuick
{

class ViewPrivate
{
public:

    ViewPrivate(Plasma::Corona *corona, View *view);
    ~ViewPrivate();

    void setContainment(Plasma::Containment *cont);
    Plasma::Types::FormFactor formFactor() const;
    Plasma::Types::Location location() const;
    void showConfigurationInterface(Plasma::Applet *applet);
    void updateDestroyed(bool destroyed);

    View *q;
    friend class View;
    Plasma::Corona *corona;
    QPointer<Plasma::Containment> containment;
    QPointer<ConfigView> configView;
};

ViewPrivate::ViewPrivate(Plasma::Corona *cor, View *view)
    : q(view),
      corona(cor)
{
}

ViewPrivate::~ViewPrivate()
{
}

void ViewPrivate::setContainment(Plasma::Containment *cont)
{
    if (containment == cont) {
        return;
    }

    Plasma::Types::Location oldLoc = location();
    Plasma::Types::FormFactor oldForm = formFactor();

    if (containment) {
        QObject::disconnect(containment, nullptr, q, nullptr);
        QObject *oldGraphicObject = containment->property("_plasma_graphicObject").value<QObject *>();
        if (oldGraphicObject) {
//             qDebug() << "Old graphics Object:" << oldGraphicObject << "Old containment" << containment.data();
            //make sure the graphic object won't die with us
            //FIXME:we need a way to reparent to *NO* graphics item, but this makes Qt crash
            oldGraphicObject->setParent(containment);
        }
        containment->reactToScreenChange();
    }

    containment = cont;

    if (oldLoc != location()) {
        emit q->locationChanged(location());
    }
    if (oldForm != formFactor()) {
        emit q->formFactorChanged(formFactor());
    }

    emit q->containmentChanged();

    if (cont) {
        cont->reactToScreenChange();
        QObject::connect(cont, &Plasma::Containment::locationChanged,
                         q, &View::locationChanged);
        QObject::connect(cont, &Plasma::Containment::formFactorChanged,
                         q, &View::formFactorChanged);
        QObject::connect(cont, &Plasma::Containment::configureRequested,
                         q, &View::showConfigurationInterface);
        QObject::connect(cont, SIGNAL(destroyedChanged(bool)),
                         q, SLOT(updateDestroyed(bool)));
        if (cont->containmentType() == Plasma::Types::PanelContainment ||
            cont->containmentType() == Plasma::Types::CustomPanelContainment) {
            q->setVisible(!cont->destroyed() && cont->isUiReady());
        }
    } else {
        return;
    }

    QQuickItem *graphicObject = qobject_cast<QQuickItem *>(containment->property("_plasma_graphicObject").value<QObject *>());

    if (graphicObject) {
//         qDebug() << "using as graphic containment" << graphicObject << containment.data();

        //by resizing before adding, it will avoid some resizes in most cases
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

Plasma::Types::Location ViewPrivate::location() const
{
    if (!containment) {
        return Plasma::Types::Desktop;
    }
    return containment->location();
}

Plasma::Types::FormFactor ViewPrivate::formFactor() const
{
    if (!containment) {
        return Plasma::Types::Planar;
    }
    return containment->formFactor();
}

void ViewPrivate::showConfigurationInterface(Plasma::Applet *applet)
{
    if (configView) {
        if (configView->applet() != applet) {
            configView->hide();
            configView->deleteLater();
        } else {
            configView->raise();
            configView->requestActivate();
            return;
        }
    }

    if (!applet || !applet->containment()) {
        return;
    }

    configView = new ConfigView(applet);

    configView->init();
    configView->show();
}

void ViewPrivate::updateDestroyed(bool destroyed)
{
    q->setVisible(!destroyed);
}

View::View(Plasma::Corona *corona, QWindow *parent)
    : QQuickView(parent),
      d(new ViewPrivate(corona, this))
{
    setColor(Qt::transparent);

    QObject::connect(screen(), &QScreen::geometryChanged,
                     this, &View::screenGeometryChanged);

    const auto pkg = corona->kPackage();
    if (pkg.isValid()) {
        PackageUrlInterceptor *interceptor = new PackageUrlInterceptor(engine(), pkg);
        interceptor->setForcePlasmaStyle(true);
        engine()->setUrlInterceptor(interceptor);

        KDeclarative::KDeclarative kdeclarative;
        kdeclarative.setDeclarativeEngine(engine());
        //binds things like kconfig and icons
        kdeclarative.setTranslationDomain(QStringLiteral("plasma_shell_") + pkg.metadata().pluginId());
        kdeclarative.setupBindings();
    } else {
        qWarning() << "Invalid home screen package";
    }

    //Force QtQuickControls to use the "Plasma" style for this engine.
    //this way is possible to mix QtQuickControls and plasma components in applets
    //while still having the desktop style in configuration dialogs
    QQmlComponent c(engine());
    c.setData("import QtQuick 2.1\n\
        import QtQuick.Controls 1.0\n\
        import QtQuick.Controls.Private 1.0\n \
        Item {\
          Component.onCompleted: {\
            Settings.styleName = \"Plasma\";\
          }\
        }", QUrl());
    QObject *o = c.create();
    o->deleteLater();

    setResizeMode(View::SizeRootObjectToView);
}

View::~View()
{
    delete d;
}

Plasma::Corona *View::corona() const
{
    return d->corona;
}

KConfigGroup View::config() const
{
    if (!containment()) {
        return KConfigGroup();
    }
    KConfigGroup views(KSharedConfig::openConfig(), "PlasmaViews");
    return KConfigGroup(&views, QString::number(containment()->screen()));
}

void View::setContainment(Plasma::Containment *cont)
{
    d->setContainment(cont);
}

Plasma::Containment *View::containment() const
{
    return d->containment;
}

void View::setLocation(Plasma::Types::Location location)
{
    d->containment->setLocation(location);
}

Plasma::Types::Location View::location() const
{
    return d->location();
}

Plasma::Types::FormFactor View::formFactor() const
{
    return d->formFactor();
}

QRectF View::screenGeometry()
{
    return screen()->geometry();
}

void View::showConfigurationInterface(Plasma::Applet *applet)
{
    d->showConfigurationInterface(applet);
}

}

#include "moc_view.cpp"
