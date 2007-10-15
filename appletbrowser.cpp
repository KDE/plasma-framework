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
#include "plasma/appletbrowser/plasmaappletitemmodel_p.h"
#include "plasma/appletbrowser/kcategorizeditemsview_p.h"

namespace Plasma
{

class AppletBrowser::Private
{
public:
    Private(Corona* co, Containment* cont, AppletBrowser* q)
        : corona(co),
          containment(cont),
          appletList(0),
          config("plasmarc"),
          configGroup(&config, "Applet Browser"),
          itemModel(configGroup, q),
          filterModel(q)
    {
    }

    Plasma::Corona *corona;
    Plasma::Containment *containment;
    KCategorizedItemsView *appletList;
    
    KConfig config;
    KConfigGroup configGroup;

    PlasmaAppletItemModel itemModel;
    KCategorizedItemsViewModels::DefaultFilterModel filterModel;
    

};

AppletBrowser::AppletBrowser(Plasma::Corona * corona, QWidget * parent, Qt::WindowFlags f)
    : KDialog(parent, f),
      d(new Private(corona, 0, this))
{
    init();
}

AppletBrowser::AppletBrowser(Plasma::Containment * containment, QWidget * parent, Qt::WindowFlags f)
    : KDialog(parent, f),
      d(new Private(0, containment, this))
{
    init();
}

void AppletBrowser::init()
{
    d->appletList = new KCategorizedItemsView(this);
    setMainWidget(d->appletList);

    setWindowTitle("Add Applets");

    setButtons(KDialog::Apply | KDialog::Close | KDialog::User1);
    setButtonText(KDialog::Apply, i18n("Add Applet"));
    setButtonText(KDialog::User1, i18n("Get New Applets")); //TODO: not overly happy with this text
    enableButton(KDialog::User1, false); //TODO: enable when GHNS integration is implemented

    connect(this, SIGNAL(applyClicked()), this, SLOT(addApplet()));
    connect(this, SIGNAL(user1Clicked()), this, SLOT(downloadApplets()));

    QAction* quit = KStandardAction::quit(qApp, SLOT(quit()), this);
    addAction(quit);

    d->filterModel.addFilter(i18n("All Applets"),
        KCategorizedItemsViewModels::Filter(), new KIcon("application-x-plasma"));
    
    // Recommended emblems and filters
    QRegExp rx("recommended[.]([0-9A-Za-z]+)[.]caption");
    QMapIterator<QString, QString> i(d->configGroup.entryMap());
    while (i.hasNext()) {
        i.next();
        if (!rx.exactMatch(i.key())) continue;
        kDebug() << "These are the key/vals in rc file " << rx.cap(1) << "\n";
        
        QString id = rx.cap(1);
        QString caption = d->configGroup.readEntry("recommended." + id + ".caption");
        QString icon    = d->configGroup.readEntry("recommended." + id + ".icon");
        QString plugins = d->configGroup.readEntry("recommended." + id + ".plugins");
        
        d->appletList->addEmblem(i18n("Recommended by %1", caption), new KIcon(icon), 
            KCategorizedItemsViewModels::Filter("recommended." + id, true));
        d->filterModel.addFilter(i18n("Recommended by %1", caption),
            KCategorizedItemsViewModels::Filter("recommended." + id, true), new KIcon(icon));
        
        //foreach (QString plugin, plugins.split(",")) {}
    }

    // Other Emblems
    d->appletList->addEmblem(i18n("Used in Past"), new KIcon("history"), 
                                KCategorizedItemsViewModels::Filter("used", true));

    // Filters: Special
    d->filterModel.addFilter(i18n("Favorites"),
        KCategorizedItemsViewModels::Filter("favorite", true), new KIcon("bookmark"));
    d->filterModel.addFilter(i18n("Used in past"),
        KCategorizedItemsViewModels::Filter("used", true), new KIcon("history"));

    d->filterModel.addSeparator(i18n("Categories:"));

    // Filters: Categories
    foreach (const QString& category, Plasma::Applet::knownCategories()) {
        d->filterModel.addFilter(category, 
            KCategorizedItemsViewModels::Filter("category", category));
    }

    d->appletList->setFilterModel(&d->filterModel);

    // Other models
    d->appletList->setItemModel(&d->itemModel);
}

AppletBrowser::~AppletBrowser()
{
}

void AppletBrowser::addApplet()
{
    kDebug() << "Button ADD clicked";

    foreach (AbstractItem *item, d->appletList->selectedItems()) {
        PlasmaAppletItem *selectedItem = (PlasmaAppletItem *) item;
        kDebug() << "Adding applet " << selectedItem->name();
        if (d->corona) {
            kDebug() << " to corona\n";
            d->corona->addApplet(selectedItem->pluginName(),
                    selectedItem->arguments());
        } else if (d->containment) {
            kDebug() << " to containment\n";
            d->containment->addApplet(selectedItem->pluginName(),
                    selectedItem->arguments());
        }

    }
}

void AppletBrowser::downloadApplets()
{
    //TODO: implement
    kDebug() << "GHNS button clicked";
}

} // namespace Plasma

#include "appletbrowser.moc"
