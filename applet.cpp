/*
 *   Copyright 2005 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 by Riccardo Iaconelli <riccardo@kde.org>
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

#include "applet.h"

#include <cmath>
#include <limits>

#include <QApplication>
#include <QEvent>
#include <QFile>
#include <QList>
#include <QPainter>
#include <QSize>
#include <QStyleOptionGraphicsItem>
#include <QTextDocument>
#include <QTimer>
#include <QUiLoader>

#include <KIcon>
#include <KColorScheme>
#include <KConfigDialog>
#include <KDialog>
#include <KPluginInfo>
#include <KStandardDirs>
#include <KService>
#include <KServiceTypeTrader>
#include <KIconLoader>

#include "plasma/configxml.h"
#include "plasma/containment.h"
#include "plasma/corona.h"
#include "plasma/dataenginemanager.h"
#include "plasma/package.h"
#include "plasma/packages_p.h"
#include "plasma/plasma.h"
#include "plasma/scriptengine.h"
#include "plasma/shadowitem_p.h"
#include "plasma/svg.h"
#include "plasma/theme.h"
#include "plasma/view.h"

#include "plasma/widgets/widget.h"
#include "plasma/widgets/lineedit.h"
#include "plasma/widgets/pushbutton.h"
#include "plasma/widgets/boxlayout.h"

//#define DYNAMIC_SHADOWS
namespace Plasma
{

class Applet::Private
{
public:
    Private(KService::Ptr service, int uniqueID)
        : appletId(uniqueID),
          globalConfig(0),
          appletConfig(0),
          appletDescription(service),
          package(0),
          background(0),
          failureText(0),
          scriptEngine(0),
          configXml(0),
          shadow(0),
          cachedBackground(0),
          pendingConstraints(NoConstraint),
          kioskImmutable(false),
          immutable(false),
          hasConfigurationInterface(false),
          failed(false),
          needsConfig(false),
          isContainment(false)
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
        delete background;
        delete package;
        delete configXml;
        delete shadow;
        delete cachedBackground;
    }

    void init(Applet* applet)
    {
        applet->setAcceptsHoverEvents(true);
        applet->setZValue(100);
        kioskImmutable = applet->globalConfig().isImmutable() ||
                         applet->config().isImmutable();
        applet->setImmutable(kioskImmutable);

        if (!appletDescription.isValid()) {
            applet->setFailedToLaunch(true);
            return;
        }

        QString language = appletDescription.property("X-Plasma-Language").toString();

        // we have a scripted plasmoid
        if (!language.isEmpty()) {
            // find where the Package is
            QString path = KStandardDirs::locate("appdata",
                                                 "plasmoids/" +
                                                 appletDescription.pluginName());

            if (!path.isEmpty()) {
                // create the package and see if we have something real
                package = new Package(path,
                                      appletDescription.pluginName(),
                                      PlasmoidStructure());
                if (package->isValid()) {
                    // now we try and set up the script engine.
                    // it will be parented to this applet and so will get
                    // deleted when the applet does

                    scriptEngine = ScriptEngine::load(language, applet);
                    if (!scriptEngine) {
                        delete package;
                        package = 0;
                    }
                } else {
                    delete package;
                    package = 0;
                }

                if (!package) {
                    applet->setFailedToLaunch(true);
                } else {
                    setupScriptSupport(applet);
                }
            }
        }
        applet->setDrawStandardBackground(true);
    }

    // put all setup routines for script here. at this point we can assume that
    // package exists and that we have a script engin
    void setupScriptSupport(Applet* applet)
    {
        Q_ASSERT(package);
        QString xmlPath = package->filePath("mainconfigxml");
        if (!xmlPath.isEmpty()) {
            QFile file(xmlPath);
            configXml = new ConfigXml(config(), &file);
        }

        if (!package->filePath("mainconfigui").isEmpty()) {
            applet->setHasConfigurationInterface(true);
        }
    }

    void paintBackground(QPainter* p2, Applet* q, QRectF paintRect)
    {
        if (q->formFactor() != Plasma::Planar) {
            // we don't paint special backgrounds for other form factors
            // if that changes in the future, this method is where such
            // background painting code should be added
            return;
        }

        // TODO: caching these values might be an idea
        const int topHeight = background->elementSize("top").height();
        const int leftWidth = background->elementSize("left").width();
        const int topOffset = 0 - topHeight;
        const int leftOffset = 0 - leftWidth;

        if (!cachedBackground || cachedBackground->size() != q->boundingRect().toRect().size()) {
            QSize contents = contentSize(q).toSize();

            const int contentWidth = contents.width();
            const int contentHeight = contents.height();

            background->resize();

            const int topWidth = background->elementSize("top").width();
            const int leftHeight = background->elementSize("left").height();
            const int rightWidth = background->elementSize("right").width();
            const int bottomHeight = background->elementSize("bottom").height();

            // contents top-left corner is (0,0).  We need to draw up and left of that
            const int rightOffset = contentWidth;
            const int bottomOffset = contentHeight;
            const int contentTop = 0;
            const int contentLeft = 0;

            delete cachedBackground;
            cachedBackground = new QPixmap(leftWidth + contentWidth + rightWidth, topHeight + contentHeight + bottomHeight);
            cachedBackground->fill(Qt::transparent);
            QPainter p(cachedBackground);
            p.translate(leftWidth, topHeight);
            p.setCompositionMode(QPainter::CompositionMode_Source);
            p.setRenderHint(QPainter::SmoothPixmapTransform);

            //FIXME: This is a hack to fix a drawing problems with svg files where a thin transparent border is drawn around the svg image.
            //       the transparent border around the svg seems to vary in size depending on the size of the svg and as a result increasing the
            //       svn image by 2 all around didn't resolve the issue. For now it resizes based on the border size.

            background->resize(contentWidth, contentHeight);
            background->paint(&p, QRect(contentLeft-leftWidth, contentTop-topHeight, contentWidth+leftWidth*2, contentHeight+topHeight*2), "center");
            background->resize();

            background->paint(&p, QRect(leftOffset, topOffset, leftWidth, topHeight), "topleft");
            background->paint(&p, QRect(rightOffset, topOffset,rightWidth, topHeight), "topright");
            background->paint(&p, QRect(leftOffset, bottomOffset, leftWidth, bottomHeight), "bottomleft");
            background->paint(&p, QRect(rightOffset, bottomOffset, rightWidth, bottomHeight), "bottomright");

            if (background->elementExists("hint-stretch-borders")) {
                background->paint(&p, QRect(leftOffset, contentTop, leftWidth, contentHeight), "left");
                background->paint(&p, QRect(rightOffset, contentTop, rightWidth, contentHeight), "right");
                background->paint(&p, QRect(contentLeft, topOffset, contentWidth, topHeight), "top");
                background->paint(&p, QRect(contentLeft, bottomOffset, contentWidth, bottomHeight), "bottom");
            } else {
                QPixmap left(leftWidth, leftHeight);
                left.fill(Qt::transparent);
                {
                    QPainter sidePainter(&left);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    background->paint(&sidePainter, QPoint(0, 0), "left");
                }
                p.drawTiledPixmap(QRect(leftOffset, contentTop, leftWidth, contentHeight), left);

                QPixmap right(rightWidth, leftHeight);
                right.fill(Qt::transparent);
                {
                    QPainter sidePainter(&right);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    background->paint(&sidePainter, QPoint(0, 0), "right");
                }
                p.drawTiledPixmap(QRect(rightOffset, contentTop, rightWidth, contentHeight), right);

                QPixmap top(topWidth, topHeight);
                top.fill(Qt::transparent);
                {
                    QPainter sidePainter(&top);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    background->paint(&sidePainter, QPoint(0, 0), "top");
                }
                p.drawTiledPixmap(QRect(contentLeft, topOffset, contentWidth, topHeight), top);

                QPixmap bottom(topWidth, bottomHeight);
                bottom.fill(Qt::transparent);
                {
                    QPainter sidePainter(&bottom);
                    sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
                    background->paint(&sidePainter, QPoint(0, 0), "bottom");
                }
                p.drawTiledPixmap(QRect(contentLeft, bottomOffset, contentWidth, bottomHeight), bottom);
            }

            // re-enable this once Qt's svg rendering is un-buggered
            //background->resize(contentWidth, contentHeight);
            //background->paint(&p, QRect(contentLeft, contentTop, contentWidth, contentHeight), "center");
        }

        p2->drawPixmap(paintRect, *cachedBackground, paintRect.translated(-leftOffset,-topOffset));
    }

    QSizeF contentSize(const Applet* q)
    {
        if (scriptEngine) {
            return scriptEngine->size();
        }

        if (failureText) {
            return failureText->geometry().size();
        }

        return q->contentSize();
    }

    static uint nextId()
    {
        ++s_maxAppletId;
        return s_maxAppletId;
    }

    KSharedConfig::Ptr config() {
        if (!appletConfig) {
            QString file = KStandardDirs::locateLocal("appdata",
                                                      "applets/" + instanceName() + "rc",
                                                      true);
            appletConfig = KSharedConfig::openConfig(file);
        }

        return appletConfig;
    }

    QString instanceName()
    {
        if (!appletDescription.isValid()) {
            return QString();
        }

        return appletDescription.service()->library() + QString::number(appletId);
    }

    void getBorderSize(int& left , int& top, int &right, int& bottom)
    {
        if (background) {
            top = background->elementSize("top").height();
            left = background->elementSize("left").width();
            right = background->elementSize("right").width();
            bottom = background->elementSize("bottom").height();
        } else {
            top = left = right = bottom = 0;
        }
    }

    void scheduleConstraintsUpdate(Plasma::Constraints c, Applet* applet)
    {
        if (pendingConstraints == NoConstraint) {
            QTimer::singleShot(0, applet, SLOT(flushUpdatedConstraints()));
        }
        pendingConstraints |= c;
    }

    //TODO: examine the usage of memory here; there's a pretty large
    //      number of members at this point.
    uint appletId;
    KSharedConfig::Ptr globalConfig;
    KSharedConfig::Ptr appletConfig;
    KPluginInfo appletDescription;
    Package* package;
    QList<QObject*> watchedForFocus;
    QStringList loadedEngines;
    static uint s_maxAppletId;
    Plasma::Svg *background;
    Plasma::LineEdit *failureText;
    ScriptEngine* scriptEngine;
    ConfigXml* configXml;
    ShadowItem* shadow;
    QPixmap* cachedBackground;
    Plasma::Constraints pendingConstraints;
    bool kioskImmutable : 1;
    bool immutable : 1;
    bool hasConfigurationInterface : 1;
    bool failed : 1;
    bool needsConfig : 1;
    bool isContainment : 1;
};

uint Applet::Private::s_maxAppletId = 0;

Applet::Applet(QGraphicsItem *parent,
               const QString& serviceID,
               uint appletId)
    :  Widget(parent),
       d(new Private(KService::serviceByStorageId(serviceID), appletId))
{
    d->init(this);
}

Applet::Applet(QObject* parentObject, const QVariantList& args)
    :  Widget(0,parentObject),
       d(new Private(KService::serviceByStorageId(args.count() > 0 ? args[0].toString() : QString()),
                     args.count() > 1 ? args[1].toInt() : 0))
{
    d->init(this);
    // the brain damage seen in the initialization list is due to the
    // inflexibility of KService::createInstance
}

Applet::~Applet()
{
    needsFocus( false );

    if (d->appletConfig) {
        d->appletConfig->sync();
    }

    if (d->globalConfig) {
        d->globalConfig->sync();
    }

    delete d;
}

void Applet::init()
{
}

uint Applet::id() const
{
    return d->appletId;
}

KConfigGroup Applet::config() const
{
    return KConfigGroup(d->config(), "General");
}

void Applet::save(KConfigGroup* group) const
{
    group->writeEntry("plugin", pluginName());
    //FIXME: for containments, we need to have some special values here w/regards to
    //       screen affinity (e.g. "bottom of screen 0")
    //kDebug() << pluginName() << "geometry is" << geometry() << "pos is" << pos() << "bounding rect is" << boundingRect();
    group->writeEntry("geometry", geometry());

    if (transform() == QTransform()) {
        group->deleteEntry("transform");
    } else {
        QList<qreal> m;
        QTransform t = transform();
        m << t.m11() << t.m12() << t.m13() << t.m21() << t.m22() << t.m23() << t.m31() << t.m32() << t.m33();
        group->writeEntry("transform", m);
        //group->writeEntry("transform", transformToString(transform()));
    }

    Containment* c = containment();
    if (c) {
        group->writeEntry("containment", c->id());
    }

    saveState(group);
}

void Applet::saveState(KConfigGroup* group) const
{
    Q_UNUSED(group)
}

KConfigGroup Applet::config(const QString& group) const
{
    KConfigGroup cg = config();
    return KConfigGroup(cg.config(), instanceName() + '-' + group);
}

KConfigGroup Applet::globalConfig() const
{
    if ( !d->globalConfig ) {
        QString file = KStandardDirs::locateLocal( "config", "plasma_" + globalName() + "rc" );
        d->globalConfig = KSharedConfig::openConfig( file );
    }

    return KConfigGroup(d->globalConfig, "General");
}

void Applet::destroy()
{
    if (d->configXml) {
        d->configXml->setDefaults();
    }

    if (d->appletConfig) {
        foreach (const QString& group, d->appletConfig->groupList()) {
            d->appletConfig->deleteGroup(group);
        }
        d->appletConfig = 0;
    }

    deleteLater();
}

ConfigXml* Applet::configXml() const
{
    return d->configXml;
}

DataEngine* Applet::dataEngine(const QString& name) const
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

const Package* Applet::package() const
{
    return d->package;
}

void Applet::updateConstraints(Plasma::Constraints constraints)
{
    d->scheduleConstraintsUpdate(constraints, this);
}

void Applet::constraintsUpdated(Plasma::Constraints constraints)
{
    Q_UNUSED(constraints)
    //kDebug() << constraints << "constraints are FormFactor: " << formFactor() << ", Location: " << location();
}

QString Applet::name() const
{
    if (!d->appletDescription.isValid()) {
        return i18n("Unknown Applet");
    }

    return d->appletDescription.name();
}

QString Applet::icon() const
{
    if (!d->appletDescription.isValid()) {
        return QString();
    }

    return d->appletDescription.icon();
}

QString Applet::pluginName() const
{
    if (!d->appletDescription.isValid()) {
        return QString();
    }

    return d->appletDescription.pluginName();
}

QString Applet::category() const
{
    if (!d->appletDescription.isValid()) {
        return i18n("Miscellaneous");
    }

    return d->appletDescription.category();
}

QString Applet::category(const KPluginInfo& applet)
{
    return applet.property("X-KDE-PluginInfo-Category").toString();
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

    return offers.first()->property("X-KDE-PluginInfo-Category").toString();
}

bool Applet::isImmutable() const
{
    return d->immutable || d->kioskImmutable;
}

void Applet::setImmutable(bool immutable)
{
    d->immutable = immutable;
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
            updateGeometry();
        }
    } else if (d->background) {
        delete d->background;
        d->background = 0;
        updateGeometry();
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
        text = i18n("This object could not be created for the following reason:<p><b>%1</b></p>", reason);
    }

    return text;
}

void Applet::setFailedToLaunch(bool failed, const QString& reason)
{
    if (d->failed == failed) {
        if (d->failureText) {
            d->failureText->setHtml(visibleFailureText(reason));
            setGeometry(QRectF(geometry().topLeft(), d->failureText->sizeHint()));
        }
        return;
    }

    d->failed = failed;
    prepareGeometryChange();
    qDeleteAll(QGraphicsItem::children());
    delete layout();

    if (failed) {
        setDrawStandardBackground(true);
        Layout* failureLayout = new BoxLayout(BoxLayout::TopToBottom, this);
        failureLayout->setMargin(0);
        d->failureText = new LineEdit(this);
        d->failureText->setStyled(false);
        d->failureText->document()->setTextWidth(200);
        d->failureText->setHtml(visibleFailureText(reason));
        //FIXME: this needs to get the colour from the theme's colour scheme
        d->failureText->setDefaultTextColor(KStatefulBrush(KColorScheme::Window,
                                                           KColorScheme::NormalText,
                                                           Theme::self()->colors())
                                                        .brush(QPalette::Normal).color());
        failureLayout->addItem(d->failureText);
        setGeometry(QRectF(geometry().topLeft(), d->failureText->sizeHint()));
    } else {
        d->failureText = 0;
    }

    update();
}

bool Applet::needsConfiguring() const
{
    return d->needsConfig;
}

void Applet::setNeedsConfiguring(bool needsConfig)
{
    if (d->needsConfig == needsConfig) {
        return;
    }

    d->needsConfig = needsConfig;
    prepareGeometryChange();
    qDeleteAll(QGraphicsItem::children());
    delete layout();

    if (needsConfig) {
        setDrawStandardBackground(true);
        Layout* layout = new BoxLayout(BoxLayout::TopToBottom,this);
        PushButton* button = new PushButton(this);
        button->setText(i18n("Configure..."));
        connect(button, SIGNAL(clicked()), this, SLOT(performSetupConfig()));
        layout->addItem(button);
    }
}

void Applet::performSetupConfig()
{
    qDeleteAll(QGraphicsItem::children());
    delete layout();
    showConfigurationInterface();
}

void Applet::flushUpdatedConstraints()
{
    if (d->pendingConstraints == NoConstraint) {
        return;
    }

    //kDebug() << "fushing constraints: " << d->pendingConstraints << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
    Plasma::Constraints c = d->pendingConstraints;
    d->pendingConstraints = NoConstraint;

    Containment* containment = qobject_cast<Plasma::Containment*>(this);
    if (c & Plasma::FormFactorConstraint) {
        FormFactor f = formFactor();
        setShadowShown(f == Planar);
        setDrawStandardBackground(!containment && f != Vertical && f != Horizontal);
    }

    if (isContainment() && containment) {
        containment->containmentConstraintsUpdated(c);
    }

    constraintsUpdated(c);

    if (layout()) {
        layout()->update();
    }
}

int Applet::type() const
{
    return Type;
}

QRectF Applet::boundingRect() const
{
    QRectF rect = QRectF(QPointF(0,0), d->contentSize(this));

    int left;
    int right;
    int top;
    int bottom;

    d->getBorderSize(left,top,right,bottom);


    //kDebug() << "Background , Border size" << d->background << left << top << right << bottom;

    return rect.adjusted(-left,-top,right,bottom);
}

QSizeF Applet::sizeHint() const
{
    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;

    d->getBorderSize(left, top, right, bottom);

    //kDebug() << "Applet content size hint: " << contentSizeHint() << "plus our borders" << left << right << top << bottom;

    return contentSizeHint() + QSizeF(left + right, top + bottom);
}

QList<QAction*> Applet::contextActions()
{
    kDebug() << "empty actions";
    return QList<QAction*>();
}

QColor Applet::color() const
{
    // TODO: add more colors for more categories and
    // maybe read from config?
    QString c = category();
    int alpha = 200;
    // Colors taken from Oxygen color palette
    if (c == "Date and Time") {
        return QColor(191, 94, 0, alpha);
    } else if (c == "Environment & Weather") {
        return QColor(191, 0, 0, alpha);
    } else if (c == "Examples") {
        return QColor(204, 0, 154, alpha);
    } else if (c == "File System") {
        return QColor(90, 0, 179, alpha);
    } else if (c == "Graphics") {
        return QColor(0, 0, 255, alpha);
    } else if (c == "Language") {
        return QColor(0, 191, 0, alpha);
    } else if (c == "Mapping") {
        return QColor(191, 245, 0, alpha);
    } else if (c == "Online Services") {
        return QColor(255, 213, 0, alpha);
    } else if (c == "System Information") {
        return QColor(0, 196, 204, alpha);
    } else if (c == "Windows and Tasks") {
        return QColor(255, 126, 0, alpha);
    } else {
        return QColor(136, 136, 136, alpha);
    }
}

void Applet::paintWidget(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    if (d->shadow && d->shadow->shadowedSize() != boundingRect().size()) {
        //kDebug() << "sizes are " << d->shadow->shadowedSize() << boundingRect().size();
        d->shadow->generate();
    }

    painter->save();
    if (transform().isRotating()) {
        painter->setRenderHint(QPainter::SmoothPixmapTransform);
        painter->setRenderHint(QPainter::Antialiasing);
    }

    if (d->background) {
        //kDebug() << "option rect is" << option->rect;
        d->paintBackground(painter, this, option->rect);
    }

    if (!d->failed && !d->needsConfig) {
        if (widget && isContainment()) {
            // note that the widget we get is actually the viewport of the view, not the view itself
            View* v = qobject_cast<Plasma::View*>(widget->parent());
            if (v && !v->drawWallpaper()) {
                return;
            }
        }

        paintInterface(painter, option, QRect(QPoint(0,0), d->contentSize(this).toSize()));
    }

    painter->restore();
}

void Applet::paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option,
                            const QRect & contentsRect)
{
    Q_UNUSED(contentsRect)

    if (d->scriptEngine) {
        d->scriptEngine->paintInterface(painter, option);
    } else {
        //kDebug() << "Applet::paintInterface() default impl";
    }
}

FormFactor Applet::formFactor() const
{
    Containment* c = containment();

    if (!c) {
        return Plasma::Planar;
    }

    return c->formFactor();
}

Containment* Applet::containment() const
{
    QGraphicsItem *parent = parentItem();
    while (parent) {
        if (dynamic_cast<Containment*>(parent)) {
            break;
        }
        parent = parent->parentItem();
    }
    return dynamic_cast<Containment*>(parent);
}

Location Applet::location() const
{
    Containment* c = containment();

    if (!c) {
        return Plasma::Desktop;
    }

    return c->location();
}

QSizeF Applet::contentSize() const
{
    int top , left , right , bottom;
    d->getBorderSize(left,top,right,bottom);

    // kDebug() << "Geometry size: " << geometry().size();
    // kDebug() << "Borders: " << left << top << right << bottom;

    return geometry().size() - QSizeF(left+right,top+bottom);
}

QSizeF Applet::contentSizeHint() const
{
    if (layout()) {
        return layout()->sizeHint();
    }

    return QSizeF(0, 0);
}

QString Applet::globalName() const
{
    if (!d->appletDescription.isValid()) {
        return QString();
    }

    return d->appletDescription.service()->library();
}

QString Applet::instanceName() const
{
    return d->instanceName();
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
    if (d->package && d->configXml) {
        QString uiFile = d->package->filePath("mainconfigui");
        if (uiFile.isEmpty()) {
            return;
        }

        KConfigDialog *dialog = new KConfigDialog(0, "", d->configXml);
        dialog->setWindowTitle(i18n("%1 Settings", name()));
        dialog->setAttribute(Qt::WA_DeleteOnClose, true);

        QUiLoader loader;
        QString filename = d->package->filePath("mainconfigui");
        QFile f(filename);
        if (!f.open(QIODevice::ReadOnly)) {
            delete dialog;
            return;
        }

        QWidget *w = loader.load(&f);
        f.close();

        dialog->addPage(w, i18n("Settings"), icon(), i18n("%1 Settings", name()));
        dialog->show();
    }
}

KPluginInfo::List Applet::knownApplets(const QString &category,
                                       const QString &parentApp)
{
    QString constraint;

    if (parentApp.isEmpty()) {
        constraint.append("not exist [X-KDE-ParentApp]");
    } else {
        constraint.append("[X-KDE-ParentApp] == '").append(parentApp).append("'");
    }

    if (!category.isEmpty()) {
        if (!constraint.isEmpty()) {
            constraint.append(" and ");
        }

        constraint.append("[X-KDE-PluginInfo-Category] == '").append(category).append("'");
        if (category == "Miscellaneous") {
            constraint.append(" or (not exist [X-KDE-PluginInfo-Category] or [X-KDE-PluginInfo-Category] == '')");
        }
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);
    //kDebug() << "Applet::knownApplets constraint was '" << constraint << "' which got us " << offers.count() << " matches";
    return KPluginInfo::fromServices(offers);
}

KPluginInfo::List Applet::knownAppletsForMimetype(const QString &mimetype)
{
    QString constraint = QString("'%1' in MimeTypes").arg(mimetype);
    //kDebug() << "knownAppletsForMimetype with" << mimetype << constraint;
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);
    return KPluginInfo::fromServices(offers);
}

QStringList Applet::knownCategories(const QString &parentApp, bool visibleOnly)
{
    QString constraint = "exist [X-KDE-PluginInfo-Category]";

    if (parentApp.isEmpty()) {
        constraint.append(" and not exist [X-KDE-ParentApp]");
    } else {
        constraint.append(" and [X-KDE-ParentApp] == '").append(parentApp).append("'");
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);
    QStringList categories;
    foreach (const KService::Ptr applet, offers) {
        QString appletCategory = applet->property("X-KDE-PluginInfo-Category").toString();
        if (visibleOnly && applet->property("NoDisplay").toBool()) {
            // we don't want to show the hidden category
            continue;
        }

        //kDebug() << "   and we have " << appletCategory;
        if (appletCategory.isEmpty()) {
            if (!categories.contains(i18n("Miscellaneous"))) {
                categories << i18n("Miscellaneous");
            }
        } else  if (!categories.contains(appletCategory)) {
            categories << appletCategory;
        }
    }

    categories.sort();
    return categories;
}

Applet* Applet::loadApplet(const QString& appletName, uint appletId, const QVariantList& args)
{
    if (appletName.isEmpty()) {
        return 0;
    }

    QString constraint = QString("[X-KDE-PluginInfo-Name] == '%1'").arg(appletName);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Applet", constraint);

    if (offers.isEmpty()) {
        //TODO: what would be -really- cool is offer to try and download the applet
        //      from the network at this point
        kDebug() << "Applet::loadApplet: offers is empty for \"" << appletName << "\"";
        return 0;
    } /* else if (offers.count() > 1) {
        kDebug() << "hey! we got more than one! let's blindly take the first one";
    } */

    if (appletId == 0) {
        appletId = Private::nextId();
    }

    QVariantList allArgs;
    allArgs << offers.first()->storageId() << appletId << args;
    QString error;
    Applet* applet = offers.first()->createInstance<Plasma::Applet>(0, allArgs, &error);

    if (!applet) {
        kDebug() << "Couldn't load applet \"" << appletName << "\"! reason given: " << error;
    }

    return applet;
}

Applet* Applet::loadApplet(const KPluginInfo& info, uint appletId, const QVariantList& args)
{
    if (!info.isValid()) {
        return 0;
    }

    return loadApplet(info.pluginName(), appletId, args);
}

void Applet::setShadowShown(bool shown)
{
    //There are various problems with shadows right now:
    //
    //1) shadows can be seen through translucent areas, which is probably technically correct ubt
    //looks odd
    //2) the shape of the item odesn't conform to the shape of the standard background, e.g. with
    //rounded corners
#ifdef DYNAMIC_SHADOWS
    if (shown) {
        if (d->shadow) {
            d->shadow->setVisible(true);
        } else {
            d->shadow = new ShadowItem(this);
            if (scene()) {
                scene()->addItem(d->shadow);
                d->shadow->show();
            }
        }
    } else {
        delete d->shadow;
        d->shadow = 0;
    }
#else
    Q_UNUSED(shown);
#endif
}

bool Applet::isShadowShown() const
{
    return d->shadow && d->shadow->isVisible();
}

QVariant Applet::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (!d->shadow) {
        return QGraphicsItem::itemChange(change, value);
    }

    switch (change) {
    case ItemPositionChange:
        d->shadow->adjustPosition();
        break;
    case ItemSceneChange: {
        QGraphicsScene *newScene = qvariant_cast<QGraphicsScene*>(value);
        if (d->shadow->scene())
            d->shadow->scene()->removeItem(d->shadow);
        if (newScene) {
            newScene->addItem(d->shadow);
            d->shadow->generate();
            d->shadow->adjustPosition();
            d->shadow->show();
        }
    }
        break;
    case ItemVisibleChange:
        d->shadow->setVisible(isVisible());
        break;
    default:
        break;
    };

    return QGraphicsItem::itemChange(change, value);
}

void Applet::setGeometry(const QRectF& geometry)
{
    if (geometry.size().width() > 0 && geometry.size().height() > 0 && size() != geometry.size()) {
        prepareGeometryChange();
        qreal width = qBound(minimumSize().width(), geometry.size().width(), maximumSize().width());
        qreal height = qBound(minimumSize().height(), geometry.size().height(), maximumSize().height());

        setSize(QSizeF(width, height));

        if (layout()) {
            layout()->setGeometry(QRectF(QPoint(0, 0), contentSize()));
        }

        if (managingLayout()) {
            managingLayout()->invalidate();
        }
    }

    setPos(geometry.topLeft());
    emit geometryChanged();
    update();
}

void Applet::setIsContainment(bool isContainment)
{
    d->isContainment = isContainment;
}

bool Applet::isContainment() const
{
    return d->isContainment;
}

void Applet::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    //kDebug() << "context menu event!";
    if (!scene()) {
        //kDebug() << "no scene?!";
        return;
    }

    Applet* containment = dynamic_cast<Plasma::Applet*>(topLevelItem());

    if (!containment) {
        //kDebug() << "no containment. hm.";
        Widget::contextMenuEvent(event);
        return;
    }

    // we want to pass up the context menu event to the Containment at
    // this point
    containment->contextMenuEvent(event);
}

} // Plasma namespace

#include "applet.moc"
