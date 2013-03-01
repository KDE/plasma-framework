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
#include "wallpaperinterface.h"
#include "declarative/qmlobject.h"

#include <QQmlExpression>
#include <QQmlProperty>

#include <KActionCollection>
#include <KAuthorized>
#include <KDebug>
#include <KLocalizedString>

#include <Plasma/ContainmentActions>
#include <Plasma/Corona>
#include <Plasma/Package>
#include <Plasma/PluginLoader>


ContainmentInterface::ContainmentInterface(DeclarativeAppletScript *parent)
    : AppletInterface(parent),
      m_wallpaperInterface(0)
{
    //TODO: will accept all events specified as registered with containment actions
    setAcceptedMouseButtons(Qt::RightButton);

    qmlRegisterType<ContainmentInterface>();

    connect(containment(), &Plasma::Containment::appletRemoved,
            this, &ContainmentInterface::appletRemovedForward);
    connect(containment(), &Plasma::Containment::appletAdded,
            this, &ContainmentInterface::appletAddedForward);
    connect(containment(), &Plasma::Containment::screenChanged,
            this, &ContainmentInterface::screenChanged);
    connect(containment(), &Plasma::Containment::activityChanged,
            this, &ContainmentInterface::activityChanged);
    connect(containment(), &Plasma::Containment::wallpaperChanged,
            this, &ContainmentInterface::loadWallpaper);

     if (containment()->corona()) {
         connect(containment()->corona(), &Plasma::Corona::availableScreenRegionChanged,
                 this, &ContainmentInterface::availableScreenRegionChanged);
     }
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
    m_appletScriptEngine->setContainmentType((Plasma::ContainmentType)type);
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

void ContainmentInterface::appletAddedForward(Plasma::Applet *applet)
{
    QObject *appletGraphicObject = applet->property("graphicObject").value<QObject *>();
    QObject *contGraphicObject = containment()->property("graphicObject").value<QObject *>();

    qDebug() << "Applet added:" << applet << applet->title() << appletGraphicObject;

    if (applet && contGraphicObject && appletGraphicObject) {
        appletGraphicObject->setProperty("visible", false);
        appletGraphicObject->setProperty("parent", QVariant::fromValue(contGraphicObject));

    //if an appletGraphicObject is not set, we have to display some error message
    } else if (applet && contGraphicObject) {
        QObject *errorUi = qmlObject()->createObjectFromSource(QUrl::fromLocalFile(containment()->corona()->package().filePath("ui", "AppletError.qml")));

        if (errorUi) {
            errorUi->setProperty("visible", false);
            errorUi->setProperty("parent", QVariant::fromValue(contGraphicObject));
            errorUi->setProperty("reason", applet->launchErrorMessage());
            appletGraphicObject = errorUi;
        }
    }

    m_appletInterfaces << appletGraphicObject;
    emit appletAdded(appletGraphicObject);
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
        if (m_wallpaperInterface || containment()->wallpaper().isEmpty()) {
            return;
        }

        m_wallpaperInterface = new WallpaperInterface(this);
        m_wallpaperInterface->setZ(-1000);
        //Qml seems happier if the parent gets set in this way
        m_wallpaperInterface->setProperty("parent", QVariant::fromValue(this));

        //set anchors
        QQmlExpression expr(qmlObject()->engine()->rootContext(), m_wallpaperInterface, "parent");
        QQmlProperty prop(m_wallpaperInterface, "anchors.fill");
        prop.write(expr.evaluate());

    } else {
        if (m_wallpaperInterface) {
            m_wallpaperInterface->deleteLater();
            m_wallpaperInterface = 0;
        }
    }
}

QString ContainmentInterface::activity() const
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
        QAction *configureApplet = applet->actions()->action("configure");
        if (configureApplet && configureApplet->isEnabled()) {
            desktopMenu.addAction(configureApplet);
        }

        QAction *runAssociatedApplication = applet->actions()->action("run associated application");
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
        QAction *closeApplet = applet->actions()->action("remove");
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

    //this is what ContainmentPrivate::prepareContainmentActions was
    const QString trigger = Plasma::ContainmentActions::eventToString(event);
    Plasma::ContainmentActions *plugin = containment()->containmentActions().value(trigger);

    if (!plugin) {
        //FIXME: this action is here only for testing purposes, remove it when plugins work
        desktopMenu.addAction(containment()->actions()->action("configure"));
        return;
    }

    if (plugin->containment() != containment()) {
        plugin->setContainment(containment());

        // now configure it
        KConfigGroup cfg = plugin->config();
        KConfigGroup pluginConfig = KConfigGroup(&cfg, trigger);
        plugin->restore(pluginConfig);
    }


    QList<QAction*> actions = plugin->contextualActions();

    if (actions.isEmpty()) {
        //it probably didn't bother implementing the function. give the user a chance to set
        //a better plugin.  note that if the user sets no-plugin this won't happen...
        if ((containment()->containmentType() != Plasma::PanelContainment &&
             containment()->containmentType() != Plasma::CustomPanelContainment) &&
            containment()->actions()->action("configure")) {
            desktopMenu.addAction(containment()->actions()->action("configure"));
        }
    } else {
        desktopMenu.addActions(actions);
    }


    return;
}

#include "moc_containmentinterface.cpp"
