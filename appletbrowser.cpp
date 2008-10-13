/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "plasma/appletbrowser.h"

#include <QVBoxLayout>
#include <QLabel>

#include <KAction>
#include <KConfig>
#include <KConfigGroup>
#include <KMenu>
#include <KPageWidgetItem>
#include <KPushButton>
#include <KServiceTypeTrader>
#include <KStandardAction>

#include "plasma/applet.h"
#include "plasma/corona.h"
#include "plasma/containment.h"
#include "plasma/appletbrowser/kcategorizeditemsview_p.h"
#include "plasma/appletbrowser/plasmaappletitemmodel_p.h"
#include "plasma/appletbrowser/openwidgetassistant_p.h"
#include "plasma/private/packages_p.h"

namespace Plasma
{

class AppletBrowserWidgetPrivate
{
public:
    AppletBrowserWidgetPrivate(AppletBrowserWidget *w)
        : q(w),
          containment(0),
          appletList(0),
          config("plasmarc"),
          configGroup(&config, "Applet Browser"),
          itemModel(configGroup, w),
          filterModel(w)
    {
    }

    void initFilters();
    void init();
    void initRunningApplets();
    void containmentDestroyed();

    /**
     * Tracks a new running applet
     */
    void appletAdded(Plasma::Applet *applet);

    /**
     * A running applet is no more
     */
    void appletRemoved(Plasma::Applet *applet);

    AppletBrowserWidget *q;
    QString application;
    Plasma::Containment *containment;
    KCategorizedItemsView *appletList;
    QHash<QString, int> runningApplets; // applet name => count
    //extra hash so we can look up the names of deleted applets
    QHash<Plasma::Applet *,QString> appletNames;

    KConfig config;
    KConfigGroup configGroup;

    PlasmaAppletItemModel itemModel;
    KCategorizedItemsViewModels::DefaultFilterModel filterModel;
};

void AppletBrowserWidgetPrivate::initFilters()
{
    filterModel.clear();

    filterModel.addFilter(i18n("All Widgets"),
                          KCategorizedItemsViewModels::Filter(), KIcon("plasma"));

    // Recommended emblems and filters
    QRegExp rx("recommended[.]([0-9A-Za-z]+)[.]caption");
    QMapIterator<QString, QString> i(configGroup.entryMap());
    while (i.hasNext()) {
        i.next();
        if (!rx.exactMatch(i.key())) {
            continue;
        }
        //kDebug() << "These are the key/vals in rc file " << rx.cap(1) << "\n";

        QString id = rx.cap(1);
        QString caption = configGroup.readEntry("recommended." + id + ".caption");
        QString icon    = configGroup.readEntry("recommended." + id + ".icon");
        QString plugins = configGroup.readEntry("recommended." + id + ".plugins");

        appletList->addEmblem(i18n("Recommended by %1", caption), KIcon(icon),
                              KCategorizedItemsViewModels::Filter("recommended." + id, true));
        filterModel.addFilter(i18n("Recommended by %1", caption),
                              KCategorizedItemsViewModels::Filter("recommended." + id, true),
                              KIcon(icon));
    }

    // Filters: Special
    filterModel.addFilter(i18n("My Favorite Widgets"),
                          KCategorizedItemsViewModels::Filter("favorite", true),
                          KIcon("bookmarks"));
    filterModel.addFilter(i18n("Widgets I Have Used Before"),
                          KCategorizedItemsViewModels::Filter("used", true),
                          KIcon("view-history"));
    filterModel.addFilter(i18n("Currently Running Widgets"),
                          KCategorizedItemsViewModels::Filter("running", true),
                          KIcon("view-history"));

    filterModel.addSeparator(i18n("Categories:"));

    foreach (const QString &category, Plasma::Applet::listCategories(application)) {
        filterModel.addFilter(category,
                              KCategorizedItemsViewModels::Filter("category", category));
    }
}

AppletBrowserWidget::AppletBrowserWidget(QWidget * parent, Qt::WindowFlags f)
    : QWidget(parent, f),
    d(new AppletBrowserWidgetPrivate(this))
{
    d->init();
}

AppletBrowserWidget::~AppletBrowserWidget()
{
    delete d;
}

void AppletBrowserWidgetPrivate::init()
{
    QVBoxLayout *layout = new QVBoxLayout(q);

    appletList = new KCategorizedItemsView(q);
    QObject::connect(appletList, SIGNAL(doubleClicked(const QModelIndex &)), q, SLOT(addApplet()));
    layout->addWidget(appletList);

    // Other Emblems
    appletList->addEmblem(i18n("Widgets I Have Used Before"), KIcon("view-history"),
                          KCategorizedItemsViewModels::Filter("used", true));

    initFilters();
    appletList->setFilterModel(&filterModel);

    // Other models
    appletList->setItemModel(&itemModel);
    initRunningApplets();

    q->setLayout(layout);
}

void AppletBrowserWidgetPrivate::initRunningApplets()
{
//get applets from corona, count them, send results to model
    if (!containment) {
        return;
    }

    //kDebug() << runningApplets.count();
    Plasma::Corona *c = containment->corona();

    //we've tried our best to get a corona
    //we don't want just one containment, we want them all
    if (!c) {
        kDebug() << "can't happen";
        return;
    }

    appletNames.clear();
    runningApplets.clear();
    QList<Containment*> containments = c->containments();
    foreach (Containment *containment, containments) {
        QObject::connect(containment, SIGNAL(appletAdded(Plasma::Applet*,QPointF)), q, SLOT(appletAdded(Plasma::Applet*)));
        QObject::connect(containment, SIGNAL(appletRemoved(Plasma::Applet*)), q, SLOT(appletRemoved(Plasma::Applet*)));

        foreach (Applet *applet, containment->applets()) {
            runningApplets[applet->name()]++;
        }
    }

    //kDebug() << runningApplets;
    itemModel.setRunningApplets(runningApplets);
}

void AppletBrowserWidget::setApplication(const QString &app)
{
    d->application = app;
    d->initFilters();
    d->itemModel.setApplication(app);

    //FIXME: AFAIK this shouldn't be necessary ... but here it is. need to find out what in that
    //       maze of models and views is screwing up
    d->appletList->setItemModel(&d->itemModel);

    //kDebug() << d->runningApplets;
    d->itemModel.setRunningApplets(d->runningApplets);
}

QString AppletBrowserWidget::application()
{
    return d->application;
}

void AppletBrowserWidget::setContainment(Plasma::Containment *containment)
{
    if (d->containment != containment) {
        if (d->containment) {
            d->containment->disconnect(this);
        }

        d->containment = containment;

        if (d->containment) {
            connect(d->containment, SIGNAL(destroyed(QObject*)), this, SLOT(containmentDestroyed()));
        }

        d->initRunningApplets();
    }
}

Containment *AppletBrowserWidget::containment() const
{
    return d->containment;
}

void AppletBrowserWidgetPrivate::containmentDestroyed()
{
    containment = 0;
}

void AppletBrowserWidget::addApplet()
{
    if (!d->containment) {
        return;
    }

    foreach (AbstractItem *item, d->appletList->selectedItems()) {
        PlasmaAppletItem *selectedItem = (PlasmaAppletItem *) item;
        //kDebug() << "Adding applet " << selectedItem->name() << "to containment";
        d->containment->addApplet(selectedItem->pluginName(), selectedItem->arguments());
    }
}

void AppletBrowserWidgetPrivate::appletAdded(Plasma::Applet *applet)
{
    QString name = applet->name();
    //kDebug() << name;

    runningApplets[name]++;
    appletNames.insert(applet, name);
    itemModel.setRunningApplets(name, runningApplets[name]);
}

void AppletBrowserWidgetPrivate::appletRemoved(Plasma::Applet *applet)
{
    //kDebug() << (QObject*)applet;
    Plasma::Applet *a = (Plasma::Applet *)applet; //don't care if it's valid, just need the address

    QString name = appletNames.take(a);

    int count = 0;
    if (runningApplets.contains(name)) {
        count = runningApplets[name] - 1;

        if (count < 1) {
            runningApplets.remove(name);
        } else {
            runningApplets[name] = count;
        }
    }

    itemModel.setRunningApplets(name, count);
}

void AppletBrowserWidget::destroyApplets(const QString &name)
{
    if (!d->containment) {
        return;
    }

    Plasma::Corona *c = d->containment->corona();

    //we've tried our best to get a corona
    //we don't want just one containment, we want them all
    if (!c) {
        kDebug() << "can't happen";
        return;
    }

    foreach (Containment *containment, c->containments()) {
        QList<Applet*> applets = containment->applets();
        foreach (Applet *applet, applets) {
            if (applet->name() == name) {
                d->appletNames.remove(applet);
                applet->disconnect(this);
                applet->destroy();
            }
        }
    }

    d->runningApplets.remove(name);
    d->itemModel.setRunningApplets(name, 0);
}

void AppletBrowserWidget::downloadWidgets(const QString &type)
{
    kDebug() << type;
    PackageStructure *installer = 0;

    if (!type.isEmpty()) {
        QString constraint = QString("'%1' == [X-KDE-PluginInfo-Name]").arg(type);
        KService::List offers = KServiceTypeTrader::self()->query("Plasma/PackageStructure",
                                                                  constraint);

        if (offers.isEmpty()) {
            kDebug() << "could not find requested PackageStructure plugin" << type;
        } else {
            KService::Ptr service = offers.first();
            QString error;
            installer = service->createInstance<Plasma::PackageStructure>(topLevelWidget(),
                                                                          QVariantList(), &error);

            if (installer) {
                connect(installer, SIGNAL(newWidgetBrowserFinished()),
                        installer, SLOT(deleteLater()));
            } else {
                kDebug() << "found, but could not load requested PackageStructure plugin" << type
                         << "; reported error was" << error;
            }
        }
    }

    if (!installer) {
        // we don't need to delete the default Applet::packageStructure as that
        // belongs to the applet
        installer = new PlasmoidPackage();
    }

    installer->createNewWidgetBrowser(this);
}

void AppletBrowserWidget::openWidgetFile()
{
    // TODO: if we already have one of these showing and the user clicks to
    // add it again, show the same window?
    OpenWidgetAssistant *assistant = new OpenWidgetAssistant(topLevelWidget());
    assistant->setAttribute(Qt::WA_DeleteOnClose, true);
    assistant->show();
}

class AppletBrowserPrivate
{
public:
    void init(AppletBrowser *browser);
    void populateWidgetsMenu();

    AppletBrowser *q;
    AppletBrowserWidget *widget;
    QMenu *widgetsMenu;
};

void AppletBrowserPrivate::populateWidgetsMenu()
{
    if (!widgetsMenu->actions().isEmpty()) {
        // already populated.
        return;
    }

    QSignalMapper *mapper = new QSignalMapper(q);
    QObject::connect(mapper, SIGNAL(mapped(QString)), widget, SLOT(downloadWidgets(QString)));

    QAction *action = new QAction(KIcon("applications-internet"),
                                  i18n("Download New Plasma Widgets"), q);
    QObject::connect(action, SIGNAL(triggered(bool)), mapper, SLOT(map()));
    mapper->setMapping(action, QString());
    widgetsMenu->addAction(action);

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/PackageStructure");
    foreach (const KService::Ptr service, offers) {
        if (service->property("X-Plasma-ProvidesWidgetBrowser").toBool()) {
            QAction *action = new QAction(KIcon("applications-internet"),
                                          i18n("Download New %1 Widgets", service->name()), q);
            QObject::connect(action, SIGNAL(triggered(bool)), mapper, SLOT(map()));
            mapper->setMapping(action, service->property("X-KDE-PluginInfo-Name").toString());
        }
    }

    widgetsMenu->addSeparator();

    action = new QAction(KIcon("package-x-generic"),
                         i18n("Install A Widget From A Local File..."), q);
    QObject::connect(action, SIGNAL(triggered(bool)), widget, SLOT(openWidgetFile()));
    widgetsMenu->addAction(action);
}

void AppletBrowserPrivate::init(AppletBrowser *browser)
{
    q = browser;
    widget = new AppletBrowserWidget(q);

    q->setMainWidget(widget);
    q->setWindowTitle(i18n("Widgets"));

    q->setButtons(KDialog::Apply | KDialog::Close | KDialog::User1);
    q->setButtonText(KDialog::Apply, i18n("Add Widget"));
    q->setButtonText(KDialog::User1, i18n("Install New Widgets"));

    widgetsMenu = new KMenu(i18n("Get New Widgets"), q);
    QObject::connect(widgetsMenu, SIGNAL(aboutToShow()), q, SLOT(populateWidgetsMenu()));
    q->button(KDialog::User1)->setMenu(widgetsMenu);

    q->setButtonToolTip(KDialog::Close, i18n("Close the dialog"));
    q->setButtonWhatsThis(KDialog::Close, i18n("<qt>When clicking <b>Close</b>, this dialog will be closed with no further action taken.</qt>"));
    q->setButtonToolTip(KDialog::Apply, i18n("Add selected widgets"));
    q->setButtonWhatsThis(KDialog::Apply, i18n("<qt>When clicking <b>Add Widget</b>, the selected widgets will be added to your desktop.</qt>"));
    q->setButtonToolTip(KDialog::User1, i18n("Install new widgets"));
    q->setButtonWhatsThis(KDialog::User1, i18n("<qt>Selecting <b>Get New Widgets</b> will show a window that allows you to download new widgets directly from the Internet, while Install From File allows you to add new widgets from files you have on disk.</qt>"));

    QObject::connect(q, SIGNAL(applyClicked()), widget, SLOT(addApplet()));

    q->setInitialSize(QSize(400, 600));
    KConfigGroup cg(KGlobal::config(), "PlasmaAppletBrowserDialog");
    q->restoreDialogSize(cg);
}

AppletBrowser::AppletBrowser(QWidget * parent, Qt::WindowFlags f)
    : KDialog(parent, f),
      d(new AppletBrowserPrivate)
{
    d->init(this);
}

AppletBrowser::~AppletBrowser()
{
    KConfigGroup cg(KGlobal::config(), "PlasmaAppletBrowserDialog");
    saveDialogSize(cg);
}

void AppletBrowser::setApplication(const QString &app)
{
    d->widget->setApplication(app);
}

QString AppletBrowser::application()
{
    return d->widget->application();
}

void AppletBrowser::setContainment(Plasma::Containment *containment)
{
    d->widget->setContainment(containment);
}

Containment *AppletBrowser::containment() const
{
    return d->widget->containment();
}

} // namespace Plasma

#include "appletbrowser.moc"
