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

#include <QApplication>
#include <QEvent>
#include <QList>
#include <QPainter>
#include <QSize>
#include <QTimer>

#include <KDialog>
#include <KPluginInfo>
#include <KStandardDirs>
#include <KService>
#include <KServiceTypeTrader>

#include "plasma/corona.h"
#include "plasma/dataenginemanager.h"
#include "plasma/plasma.h"
#include "plasma/svg.h"

#include "plasma/widgets/widget.h"
#include "plasma/widgets/lineedit.h"
#include "plasma/widgets/vboxlayout.h"

namespace Plasma
{

class Applet::Private
{
    public:
        Private( KService::Ptr appletDescription, int uniqueID )
            : appletId( uniqueID ),
              globalConfig( 0 ),
              appletConfig( 0 ),
              appletDescription(new KPluginInfo(appletDescription)),
              background(0),
              failureText(0),
              immutable(false),
              hasConfigurationInterface(false),
              failed(false)
        {
            if (appletId == 0) {
                appletId = nextId();
            }

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
            delete background;
        }

        static uint nextId()
        {
            ++s_maxAppletId;
            return s_maxAppletId;
        }

        //TODO: examine the usage of memory here; there's a pretty large
        //      number of members at this point.
        uint appletId;
        KSharedConfig::Ptr globalConfig;
        KSharedConfig::Ptr appletConfig;
        KPluginInfo* appletDescription;
        QList<QObject*> watchedForFocus;
        QStringList loadedEngines;
        static uint s_maxAppletId;
        Plasma::Svg *background;
        Plasma::LineEdit *failureText;
        bool immutable : 1;
        bool hasConfigurationInterface : 1;
        bool failed : 1;
};

uint Applet::Private::s_maxAppletId = 0;

Applet::Applet(QGraphicsItem *parent,
               const QString& serviceID,
               uint appletId)
        : QObject(0),
          Widget(parent),
          d(new Private(KService::serviceByStorageId(serviceID), appletId))
{
    init();
}

Applet::Applet(QObject* parent, const QStringList& args)
    : QObject(parent),
      Widget(0),
      d(new Private(KService::serviceByStorageId(args.count() > 0 ? args[0] : QString()),
                    args.count() > 1 ? args[1].toInt() : 0))
{
    init();
    // the brain damage seen in the initialization list is due to the 
    // inflexibility of KService::createInstance
}

Applet::~Applet()
{
    needsFocus( false );
    delete d;
}

void Applet::init()
{
    setImmutable(globalConfig().isImmutable() ||
                 config().isImmutable());
}

KConfigGroup Applet::config() const
{
    if ( !d->appletConfig ) {
        QString file = KStandardDirs::locateLocal( "appdata",
                                                   "applets/" + instanceName() + "rc",
                                                   true );
        d->appletConfig = KSharedConfig::openConfig( file );
    }

    return KConfigGroup(d->appletConfig, "General");
}

KConfigGroup Applet::config(const QString& group) const
{
    KConfigGroup cg = config();
    cg.changeGroup(instanceName() + "-" + group);
    return cg;
}

KConfigGroup Applet::globalConfig() const
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
    if (engine->isValid()) {
        d->loadedEngines.append(name);
    }

    return engine;
}

void Applet::constraintsUpdated()
{
    kDebug() << "Applet::constraintsUpdate(): constraints are FormFactor: " << formFactor() << ", Location: " << location() << endl;
}

QString Applet::name() const
{
    return d->appletDescription->name();
}

QString Applet::category() const
{
    return d->appletDescription->property("X-PlasmoidCategory").toString();
}

QString Applet::category(const KPluginInfo* applet)
{
    return applet->property("X-PlasmoidCategory").toString();
}

QString Applet::category(const QString& appletName)
{
    if (appletName.isEmpty()) {
        return QString();
    }

    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(appletName);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);

    if (offers.isEmpty()) {
        return QString();
    }

    return offers.first()->property("X-PlasmoidCategory").toString();
}

bool Applet::immutable() const
{
    return d->immutable;
}

void Applet::setImmutable(bool immutable)
{
    d->immutable = immutable;
    QGraphicsItem::GraphicsItemFlags f = flags();
    if (immutable) {
        f ^= QGraphicsItem::ItemIsMovable;
    } else if (!scene() || !static_cast<Corona*>(scene())->immutable()) {
        f |= QGraphicsItem::ItemIsMovable;
    }
    setFlags(f);
}

bool Applet::drawStandardBackground()
{
    return d->background != 0;
}

void Applet::setDrawStandardBackground(bool drawBackground)
{
    if (drawBackground) {
        if (!d->background) {
            d->background = new Plasma::Svg("widgets/background");
        }
    } else {
        delete d->background;
        d->background = 0;
    }
}

bool Applet::failedToLaunch() const
{
    return d->failed;
}

QString visibleFailureText(const QString& reason)
{
    QString text;

    if (reason.isEmpty()) {
        text = i18n("This object could not be created.");
    } else {
        text = i18n("This object could not be created for the following reason:<p>%1", reason);
    }

    return text;
}

void Applet::setFailedToLaunch(bool failed, const QString& reason)
{
    if (d->failed == failed) {
        if (d->failureText) {
            d->failureText->setHtml(visibleFailureText(reason));
        }
        return;
    }

    d->failed = failed;

    qDeleteAll(QGraphicsItem::children());
    delete layout();

    if (failed) {
        setDrawStandardBackground(failed || d->background != 0);
        Layout* failureLayout = new VBoxLayout(this);
        d->failureText = new LineEdit(this, scene());
        d->failureText->setFlags(0);
        d->failureText->setHtml(visibleFailureText(reason));
        failureLayout->addItem(d->failureText);
    } else {
        d->failureText = 0;
    }

    update();
}

QRectF Applet::boundingRect () const
{
    //FIXME: this should be big enough to allow for the failure text?
    return QRectF(300, 300, 300, 300);
}

void Applet::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (d->background) {
        d->background->resize(boundingRect().size());
        d->background->paint(painter, boundingRect());
    }

    if (d->failed) {
        return;
    }

    paintInterface(painter, option, widget);

    //TODO: interface overlays on hover
}

void Applet::paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(widget)
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

void Applet::needsFocus(bool focus)
{
    if (focus == QGraphicsItem::hasFocus()) {
        return;
    }

    emit requestFocus(focus);
}

bool Applet::hasConfigurationInterface()
{
    return d->hasConfigurationInterface;
}

void Applet::setHasConfigurationInterface(bool hasInterface)
{
    d->hasConfigurationInterface = hasInterface;
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

void Applet::showConfigurationInterface()
{
}

KPluginInfo::List Applet::knownApplets()
{
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet");
    return KPluginInfo::fromServices(offers);
}

QStringList Applet::knownCategories()
{
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", "exist [X-PlasmoidCategory]");
    QStringList categories;
    foreach (KService::Ptr applet, offers) {
        QString appletCategory = applet->property("X-PlasmoidCategory").toString();
        if (!appletCategory.isEmpty()) {
            if (!categories.contains(appletCategory)) {
                categories << appletCategory;
            }
        }
    }
    return categories;
}

Applet* Applet::loadApplet(const QString& appletName, uint appletId, const QStringList& args)
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

    QStringList allArgs;
    QString id;
    id.setNum(appletId);
    allArgs << offers.first()->storageId() << id << args;
    Applet* applet = KService::createInstance<Plasma::Applet>(offers.first(), 0, allArgs);

    if (!applet) {
        kDebug() << "Couldn't load applet \"" << appletName << "\"!" << endl;
    }

    return applet;
}

Applet* Applet::loadApplet(const KPluginInfo* info, uint appletId, const QStringList& args)
{
    if (!info) {
        return 0;
    }

    return loadApplet(info->pluginName(), appletId, args);
}

} // Plasma namespace

#include "applet.moc"
