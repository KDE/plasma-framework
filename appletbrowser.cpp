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

#include <knewstuff2/engine.h>

#include "plasma/applet.h"
#include "plasma/corona.h"
#include "plasma/containment.h"
#include "plasma/appletbrowser/kcategorizeditemsview_p.h"
#include "plasma/appletbrowser/plasmaappletitemmodel_p.h"
#include "plasma/appletbrowser/openwidgetassistant_p.h"

namespace Plasma
{

class AppletBrowserWidget::Private
{
public:
    Private(AppletBrowserWidget* w)
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

    /**
     * Tracks a new running applet
     */
    void appletAdded(Plasma::Applet* applet);

    /**
     * A running applet is no more
     */
    void appletDestroyed(QObject* applet);

    AppletBrowserWidget *q;
    QString application;
    Plasma::Containment *containment;
    KCategorizedItemsView *appletList;
    QHash<QString, int> runningApplets; // applet name => count
    //extra hash so we can look up the names of deleted applets
    QHash<Plasma::Applet*, QString> appletNames;

    KConfig config;
    KConfigGroup configGroup;

    PlasmaAppletItemModel itemModel;
    KCategorizedItemsViewModels::DefaultFilterModel filterModel;
};

void AppletBrowserWidget::Private::initFilters()
{
    filterModel.clear();

    filterModel.addFilter(i18n("All Widgets"),
                          KCategorizedItemsViewModels::Filter(), new KIcon("plasma"));

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

        appletList->addEmblem(i18n("Recommended by %1", caption), new KIcon(icon),
                              KCategorizedItemsViewModels::Filter("recommended." + id, true));
        filterModel.addFilter(i18n("Recommended by %1", caption),
                              KCategorizedItemsViewModels::Filter("recommended." + id, true), new KIcon(icon));
    }

    // Filters: Special
    filterModel.addFilter(i18n("My Favorite Widgets"),
                          KCategorizedItemsViewModels::Filter("favorite", true),
                          new KIcon("bookmarks"));
    filterModel.addFilter(i18n("Widgets I Have Used Before"),
                          KCategorizedItemsViewModels::Filter("used", true),
                          new KIcon("view-history"));
    filterModel.addFilter(i18n("Currently Running Widgets"),
                          KCategorizedItemsViewModels::Filter("running", true),
                          new KIcon("view-history"));

    filterModel.addSeparator(i18n("Categories:"));

    foreach (const QString& category, Plasma::Applet::listCategories(application)) {
        filterModel.addFilter(category,
                              KCategorizedItemsViewModels::Filter("category", category));
    }
}

AppletBrowserWidget::AppletBrowserWidget(QWidget * parent, Qt::WindowFlags f)
    : QWidget(parent, f),
    d(new Private(this))
{
    d->init();
}

AppletBrowserWidget::~AppletBrowserWidget()
{
    delete d;
}

void AppletBrowserWidget::Private::init()
{
    QVBoxLayout *layout = new QVBoxLayout(q);

    appletList = new KCategorizedItemsView(q);
    connect(appletList, SIGNAL(activated(const QModelIndex &)), q, SLOT(addApplet()));
    layout->addWidget(appletList);

    // Other Emblems
    appletList->addEmblem(i18n("Widgets I Have Used Before"), new KIcon("view-history"),
                          KCategorizedItemsViewModels::Filter("used", true));

    initFilters();
    appletList->setFilterModel(&filterModel);

    // Other models
    appletList->setItemModel(&itemModel);
    initRunningApplets();

    q->setLayout(layout);
}

void AppletBrowserWidget::Private::initRunningApplets()
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
    foreach (Containment * containment,containments) {
        connect(containment, SIGNAL(appletAdded(Plasma::Applet*,QPointF)), q, SLOT(appletAdded(Plasma::Applet*)));
        //TODO track containments too?
        QList<Applet*>applets = containment->applets();
        foreach (Applet *applet,applets) {
            runningApplets[applet->name()]++;
            appletNames.insert(applet, applet->name());
            connect(applet, SIGNAL(destroyed(QObject*)), q, SLOT(appletDestroyed(QObject*)));
        }
    }

    //kDebug() << runningApplets;
    itemModel.setRunningApplets(runningApplets);
}

void AppletBrowserWidget::setApplication(const QString& app)
{
    d->application = app;
    d->initFilters();
    d->itemModel.setApplication(app);

    //FIXME: AFAIK this shouldn't be necessary ... but here it is. need to find out what in that
    //       maze of models and views is screwing up
    d->appletList->setItemModel(&d->itemModel);

    kDebug() << d->runningApplets;
    d->itemModel.setRunningApplets(d->runningApplets);
}

QString AppletBrowserWidget::application()
{
    return d->application;
}

void AppletBrowserWidget::setContainment(Plasma::Containment *containment)
{
    if (d->containment != containment) {
        d->containment = containment;
        d->initRunningApplets();
    }
}

Containment *AppletBrowserWidget::containment() const
{
    return d->containment;
}

void AppletBrowserWidget::addApplet()
{
    kDebug() << "Button ADD clicked";
    if (!d->containment) {
        return;
    }

    foreach (AbstractItem *item, d->appletList->selectedItems()) {
        PlasmaAppletItem *selectedItem = (PlasmaAppletItem *) item;
        kDebug() << "Adding applet " << selectedItem->name() << "to containment";
        d->containment->addApplet(selectedItem->pluginName(), selectedItem->arguments());
    }
}

void AppletBrowserWidget::Private::appletAdded(Plasma::Applet* applet)
{
    QString name = applet->name();
    //kDebug() << name;

    runningApplets[name]++;
    appletNames.insert(applet, name);
    connect(applet, SIGNAL(destroyed(QObject*)), q, SLOT(appletDestroyed(QObject*)));
    itemModel.setRunningApplets(name, runningApplets[name]);
}

void AppletBrowserWidget::Private::appletDestroyed(QObject* applet)
{
    //kDebug() << applet;
    Plasma::Applet* a = (Plasma::Applet*)applet; //don't care if it's valid, just need the address

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

void AppletBrowserWidget::downloadWidgets()
{
    KNS::Engine engine(0);
    if (engine.init("plasmoids.knsrc")) {
        KNS::Entry::List entries = engine.downloadDialogModal(this);
    }
}

void AppletBrowserWidget::openWidgetFile()
{
    // TODO: if we already have one of these showing and the user clicks to add it again, show the same window?
    OpenWidgetAssistant *assistant = new OpenWidgetAssistant(topLevelWidget());
    assistant->setAttribute(Qt::WA_DeleteOnClose, true);
    assistant->show();
}

class AppletBrowser::Private
{
public:
    void init(AppletBrowser*);
    AppletBrowserWidget *widget;
};

AppletBrowser::AppletBrowser(QWidget * parent, Qt::WindowFlags f)
    : KDialog(parent, f),
      d(new Private)
{
    d->init(this);
}

void AppletBrowser::Private::init(AppletBrowser *q)
{
    widget = new AppletBrowserWidget(q);

    q->setMainWidget(widget);
    q->setWindowTitle(i18n("Widgets"));

    q->setButtons(KDialog::Apply | KDialog::Close | KDialog::User1);
    q->setButtonText(KDialog::Apply, i18n("Add Widget"));
    q->setButtonText(KDialog::User1, i18n("Install New Widgets"));

    KMenu *widgetsMenu = new KMenu(i18n("Get New Widgets"), q);
    QAction *action = new QAction(KIcon("applications-internet"),
                                  i18n("Download From Internet"), q);
    connect(action, SIGNAL(triggered(bool)), widget, SLOT(downloadWidgets()));
    widgetsMenu->addAction(action);

    action = new QAction(KIcon("applications-internet"),
                         i18n("Install From File..."), q);
    connect(action, SIGNAL(triggered(bool)), widget, SLOT(openWidgetFile()));
    widgetsMenu->addAction(action);
    q->button(KDialog::User1)->setMenu(widgetsMenu);

    q->setButtonToolTip(KDialog::Close, i18n("Close the dialog"));
    q->setButtonWhatsThis(KDialog::Close, i18n("<qt>When clicking <b>Close</b>, this dialog will be closed with no further action taken.</qt>"));
    q->setButtonToolTip(KDialog::Apply, i18n("Add selected widgets"));
    q->setButtonWhatsThis(KDialog::Apply, i18n("<qt>When clicking <b>Add Widget</b>, the selected widgets will be added to your desktop.</qt>"));
    q->setButtonToolTip(KDialog::User1, i18n("Install new widgets"));
    q->setButtonWhatsThis(KDialog::User1, i18n("<qt>Selecting <b>Get New Widgets</b> will show a window that allows you to download new widgets directly from the Internet, while Install From File allows you to add new widgets from files you have on disk.</qt>"));

    connect(q, SIGNAL(applyClicked()), widget, SLOT(addApplet()));

    q->setInitialSize(QSize(400, 600));
    KConfigGroup cg(KGlobal::config(), "PlasmaAppletBrowserDialog");
    q->restoreDialogSize(cg);
}

AppletBrowser::~AppletBrowser()
{
    KConfigGroup cg(KGlobal::config(), "PlasmaAppletBrowserDialog");
    saveDialogSize(cg);
}

void AppletBrowser::setApplication(const QString& app)
{
    d->widget->setApplication( app );
}

QString AppletBrowser::application()
{
    return d->widget->application();
}

void AppletBrowser::setContainment(Plasma::Containment *containment)
{
    d->widget->setContainment(containment);
}

Containment* AppletBrowser::containment() const
{
    return d->widget->containment();
}

} // namespace Plasma

#include "appletbrowser.moc"
