/*
    SPDX-FileCopyrightText: 2008 Chani Armitage <chani@kde.org>
    SPDX-FileCopyrightText: 2008, 2009 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2010 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "containmentitem.h"
#include "dropmenu.h"
#include "private/appletquickitem_p.h"
#include "sharedqmlengine.h"
#include "wallpaperitem.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QQmlExpression>
#include <QQmlProperty>
#include <QScreen>
#include <QVersionNumber>

#include <KAcceleratorManager>
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
#include <KPackage/PackageJob>
#include <KPackage/PackageLoader>

ContainmentItem::ContainmentItem(QQuickItem *parent)
    : PlasmoidItem(parent)
    , m_wallpaperItem(nullptr)
    , m_wheelDelta(0)
{
    setAcceptedMouseButtons(Qt::AllButtons);
}

void ContainmentItem::classBegin()
{
    PlasmoidItem::classBegin();
    m_containment = static_cast<Plasma::Containment *>(applet());

    connect(m_containment.data(), &Plasma::Containment::appletAboutToBeRemoved, this, &ContainmentItem::appletRemovedForward);
    connect(m_containment.data(), &Plasma::Containment::appletAboutToBeAdded, this, &ContainmentItem::appletAddedForward);

    connect(m_containment->corona(), &Plasma::Corona::editModeChanged, this, &ContainmentItem::editModeChanged);
}

void ContainmentItem::init()
{
    PlasmoidItem::init();

    for (auto *applet : m_containment->applets()) {
        auto appletGraphicObject = AppletQuickItem::itemForApplet(applet);
        m_plasmoidItems.append(appletGraphicObject);
        connect(appletGraphicObject, &QObject::destroyed, this, [this, appletGraphicObject]() {
            m_plasmoidItems.removeAll(appletGraphicObject);
        });
    }
    if (!m_plasmoidItems.isEmpty()) {
        Q_EMIT appletsChanged();
    }

    // Create the ToolBox
    if (m_containment) {
        KConfigGroup defaults;
        if (m_containment->containmentType() == Plasma::Containment::Type::Desktop) {
            defaults = KConfigGroup(KSharedConfig::openConfig(m_containment->corona()->kPackage().filePath("defaults")), "Desktop");
        } else if (m_containment->containmentType() == Plasma::Containment::Type::Panel) {
            defaults = KConfigGroup(KSharedConfig::openConfig(m_containment->corona()->kPackage().filePath("defaults")), "Panel");
        }

        if (defaults.isValid()) {
            KPackage::Package pkg = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/Generic"));
            pkg.setDefaultPackageRoot(QStringLiteral("plasma/packages"));

            if (defaults.isValid()) {
                pkg.setPath(defaults.readEntry("ToolBox", "org.kde.desktoptoolbox"));
            } else {
                pkg.setPath(QStringLiteral("org.kde.desktoptoolbox"));
            }

            if (pkg.metadata().isValid() && !pkg.metadata().isHidden()) {
                if (pkg.isValid()) {
                    QObject *containmentGraphicObject = qmlObject()->rootObject();

                    QVariantHash toolboxProperties;
                    toolboxProperties[QStringLiteral("parent")] = QVariant::fromValue(this);
                    QObject *toolBoxObject = qmlObject()->createObjectFromSource(pkg.fileUrl("mainscript"), nullptr, toolboxProperties);
                    if (toolBoxObject && containmentGraphicObject) {
                        connect(this, &QObject::destroyed, [toolBoxObject]() {
                            delete toolBoxObject;
                        });
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

    connect(m_containment.data(), &Plasma::Containment::wallpaperPluginChanged, this, &ContainmentItem::loadWallpaper);

    connect(m_containment, &Plasma::Containment::internalActionsChanged, this, &ContainmentItem::actionsChanged);
    connect(m_containment, &Plasma::Containment::contextualActionsChanged, this, &ContainmentItem::actionsChanged);
}

PlasmaQuick::AppletQuickItem *ContainmentItem::itemFor(Plasma::Applet *applet) const
{
    if (!applet) {
        return nullptr;
    }
    if (applet->containment() == m_containment) {
        return AppletQuickItem::itemForApplet(applet);
    } else {
        return nullptr;
    }
}

Plasma::Applet *ContainmentItem::createApplet(const QString &plugin, const QVariantList &args, const QRectF &geom)
{
    return m_containment->createApplet(plugin, args, geom);
}

void ContainmentItem::setAppletArgs(Plasma::Applet *applet, const QString &mimetype, const QVariant &data)
{
    if (!applet) {
        return;
    }

    PlasmoidItem *plasmoidItem = qobject_cast<PlasmoidItem *>(AppletQuickItem::itemForApplet(applet));

    if (plasmoidItem) {
        Q_EMIT plasmoidItem->externalData(mimetype, data);
    }
}

QObject *ContainmentItem::containmentItemAt(int x, int y)
{
    QObject *desktop = nullptr;
    const auto lst = m_containment->corona()->containments();
    for (Plasma::Containment *c : lst) {
        ContainmentItem *contInterface = qobject_cast<ContainmentItem *>(AppletQuickItem::itemForApplet(c));

        if (contInterface && contInterface->isVisible()) {
            QWindow *w = contInterface->window();
            if (w && w->geometry().contains(QPoint(window()->x(), window()->y()) + QPoint(x, y))) {
                if (c->containmentType() == Plasma::Containment::Type::CustomEmbedded) {
                    continue;
                }
                if (c->containmentType() == Plasma::Containment::Type::Desktop) {
                    desktop = contInterface;
                } else {
                    return contInterface;
                }
            }
        }
    }
    return desktop;
}

QPointF ContainmentItem::mapFromApplet(Plasma::Applet *applet, int x, int y)
{
    PlasmoidItem *appletItem = qobject_cast<PlasmoidItem *>(AppletQuickItem::itemForApplet(applet));
    if (!appletItem || !appletItem->window() || !window()) {
        return QPointF();
    }

    // x,y in absolute screen coordinates of current view
    QPointF pos = appletItem->mapToScene(QPointF(x, y));
    pos = QPointF(pos + appletItem->window()->geometry().topLeft());
    // return the coordinate in the relative view's coords
    return pos - window()->geometry().topLeft();
}

QPointF ContainmentItem::mapToApplet(Plasma::Applet *applet, int x, int y)
{
    PlasmoidItem *appletItem = qobject_cast<PlasmoidItem *>(AppletQuickItem::itemForApplet(applet));
    if (!appletItem || !appletItem->window() || !window()) {
        return QPointF();
    }

    // x,y in absolute screen coordinates of current view
    QPointF pos(x, y);
    pos = QPointF(pos + window()->geometry().topLeft());
    // the coordinate in the relative view's coords
    pos = pos - appletItem->window()->geometry().topLeft();
    // make it relative to applet coords
    return pos - appletItem->mapToScene(QPointF(0, 0));
}

QPointF ContainmentItem::adjustToAvailableScreenRegion(int x, int y, int w, int h) const
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

void ContainmentItem::openContextMenu(const QPointF &globalPos)
{
    if (globalPos.isNull()) {
        return;
    }

    QMouseEvent me(QEvent::MouseButtonRelease, QPointF(), globalPos, Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    mousePressEvent(&me);
}

void ContainmentItem::processMimeData(QObject *mimeDataProxy, int x, int y, KIO::DropJob *dropJob)
{
    QMimeData *mime = qobject_cast<QMimeData *>(mimeDataProxy);
    if (mime) {
        processMimeData(mime, x, y, dropJob);
    } else {
        processMimeData(mimeDataProxy->property("mimeData").value<QMimeData *>(), x, y, dropJob);
    }
}

void ContainmentItem::processMimeData(QMimeData *mimeData, int x, int y, KIO::DropJob *dropJob)
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

        QObject::connect(job, &KJob::result, this, &ContainmentItem::dropJobResult);
        QObject::connect(job, &KIO::MimetypeJob::mimeTypeFound, this, &ContainmentItem::mimeTypeRetrieved);

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
            setAppletArgs(applet, pluginFormats[selectedPlugin], mimeData->data(pluginFormats[selectedPlugin]));

        } else {
            QHash<QAction *, QString> actionsToPlugins;
            for (const auto &info : std::as_const(seenPlugins)) {
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
                    setAppletArgs(applet, selectedPlugin, mimeData->data(selectedPlugin));
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

void ContainmentItem::clearDataForMimeJob(KIO::Job *job)
{
    QObject::disconnect(job, nullptr, this, nullptr);
    job->kill();

    m_dropMenu->show();

    if (!m_dropMenu->urls().at(0).isLocalFile()) {
        QApplication::restoreOverrideCursor();
    }
}

void ContainmentItem::dropJobResult(KJob *job)
{
    if (job->error()) {
        qDebug() << "ERROR" << job->error() << ' ' << job->errorString();
        clearDataForMimeJob(dynamic_cast<KIO::Job *>(job));
    }
}

void ContainmentItem::mimeTypeRetrieved(KIO::Job *job, const QString &mimetype)
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

        if (m_containment->containmentType() != Plasma::Containment::Type::Panel
            && m_containment->containmentType() != Plasma::Containment::Type::CustomPanel) {
            if (m_wallpaperItem && m_wallpaperItem->supportsMimetype(mimetype)) {
                wallpaperList << m_wallpaperItem->kPackage().metadata();
            } else {
                wallpaperList = WallpaperItem::listWallpaperMetadataForMimetype(mimetype);
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

                    PackageJob *job = PackageJob::update(QStringLiteral("Plasma/Applet"), packagePath);
                    connect(job, &KJob::finished, this, [this, packagePath, job]() {
                        auto fail = [](const QString &text) {
                            KNotification::event(QStringLiteral("plasmoidInstallationFailed"),
                                                 i18n("Package Installation Failed"),
                                                 text,
                                                 QStringLiteral("dialog-error"),
                                                 KNotification::CloseOnTimeout,
                                                 QStringLiteral("plasma_workspace"));
                        };

                        // if the applet is already installed, just add it to the containment
                        if (job->error() != KJob::NoError && job->error() != PackageJob::PackageAlreadyInstalledError
                            && job->error() != PackageJob::NewerVersionAlreadyInstalledError) {
                            fail(job->errorText());
                            return;
                        }

                        const Package package = job->package();
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

            for (const auto &info : std::as_const(appletList)) {
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
                    setAppletArgs(applet, mimetype, url);
                });
            }
            {
                QAction *action = new QAction(i18nc("Add icon widget", "Add Icon"), m_dropMenu);
                m_dropMenu->addAction(action);
                action->setData(QStringLiteral("org.kde.plasma.icon"));
                const QUrl url = tjob->url();
                connect(action, &QAction::triggered, this, [this, action, mimetype, url]() {
                    Plasma::Applet *applet = createApplet(action->data().toString(), QVariantList(), QRect(m_dropMenu->dropPoint(), QSize(-1, -1)));
                    setAppletArgs(applet, mimetype, url);
                });
            }

            QHash<QAction *, QString> actionsToWallpapers;
            if (!wallpaperList.isEmpty()) {
                QAction *action = new QAction(i18n("Wallpaper"), m_dropMenu);
                action->setSeparator(true);
                m_dropMenu->addAction(action);

                QMap<QString, KPluginMetaData> sorted;
                for (const auto &info : std::as_const(appletList)) {
                    sorted.insert(info.name(), info);
                }

                for (const KPluginMetaData &info : std::as_const(wallpaperList)) {
                    const QString actionText = i18nc("Set wallpaper", "Set %1", info.name());
                    QAction *action = new QAction(actionText, m_dropMenu);
                    if (!info.iconName().isEmpty()) {
                        action->setIcon(QIcon::fromTheme(info.iconName()));
                    }
                    m_dropMenu->addAction(action);
                    actionsToWallpapers.insert(action, info.pluginId());
                    const QUrl url = tjob->url();
                    connect(action, &QAction::triggered, this, [this, info, url]() {
                        // Change wallpaper plugin if it's not the current one
                        if (containment()->wallpaperPlugin() != info.pluginId()) {
                            containment()->setWallpaperPlugin(info.pluginId());
                        }

                        // set wallpapery stuff
                        if (m_wallpaperItem && url.isValid()) {
                            m_wallpaperItem->requestOpenUrl(url);
                        }
                    });
                }
            }
        } else {
            // case in which we created the menu ourselves, just the "fetching type entry, directly create the icon applet
            if (!m_dropMenu->isDropjobMenu()) {
                Plasma::Applet *applet = createApplet(QStringLiteral("org.kde.plasma.icon"), QVariantList(), QRect(m_dropMenu->dropPoint(), QSize(-1, -1)));
                setAppletArgs(applet, mimetype, tjob->url());
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
                    setAppletArgs(applet, mimetype, url);
                });
            }
        }
        clearDataForMimeJob(tjob);
    }
}

void ContainmentItem::appletAddedForward(Plasma::Applet *applet, const QRectF &geometryHint)
{
    if (!applet) {
        return;
    }
    qWarning() << "ContainmentItem::appletAddedForward" << applet << geometryHint;
    PlasmoidItem *appletGraphicObject = qobject_cast<PlasmoidItem *>(AppletQuickItem::itemForApplet(applet));
    m_plasmoidItems.append(appletGraphicObject);
    connect(appletGraphicObject, &QObject::destroyed, this, [this, appletGraphicObject]() {
        m_plasmoidItems.removeAll(appletGraphicObject);
    });

    QPointF removalPosition = appletGraphicObject->m_positionBeforeRemoval;
    QPointF position = appletGraphicObject->position();

    if (geometryHint.x() > 0 || geometryHint.y() > 0) {
        position = geometryHint.topLeft();
        if (geometryHint.width() > 0 && geometryHint.height() > 0) {
            appletGraphicObject->setSize(geometryHint.size());
        }
    } else if (removalPosition.x() > 0.0 && removalPosition.y() > 0.0) {
        position = removalPosition;
    } else if (position.isNull() && m_containment->containmentType() == Plasma::Containment::Type::Desktop) {
        // If no position was provided, and we're adding an applet to the desktop,
        // add the applet to the center. This avoids always placing new applets
        // in the top left corner, which is likely to be covered by something.
        position = QPointF{width() / 2.0 - appletGraphicObject->width() / 2.0, //
                           height() / 2.0 - appletGraphicObject->height() / 2.0};
    }

    appletGraphicObject->setX(position.x());
    appletGraphicObject->setY(position.y());
}

void ContainmentItem::appletRemovedForward(Plasma::Applet *applet)
{
    if (!AppletQuickItem::hasItemForApplet(applet)) {
        return;
    }
    PlasmoidItem *appletGraphicObject = qobject_cast<PlasmoidItem *>(AppletQuickItem::itemForApplet(applet));
    if (appletGraphicObject) {
        m_plasmoidItems.removeAll(appletGraphicObject);
        appletGraphicObject->m_positionBeforeRemoval = appletGraphicObject->mapToItem(this, QPointF());
    }
}

void ContainmentItem::loadWallpaper()
{
    if (!m_containment->isContainment()) {
        return;
    }

    if (m_containment->containmentType() != Plasma::Containment::Type::Desktop && m_containment->containmentType() != Plasma::Containment::Type::Custom) {
        if (!isLoading()) {
            applet()->updateConstraints(Plasma::Applet::UiReadyConstraint);
        }
        return;
    }

    auto *oldWallpaper = m_wallpaperItem;

    if (!m_containment->wallpaperPlugin().isEmpty()) {
        m_wallpaperItem = WallpaperItem::loadWallpaper(this);
    }

    if (m_wallpaperItem) {
        m_wallpaperItem->setZ(-1000);
        // Qml seems happier if the parent gets set in this way
        m_wallpaperItem->setProperty("parent", QVariant::fromValue(this));

        connect(m_wallpaperItem, &WallpaperItem::isLoadingChanged, this, [this]() {
            if (!isLoading()) {
                applet()->updateConstraints(Plasma::Applet::UiReadyConstraint);
            }
        });

        // set anchors
        QQmlExpression expr(qmlObject()->engine()->rootContext(), m_wallpaperItem, QStringLiteral("parent"));
        QQmlProperty prop(m_wallpaperItem, QStringLiteral("anchors.fill"));
        prop.write(expr.evaluate());

        m_containment->setProperty("wallpaperGraphicsObject", QVariant::fromValue(m_wallpaperItem));
    }
    delete oldWallpaper;

    Q_EMIT wallpaperItemChanged();
}

// PROTECTED--------------------

void ContainmentItem::mouseReleaseEvent(QMouseEvent *event)
{
    event->setAccepted(m_containment->containmentActions().contains(Plasma::ContainmentActions::eventToString(event)));
}

void ContainmentItem::mousePressEvent(QMouseEvent *event)

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
    for (QObject *appletObject : std::as_const(m_plasmoidItems)) {
        if (PlasmoidItem *ai = qobject_cast<PlasmoidItem *>(appletObject)) {
            if (ai->isVisible() && ai->contains(ai->mapFromItem(this, event->position()))) {
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

    QPoint pos = event->globalPosition().toPoint();
    if (window() && m_containment->containmentType() == Plasma::Containment::Type::Panel) {
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

void ContainmentItem::wheelEvent(QWheelEvent *event)
{
    const QString trigger = Plasma::ContainmentActions::eventToString(event);
    Plasma::ContainmentActions *plugin = m_containment->containmentActions().value(trigger);

    if (!plugin) {
        event->setAccepted(false);
        return;
    }

    if (std::abs(event->angleDelta().x()) > std::abs(event->angleDelta().y())) {
        m_wheelDelta += event->angleDelta().x();
    } else {
        m_wheelDelta += event->angleDelta().y();
    }

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

void ContainmentItem::keyPressEvent(QKeyEvent *event)
{
    PlasmoidItem::keyPressEvent(event);
    if (event->isAccepted()) {
        return;
    }

    if (event->key() == Qt::Key_Menu) {
        QPointF localPos;
        auto focusedItem = window()->activeFocusItem();
        if (focusedItem) {
            localPos = focusedItem->mapToItem(this, QPointF(0, 0));
        }

        QMouseEvent me(QEvent::MouseButtonRelease, localPos, QPointF(), Qt::RightButton, Qt::RightButton, event->modifiers());
        mousePressEvent(&me);
        event->accept();
    }
}

void ContainmentItem::addAppletActions(QMenu *desktopMenu, Plasma::Applet *applet, QEvent *event)
{
    const auto listActions = applet->contextualActions();
    for (QAction *action : listActions) {
        if (action) {
            desktopMenu->addAction(action);
        }
    }

    if (!applet->failedToLaunch()) {
        QAction *configureApplet = applet->internalAction(QStringLiteral("configure"));
        if (configureApplet && configureApplet->isEnabled()) {
            desktopMenu->addAction(configureApplet);
        }
        QAction *appletAlternatives = applet->internalAction(QStringLiteral("alternatives"));
        if (appletAlternatives && appletAlternatives->isEnabled()) {
            desktopMenu->addAction(appletAlternatives);
        }
    }

    desktopMenu->addSeparator();
    if (m_containment->containmentType() == Plasma::Containment::Type::Desktop) {
        auto action = m_containment->corona()->action(QStringLiteral("edit mode"));
        if (action) {
            desktopMenu->addAction(action);
        }
    } else {
        addContainmentActions(desktopMenu, event);
    }

    if (m_containment->immutability() == Plasma::Types::Mutable
        && (m_containment->containmentType() != Plasma::Containment::Type::Panel || m_containment->isUserConfiguring())) {
        QAction *closeApplet = applet->internalAction(QStringLiteral("remove"));
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

void ContainmentItem::addContainmentActions(QMenu *desktopMenu, QEvent *event)
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
        cfg = KConfigGroup(&cfg, QString::number((int)m_containment->containmentType()));
        KConfigGroup pluginConfig = KConfigGroup(&cfg, trigger);
        plugin->restore(pluginConfig);
    }

    QList<QAction *> actions = plugin->contextualActions();

    if (actions.isEmpty()) {
        // it probably didn't bother implementing the function. give the user a chance to set
        // a better plugin.  note that if the user sets no-plugin this won't happen...
        /* clang-format off */
        if ((m_containment->containmentType() != Plasma::Containment::Type::Panel
                && m_containment->containmentType() != Plasma::Containment::Type::CustomPanel)
            && m_containment->internalAction(QStringLiteral("configure"))) { /* clang-format on */
            desktopMenu->addAction(m_containment->internalAction(QStringLiteral("configure")));
        }
    } else {
        desktopMenu->addActions(actions);
    }

    return;
}

bool ContainmentItem::isLoading() const
{
    return false;
    return m_wallpaperItem && m_wallpaperItem->isLoading();
}

void ContainmentItem::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == QQuickItem::ItemSceneChange) {
        // we have a window: create the representations if needed
        if (value.window && !m_containment->wallpaperPlugin().isEmpty()) {
            loadWallpaper();
        } else if (m_wallpaperItem) {
            deleteWallpaperItem();
            Q_EMIT wallpaperItemChanged();
        }
    }

    PlasmoidItem::itemChange(change, value);
}

void ContainmentItem::deleteWallpaperItem()
{
    m_containment->setProperty("wallpaperGraphicsObject", QVariant());
    m_wallpaperItem->deleteLater();
    m_wallpaperItem = nullptr;
}

#include "moc_containmentitem.cpp"
