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

#include "containmentview.h"
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
    : q(view),
      corona(cor)
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
        QObject::disconnect(containment, 0, q, 0);
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

    //we are QuickViewSharedEngine::SizeRootObjectToView, but that's not enough, as
    //the root object isn't immediately resized (done at the resizeEvent handler).
    //by resising it just before restoring the containment, it removes a chain of resizes at startup
    if (q->rootObject()) {
        q->rootObject()->setSize(q->size());
    }
    if (cont) {
        cont->reactToScreenChange();
        QObject::connect(cont, &Plasma::Containment::locationChanged,
                         q, &ContainmentView::locationChanged);
        QObject::connect(cont, &Plasma::Containment::formFactorChanged,
                         q, &ContainmentView::formFactorChanged);
        QObject::connect(cont, &Plasma::Containment::configureRequested,
                         q, &ContainmentView::showConfigurationInterface);
        QObject::connect(cont, SIGNAL(destroyedChanged(bool)),
                         q, SLOT(updateDestroyed(bool)));

        // Panels are created invisible and the code below ensures they are only
        // shown once their contents have settled to avoid visual glitches on startup
        if (cont->containmentType() == Plasma::Types::PanelContainment ||
            cont->containmentType() == Plasma::Types::CustomPanelContainment) {

            QObject::connect(cont, &Plasma::Containment::uiReadyChanged,
                             q, [this, cont](bool ready) {
                if (ready && !cont->destroyed()) {
                    q->setVisible(true);
                }
            }, Qt::QueuedConnection);

            q->setVisible(!cont->destroyed() && cont->isUiReady());
        }
    } else {
        return;
    }

    QQuickItem *graphicObject = qobject_cast<QQuickItem *>(containment->property("_plasma_graphicObject").value<QObject *>());

    if (graphicObject) {
//         qDebug() << "using as graphic containment" << graphicObject << containment.data();

        //by resizing before adding, it will avoid some resizes in most cases
        graphicObject->setProperty("width", q->width());
        graphicObject->setProperty("height", q->height());
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
    : KQuickAddons::QuickViewSharedEngine(parent),
      d(new ContainmentViewPrivate(corona, this))
{
    setColor(Qt::transparent);

    QObject::connect(screen(), &QScreen::geometryChanged,
                     this, &ContainmentView::screenGeometryChanged);

    if (corona->kPackage().isValid()) {
        const auto info = corona->kPackage().metadata();
        if (info.isValid()) {
            setTranslationDomain("plasma_shell_" + info.pluginId());
        } else {
            qWarning() << "Invalid corona package metadata";
        }
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

    setResizeMode(ContainmentView::SizeRootObjectToView);
}

ContainmentView::~ContainmentView()
{
    delete d;
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
