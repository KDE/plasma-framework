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
#include <kdeclarative/qmlobject.h>

#include <QClipboard>
#include <QQmlExpression>
#include <QQmlProperty>
#include <QMimeData>

#include <kactioncollection.h>
#include <KAcceleratorManager>
#include <kauthorized.h>
#include <QDebug>
#include <klocalizedstring.h>
#include <kurlmimedata.h>
#include <QMimeDatabase>
#include <KNotification>

#ifndef PLASMA_NO_KIO
#include "kio/jobclasses.h" // for KIO::JobFlags
#include "kio/job.h"
#include "kio/scheduler.h"
#include <KIO/DropJob>
#endif

#include <plasma.h>
#include <Plasma/ContainmentActions>
#include <Plasma/Corona>
#include <Plasma/PluginLoader>

#include <KPackage/Package>
#include <KPackage/PackageLoader>

#include <kactivities/info.h>

#include "kdeclarative/configpropertymap.h"
#include <packageurlinterceptor.h>

ContainmentInterface::ContainmentInterface(DeclarativeAppletScript *parent, const QVariantList &args)
    : AppletInterface(parent, args),
      m_wallpaperInterface(0),
      m_activityInfo(0),
      m_wheelDelta(0),
      m_editMode(false)
{
    m_containment = static_cast<Plasma::Containment *>(appletScript()->applet()->containment());

    setAcceptedMouseButtons(Qt::AllButtons);

    connect(m_containment.data(), &Plasma::Containment::appletRemoved,
            this, &ContainmentInterface::appletRemovedForward);
    connect(m_containment.data(), &Plasma::Containment::appletAdded,
            this, &ContainmentInterface::appletAddedForward);

    if (!m_appletInterfaces.isEmpty()) {
        emit appletsChanged();
    }
}

void ContainmentInterface::init()
{
    if (qmlObject()->rootObject()) {
        return;
    }

    m_activityInfo = new KActivities::Info(m_containment->activity(), this);
    connect(m_activityInfo, &KActivities::Info::nameChanged,
            this, &ContainmentInterface::activityNameChanged);
    emit activityNameChanged();

    AppletInterface::init();

    //Create the ToolBox
    if (m_containment) {
        KConfigGroup defaults;
        if (m_containment->containmentType() == Plasma::Types::DesktopContainment) {
            defaults = KConfigGroup(KSharedConfig::openConfig(m_containment->corona()->kPackage().filePath("defaults")), "Desktop");
        } else if (m_containment->containmentType() == Plasma::Types::PanelContainment) {
            defaults = KConfigGroup(KSharedConfig::openConfig(m_containment->corona()->kPackage().filePath("defaults")), "Panel");
        }

        if (defaults.isValid()) {
            KPackage::Package pkg = KPackage::PackageLoader::self()->loadPackage("KPackage/GenericQML");
            pkg.setDefaultPackageRoot("plasma/packages");

            if (defaults.isValid()) {
                pkg.setPath(defaults.readEntry("ToolBox", "org.kde.desktoptoolbox"));
            } else {
                pkg.setPath("org.kde.desktoptoolbox");
            }

            PlasmaQuick::PackageUrlInterceptor *interceptor = dynamic_cast<PlasmaQuick::PackageUrlInterceptor *>(qmlObject()->engine()->urlInterceptor());
            if (interceptor) {
                interceptor->addAllowedPath(pkg.path());
            }

            if (pkg.metadata().isValid() && !pkg.metadata().isHidden()) {
                if (pkg.isValid()) {
                    QObject *containmentGraphicObject = qmlObject()->rootObject();

                    QVariantHash toolboxProperties;
                    toolboxProperties["parent"] = QVariant::fromValue(this);
                    QObject *toolBoxObject = qmlObject()->createObjectFromSource(QUrl::fromLocalFile(pkg.filePath("mainscript")), 0, toolboxProperties);
                    if (toolBoxObject && containmentGraphicObject) {
                        containmentGraphicObject->setProperty("toolBox", QVariant::fromValue(toolBoxObject));
                    }
                } else {
                    qWarning() << "Could not load toolbox package." << pkg.path();
                }
            } else {
                qWarning() << "Toolbox not loading, toolbox package is either invalid or disabled.";
            }
        }

    }

    //set parent, both as object hierarchically and visually
    //do this only for containments, applets will do it in compactrepresentationcheck
    if (qmlObject()->rootObject()) {
        qmlObject()->rootObject()->setProperty("parent", QVariant::fromValue(this));

        //set anchors
        QQmlExpression expr(qmlObject()->engine()->rootContext(), qmlObject()->rootObject(), QStringLiteral("parent"));
        QQmlProperty prop(qmlObject()->rootObject(), QStringLiteral("anchors.fill"));
        prop.write(expr.evaluate());
    }

    if (!m_containment->wallpaper().isEmpty()) {
        loadWallpaper();
    }

    connect(m_containment.data(), &Plasma::Containment::activityChanged,
            this, &ContainmentInterface::activityChanged);
    connect(m_containment.data(), &Plasma::Containment::activityChanged,
            [ = ]() {
                delete m_activityInfo;
                m_activityInfo = new KActivities::Info(m_containment->activity(), this);
                connect(m_activityInfo, &KActivities::Info::nameChanged,
                        this, &ContainmentInterface::activityNameChanged);
                emit activityNameChanged();
            });
    connect(m_containment.data(), &Plasma::Containment::wallpaperChanged,
            this, &ContainmentInterface::loadWallpaper);
    connect(m_containment.data(), &Plasma::Containment::containmentTypeChanged,
            this, &ContainmentInterface::containmentTypeChanged);

    connect(m_containment.data()->actions(), &KActionCollection::inserted,
            this, &ContainmentInterface::actionsChanged);
    connect(m_containment.data()->actions(), &KActionCollection::removed,
            this, &ContainmentInterface::actionsChanged);

    if (m_containment->corona()) {
        connect(m_containment->corona(), &Plasma::Corona::availableScreenRegionChanged,
                this, &ContainmentInterface::availableScreenRegionChanged);
        connect(m_containment->corona(), &Plasma::Corona::availableScreenRectChanged,
                this, &ContainmentInterface::availableScreenRectChanged);
    }
}

QList <QObject *> ContainmentInterface::applets()
{
    return m_appletInterfaces;
}

Plasma::Types::ContainmentType ContainmentInterface::containmentType() const
{
    return appletScript()->containmentType();
}

void ContainmentInterface::setContainmentType(Plasma::Types::ContainmentType type)
{
    appletScript()->setContainmentType(type);
}

Plasma::Applet *ContainmentInterface::createApplet(const QString &plugin, const QVariantList &args, const QPoint &pos)
{
    return createApplet(plugin, args, QRectF(pos, QSize()));
}

Plasma::Applet *ContainmentInterface::createApplet(const QString &plugin, const QVariantList &args, const QRectF &geom)
{
    //HACK
    //This is necessary to delay the appletAdded signal (of containmentInterface) AFTER the applet graphics object has been created
    blockSignals(true);
    Plasma::Applet *applet = m_containment->createApplet(plugin, args);

    if (applet) {
        QQuickItem *appletGraphicObject = applet->property("_plasma_graphicObject").value<QQuickItem *>();
        //invalid applet?
        if (!appletGraphicObject) {
            blockSignals(false);
            return applet;
        }
        if (geom.width() > 0 && geom.height() > 0) {
            appletGraphicObject->setSize(geom.size());
        }

        blockSignals(false);

        emit appletAdded(appletGraphicObject, geom.x(), geom.y());
        emit appletsChanged();
    } else {
        blockSignals(false);
    }
    return applet;
}

void ContainmentInterface::setAppletArgs(Plasma::Applet *applet, const QString &mimetype, const QString &data)
{
    if (!applet) {
        return;
    }

    AppletInterface *appletInterface = applet->property("_plasma_graphicObject").value<AppletInterface *>();
    if (appletInterface) {
        emit appletInterface->externalData(mimetype, data);
    }
}

QObject *ContainmentInterface::containmentAt(int x, int y)
{
    QObject *desktop = nullptr;
    foreach (Plasma::Containment *c, m_containment->corona()->containments()) {
        ContainmentInterface *contInterface = c->property("_plasma_graphicObject").value<ContainmentInterface *>();

        if (contInterface && contInterface->isVisible()) {
            QWindow *w = contInterface->window();
            if (w && w->geometry().contains(QPoint(window()->x(), window()->y()) + QPoint(x, y))) {
                if (c->containmentType() == Plasma::Types::CustomEmbeddedContainment) {
                    continue;
                }
                if (c->containmentType() == Plasma::Types::DesktopContainment) {
                    desktop = contInterface;
                } else {
                    return contInterface;
                }
            }
        }
    }
    return desktop;
}

void ContainmentInterface::addApplet(AppletInterface *applet, int x, int y)
{
    if (!applet || applet->applet()->containment() == m_containment) {
        return;
    }

    blockSignals(true);
    m_containment->addApplet(applet->applet());
    blockSignals(false);
    emit appletAdded(applet, x, y);
}

QPointF ContainmentInterface::mapFromApplet(AppletInterface *applet, int x, int y)
{
    if (!applet->window() || !window()) {
        return QPointF();
    }

    //x,y in absolute screen coordinates of current view
    QPointF pos = applet->mapToScene(QPointF(x, y));
    pos = QPointF(pos + applet->window()->geometry().topLeft());
    //return the coordinate in the relative view's coords
    return pos - window()->geometry().topLeft();
}

QPointF ContainmentInterface::mapToApplet(AppletInterface *applet, int x, int y)
{
    if (!applet->window() || !window()) {
        return QPointF();
    }

    //x,y in absolute screen coordinates of current view
    QPointF pos(x, y);
    pos = QPointF(pos + window()->geometry().topLeft());
    //the coordinate in the relative view's coords
    pos = pos - applet->window()->geometry().topLeft();
    //make it relative to applet coords
    return pos - applet->mapToScene(QPointF(0, 0));
}

QPointF ContainmentInterface::adjustToAvailableScreenRegion(int x, int y, int w, int h) const
{
    QRegion reg;
    int screenId = screen();
    if (screenId > -1 && m_containment->corona()) {
        reg = m_containment->corona()->availableScreenRegion(screenId);
    }

    if (!reg.isEmpty()) {
        //make it relative
        QRect geometry = m_containment->corona()->screenGeometry(screenId);
        reg.translate(- geometry.topLeft());
    } else {
        reg = QRect(0, 0, width(), height());
    }

    const QRect rect(qBound(reg.boundingRect().left(), x, reg.boundingRect().right() + 1 - w),
                     qBound(reg.boundingRect().top(), y, reg.boundingRect().bottom() + 1 - h), w, h);
    const QRectF ar = availableScreenRect();
    QRect tempRect(rect);

    // in the case we are in the topleft quadrant
    // * see if the passed rect is completely in the region, if yes, return
    // * otherwise, try to move it horizontally to the screenrect x
    // * if now fits, return
    // * if fail, move vertically
    // * as last resort, move horizontally and vertically

    // top left corner
    if (rect.center().x() <= ar.center().x() && rect.center().y() <= ar.center().y()) {
        //QRegion::contains doesn't do what it would suggest, so do reg.intersected(rect) != rect instead
        if (reg.intersected(rect) != rect) {
            tempRect = QRect(qMax(rect.left(), (int)ar.left()), rect.top(), w, h);
            if (reg.intersected(tempRect) == tempRect) {
                return tempRect.topLeft();
            }

            tempRect = QRect(rect.left(), qMax(rect.top(), (int)ar.top()), w, h);
            if (reg.intersected(tempRect) == tempRect) {
                return tempRect.topLeft();
            }

            tempRect = QRect(qMax(rect.left(), (int)ar.left()), qMax(rect.top(), (int)ar.top()), w, h);
            return tempRect.topLeft();
        } else {
            return rect.topLeft();
        }

    //bottom left corner
    } else if (rect.center().x() <= ar.center().x() && rect.center().y() > ar.center().y()) {
        if (reg.intersected(rect) != rect) {
            tempRect = QRect(qMax(rect.left(), (int)ar.left()), rect.top(), w, h);
            if (reg.intersected(tempRect) == tempRect) {
                return tempRect.topLeft();
            }

            tempRect = QRect(rect.left(), qMin(rect.top(), (int)(ar.bottom() + 1 - h)), w, h);
            if (reg.intersected(tempRect) == tempRect) {
                return tempRect.topLeft();
            }

            tempRect = QRect(qMax(rect.left(), (int)ar.left()), qMin(rect.top(), (int)(ar.bottom() + 1 - h)), w, h);
            return tempRect.topLeft();
        } else {
            return rect.topLeft();
        }

    //top right corner
    } else if (rect.center().x() > ar.center().x() && rect.center().y() <= ar.center().y()) {
        if (reg.intersected(rect) != rect) {
            tempRect = QRect(qMin(rect.left(), (int)(ar.right() + 1 - w)), rect.top(), w, h);
            if (reg.intersected(tempRect) == tempRect) {
                return tempRect.topLeft();
            }

            tempRect = QRect(rect.left(), qMax(rect.top(), (int)ar.top()), w, h);
            if (reg.intersected(tempRect) == tempRect) {
                return tempRect.topLeft();
            }

            tempRect = QRect(qMin(rect.left(), (int)(ar.right() + 1 - w)), qMax(rect.top(), (int)ar.top()), w, h);
            return tempRect.topLeft();
        } else {
            return rect.topLeft();
        }

    //bottom right corner
    } else if (rect.center().x() > ar.center().x() && rect.center().y() > ar.center().y()) {
        if (reg.intersected(rect) != rect) {
            tempRect = QRect(qMin(rect.left(), (int)(ar.right() + 1 - w)), rect.top(), w, h);
            if (reg.intersected(tempRect) == tempRect) {
                return tempRect.topLeft();
            }

            tempRect = QRect(rect.left(), qMin(rect.top(), (int)(ar.bottom() + 1 - h)), w, h);
            if (reg.intersected(tempRect) == tempRect) {
                return tempRect.topLeft();
            }

            tempRect = QRect(qMin(rect.left(), (int)(ar.right() + 1 - w)), qMin(rect.top(), (int)(ar.bottom() + 1 - h)), w, h);
            return tempRect.topLeft();
        } else {
            return rect.topLeft();
        }
    }

    return rect.topLeft();
}

bool ContainmentInterface::isEditMode() const
{
    return m_editMode;
}

void ContainmentInterface::setEditMode(bool edit)
{
    if (edit == m_editMode) {
        return;
    }

    m_editMode = edit;
    emit editModeChanged();
}

void ContainmentInterface::processMimeData(QObject *mimeDataProxy, int x, int y, KIO::DropJob *dropJob)
{
    QMimeData* mime = qobject_cast<QMimeData*>(mimeDataProxy);
    if (mime) {
        processMimeData(mime, x, y, dropJob);
    } else {
        processMimeData(mimeDataProxy->property("mimeData").value<QMimeData*>(), x, y, dropJob);
    }
}

void ContainmentInterface::processMimeData(QMimeData *mimeData, int x, int y, KIO::DropJob *dropJob)
{
    if (!mimeData) {
        return;
    }

    //const QMimeData *mimeData = data;

    qDebug() << "Arrived mimeData" << mimeData->urls() << mimeData->formats() << "at" << x << ", " << y;

    if (mimeData->hasFormat(QStringLiteral("text/x-plasmoidservicename"))) {
        QString data = mimeData->data(QStringLiteral("text/x-plasmoidservicename"));
        const QStringList appletNames = data.split('\n', QString::SkipEmptyParts);
        foreach (const QString &appletName, appletNames) {
            qDebug() << "adding" << appletName;

            metaObject()->invokeMethod(this, "createApplet", Qt::QueuedConnection, Q_ARG(QString, appletName), Q_ARG(QVariantList, QVariantList()), Q_ARG(QRectF, QRectF(x, y, -1, -1)));
        }
    } else if (mimeData->hasUrls()) {
        //TODO: collect the mimetypes of available script engines and offer
        //      to create widgets out of the matching URLs, if any
        const QList<QUrl> urls = KUrlMimeData::urlsFromMimeData(mimeData);
        foreach (const QUrl &url, urls) {

#ifndef PLASMA_NO_KIO
            QMimeDatabase db;
            const QMimeType &mime = db.mimeTypeForUrl(url);
            QString mimeName = mime.name();
            QVariantList args;
            args << url.url();
            qDebug() << "can decode" << mimeName << args;

            // It may be a directory or a file, let's stat
            KIO::JobFlags flags = KIO::HideProgressInfo;
            KIO::MimetypeJob *job = KIO::mimetype(url, flags);
            m_dropPoints[job] = QPoint(x, y);

            QObject::connect(job, SIGNAL(result(KJob*)), this, SLOT(dropJobResult(KJob*)));
            QObject::connect(job, SIGNAL(mimetype(KIO::Job*,QString)),
                             this, SLOT(mimeTypeRetrieved(KIO::Job*,QString)));

            if (dropJob) {
                m_dropJobs[job] = dropJob;
            } else {
                QMenu *choices = new QMenu(i18n("Content dropped"));
                choices->addAction(QIcon::fromTheme(QStringLiteral("process-working")), i18n("Fetching file type..."));
                choices->popup(window() ? window()->mapToGlobal(QPoint(x, y)) : QPoint(x, y));

                m_dropMenus[job] = choices;
            }
#endif
        }

    } else {
        QStringList formats = mimeData->formats();
        QHash<QString, KPluginInfo> seenPlugins;
        QHash<QString, QString> pluginFormats;

        foreach (const QString &format, formats) {
            KPluginInfo::List plugins = Plasma::PluginLoader::self()->listAppletInfoForMimeType(format);

            foreach (const KPluginInfo &plugin, plugins) {
                if (seenPlugins.contains(plugin.pluginName())) {
                    continue;
                }

                seenPlugins.insert(plugin.pluginName(), plugin);
                pluginFormats.insert(plugin.pluginName(), format);
            }
        }
        //qDebug() << "Mimetype ..." << formats << seenPlugins.keys() << pluginFormats.values();

        QString selectedPlugin;

        if (seenPlugins.isEmpty()) {
            // do nothing
        //directly create if only one offer only if the conteinment didn't pass an existing plugin
        } else if (seenPlugins.count() == 1) {
            selectedPlugin = seenPlugins.constBegin().key();
            Plasma::Applet *applet = createApplet(selectedPlugin, QVariantList(), QRect(x, y, -1, -1));
            setAppletArgs(applet, pluginFormats[selectedPlugin], mimeData->data(pluginFormats[selectedPlugin]));
        } else {
            QMenu *choices = nullptr;
            if (!dropJob) {
                choices = new QMenu();
            }
            QList<QAction *> extraActions;
            QHash<QAction *, QString> actionsToPlugins;
            foreach (const KPluginInfo &info, seenPlugins) {
                QAction *action;
                if (!info.icon().isEmpty()) {
                    action = new QAction(QIcon::fromTheme(info.icon()), info.name(), nullptr);
                } else {
                    action = new QAction(info.name(), nullptr);
                }
                extraActions << action;
                if (choices) {
                    choices->addAction(action);
                }
                action->setData(info.pluginName());
                connect(action, &QAction::triggered, this, [this, x, y, mimeData, action]() {
                    const QString selectedPlugin = action->data().toString();
                    Plasma::Applet *applet = createApplet(selectedPlugin, QVariantList(), QRect(x, y, -1, -1));
                    setAppletArgs(applet, selectedPlugin, mimeData->data(selectedPlugin));
                });

                actionsToPlugins.insert(action, info.pluginName());
            }

            //if the menu was created by ourselves, delete it
            if (choices) {
                QAction *choice = choices->exec(window() ? window()->mapToGlobal(QPoint(x, y)) : QPoint(x, y));

                delete choices;
            } else {
                Q_ASSERT(dropJob);
                dropJob->setApplicationActions(extraActions);
            }
        }
    }
}

void ContainmentInterface::clearDataForMimeJob(KIO::Job *job)
{
#ifndef PLASMA_NO_KIO
    QObject::disconnect(job, 0, this, 0);
    m_dropPoints.remove(job);
    QMenu *choices = m_dropMenus.take(job);
    m_dropJobs.remove(job);
    job->kill();
#endif // PLASMA_NO_KIO
}

void ContainmentInterface::dropJobResult(KJob *job)
{
#ifndef PLASMA_NO_KIO
    if (job->error()) {
        qDebug() << "ERROR" << job->error() << ' ' << job->errorString();
    }
#endif // PLASMA_NO_KIO
}

void ContainmentInterface::mimeTypeRetrieved(KIO::Job *job, const QString &mimetype)
{
#ifndef PLASMA_NO_KIO
    qDebug() << "Mimetype Job returns." << mimetype;

    KIO::TransferJob *tjob = dynamic_cast<KIO::TransferJob *>(job);
    if (!tjob) {
        qDebug() << "job should be a TransferJob, but isn't";
        clearDataForMimeJob(job);
        return;
    }
    KPluginInfo::List appletList = Plasma::PluginLoader::self()->listAppletInfoForUrl(tjob->url());
    if (mimetype.isEmpty() && appletList.isEmpty()) {
        clearDataForMimeJob(job);
        qDebug() << "No applets found matching the url (" << tjob->url() << ") or the mimetype (" << mimetype << ")";
        return;
    } else {

        QPoint posi; // will be overwritten with the event's position
        if (m_dropPoints.contains(tjob)) {
            posi = m_dropPoints.value(tjob);
            qDebug() << "Received a suitable dropEvent at" << posi;
        } else {
            qDebug() << "Bailing out. Cannot find associated dropEvent related to the TransferJob";
            clearDataForMimeJob(job);
            return;
        }

        QMenu *choices = m_dropMenus.value(tjob);
        QList<QAction *> dropActions;
        KIO::DropJob *dropJob = m_dropJobs.value(tjob);

        if (!choices && !dropJob) {
            qDebug() << "Bailing out. No QMenu or drop job found for this job.";
            clearDataForMimeJob(job);
            return;
        }

        qDebug() << "Creating menu for:" << mimetype  << posi;

        appletList << Plasma::PluginLoader::self()->listAppletInfoForMimeType(mimetype);

        KPluginInfo::List wallpaperList;

        if (m_containment->containmentType() != Plasma::Types::PanelContainment
            && m_containment->containmentType() != Plasma::Types::CustomPanelContainment) {

            if (m_wallpaperInterface && m_wallpaperInterface->supportsMimetype(mimetype)) {
                wallpaperList << m_wallpaperInterface->package().metadata();
            } else {
                wallpaperList = WallpaperInterface::listWallpaperInfoForMimetype(mimetype);
            }
        }

        const bool isPlasmaPackage = (mimetype == QLatin1String("application/x-plasma"));

        if (!appletList.isEmpty() || !wallpaperList.isEmpty() || isPlasmaPackage) {
            QAction *installPlasmaPackageAction = nullptr;
            if (isPlasmaPackage) {
                if (choices) {
                    choices->addSection(i18n("Plasma Package"));
                    installPlasmaPackageAction = choices->addAction(QIcon::fromTheme(QStringLiteral("application-x-plasma")), i18n("Install"));
                } else {
                    QAction *action = new QAction(i18n("Plasma Package"), nullptr);
                    action->setSeparator(true);
                    dropActions << action;

                    installPlasmaPackageAction = new QAction(QIcon::fromTheme(QStringLiteral("application-x-plasma")), i18n("Install"), nullptr);
                    Q_ASSERT(dropJob);
                    dropActions << installPlasmaPackageAction;
                    dropJob->setApplicationActions(dropActions);
                }
                const QString &packagePath = tjob->url().toLocalFile();
                connect(installPlasmaPackageAction, &QAction::triggered, this, [this, tjob, posi, packagePath]() {
                    using namespace KPackage;
                    PackageStructure *structure = PackageLoader::self()->loadPackageStructure(QStringLiteral("Plasma/Applet"));
                    Package package(structure);

                    KJob *installJob = package.update(packagePath);
                    connect(installJob, &KJob::result, this, [this, packagePath, structure, posi](KJob *job) {
                        auto fail = [job](const QString &text) {
                            KNotification::event(QStringLiteral("plasmoidInstallationFailed"), i18n("Package Installation Failed"),
                                                 text, QStringLiteral("dialog-error"), 0, KNotification::CloseOnTimeout, QStringLiteral("plasma_workspace"));
                        };

                        // if the applet is already installed, just add it to the containment
                        if (job->error() != KJob::NoError
                            && job->error() != Package::PackageAlreadyInstalledError
                            && job->error() != Package::NewerVersionAlreadyInstalledError) {
                            fail(job->errorText());
                            return;
                        }

                        using namespace KPackage;
                        Package package(structure);
                        // TODO how can I get the path of the actual package?

                        package.setPath(packagePath);

                        // TODO how can I get the plugin id? Package::metadata() is deprecated
                        if (!package.isValid() || !package.metadata().isValid()) {
                            fail(i18n("The package you just dropped is invalid."));
                            return;
                        }

                        createApplet(package.metadata().pluginId(), QVariantList(), QRect(posi, QSize(-1,-1)));
                    });
                });
            }

            if (choices) {
                choices->addSection(i18n("Widgets"));
            } else {
                QAction *action = new QAction(i18n("Widgets"), nullptr);
                action->setSeparator(true);
                dropActions << action;
            }
            foreach (const KPluginInfo &info, appletList) {
                qDebug() << info.name();
                QAction *action;
                if (!info.icon().isEmpty()) {
                    action = new QAction(QIcon::fromTheme(info.icon()), info.name(), nullptr);
                } else {
                    action = new QAction(info.name(), nullptr);
                }

                if (choices) {
                    choices->addAction(action);
                }
                dropActions << action;
                action->setData(info.pluginName());
                qDebug() << info.pluginName();
                const QUrl url = tjob->url();
                connect(action, &QAction::triggered, this, [this, action, posi, mimetype, url]() {
                    Plasma::Applet *applet = createApplet(action->data().toString(), QVariantList(), QRect(posi, QSize(-1,-1)));
                    setAppletArgs(applet, mimetype, url.toString());
                });
            }
            {
                QAction *action = new QAction(i18n("Icon"), nullptr);
                if (choices) {
                    choices->addAction(action);
                }
                dropActions << action;
                action->setData(QStringLiteral("org.kde.plasma.icon"));
                const QUrl url = tjob->url();
                connect(action, &QAction::triggered, this, [this, action, posi, mimetype, url](){
                    Plasma::Applet *applet = createApplet(action->data().toString(), QVariantList(), QRect(posi, QSize(-1,-1)));
                    setAppletArgs(applet, mimetype, url.toString());
                });
            }

            QHash<QAction *, QString> actionsToWallpapers;
            if (!wallpaperList.isEmpty())  {
                if (choices) {
                    choices->addSection(i18n("Wallpaper"));
                } else {
                    QAction *action = new QAction(i18n("Wallpaper"), nullptr);
                    action->setSeparator(true);
                    dropActions << action;
                }

                QMap<QString, KPluginInfo> sorted;
                foreach (const KPluginInfo &info, appletList) {
                    sorted.insert(info.name(), info);
                }

                foreach (const KPluginInfo &info, wallpaperList) {
                    QAction *action;
                    if (!info.icon().isEmpty()) {
                        action = new QAction(QIcon::fromTheme(info.icon()), info.name(), nullptr);
                    } else {
                        action = new QAction(info.name(), nullptr);
                    }

                    if (choices) {
                        choices->addAction(action);
                    }
                    dropActions << action;
                    actionsToWallpapers.insert(action, info.pluginName());
                    const QUrl url = tjob->url();
                    connect(action, &QAction::triggered, this, [this, action, url]() {
                        //set wallpapery stuff
                        if (m_wallpaperInterface && url.isValid()) {
                            m_wallpaperInterface->setUrl(url);
                        }
                    });
                }
            }

        } else {
            //case in which we created the menu ourselves, just the "fetching type entry, directly create the icon applet
            if (choices) {
                Plasma::Applet *applet = createApplet(QStringLiteral("org.kde.plasma.icon"), QVariantList(), QRect(posi, QSize(-1,-1)));
                setAppletArgs(applet, mimetype, tjob->url().toString());
            } else {
                QAction *action;
                if (choices) {
                    choices->addSection(i18n("Widgets"));
                    action = choices->addAction(i18n("Icon"));
                } else {
                    QAction *sep = new QAction(i18n("Widgets"), nullptr);
                    sep->setSeparator(true);
                    dropActions << sep;
                    // we can at least create an icon as a link to the URL
                    action = new QAction(i18n("Icon"), nullptr);
                    dropActions << action;
                }

                const QUrl url = tjob->url();
                connect(action, &QAction::triggered, this, [this, posi, mimetype, url](){
                    Plasma::Applet *applet = createApplet(QStringLiteral("org.kde.plasma.icon"), QVariantList(), QRect(posi, QSize(-1,-1)));
                    setAppletArgs(applet, mimetype, url.toString());
                });
            }
        }

        if (choices) {
            // HACK If the QMenu becomes empty at any point after the "determining mimetype"
            // popup was shown, it self-destructs, does not matter if we call clear() or remove
            // the action manually, hence we remove the aforementioned item after we populated the menu
            choices->removeAction(choices->actions().at(0));
            choices->exec();
        } else {
            dropJob->setApplicationActions(dropActions);
        }

        clearDataForMimeJob(tjob);
    }
#endif // PLASMA_NO_KIO
}

void ContainmentInterface::appletAddedForward(Plasma::Applet *applet)
{
    if (!applet) {
        return;
    }

    AppletInterface *appletGraphicObject = applet->property("_plasma_graphicObject").value<AppletInterface *>();
    AppletInterface *contGraphicObject = m_containment->property("_plasma_graphicObject").value<AppletInterface *>();

//     qDebug() << "Applet added on containment:" << m_containment->title() << contGraphicObject
//              << "Applet: " << applet << applet->title() << appletGraphicObject;

    //Every applet should have a graphics object, otherwise don't disaplay anything
    if (!appletGraphicObject) {
        return;
    }

    if (contGraphicObject) {
        appletGraphicObject->setProperty("visible", false);
        appletGraphicObject->setProperty("parent", QVariant::fromValue(contGraphicObject));
    }

    m_appletInterfaces << appletGraphicObject;
    connect(appletGraphicObject, &QObject::destroyed, this,
            [this](QObject *obj) {
                m_appletInterfaces.removeAll(obj);
            });
    emit appletAdded(appletGraphicObject, appletGraphicObject->m_positionBeforeRemoval.x(), appletGraphicObject->m_positionBeforeRemoval.y());
    emit appletsChanged();
}

void ContainmentInterface::appletRemovedForward(Plasma::Applet *applet)
{
    AppletInterface *appletGraphicObject = applet->property("_plasma_graphicObject").value<AppletInterface *>();
    m_appletInterfaces.removeAll(appletGraphicObject);
    appletGraphicObject->m_positionBeforeRemoval = appletGraphicObject->mapToItem(this, QPointF());
    emit appletRemoved(appletGraphicObject);
    emit appletsChanged();
}

void ContainmentInterface::loadWallpaper()
{
    if (m_containment->containmentType() != Plasma::Types::DesktopContainment &&
            m_containment->containmentType() != Plasma::Types::CustomContainment) {
        return;
    }

    if (!m_containment->wallpaper().isEmpty()) {
        delete m_wallpaperInterface;

        m_wallpaperInterface = new WallpaperInterface(this);
        m_wallpaperInterface->setZ(-1000);
        //Qml seems happier if the parent gets set in this way
        m_wallpaperInterface->setProperty("parent", QVariant::fromValue(this));

        //set anchors
        QQmlExpression expr(qmlObject()->engine()->rootContext(), m_wallpaperInterface, QStringLiteral("parent"));
        QQmlProperty prop(m_wallpaperInterface, QStringLiteral("anchors.fill"));
        prop.write(expr.evaluate());

        m_containment->setProperty("wallpaperGraphicsObject", QVariant::fromValue(m_wallpaperInterface));
    } else {
        if (m_wallpaperInterface) {
            m_wallpaperInterface->deleteLater();
            m_wallpaperInterface = 0;
        }
    }
}

QString ContainmentInterface::activity() const
{
    return m_containment->activity();
}

QString ContainmentInterface::activityName() const
{
    if (!m_activityInfo) {
        return QString();
    }
    return m_activityInfo->name();
}

QList<QObject *> ContainmentInterface::actions() const
{
    //FIXME: giving directly a QList<QAction*> crashes

    QStringList actionOrder;
    actionOrder << QStringLiteral("add widgets") << QStringLiteral("manage activities") << QStringLiteral("remove") << QStringLiteral("lock widgets") << QStringLiteral("run associated application") << QStringLiteral("configure");
    QHash<QString, QAction *> orderedActions;
    //use a multimap to sort by action type
    QMultiMap<int, QObject *> actions;
    int i = 0;
    foreach (QAction *a, m_containment->actions()->actions()) {
        if (!actionOrder.contains(a->objectName())) {
            //FIXME QML visualizations don't support menus for now, *and* there is no way to
            //distinguish them on QML side
            if (!a->menu()) {
                actions.insert(a->data().toInt()*100 + i, a);
                ++i;
            }
        } else {
            orderedActions[a->objectName()] = a;
        }
    }

    i = 0;
    foreach (QAction *a, m_containment->corona()->actions()->actions()) {
        if (a->objectName() == QStringLiteral("lock widgets") || a->menu()) {
            //It is up to the Containment to decide if the user is allowed or not
            //to lock/unluck the widgets, so corona should not add one when there is none
            //(user is not allow) and it shouldn't add another one when there is already
            //one
            continue;
        }

        if (!actionOrder.contains(a->objectName())) {
            actions.insert(a->data().toInt()*100 + i, a);
        } else {
            orderedActions[a->objectName()] = a;
        }
        ++i;
    }
    QList<QObject *> actionList = actions.values();

    foreach (const QString &name, actionOrder) {
        QAction *a = orderedActions.value(name);
        if (a && !a->menu()) {
            actionList << a;
        }
        ++i;
    }

    return actionList;
}

//PROTECTED--------------------

void ContainmentInterface::mouseReleaseEvent(QMouseEvent *event)
{
    event->setAccepted(m_containment->containmentActions().contains(Plasma::ContainmentActions::eventToString(event)));
}

void ContainmentInterface::mousePressEvent(QMouseEvent *event)

{
    //even if the menu is executed synchronously, other events may be processed
    //by the qml incubator when plasma is loading, so we need to guard there
    if (m_contextMenu) {
        m_contextMenu.data()->close();
        return;
    }

    const QString trigger = Plasma::ContainmentActions::eventToString(event);
    Plasma::ContainmentActions *plugin = m_containment->containmentActions().value(trigger);

    if (!plugin || plugin->contextualActions().isEmpty()) {
        event->setAccepted(false);
        return;
    }

    //the plugin can be a single action or a context menu
    //Don't have an action list? execute as single action
    //and set the event position as action data
    if (plugin->contextualActions().length() == 1) {
        QAction *action = plugin->contextualActions().at(0);
        action->setData(event->pos());
        action->trigger();
        event->accept();
        return;
    }

    //FIXME: very inefficient appletAt() implementation
    Plasma::Applet *applet = 0;
    foreach (QObject *appletObject, m_appletInterfaces) {
        if (AppletInterface *ai = qobject_cast<AppletInterface *>(appletObject)) {
            if (ai->isVisible() && ai->contains(ai->mapFromItem(this, event->posF()))) {
                applet = ai->applet();
                break;
            } else {
                ai = 0;
            }
        }
    }
    //qDebug() << "Invoking menu for applet" << applet;

    QMenu *desktopMenu = new QMenu;
    desktopMenu->setAttribute(Qt::WA_DeleteOnClose);

    m_contextMenu = desktopMenu;

    if (applet) {
        emit applet->contextualActionsAboutToShow();
        addAppletActions(desktopMenu, applet, event);
    } else {
        emit m_containment->contextualActionsAboutToShow();
        addContainmentActions(desktopMenu, event);
    }

    //this is a workaround where Qt now creates the menu widget
    //in .exec before oxygen can polish it and set the following attribute
    desktopMenu->setAttribute(Qt::WA_TranslucentBackground);
    //end workaround

    if (window() && window()->mouseGrabberItem()) {
        window()->mouseGrabberItem()->ungrabMouse();
    }

    QPoint pos = event->globalPos();
    if (window() && m_containment->containmentType() == Plasma::Types::PanelContainment) {
        desktopMenu->adjustSize();

        if (QScreen *screen = window()->screen()) {
            const QRect geo = screen->availableGeometry();

            pos = QPoint(qBound(geo.left(), pos.x(), geo.right() + 1 - desktopMenu->width()),
                         qBound(geo.top(), pos.y(), geo.bottom() + 1 - desktopMenu->height()));
        }
    }

    if (desktopMenu->isEmpty()) {
        delete desktopMenu;
        event->accept();
        return;
    }

    KAcceleratorManager::manage(desktopMenu);

    desktopMenu->popup(pos);
    event->setAccepted(true);
}

void ContainmentInterface::wheelEvent(QWheelEvent *event)
{
    const QString trigger = Plasma::ContainmentActions::eventToString(event);
    Plasma::ContainmentActions *plugin = m_containment->containmentActions().value(trigger);

    if (!plugin) {
        event->setAccepted(false);
        return;
    }

    m_wheelDelta += event->delta();

    // Angle delta 120 for common "one click"
    // See: http://qt-project.org/doc/qt-5/qml-qtquick-wheelevent.html#angleDelta-prop
    while (m_wheelDelta >= 120) {
        m_wheelDelta -= 120;
        plugin->performPreviousAction();
    }
    while (m_wheelDelta <= -120) {
        m_wheelDelta += 120;
        plugin->performNextAction();
    }
}

void ContainmentInterface::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Menu) {
        QMouseEvent me(QEvent::MouseButtonRelease, QPoint(), Qt::RightButton, Qt::RightButton, event->modifiers());
        mousePressEvent(&me);
        event->accept();
    }

    AppletInterface::keyPressEvent(event);
}

void ContainmentInterface::addAppletActions(QMenu *desktopMenu, Plasma::Applet *applet, QEvent *event)
{
    foreach (QAction *action, applet->contextualActions()) {
        if (action) {
            desktopMenu->addAction(action);
        }
    }

    if (!applet->failedToLaunch()) {
        QAction *runAssociatedApplication = applet->actions()->action(QStringLiteral("run associated application"));
        if (runAssociatedApplication && runAssociatedApplication->isEnabled()) {
            desktopMenu->addAction(runAssociatedApplication);
        }

        QAction *configureApplet = applet->actions()->action(QStringLiteral("configure"));
        if (configureApplet && configureApplet->isEnabled()) {
            desktopMenu->addAction(configureApplet);
        }
        QAction *appletAlternatives = applet->actions()->action(QStringLiteral("alternatives"));
        if (appletAlternatives && appletAlternatives->isEnabled()) {
            desktopMenu->addAction(appletAlternatives);
        }
    }

    QMenu *containmentMenu = new QMenu(i18nc("%1 is the name of the containment", "%1 Options", m_containment->title()), desktopMenu);
    addContainmentActions(containmentMenu, event);

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
                        desktopMenu->addAction(action);
                    }
                }
            } else {
                desktopMenu->addMenu(containmentMenu);
            }
        }
    }

    if (m_containment->immutability() == Plasma::Types::Mutable &&
        (m_containment->containmentType() != Plasma::Types::PanelContainment || m_containment->isUserConfiguring())) {
        QAction *closeApplet = applet->actions()->action(QStringLiteral("remove"));
        //qDebug() << "checking for removal" << closeApplet;
        if (closeApplet) {
            if (!desktopMenu->isEmpty()) {
                desktopMenu->addSeparator();
            }

            //qDebug() << "adding close action" << closeApplet->isEnabled() << closeApplet->isVisible();
            desktopMenu->addAction(closeApplet);
        }
    }
}

void ContainmentInterface::addContainmentActions(QMenu *desktopMenu, QEvent *event)
{
    if (m_containment->corona()->immutability() != Plasma::Types::Mutable &&
            !KAuthorized::authorizeKAction(QStringLiteral("plasma/containment_actions"))) {
        //qDebug() << "immutability";
        return;
    }

    //this is what ContainmentPrivate::prepareContainmentActions was
    const QString trigger = Plasma::ContainmentActions::eventToString(event);
    Plasma::ContainmentActions *plugin = m_containment->containmentActions().value(trigger);

    if (!plugin) {
        return;
    }

    if (plugin->containment() != m_containment) {
        plugin->setContainment(m_containment);

        // now configure it
        KConfigGroup cfg(m_containment->corona()->config(), "ActionPlugins");
        cfg = KConfigGroup(&cfg, QString::number(m_containment->containmentType()));
        KConfigGroup pluginConfig = KConfigGroup(&cfg, trigger);
        plugin->restore(pluginConfig);
    }

    QList<QAction *> actions = plugin->contextualActions();

    if (actions.isEmpty()) {
        //it probably didn't bother implementing the function. give the user a chance to set
        //a better plugin.  note that if the user sets no-plugin this won't happen...
        if ((m_containment->containmentType() != Plasma::Types::PanelContainment &&
                m_containment->containmentType() != Plasma::Types::CustomPanelContainment) &&
                m_containment->actions()->action(QStringLiteral("configure"))) {
            desktopMenu->addAction(m_containment->actions()->action(QStringLiteral("configure")));
        }
    } else {
        desktopMenu->addActions(actions);
    }

    return;
}

#include "moc_containmentinterface.cpp"
