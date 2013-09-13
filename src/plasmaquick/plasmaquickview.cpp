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

#include "plasmaquickview.h"
#include "private/containmentconfigview_p.h"
#include "configview.h"

#include <QDebug>
#include <QQuickItem>
#include <QQmlContext>
#include <QTimer>
#include <QScreen>

#include "plasma/pluginloader.h"

class PlasmaQuickViewPrivate
{
public:

    PlasmaQuickViewPrivate(Plasma::Corona *corona, PlasmaQuickView *view);
    ~PlasmaQuickViewPrivate();

    void setContainment(Plasma::Containment *cont);
    Plasma::Types::FormFactor formFactor() const;
    Plasma::Types::Location location() const;
    void showConfigurationInterface(Plasma::Applet *applet);

    PlasmaQuickView *q;
    friend class PlasmaQuickView;
    Plasma::Corona *corona;
    QWeakPointer<Plasma::Containment> containment;
    QWeakPointer<ConfigView> configView;
};

PlasmaQuickViewPrivate::PlasmaQuickViewPrivate(Plasma::Corona *cor, PlasmaQuickView *view)
    : q(view),
      corona(cor)
{
}

PlasmaQuickViewPrivate::~PlasmaQuickViewPrivate()
{
}

void PlasmaQuickViewPrivate::setContainment(Plasma::Containment *cont)
{
    if (containment.data() == cont) {
        return;
    }

    Plasma::Types::Location oldLoc = (Plasma::Types::Location)location();
    Plasma::Types::FormFactor oldForm = formFactor();

    if (containment) {
        QObject::disconnect(containment.data(), 0, q, 0);
        QObject *oldGraphicObject = containment.data()->property("graphicObject").value<QObject *>();
        if (oldGraphicObject) {
            //make sure the graphic object won't die with us
            oldGraphicObject->setParent(cont);
        }
    }

    containment = cont;

    if (oldLoc != location()) {
        emit q->locationChanged((Plasma::Types::Location)location());
    }
    if (oldForm != formFactor()) {
        emit q->formFactorChanged(formFactor());
    }

    emit q->containmentChanged();

    if (cont) {
        QObject::connect(cont, &Plasma::Containment::locationChanged,
                q, &PlasmaQuickView::locationChanged);
        QObject::connect(cont, &Plasma::Containment::formFactorChanged,
                q, &PlasmaQuickView::formFactorChanged);
        QObject::connect(cont, &Plasma::Containment::configureRequested,
                q, &PlasmaQuickView::showConfigurationInterface);
    } else {
        return;
    }

    QObject *graphicObject = containment.data()->property("graphicObject").value<QObject *>();


    if (graphicObject) {
        qDebug() << "using as graphic containment" << graphicObject << containment.data();

        //graphicObject->setProperty("visible", false);
        graphicObject->setProperty("drawWallpaper",
                                   (cont->containmentType() == Plasma::Types::DesktopContainment ||
                                    cont->containmentType() == Plasma::Types::CustomContainment));
        graphicObject->setProperty("parent", QVariant::fromValue(q->rootObject()));
        q->rootObject()->setProperty("containment", QVariant::fromValue(graphicObject));
    } else {
        qWarning() << "Containment graphic object not valid";
    }
}

Plasma::Types::Location PlasmaQuickViewPrivate::location() const
{
    if (!containment) {
        return Plasma::Types::Desktop;
    }
    return containment.data()->location();
}

Plasma::Types::FormFactor PlasmaQuickViewPrivate::formFactor() const
{
    if (!containment) {
        return Plasma::Types::Planar;
    }
    return containment.data()->formFactor();
}

void PlasmaQuickViewPrivate::showConfigurationInterface(Plasma::Applet *applet)
{
    if (configView) {
        configView.data()->hide();
        configView.data()->deleteLater();
    }

    if (!applet || !applet->containment()) {
        return;
    }

    Plasma::Containment *cont = qobject_cast<Plasma::Containment *>(applet);

    if (cont) {
        configView = new ContainmentConfigView(cont);
    } else {
        configView = new ConfigView(applet);
    }
    configView.data()->init();
    configView.data()->show();
}




PlasmaQuickView::PlasmaQuickView(Plasma::Corona *corona, QWindow *parent)
    : QQuickView(parent),
      d(new PlasmaQuickViewPrivate(corona, this))
{
    QSurfaceFormat format;
    format.setAlphaBufferSize(8);

    setFormat(format);
    setColor(Qt::transparent);


    QObject::connect(screen(), &QScreen::virtualGeometryChanged,
            this, &PlasmaQuickView::screenGeometryChanged);

    if (!corona->package().isValid()) {
        qWarning() << "Invalid home screen package";
    }

    setResizeMode(PlasmaQuickView::SizeRootObjectToView);
    setSource(QUrl::fromLocalFile(corona->package().filePath("views", "Desktop.qml")));

    QObject::connect(corona, &Plasma::Corona::packageChanged,
            this, &PlasmaQuickView::coronaPackageChanged);
}

PlasmaQuickView::~PlasmaQuickView()
{
    delete d;
}

Plasma::Corona *PlasmaQuickView::corona() const
{
    return d->corona;
}

KConfigGroup PlasmaQuickView::config() const
{
    if (!containment()) {
        return KConfigGroup();
    }
    KConfigGroup views(KSharedConfig::openConfig(), "PlasmaViews");
    return KConfigGroup(&views, QString::number(containment()->screen()));
}

void PlasmaQuickView::setContainment(Plasma::Containment *cont)
{
    d->setContainment(cont);
}

Plasma::Containment *PlasmaQuickView::containment() const
{
    return d->containment.data();
}

void PlasmaQuickView::setLocation(Plasma::Types::Location location)
{
    d->containment.data()->setLocation(location);
}

Plasma::Types::Location PlasmaQuickView::location() const
{
    return d->location();
}

Plasma::Types::FormFactor PlasmaQuickView::formFactor() const
{
    return d->formFactor();
}

QRectF PlasmaQuickView::screenGeometry()
{
    return screen()->geometry();
}

void PlasmaQuickView::showConfigurationInterface(Plasma::Applet *applet)
{
    d->showConfigurationInterface(applet);
}

void PlasmaQuickView::coronaPackageChanged(const Plasma::Package &package)
{
    setContainment(0);
    setSource(QUrl::fromLocalFile(package.filePath("views", "Desktop.qml")));
}

#include "moc_plasmaquickview.cpp"
