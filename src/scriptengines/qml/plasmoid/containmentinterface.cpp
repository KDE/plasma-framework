/*
    SPDX-FileCopyrightText: 2008 Chani Armitage <chani@kde.org>
    SPDX-FileCopyrightText: 2008, 2009 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2010 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "containmentinterface.h"
#include "dropmenu.h"
#include "wallpaperinterface.h"
#include <kdeclarative/qmlobject.h>

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QQmlExpression>
#include <QQmlProperty>
#include <QScreen>
#include <QVersionNumber>

#include <KAcceleratorManager>
#include <KActionCollection>
#include <KAuthorized>
#include <KLocalizedString>
#include <KNotification>
#include <KUrlMimeData>
#include <QDebug>
#include <QMimeDatabase>

#include <KIO/DropJob>
#include <KIO/MimetypeJob>
#include <KIO/TransferJob>

#include <Plasma/ContainmentActions>
#include <Plasma/Corona>
#include <Plasma/PluginLoader>
#include <plasma.h>

#include <KPackage/Package>
#include <KPackage/PackageLoader>

#include <kactivities/info.h>

#include <packageurlinterceptor.h>

ContainmentInterface::ContainmentInterface(DeclarativeAppletScript *parent, const QVariantList &args)
    : AppletInterface(parent, args)
    , m_wallpaperInterface(nullptr)
    , m_activityInfo(nullptr)
    , m_wheelDelta(0)
{
    m_containment = static_cast<Plasma::Containment *>(appletScript()->applet()->containment());

    setAcceptedMouseButtons(Qt::AllButtons);

    connect(m_containment.data(), &Plasma::Containment::appletRemoved, this, &ContainmentInterface::appletRemovedForward);
    connect(m_containment.data(), &Plasma::Containment::appletAdded, this, &ContainmentInterface::appletAddedForward);

    connect(m_containment->corona(), &Plasma::Corona::editModeChanged, this, &ContainmentInterface::editModeChanged);

    if (!m_appletInterfaces.isEmpty()) {
        Q_EMIT appletsChanged();
    }
}

void ContainmentInterface::init()
{
    if (qmlObject()->rootObject()) {
        return;
    }

    m_activityInfo = new KActivities::Info(m_containment->activity(), this);
    connect(m_activityInfo, &KActivities::Info::nameChanged, this, &ContainmentInterface::activityNameChanged);
    Q_EMIT activityNameChanged();

    if (!m_containment->wallpaper().isEmpty()) {
        loadWallpaper();
    }

    AppletInterface::init();

    // Create the ToolBox
    if (m_containment) {
        KConfigGroup defaults;
        if (m_containment->containmentType() == Plasma::Types::DesktopContainment) {
            defaults = KConfigGroup(KSharedConfig::openConfig(m_containment->corona()->kPackage().filePath("defaults")), "Desktop");
        } else if (m_containment->containmentType() == Plasma::Types::PanelContainment) {
            defaults = KConfigGroup(KSharedConfig::openConfig(m_containment->corona()->kPackage().filePath("defaults")), "Panel");
        }

        if (defaults.isValid()) {
            KPackage::Package pkg = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("KPackage/GenericQML"));
            pkg.setDefaultPackageRoot(QStringLiteral("plasma/packages"));

            if (defaults.isValid()) {
                pkg.setPath(defaults.readEntry("ToolBox", "org.kde.desktoptoolbox"));
            } else {
                pkg.setPath(QStringLiteral("org.kde.desktoptoolbox"));
            }

            PlasmaQuick::PackageUrlInterceptor *interceptor = dynamic_cast<PlasmaQuick::PackageUrlInterceptor *>(qmlObject()->engine()->urlInterceptor());
            if (interceptor) {
                interceptor->addAllowedPath(pkg.path());
            }

            if (pkg.metadata().isValid() && !pkg.metadata().isHidden()) {
                if (pkg.isValid()) {
                    QObject *containmentGraphicObject = qmlObject()->rootObject();

                    QVariantHash toolboxProperties;
                    toolboxProperties[QStringLiteral("parent")] = QVariant::fromValue(this);
                    QObject *toolBoxObject = qmlObject()->createObjectFromSource(pkg.fileUrl("mainscript"), nullptr, toolboxProperties);
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

    // set parent, both as object hierarchically and visually
    // do this only for containments, applets will do it in compactrepresentationcheck
    if (qmlObject()->rootObject()) {
        qmlObject()->rootObject()->setProperty("parent", QVariant::fromValue(this));

        // set anchors
        QQmlExpression expr(qmlObject()->engine()->rootContext(), qmlObject()->rootObject(), QStringLiteral("parent"));
        QQmlProperty prop(qmlObject()->rootObject(), QStringLiteral("anchors.fill"));
        prop.write(expr.evaluate());
    }

    connect(m_containment.data(), &Plasma::Containment::activityChanged, this, &ContainmentInterface::activityChanged);
    connect(m_containment.data(), &Plasma::Containment::activityChanged, this, [=]() {
        delete m_activityInfo;
        m_activityInfo = new KActivities::Info(m_containment->activity(), this);
        connect(m_activityInfo, &KActivities::Info::nameChanged, this, &ContainmentInterface::activityNameChanged);
        Q_EMIT activityNameChanged();
    });
    connect(m_containment.data(), &Plasma::Containment::wallpaperChanged, this, &ContainmentInterface::loadWallpaper);
    connect(m_containment.data(), &Plasma::Containment::containmentTypeChanged, this, &ContainmentInterface::containmentTypeChanged);

    connect(m_containment.data()->actions(), &KActionCollection::changed, this, &ContainmentInterface::actionsChanged);
}

QList<QObject *> ContainmentInterface::applets()
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
    // HACK
    // This is necessary to delay the appletAdded signal (of containmentInterface) AFTER the applet graphics object has been created
    blockSignals(true);
    Plasma::Applet *applet = m_containment->createApplet(plugin, args);

    if (applet) {
        QQuickItem *appletGraphicObject = applet->property("_plasma_graphicObject").value<QQuickItem *>();
        // invalid applet?
        if (!appletGraphicObject) {
            blockSignals(false);
            return applet;
        }
        if (geom.width() > 0 && geom.height() > 0) {
            appletGraphicObject->setSize(geom.size());
        }

        blockSignals(false);

        Q_EMIT appletAdded(appletGraphicObject, geom.x(), geom.y());
        Q_EMIT appletsChanged();
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
        Q_EMIT appletInterface->externalData(mimetype, data);
    }
}

QObject *ContainmentInterface::containmentAt(int x, int y)
{
    QObject *desktop = nullptr;
    const auto lst = m_containment->corona()->containments();
    for (Plasma::Containment *c : lst) {
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
    Q_EMIT appletAdded(applet, x, y);
}

QPointF ContainmentInterface::mapFromApplet(AppletInterface *applet, int x, int y)
{
    if (!applet->window() || !window()) {
        return QPointF();
    }

    // x,y in absolute screen coordinates of current view
    QPointF pos = applet->mapToScene(QPointF(x, y));
    pos = QPointF(pos + applet->window()->geometry().topLeft());
    // return the coordinate in the relative view's coords
    return pos - window()->geometry().topLeft();
}

QPointF ContainmentInterface::mapToApplet(AppletInterface *applet, int x, int y)
{
    if (!applet->window() || !window()) {
        return QPointF();
    }

    // x,y in absolute screen coordinates of current view
    QPointF pos(x, y);
    pos = QPointF(pos + window()->geometry().topLeft());
    // the coordinate in the relative view's coords
    pos = pos - applet->window()->geometry().topLeft();
    // make it relative to applet coords
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
        // make it relative
        QRect geometry = m_containment->corona()->screenGeometry(screenId);
        reg.translate(-geometry.topLeft());
    } else {
        reg = QRect(0, 0, width(), height());
    }

    const QRect rect(qBound(reg.boundingRect().left(), x, reg.boundingRect().right() + 1 - w),
                     qBound(reg.boundingRect().top(), y, reg.boundingRect().bottom() + 1 - h),
                     w,
                     h);
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
        // QRegion::contains doesn't do what it would suggest, so do reg.intersected(rect) != rect instead
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

        // bottom left corner
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

        // top right corner
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

        // bottom right corner
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

QAction *ContainmentInterface::globalAction(QString name) const
{
    return m_containment->corona()->actions()->action(name);
}

bool ContainmentInterface::isEditMode() const
{
    return m_containment->corona()->isEditMode();
}

void ContainmentInterface::setEditMode(bool edit)
{
    m_containment->corona()->setEditMode(edit);
}

void ContainmentInterface::processMimeData(QObject *mimeDataProxy, int x, int y, KIO::DropJob *dropJob)
{
    QMimeData *mime = qobject_cast<QMimeData *>(mimeDataProxy);
    if (mime) {
        processMimeData(mime, x, y, dropJob);
    } else {
        processMimeData(mimeDataProxy->property("mimeData").value<QMimeData *>(), x, y, dropJob);
    }
}

void ContainmentInterface::processMimeData(QMimeData *mimeData, int x, int y, KIO::DropJob *dropJob)
{
    if (!mimeData) {
        return;
    }

    if (m_dropMenu) {
        if (dropJob) {
            dropJob->kill();
        }
        return;
    }
    m_dropMenu = QPointer<DropMenu>(new DropMenu(dropJob, mapToGlobal(QPoint(x, y)).toPoint(), this));
    if (dropJob) {
        dropJob->setParent(m_dropMenu);
    }

    // const QMimeData *mimeData = data;

    qDebug() << "Arrived mimeData" << mimeData->urls() << mimeData->formats() << "at" << x << ", " << y;

    // Catch drops from a Task Manager and convert to usable URL.
    if (!mimeData->hasUrls() && mimeData->hasFormat(QStringLiteral("text/x-orgkdeplasmataskmanager_taskurl"))) {
        QList<QUrl> urls = {QUrl(QString::fromUtf8(mimeData->data(QStringLiteral("text/x-orgkdeplasmataskmanager_taskurl"))))};
        mimeData->setUrls(urls);
    }

    if (mimeData->hasFormat(QStringLiteral("text/x-plasmoidservicename"))) {
        QString data = QString::fromUtf8(mimeData->data(QStringLiteral("text/x-plasmoidservicename")));
        const QStringList appletNames = data.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
        for (const QString &appletName : appletNames) {
            qDebug() << "adding" << appletName;

            metaObject()->invokeMethod(this,
                                       "createApplet",
                                       Qt::QueuedConnection,
                                       Q_ARG(QString, appletName),
                                       Q_ARG(QVariantList, QVariantList()),
                                       Q_ARG(QRectF, QRectF(x, y, -1, -1)));
        }
        delete m_dropMenu.data();
    } else if (mimeData->hasUrls()) {
        // TODO: collect the mimetypes of available script engines and offer
        //      to create widgets out of the matching URLs, if any
        const QList<QUrl> urls = KUrlMimeData::urlsFromMimeData(mimeData);
        m_dropMenu->setUrls(urls);

        if (!urls.at(0).isLocalFile()) {
            QApplication::setOverrideCursor(Qt::WaitCursor);
        }

        QMimeDatabase db;
        QMimeType firstMimetype = db.mimeTypeForUrl(urls.at(0));
        for (const QUrl &url : urls) {
            if (firstMimetype != db.mimeTypeForUrl(url)) {
                m_dropMenu->setMultipleMimetypes(true);
                break;
            }
        }

        // It may be a directory or a file, let's stat
        KIO::JobFlags flags = KIO::HideProgressInfo;
        KIO::MimetypeJob *job = KIO::mimetype(m_dropMenu->urls().at(0), flags);
        job->setParent(m_dropMenu.data());

        QObject::connect(job, &KJob::result, this, &ContainmentInterface::dropJobResult);
        QObject::connect(job, &KIO::MimetypeJob::mimeTypeFound, this, &ContainmentInterface::mimeTypeRetrieved);

    } else {
        bool deleteDropMenu = true;

        const QStringList formats = mimeData->formats();
        QHash<QString, KPluginMetaData> seenPlugins;
        QHash<QString, QString> pluginFormats;

        for (const QString &format : formats) {
            const auto plugins = Plasma::PluginLoader::self()->listAppletMetaDataForMimeType(format);

            for (const auto &plugin : plugins) {
                if (seenPlugins.contains(plugin.pluginId())) {
                    continue;
                }

                seenPlugins.insert(plugin.pluginId(), plugin);
                pluginFormats.insert(plugin.pluginId(), format);
            }
        }
        // qDebug() << "Mimetype ..." << formats << seenPlugins.keys() << pluginFormats.values();

        QString selectedPlugin;

        if (seenPlugins.isEmpty()) {
            // do nothing
            // directly create if only one offer only if the containment didn't pass an existing plugin
        } else if (seenPlugins.count() == 1) {
            selectedPlugin = seenPlugins.constBegin().key();
            Plasma::Applet *applet = createApplet(selectedPlugin, QVariantList(), QRect(x, y, -1, -1));
            setAppletArgs(applet, pluginFormats[selectedPlugin], QString::fromUtf8(mimeData->data(pluginFormats[selectedPlugin])));
        } else {
            QHash<QAction *, QString> actionsToPlugins;
            for (const auto &info : qAsConst(seenPlugins)) {
                QAction *action;
                if (!info.iconName().isEmpty()) {
                    action = new QAction(QIcon::fromTheme(info.iconName()), info.name(), m_dropMenu);
                } else {
                    action = new QAction(info.name(), m_dropMenu);
                }
                m_dropMenu->addAction(action);
                action->setData(info.pluginId());
                connect(action, &QAction::triggered, this, [this, x, y, mimeData, action]() {
                    const QString selectedPlugin = action->data().toString();
                    Plasma::Applet *applet = createApplet(selectedPlugin, QVariantList(), QRect(x, y, -1, -1));
                    setAppletArgs(applet, selectedPlugin, QString::fromUtf8(mimeData->data(selectedPlugin)));
                });

                actionsToPlugins.insert(action, info.pluginId());
            }
            m_dropMenu->show();
            deleteDropMenu = false;
        }

        if (deleteDropMenu) {
            // in case m_dropMenu has not been shown
            delete m_dropMenu.data();
        }
    }
}

void ContainmentInterface::clearDataForMimeJob(KIO::Job *job)
{
    QObject::disconnect(job, nullptr, this, nullptr);
    job->kill();

    m_dropMenu->show();

    if (!m_dropMenu->urls().at(0).isLocalFile()) {
        QApplication::restoreOverrideCursor();
    }
}

void ContainmentInterface::dropJobResult(KJob *job)
{
    if (job->error()) {
        qDebug() << "ERROR" << job->error() << ' ' << job->errorString();
        clearDataForMimeJob(dynamic_cast<KIO::Job *>(job));
    }
}

void ContainmentInterface::mimeTypeRetrieved(KIO::Job *job, const QString &mimetype)
{
    qDebug() << "Mimetype Job returns." << mimetype;

    KIO::TransferJob *tjob = dynamic_cast<KIO::TransferJob *>(job);
    if (!tjob) {
        qDebug() << "job should be a TransferJob, but isn't";
        clearDataForMimeJob(job);
        return;
    }

    QList<KPluginMetaData> appletList = Plasma::PluginLoader::self()->listAppletMetaDataForUrl(tjob->url());
    if (mimetype.isEmpty() && appletList.isEmpty()) {
        clearDataForMimeJob(job);
        qDebug() << "No applets found matching the url (" << tjob->url() << ") or the mimetype (" << mimetype << ")";
        return;
    } else {
        qDebug() << "Received a suitable dropEvent at " << m_dropMenu->dropPoint();
        qDebug() << "Bailing out. Cannot find associated dropEvent related to the TransferJob";

        qDebug() << "Creating menu for: " << mimetype;

        appletList << Plasma::PluginLoader::self()->listAppletMetaDataForMimeType(mimetype);

        QList<KPluginMetaData> wallpaperList;

        if (m_containment->containmentType() != Plasma::Types::PanelContainment && m_containment->containmentType() != Plasma::Types::CustomPanelContainment) {
            if (m_wallpaperInterface && m_wallpaperInterface->supportsMimetype(mimetype)) {
                wallpaperList << m_wallpaperInterface->kPackage().metadata();
            } else {
                wallpaperList = WallpaperInterface::listWallpaperMetadataForMimetype(mimetype);
            }
        }

        const bool isPlasmaPackage = (mimetype == QLatin1String("application/x-plasma"));

        if ((!appletList.isEmpty() || !wallpaperList.isEmpty() || isPlasmaPackage) && !m_dropMenu->isMultipleMimetypes()) {
            QAction *installPlasmaPackageAction = nullptr;
            if (isPlasmaPackage) {
                QAction *action = new QAction(i18n("Plasma Package"), m_dropMenu);
                action->setSeparator(true);
                m_dropMenu->addAction(action);

                installPlasmaPackageAction = new QAction(QIcon::fromTheme(QStringLiteral("application-x-plasma")), i18n("Install"), m_dropMenu);
                m_dropMenu->addAction(installPlasmaPackageAction);

                const QString &packagePath = tjob->url().toLocalFile();
                connect(installPlasmaPackageAction, &QAction::triggered, this, [this, packagePath]() {
                    using namespace KPackage;
                    PackageStructure *structure = PackageLoader::self()->loadPackageStructure(QStringLiteral("Plasma/Applet"));
                    Package package(structure);

                    KJob *installJob = package.update(packagePath);
                    connect(installJob, &KJob::result, this, [this, packagePath, structure](KJob *job) {
                        auto fail = [](const QString &text) {
                            KNotification::event(QStringLiteral("plasmoidInstallationFailed"),
                                                 i18n("Package Installation Failed"),
                                                 text,
                                                 QStringLiteral("dialog-error"),
                                                 nullptr,
                                                 KNotification::CloseOnTimeout,
                                                 QStringLiteral("plasma_workspace"));
                        };

                        // if the applet is already installed, just add it to the containment
                        if (job->error() != KJob::NoError && job->error() != Package::PackageAlreadyInstalledError
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

                        createApplet(package.metadata().pluginId(), QVariantList(), QRect(m_dropMenu->dropPoint(), QSize(-1, -1)));
                    });
                });
            }

            QAction *action = new QAction(i18n("Widgets"), m_dropMenu);
            action->setSeparator(true);
            m_dropMenu->addAction(action);

            for (const auto &info : qAsConst(appletList)) {
                const QString actionText = i18nc("Add widget", "Add %1", info.name());
                QAction *action = new QAction(actionText, m_dropMenu);
                if (!info.iconName().isEmpty()) {
                    action->setIcon(QIcon::fromTheme(info.iconName()));
                }
                m_dropMenu->addAction(action);
                action->setData(info.pluginId());
                const QUrl url = tjob->url();
                connect(action, &QAction::triggered, this, [this, action, mimetype, url]() {
                    Plasma::Applet *applet = createApplet(action->data().toString(), QVariantList(), QRect(m_dropMenu->dropPoint(), QSize(-1, -1)));
                    setAppletArgs(applet, mimetype, url.toString());
                });
            }
            {
                QAction *action = new QAction(i18nc("Add icon widget", "Add Icon"), m_dropMenu);
                m_dropMenu->addAction(action);
                action->setData(QStringLiteral("org.kde.plasma.icon"));
                const QUrl url = tjob->url();
                connect(action, &QAction::triggered, this, [this, action, mimetype, url]() {
                    Plasma::Applet *applet = createApplet(action->data().toString(), QVariantList(), QRect(m_dropMenu->dropPoint(), QSize(-1, -1)));
                    setAppletArgs(applet, mimetype, url.toString());
                });
            }

            QHash<QAction *, QString> actionsToWallpapers;
            if (!wallpaperList.isEmpty()) {
                QAction *action = new QAction(i18n("Wallpaper"), m_dropMenu);
                action->setSeparator(true);
                m_dropMenu->addAction(action);

                QMap<QString, KPluginMetaData> sorted;
                for (const auto &info : qAsConst(appletList)) {
                    sorted.insert(info.name(), info);
                }

                for (const KPluginMetaData &info : qAsConst(wallpaperList)) {
                    const QString actionText = i18nc("Set wallpaper", "Set %1", info.name());
                    QAction *action = new QAction(actionText, m_dropMenu);
                    if (!info.iconName().isEmpty()) {
                        action->setIcon(QIcon::fromTheme(info.iconName()));
                    }
                    m_dropMenu->addAction(action);
                    actionsToWallpapers.insert(action, info.pluginId());
                    const QUrl url = tjob->url();
                    connect(action, &QAction::triggered, this, [this, url]() {
                        // set wallpapery stuff
                        if (m_wallpaperInterface && url.isValid()) {
                            m_wallpaperInterface->setUrl(url);
                        }
                    });
                }
            }
        } else {
            // case in which we created the menu ourselves, just the "fetching type entry, directly create the icon applet
            if (!m_dropMenu->isDropjobMenu()) {
                Plasma::Applet *applet = createApplet(QStringLiteral("org.kde.plasma.icon"), QVariantList(), QRect(m_dropMenu->dropPoint(), QSize(-1, -1)));
                setAppletArgs(applet, mimetype, tjob->url().toString());
            } else {
                QAction *action;
                QAction *sep = new QAction(i18n("Widgets"), m_dropMenu);
                sep->setSeparator(true);
                m_dropMenu->addAction(sep);
                // we can at least create an icon as a link to the URL
                action = new QAction(i18nc("Add icon widget", "Add Icon"), m_dropMenu);
                m_dropMenu->addAction(action);

                const QUrl url = tjob->url();
                connect(action, &QAction::triggered, this, [this, mimetype, url]() {
                    Plasma::Applet *applet = createApplet(QStringLiteral("org.kde.plasma.icon"), QVariantList(), QRect(m_dropMenu->dropPoint(), QSize(-1, -1)));
                    setAppletArgs(applet, mimetype, url.toString());
                });
            }
        }
        clearDataForMimeJob(tjob);
    }
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

    // applets can not have a graphic object if they don't have a script engine loaded
    // this can happen if they were loaded with an invalid metadata
    if (!appletGraphicObject) {
        return;
    }

    if (contGraphicObject) {
        appletGraphicObject->setProperty("visible", false);
        appletGraphicObject->setProperty("parent", QVariant::fromValue(contGraphicObject));
    }

    m_appletInterfaces << appletGraphicObject;
    connect(appletGraphicObject, &QObject::destroyed, this, [this](QObject *obj) {
        m_appletInterfaces.removeAll(obj);
    });
    Q_EMIT appletAdded(appletGraphicObject, appletGraphicObject->m_positionBeforeRemoval.x(), appletGraphicObject->m_positionBeforeRemoval.y());
    Q_EMIT appletsChanged();
}

void ContainmentInterface::appletRemovedForward(Plasma::Applet *applet)
{
    AppletInterface *appletGraphicObject = applet->property("_plasma_graphicObject").value<AppletInterface *>();
    if (appletGraphicObject) {
        m_appletInterfaces.removeAll(appletGraphicObject);
        appletGraphicObject->m_positionBeforeRemoval = appletGraphicObject->mapToItem(this, QPointF());
    }
    Q_EMIT appletRemoved(appletGraphicObject);
    Q_EMIT appletsChanged();
}

void ContainmentInterface::loadWallpaper()
{
    if (m_containment->containmentType() != Plasma::Types::DesktopContainment && m_containment->containmentType() != Plasma::Types::CustomContainment) {
        return;
    }

    if (!m_wallpaperInterface && !m_containment->wallpaper().isEmpty()) {
        m_wallpaperInterface = new WallpaperInterface(this);

        m_wallpaperInterface->setZ(-1000);
        // Qml seems happier if the parent gets set in this way
        m_wallpaperInterface->setProperty("parent", QVariant::fromValue(this));

        connect(m_wallpaperInterface, &WallpaperInterface::isLoadingChanged, this, &AppletInterface::updateUiReadyConstraint);

        // set anchors
        QQmlExpression expr(qmlObject()->engine()->rootContext(), m_wallpaperInterface, QStringLiteral("parent"));
        QQmlProperty prop(m_wallpaperInterface, QStringLiteral("anchors.fill"));
        prop.write(expr.evaluate());

        m_containment->setProperty("wallpaperGraphicsObject", QVariant::fromValue(m_wallpaperInterface));
    } else if (m_wallpaperInterface && m_containment->wallpaper().isEmpty()) {
        m_wallpaperInterface->deleteLater();
        m_wallpaperInterface = nullptr;
    }

    Q_EMIT wallpaperInterfaceChanged();
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
    // FIXME: giving directly a QList<QAction*> crashes

    QStringList actionOrder;
    actionOrder << QStringLiteral("add widgets") << QStringLiteral("manage activities") << QStringLiteral("remove") << QStringLiteral("lock widgets")
                << QStringLiteral("run associated application") << QStringLiteral("configure");
    QHash<QString, QAction *> orderedActions;
    // use a multimap to sort by action type
    QMultiMap<int, QObject *> actions;
    int i = 0;
    auto listActions = m_containment->actions()->actions();
    for (QAction *a : qAsConst(listActions)) {
        if (!actionOrder.contains(a->objectName())) {
            // FIXME QML visualizations don't support menus for now, *and* there is no way to
            // distinguish them on QML side
            if (!a->menu()) {
                actions.insert(a->data().toInt() * 100 + i, a);
                ++i;
            }
        } else {
            orderedActions[a->objectName()] = a;
        }
    }

    i = 0;
    listActions = m_containment->corona()->actions()->actions();
    for (QAction *a : qAsConst(listActions)) {
        if (a->objectName() == QLatin1String("lock widgets") || a->menu()) {
            // It is up to the Containment to decide if the user is allowed or not
            // to lock/unluck the widgets, so corona should not add one when there is none
            //(user is not allow) and it shouldn't add another one when there is already
            // one
            continue;
        }

        if (!actionOrder.contains(a->objectName())) {
            actions.insert(a->data().toInt() * 100 + i, a);
        } else {
            orderedActions[a->objectName()] = a;
        }
        ++i;
    }
    QList<QObject *> actionList = actions.values();

    for (const QString &name : qAsConst(actionOrder)) {
        QAction *a = orderedActions.value(name);
        if (a && !a->menu()) {
            actionList << a;
        }
        ++i;
    }

    return actionList;
}

void ContainmentInterface::setContainmentDisplayHints(Plasma::Types::ContainmentDisplayHints hints)
{
    m_containment->setContainmentDisplayHints(hints);
}

// PROTECTED--------------------

void ContainmentInterface::mouseReleaseEvent(QMouseEvent *event)
{
    event->setAccepted(m_containment->containmentActions().contains(Plasma::ContainmentActions::eventToString(event)));
}

void ContainmentInterface::mousePressEvent(QMouseEvent *event)

{
    // even if the menu is executed synchronously, other events may be processed
    // by the qml incubator when plasma is loading, so we need to guard there
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

    // the plugin can be a single action or a context menu
    // Don't have an action list? execute as single action
    // and set the event position as action data
    if (plugin->contextualActions().length() == 1) {
        QAction *action = plugin->contextualActions().at(0);
        action->setData(event->pos());
        action->trigger();
        event->accept();
        return;
    }

    // FIXME: very inefficient appletAt() implementation
    Plasma::Applet *applet = nullptr;
    for (QObject *appletObject : qAsConst(m_appletInterfaces)) {
        if (AppletInterface *ai = qobject_cast<AppletInterface *>(appletObject)) {
            if (ai->isVisible() && ai->contains(ai->mapFromItem(this, event->localPos()))) {
                applet = ai->applet();
                break;
            } else {
                ai = nullptr;
            }
        }
    }
    // qDebug() << "Invoking menu for applet" << applet;

    QMenu *desktopMenu = new QMenu;

    // this is a workaround where Qt now creates the menu widget
    // in .exec before oxygen can polish it and set the following attribute
    desktopMenu->setAttribute(Qt::WA_TranslucentBackground);
    // end workaround

    if (desktopMenu->winId()) {
        desktopMenu->windowHandle()->setTransientParent(window());
    }
    desktopMenu->setAttribute(Qt::WA_DeleteOnClose);

    m_contextMenu = desktopMenu;

    Q_EMIT m_containment->contextualActionsAboutToShow();

    if (applet) {
        Q_EMIT applet->contextualActionsAboutToShow();
        addAppletActions(desktopMenu, applet, event);
    } else {
        addContainmentActions(desktopMenu, event);
    }

    // this is a workaround where Qt will fail to realize a mouse has been released

    // this happens if a window which does not accept focus spawns a new window that takes focus and X grab
    // whilst the mouse is depressed
    // https://bugreports.qt.io/browse/QTBUG-59044
    // this causes the next click to go missing

    // by releasing manually we avoid that situation
    auto ungrabMouseHack = [this]() {
        if (window() && window()->mouseGrabberItem()) {
            window()->mouseGrabberItem()->ungrabMouse();
        }
    };

    // pre 5.8.0 QQuickWindow code is "item->grabMouse(); sendEvent(item, mouseEvent)"
    // post 5.8.0 QQuickWindow code is sendEvent(item, mouseEvent); item->grabMouse()
    if (QVersionNumber::fromString(QLatin1String(qVersion())) > QVersionNumber(5, 8, 0)) {
        QTimer::singleShot(0, this, ungrabMouseHack);
    } else {
        ungrabMouseHack();
    }
    // end workaround

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

    // Bug 344205 keep panel visible while menu is open
    const auto oldStatus = m_containment->status();
    m_containment->setStatus(Plasma::Types::RequiresAttentionStatus);

    connect(desktopMenu, &QMenu::aboutToHide, m_containment, [this, oldStatus] {
        m_containment->setStatus(oldStatus);
    });

    KAcceleratorManager::manage(desktopMenu);

    for (auto action : desktopMenu->actions()) {
        if (action->menu()) {
            connect(action->menu(), &QMenu::aboutToShow, desktopMenu, [action, desktopMenu] {
                if (action->menu()->windowHandle()) {
                    // Need to add the transient parent otherwise Qt will create a new toplevel
                    action->menu()->windowHandle()->setTransientParent(desktopMenu->windowHandle());
                }
            });
        }
    }

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

    m_wheelDelta += event->angleDelta().y();

    // Angle delta 120 for common "one click"
    // See: https://doc.qt.io/qt-5/qml-qtquick-wheelevent.html#angleDelta-prop
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
    const auto listActions = applet->contextualActions();
    for (QAction *action : listActions) {
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

    desktopMenu->addSeparator();
    if (m_containment->containmentType() == Plasma::Types::DesktopContainment) {
        auto action = m_containment->corona()->actions()->action(QStringLiteral("edit mode"));
        if (action) {
            desktopMenu->addAction(action);
        }
    } else {
        addContainmentActions(desktopMenu, event);
    }

    if (m_containment->immutability() == Plasma::Types::Mutable
        && (m_containment->containmentType() != Plasma::Types::PanelContainment || m_containment->isUserConfiguring())) {
        QAction *closeApplet = applet->actions()->action(QStringLiteral("remove"));
        // qDebug() << "checking for removal" << closeApplet;
        if (closeApplet) {
            if (!desktopMenu->isEmpty()) {
                desktopMenu->addSeparator();
            }

            // qDebug() << "adding close action" << closeApplet->isEnabled() << closeApplet->isVisible();
            desktopMenu->addAction(closeApplet);
        }
    }
}

void ContainmentInterface::addContainmentActions(QMenu *desktopMenu, QEvent *event)
{
    if (m_containment->corona()->immutability() != Plasma::Types::Mutable //
        && !KAuthorized::authorizeAction(QStringLiteral("plasma/containment_actions"))) {
        // qDebug() << "immutability";
        return;
    }

    // this is what ContainmentPrivate::prepareContainmentActions was
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
        // it probably didn't bother implementing the function. give the user a chance to set
        // a better plugin.  note that if the user sets no-plugin this won't happen...
        /* clang-format off */
        if ((m_containment->containmentType() != Plasma::Types::PanelContainment
                && m_containment->containmentType() != Plasma::Types::CustomPanelContainment)
            && m_containment->actions()->action(QStringLiteral("configure"))) { /* clang-format on */
            desktopMenu->addAction(m_containment->actions()->action(QStringLiteral("configure")));
        }
    } else {
        desktopMenu->addActions(actions);
    }

    return;
}

bool ContainmentInterface::isLoading() const
{
    bool loading = AppletInterface::isLoading();
    if (m_wallpaperInterface) {
        loading |= m_wallpaperInterface->isLoading();
    }
    return loading;
}

#include "moc_containmentinterface.cpp"
