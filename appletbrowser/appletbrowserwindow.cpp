/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 (or,
 *   at your option, any later version) as published by the Free Software
 *   Foundation
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

#include "appletbrowserwindow_p.h"

#include <KAction>
#include <KStandardAction>


#include "plasma/corona.h"
#include "plasma/containment.h"
#include "plasmaappletitemmodel_p.h"
#include "kcategorizeditemsview_p.h"

namespace Plasma
{

class AppletBrowserWindow::Private
{
public:
    Private(Corona* co, Containment* cont, AppletBrowserWindow* q)
        : corona(co),
          containment(cont),
          appletList(0),
          itemModel(q),
          filterModel(q)
    {

    }

    Plasma::Corona *corona;
    Plasma::Containment *containment;
    KCategorizedItemsView *appletList;
    PlasmaAppletItemModel itemModel;
    KCategorizedItemsViewModels::DefaultFilterModel filterModel;
};

AppletBrowserWindow::AppletBrowserWindow(Plasma::Corona * corona, QWidget * parent, Qt::WindowFlags f)
    : KDialog(parent, f),
      d(new Private(corona, 0, this))
{
    init();
}

AppletBrowserWindow::AppletBrowserWindow(Plasma::Containment * containment, QWidget * parent, Qt::WindowFlags f)
    : KDialog(parent, f),
      d(new Private(0, containment, this))
{
    init();
}

void AppletBrowserWindow::init()
{
    d->appletList = new KCategorizedItemsView(this);
    setMainWidget(d->appletList);

    setButtons(KDialog::Apply | KDialog::Close | KDialog::User1);
    setButtonText(KDialog::Apply, i18n("Add Applet"));
    setButtonText(KDialog::User1, i18n("Get New Applets")); //TODO: not overly happy with this text
    enableButton(KDialog::User1, false); //TODO: enable when GHNS integration is implemented

    connect(this, SIGNAL(applyClicked()), this, SLOT(addApplet()));
    connect(this, SIGNAL(user1Clicked()), this, SLOT(downloadApplets()));

    QAction* quit = KStandardAction::quit(qApp, SLOT(quit()), this);
    addAction(quit);

    // Emblems
    d->appletList->addEmblem(i18n("Recommended by KDE"), new KIcon("about-kde"), 
                                KCategorizedItemsViewModels::Filter("recommended", true));
    d->appletList->addEmblem(i18n("Used in past"), new KIcon("history"), 
                                KCategorizedItemsViewModels::Filter("used", true));

    // Filters: Special
    d->filterModel.addFilter(i18n("All applets"),
        KCategorizedItemsViewModels::Filter(), new KIcon("application-x-plasma"));
    d->filterModel.addFilter(i18n("Recommended by KDE"),
        KCategorizedItemsViewModels::Filter("recommended", true), new KIcon("about-kde"));
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

AppletBrowserWindow::~AppletBrowserWindow()
{
}

void AppletBrowserWindow::addApplet()
{
    kDebug() << "Button ADD clicked";

    foreach (AbstractItem *item, d->appletList->selectedItems()) {
        PlasmaAppletItem *selectedItem = (PlasmaAppletItem *) item;
        kDebug() << "Adding applet " << selectedItem->name();
        if (d->corona) {
            kDebug() << " to corona\n";
            d->corona->addApplet(selectedItem->pluginName());
        } else if (d->containment) {
            kDebug() << " to conatainment\n";
            d->containment->addApplet(selectedItem->pluginName());
        }

    }
}

void AppletBrowserWindow::downloadApplets()
{
    //TODO: implement
    kDebug() << "GHNS button clicked";
}

} // namespace Plasma

#include "appletbrowserwindow_p.moc"
