/*
 *   Copyright 2008 Chani Armitage <chani@kde.org>
 *   Copyright 2008, 2009 Aaron Seigo <aseigo@kde.org>
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

#include "containmentinterface.h"

#include <QQmlExpression>
#include <QQmlProperty>

#include <KAuthorized>
#include <KDebug>
#include <KLocalizedString>

#include <Plasma/Corona>
#include <Plasma/Package>
#include <Plasma/PluginLoader>

#include "declarative/qmlobject.h"

ContainmentInterface::ContainmentInterface(DeclarativeAppletScript *parent)
    : AppletInterface(parent),
      m_wallpaperQmlObject(0)
{
    //TODO: will accept all events specified as registered with containment actions
    setAcceptedMouseButtons(Qt::RightButton);

    qmlRegisterType<ContainmentInterface>();

    connect(containment(), SIGNAL(appletRemoved(Plasma::Applet *)), this, SLOT(appletRemovedForward(Plasma::Applet *)));
    connect(containment(), SIGNAL(appletAdded(Plasma::Applet *, const QPointF &)), this, SLOT(appletAddedForward(Plasma::Applet *, const QPointF &)));
    connect(containment(), SIGNAL(screenChanged(int, int, Plasma::Containment*)), this, SIGNAL(screenChanged()));
    connect(containment(), SIGNAL(activityChanged()), this, SIGNAL(activityChanged()));
    connect(containment(), SIGNAL(wallpaperChanged()), this, SLOT(loadWallpaper()));

     if (containment()->corona()) {
         connect(containment()->corona(), SIGNAL(availableScreenRegionChanged()),
                 this, SIGNAL(availableScreenRegionChanged()));
     }
     loadWallpaper();
}

QList <QObject *> ContainmentInterface::applets()
{
    return m_appletInterfaces;
}

void ContainmentInterface::setDrawWallpaper(bool drawWallpaper)
{
    if (drawWallpaper == m_appletScriptEngine->drawWallpaper()) {
        return;
    }

    m_appletScriptEngine->setDrawWallpaper(drawWallpaper);

    loadWallpaper();
}

bool ContainmentInterface::drawWallpaper()
{
    return m_appletScriptEngine->drawWallpaper();
}

ContainmentInterface::Type ContainmentInterface::containmentType() const
{
    return (ContainmentInterface::Type)m_appletScriptEngine->containmentType();
}

void ContainmentInterface::setContainmentType(ContainmentInterface::Type type)
{
    m_appletScriptEngine->setContainmentType((Plasma::Containment::Type)type);
}

int ContainmentInterface::screen() const
{
    return containment()->screen();
}

QRectF ContainmentInterface::screenGeometry(int id) const
{
    QRectF rect;
    if (containment()->corona()) {
        rect = QRectF(containment()->corona()->screenGeometry(id));
    }

    return rect;
}

QVariantList ContainmentInterface::availableScreenRegion(int id) const
{
    QRegion reg;
    if (containment()->corona()) {
        reg = containment()->corona()->availableScreenRegion(id);
    }

    QVariantList regVal;
    foreach (QRect rect, reg.rects()) {
        regVal << QVariant::fromValue(QRectF(rect));
    }
    return regVal;
}

void ContainmentInterface::appletAddedForward(Plasma::Applet *applet, const QPointF &pos)
{
    QObject *appletGraphicObject = applet->property("graphicObject").value<QObject *>();
    QObject *contGraphicObject = containment()->property("graphicObject").value<QObject *>();

    qDebug() << "Applet added:" << applet << applet->title() << appletGraphicObject;

    if (applet && contGraphicObject && appletGraphicObject) {
        appletGraphicObject->setProperty("visible", false);
        appletGraphicObject->setProperty("parent", QVariant::fromValue(contGraphicObject));

    //if an appletGraphicObject is not set, we have to display some error message
    } else if (applet && contGraphicObject) {
        QQmlComponent *component = new QQmlComponent(m_appletScriptEngine->engine(), applet);
        component->loadUrl(QUrl::fromLocalFile(containment()->corona()->package().filePath("ui", "AppletError.qml")));
        QObject *errorUi = component->create();

        if (errorUi) {
            errorUi->setProperty("visible", false);
            errorUi->setProperty("parent", QVariant::fromValue(contGraphicObject));
            errorUi->setProperty("reason", applet->launchErrorMessage());
            appletGraphicObject = errorUi;
        }
    }

    m_appletInterfaces << appletGraphicObject;
    emit appletAdded(appletGraphicObject, pos);
    emit appletsChanged();
}

void ContainmentInterface::appletRemovedForward(Plasma::Applet *applet)
{
    QObject *appletGraphicObject = applet->property("graphicObject").value<QObject *>();
    m_appletInterfaces.removeAll(appletGraphicObject);
    emit appletRemoved(appletGraphicObject);
    emit appletsChanged();
}

void ContainmentInterface::loadWallpaper()
{
    if (m_appletScriptEngine->drawWallpaper()) {
        if (m_wallpaperQmlObject) {
            return;
        }

        Plasma::Package pkg = Plasma::PluginLoader::self()->loadPackage("Plasma/Generic");
        pkg.setPath("org.kde.wallpaper.image");

        m_wallpaperQmlObject = new QmlObject(this);
        m_wallpaperQmlObject->setQmlPath(pkg.filePath("mainscript"));

        if (m_wallpaperQmlObject->mainComponent() &&
            m_wallpaperQmlObject->rootObject() &&
            !m_wallpaperQmlObject->mainComponent()->isError()) {
            m_wallpaperQmlObject->rootObject()->setProperty("z", -1000);
            m_wallpaperQmlObject->rootObject()->setProperty("parent", QVariant::fromValue(this));

            //set anchors
            QQmlExpression expr(m_appletScriptEngine->engine()->rootContext(), m_wallpaperQmlObject->rootObject(), "parent");
            QQmlProperty prop(m_wallpaperQmlObject->rootObject(), "anchors.fill");
            prop.write(expr.evaluate());

        } else if (m_wallpaperQmlObject->mainComponent()) {
            qWarning() << "Error loading the wallpaper" << m_wallpaperQmlObject->mainComponent()->errors();
            m_wallpaperQmlObject->deleteLater();
            m_wallpaperQmlObject = 0;

        } else {
            qWarning() << "Error loading the wallpaper, package not found";
        }

    } else {
        if (m_wallpaperQmlObject) {
            m_wallpaperQmlObject->deleteLater();
            m_wallpaperQmlObject = 0;
        }
    }
}

QString ContainmentInterface::activityId() const
{
    return containment()->activity();
}





//PROTECTED--------------------

void ContainmentInterface::mousePressEvent(QMouseEvent *event)
{
    event->accept();
}

void ContainmentInterface::mouseReleaseEvent(QMouseEvent *event)
{
    KMenu desktopMenu;

    //FIXME: very inefficient appletAt() implementation
    Plasma::Applet *applet = 0;
    foreach (QObject *appletObject, m_appletInterfaces) {
        if (AppletInterface *ai = qobject_cast<AppletInterface *>(appletObject)) {
            if (ai->contains(ai->mapFromItem(this, event->posF()))) {
                applet = ai->applet();
                break;
            } else {
                ai = 0;
            }
        }
    }
    qDebug() << "Invoking menu for applet" << applet;

    if (applet) {
        addAppletActions(desktopMenu, applet, event);
    } else {
        addContainmentActions(desktopMenu, event);
    }
    desktopMenu.exec(event->globalPos());
    event->accept();
}



void ContainmentInterface::addAppletActions(KMenu &desktopMenu, Plasma::Applet *applet, QEvent *event)
{
    foreach (QAction *action, applet->contextualActions()) {
        if (action) {
            desktopMenu.addAction(action);
        }
    }

    if (!applet->failedToLaunch()) {
        QAction *configureApplet = applet->action("configure");
        if (configureApplet && configureApplet->isEnabled()) {
            desktopMenu.addAction(configureApplet);
        }

        QAction *runAssociatedApplication = applet->action("run associated application");
        if (runAssociatedApplication && runAssociatedApplication->isEnabled()) {
            desktopMenu.addAction(runAssociatedApplication);
        }
    }

    KMenu *containmentMenu = new KMenu(i18nc("%1 is the name of the containment", "%1 Options", containment()->title()), &desktopMenu);
    addContainmentActions(*containmentMenu, event);

    if (!containmentMenu->isEmpty()) {
        int enabled = 0;
        //count number of real actions
        QListIterator<QAction *> actionsIt(containmentMenu->actions());
        while (enabled < 3 && actionsIt.hasNext()) {
            QAction *action = actionsIt.next();
            if (action->isVisible() && !action->isSeparator()) {
                ++enabled;
            }
        }

        if (enabled) {
            //if there is only one, don't create a submenu
            if (enabled < 2) {
                foreach (QAction *action, containmentMenu->actions()) {
                    if (action->isVisible() && !action->isSeparator()) {
                        desktopMenu.addAction(action);
                    }
                }
            } else {
                desktopMenu.addMenu(containmentMenu);
            }
        }
    }

    if (containment()->immutability() == Plasma::Mutable) {
        QAction *closeApplet = applet->action("remove");
        //kDebug() << "checking for removal" << closeApplet;
        if (closeApplet) {
            if (!desktopMenu.isEmpty()) {
                desktopMenu.addSeparator();
            }

            //kDebug() << "adding close action" << closeApplet->isEnabled() << closeApplet->isVisible();
            desktopMenu.addAction(closeApplet);
        }
    }
}

void ContainmentInterface::addContainmentActions(KMenu &desktopMenu, QEvent *event)
{
    if (containment()->corona()->immutability() != Plasma::Mutable &&
        !KAuthorized::authorizeKAction("plasma/containment_actions")) {
        //kDebug() << "immutability";
        return;
    }

    desktopMenu.addAction("Containment Menu Item 1");
    desktopMenu.addAction("Containment Menu Item 2");
    //TODO: reenable ContainmentActions plugins
    /*const QString trigger = ContainmentActions::eventToString(event);
    prepareContainmentActions(trigger, QPoint(), &desktopMenu);*/
}

#include "moc_containmentinterface.cpp"
