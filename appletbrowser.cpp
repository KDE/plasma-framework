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

#include <KAction>
#include <KStandardAction>
#include <KConfig>
#include <KConfigGroup>


#include "plasma/corona.h"
#include "plasma/containment.h"
#include "plasma/applet.h"
#include "plasma/appletbrowser/plasmaappletitemmodel_p.h"
#include "plasma/appletbrowser/kcategorizeditemsview_p.h"

namespace Plasma
{

class AppletBrowserWidget::Private
{
public:
    Private(Containment* cont, AppletBrowserWidget* w)
        : containment(cont),
          appletList(0),
          config("plasmarc"),
          configGroup(&config, "Applet Browser"),
          itemModel(configGroup, w),
          filterModel(w)
    {
    }

    void initFilters();

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

    foreach (const QString& category, Plasma::Applet::knownCategories(application)) {
        filterModel.addFilter(category,
                              KCategorizedItemsViewModels::Filter("category", category));
    }
}

/*
AppletBrowserWidget::AppletBrowserWidget(Plasma::Corona * corona, bool showButtons, QWidget * parent, Qt::WindowFlags f)
    : QWidget(parent, f),
    d(new Private(0, this)),
    m_showButtons( showButtons )
{
    init();
}
*/
AppletBrowserWidget::AppletBrowserWidget(Plasma::Containment * containment, bool showButtons, QWidget * parent, Qt::WindowFlags f)
    : QWidget(parent, f),
    d(new Private(containment, this)),
    m_showButtons( showButtons )
{
    init();
}

AppletBrowserWidget::~AppletBrowserWidget()
{
    delete d;
}

void AppletBrowserWidget::init()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    d->appletList = new KCategorizedItemsView(this);
    connect(d->appletList, SIGNAL(activated(const QModelIndex &)), this, SLOT(addApplet()));
    layout->addWidget( d->appletList );

    if( m_showButtons ) {
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->setSpacing( KDialog::spacingHint() );
        buttonLayout->setMargin( KDialog::marginHint() );

        QPushButton *addButton = new QPushButton(i18n("Add Widget"), this );
        connect(addButton, SIGNAL(clicked()), this, SLOT(addApplet()));
        buttonLayout->addWidget( addButton );

        QPushButton *newButton = new QPushButton(i18n("Get New Widgets"), this ); //TODO: not overly happy with this text
        newButton->setEnabled( false ); //TODO: enable when GHNS integration is implemented
        connect(newButton, SIGNAL(clicked()), this, SLOT(downloadApplets()));
        buttonLayout->addWidget( newButton );

        layout->addItem( buttonLayout );
    }

    // Other Emblems
    d->appletList->addEmblem(i18n("Widgets I Have Used Before"), new KIcon("view-history"), 
                                KCategorizedItemsViewModels::Filter("used", true));

    d->initFilters();
    d->appletList->setFilterModel(&d->filterModel);

    // Other models
    d->appletList->setItemModel(&d->itemModel);
    initRunningApplets();
}

void AppletBrowserWidget::initRunningApplets()
{
//get applets from corona, count them, send results to model
    kDebug() << d->runningApplets.count();
    Plasma::Corona *c = d->containment->corona();

    //we've tried our best to get a corona
    //we don't want just one containment, we want them all
    if (!c) {
        kDebug() << "can't happen";
        return;
    }

    d->appletNames.clear();
    d->runningApplets.clear();
    QList<Containment*> containments = c->containments();
    foreach (Containment * containment,containments) {
        connect(containment, SIGNAL(appletAdded(Plasma::Applet*)), this, SLOT(appletAdded(Plasma::Applet*)));
        //TODO track containments too?
        QList<Applet*>applets=containment->applets();
        foreach (Applet *applet,applets) {
            d->runningApplets[applet->name()]++;
            d->appletNames.insert(applet, applet->name());
            connect(applet, SIGNAL(destroyed(QObject*)), this, SLOT(appletDestroyed(QObject*)));
        }
    }

    kDebug() << d->runningApplets;
    d->itemModel.setRunningApplets(d->runningApplets);
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
    d->containment = containment;
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

void AppletBrowserWidget::appletAdded(Plasma::Applet* applet)
{
    QString name = applet->name();
    kDebug() << name;

    d->runningApplets[name]++;
    d->appletNames.insert(applet, name);
    connect(applet, SIGNAL(destroyed(QObject*)), this, SLOT(appletDestroyed(QObject*)));
    d->itemModel.setRunningApplets(name, d->runningApplets[name]);
}

void AppletBrowserWidget::appletDestroyed(QObject* applet)
{
    kDebug() << applet;
    Plasma::Applet* a = (Plasma::Applet*)applet; //don't care if it's valid, just need the address

    QString name = d->appletNames.take(a);

    int count = 0;
    if (d->runningApplets.contains(name)) {
        count = d->runningApplets[name] - 1;

        if (count < 1) {
            d->runningApplets.remove(name);
        } else {
            d->runningApplets[name] = count;
        }
    }

    d->itemModel.setRunningApplets(name, count);
}

void AppletBrowserWidget::destroyApplets(QString name)
{
    Plasma::Corona *c = d->containment->corona();

    //we've tried our best to get a corona
    //we don't want just one containment, we want them all
    if (!c) {
        kDebug() << "can't happen";
        return;
    }

    foreach (Containment *containment, c->containments()) {
        QList<Applet*> applets = containment->applets();
        foreach (Applet *applet,applets) {
            d->appletNames.remove(applet);
            if (applet->name() == name) {
                applet->disconnect(this);
                applet->destroy();
            }
        }
    }

    d->runningApplets.remove(name);
    d->itemModel.setRunningApplets(name, 0);
}

void AppletBrowserWidget::downloadApplets()
{
    //TODO: implement
    kDebug() << "GHNS button clicked";
}











/*AppletBrowser::AppletBrowser(Plasma::Corona * corona, QWidget * parent, Qt::WindowFlags f)
    : KDialog(parent, f),
    m_widget(new AppletBrowserWidget(corona, false, this))
{
    winId(); // this is to get us a win id so that the next line doesn't abort on us
    setWindowRole("appletbrowser");
    init();
}*/

AppletBrowser::AppletBrowser(Plasma::Containment * containment, QWidget * parent, Qt::WindowFlags f)
    : KDialog(parent, f),
    m_widget(new AppletBrowserWidget(containment, false, this))
{
    init();
}

void AppletBrowser::init()
{
    setMainWidget(m_widget);

    setWindowTitle(i18n("Widgets"));

    setButtons(KDialog::Apply | KDialog::Close | KDialog::User1);
    setButtonText(KDialog::Apply, i18n("Add Widget"));
    setButtonText(KDialog::User1, i18n("Get New Widgets")); //TODO: not overly happy with this text
    enableButton(KDialog::User1, false); //TODO: enable when GHNS integration is implemented

    setButtonToolTip(KDialog::Close, i18n("Close the dialog"));
    setButtonWhatsThis(KDialog::Close, i18n("<qt>When clicking <b>Close</b>, this dialog will be closed with no further action taken.</qt>"));
    setButtonToolTip(KDialog::Apply, i18n("Add selected widgets"));
    setButtonWhatsThis(KDialog::Apply, i18n("<qt>When clicking <b>Add Widget</b>, the selected widgets will be added to your desktop.</qt>"));
    setButtonToolTip(KDialog::User1, i18n("Download new widgets"));
    setButtonWhatsThis(KDialog::User1, i18n("<qt>When clicking <b>Get New Widgets</b>, a dialog will open to allow you to download new widgets. You need to be connected to the Internet.</qt>"));

    connect(this, SIGNAL(applyClicked()), m_widget, SLOT(addApplet()));
    connect(this, SIGNAL(user1Clicked()), m_widget, SLOT(downloadApplets()));

    QAction* quit = KStandardAction::quit(qApp, SLOT(quit()), this);
    addAction(quit);
}

AppletBrowser::~AppletBrowser()
{
}

void AppletBrowser::setApplication(const QString& app)
{
    m_widget->setApplication( app );
}

QString AppletBrowser::application()
{
    return m_widget->application();
}

void AppletBrowser::setContainment(Plasma::Containment *containment)
{
    m_widget->setContainment(containment);
}

Containment* AppletBrowser::containment() const
{
    return m_widget->containment();
}

} // namespace Plasma

#include "appletbrowser.moc"
