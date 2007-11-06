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

    void initFilters();

    QString application;
    Plasma::Corona *corona;
    Plasma::Containment *containment;
    KCategorizedItemsView *appletList;

    KConfig config;
    KConfigGroup configGroup;

    PlasmaAppletItemModel itemModel;
    KCategorizedItemsViewModels::DefaultFilterModel filterModel;
};

void AppletBrowser::Private::initFilters()
{
    filterModel.clear();

    filterModel.addFilter(i18n("All Widgets"),
                          KCategorizedItemsViewModels::Filter(), new KIcon("plasmagik"));

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
                          new KIcon("bookmark"));
    filterModel.addFilter(i18n("Widgets I Have Used Before"),
                          KCategorizedItemsViewModels::Filter("used", true),
                          new KIcon("history"));

    filterModel.addSeparator(i18n("Categories:"));

    foreach (const QString& category, Plasma::Applet::knownCategories(application)) {
        filterModel.addFilter(category,
                              KCategorizedItemsViewModels::Filter("category", category));
    }
}


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
    d->appletList = new KCategorizedItemsView(this); //TODO: focus the lineedit on startup
    connect(d->appletList, SIGNAL(activated(const QModelIndex &)), this, SLOT(addApplet()));
    setMainWidget(d->appletList);

    setWindowTitle(i18n("Widgets"));

    setButtons(KDialog::Apply | KDialog::Close | KDialog::User1);
    setButtonText(KDialog::Apply, i18n("Add Widget"));
    setButtonText(KDialog::User1, i18n("Get New Widgets")); //TODO: not overly happy with this text
    enableButton(KDialog::User1, false); //TODO: enable when GHNS integration is implemented

    connect(this, SIGNAL(applyClicked()), this, SLOT(addApplet()));
    connect(this, SIGNAL(user1Clicked()), this, SLOT(downloadApplets()));

    QAction* quit = KStandardAction::quit(qApp, SLOT(quit()), this);
    addAction(quit);

    // Other Emblems
    d->appletList->addEmblem(i18n("Widgets I Have Used Before"), new KIcon("history"), 
                                KCategorizedItemsViewModels::Filter("used", true));

    d->initFilters();
    d->appletList->setFilterModel(&d->filterModel);

    // Other models
    d->appletList->setItemModel(&d->itemModel);
}

AppletBrowser::~AppletBrowser()
{
  delete d;
}

void AppletBrowser::setApplication(const QString& app)
{
    d->application = app;
    d->initFilters();
    d->itemModel.setApplication(app);

    //FIXME: AFAIK this shouldn't be necessary ... but here it is. need to find out what in that
    //       maze of models and views is screwing up
    d->appletList->setItemModel(&d->itemModel);
}

QString AppletBrowser::Application()
{
    return d->application;
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
