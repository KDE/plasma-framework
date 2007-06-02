/*
 *   Copyright (C) 2005 by Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include "applet.h"

#include <QEvent>
#include <QList>
#include <QSize>
#include <QStringList>
#include <QTimer>

#include <KPluginInfo>
#include <KStandardDirs>
#include <KService>
#include <KServiceTypeTrader>

#include "corona.h"
#include "dataenginemanager.h"

namespace Plasma
{

class Applet::Private
{
    public:
        Private( KService::Ptr appletDescription, int uniqueID )
            : appletId( uniqueID ),
              globalConfig( 0 ),
              appletConfig( 0 ),
              appletDescription(new KPluginInfo(appletDescription))
        {
            if (appletId > s_maxAppletId) {
                s_maxAppletId = appletId;
            }
        }

        ~Private()
        {
            foreach ( const QString& engine, loadedEngines ) {
               DataEngineManager::self()->unloadDataEngine( engine );
            }
            delete appletDescription;
        }

        static uint nextId()
        {
            ++s_maxAppletId;
            return s_maxAppletId;
        }

        uint appletId;
        KSharedConfig::Ptr globalConfig;
        KSharedConfig::Ptr appletConfig;
        KPluginInfo* appletDescription;
        QList<QObject*> watchedForFocus;
        QStringList loadedEngines;
        static uint s_maxAppletId;
};

uint Applet::Private::s_maxAppletId = 0;

Applet::Applet(QGraphicsItem *parent,
               const QString& serviceID,
               int appletId)
        : QObject(0),
          QGraphicsItemGroup(parent),
          d(new Private(KService::serviceByStorageId(serviceID), appletId))
{
}

Applet::Applet(QObject* parent, const QStringList& args)
    : QObject(parent),
      QGraphicsItemGroup(0),
      d(new Private(KService::serviceByStorageId(args[0]), args[1].toInt()))
{
    // the brain damage seen in the initialization list is due to the 
    // rediculous inflexibility of KService::createInstance
    // too bad i couldn't convince others that this was a real issue.
}

Applet::~Applet()
{
    needsFocus( false );
    delete d;
}

KConfigGroup Applet::appletConfig() const
{
    if ( !d->appletConfig ) {
        QString file = KStandardDirs::locateLocal( "appdata",
                                                   "applets/" + instanceName() + "rc",
                                                   true );
        d->appletConfig = KSharedConfig::openConfig( file );
    }

    return KConfigGroup(d->appletConfig, "General");
}

KConfigGroup Applet::globalAppletConfig() const
{
    if ( !d->globalConfig ) {
        QString file = KStandardDirs::locateLocal( "config", "plasma_" + globalName() + "rc" );
        d->globalConfig = KSharedConfig::openConfig( file );
    }

    return KConfigGroup(d->globalConfig, "General");
}

DataEngine* Applet::dataEngine(const QString& name)
{
    int index = d->loadedEngines.indexOf(name);
    if (index != -1) {
        return DataEngineManager::self()->dataEngine(name);
    }

    DataEngine* engine = DataEngineManager::self()->loadDataEngine(name);
    if (engine) {
        d->loadedEngines.append(name);
        return engine;
    }

    return 0;
}

void Applet::constraintsUpdated()
{
    kDebug() << "Applet::constraintsUpdate(): constraints are FormFactor: " << formFactor() << ", Location: " << location() << endl;
}

FormFactor Applet::formFactor() const
{
    if (!scene()) {
        return Plasma::Planar;
    }

    return static_cast<Corona*>(scene())->formFactor();
}

Location Applet::location() const
{
    if (!scene()) {
        return Plasma::Desktop;
    }

    return static_cast<Corona*>(scene())->location();
}

QString Applet::globalName() const
{
    return d->appletDescription->service()->library();
}

QString Applet::instanceName() const
{
    return d->appletDescription->service()->library() + QString::number( d->appletId );
}

void Applet::watchForFocus(QObject *widget, bool watch)
{
    if ( !widget ) {
        return;
    }

    int index = d->watchedForFocus.indexOf(widget);
    if ( watch ) {
        if ( index == -1 ) {
            d->watchedForFocus.append( widget );
            widget->installEventFilter( this );
        }
    } else if ( index != -1 ) {
        d->watchedForFocus.removeAt( index );
        widget->removeEventFilter( this );
    }
}

void Applet::needsFocus( bool focus )
{
    if (focus == QGraphicsItem::hasFocus()) {
        return;
    }

    emit requestFocus(focus);
}

bool Applet::eventFilter( QObject *o, QEvent * e )
{
    if ( !d->watchedForFocus.contains( o ) )
    {
        if ( e->type() == QEvent::MouseButtonRelease ||
             e->type() == QEvent::FocusIn ) {
            needsFocus( true );
        } else if ( e->type() == QEvent::FocusOut ) {
            needsFocus( false );
        }
    }

    return QObject::eventFilter(o, e);
}

KPluginInfo::List Applet::knownApplets()
{
    QHash<QString, KPluginInfo> applets;
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet");
    return KPluginInfo::fromServices(offers);
}

Applet* Applet::loadApplet(const QString& appletName, uint appletId)
{
    if (appletName.isEmpty()) {
        return 0;
    }

    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(appletName);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);

    if (offers.isEmpty()) {
        //TODO: what would be -really- cool is offer to try and download the applet
        //      from the network at this point
        kDebug() << "Applet::loadApplet: offers is empty for \"" << appletName << "\"" << endl;
        return 0;
    }

    if (appletId == 0) {
        appletId = Private::nextId();
    }

    QStringList sillyness;
    QString id;
    id.setNum(appletId);
    sillyness << offers.first()->storageId() << id;
    Applet* applet = KService::createInstance<Plasma::Applet>(offers.first(), 0, sillyness);

    if (!applet) {
        kDebug() << "Couldn't load applet \"" << appletName << "\"!" << endl;
    }

    return applet;
}

Applet* Applet::loadApplet(const KPluginInfo* info, uint appletId)
{
    if (!info) {
        return 0;
    }

    return loadApplet(info->pluginName(), appletId);
}

} // Plasma namespace

#include "applet.moc"
