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

#include "appletbrowserwindow.h"
#include <KAction>
#include <KStandardAction>

AppletBrowserWindow::AppletBrowserWindow(Plasma::Corona * corona, QWidget * parent, Qt::WindowFlags f) :
    QDialog(parent, f), m_corona(corona), m_containment(NULL), m_itemModel(this), m_filterModel(this)
{
    init();
}

AppletBrowserWindow::AppletBrowserWindow(Plasma::Containment * containment, QWidget * parent, Qt::WindowFlags f) :
    QDialog(parent, f), m_corona(NULL), m_containment(containment), m_itemModel(this), m_filterModel(this)
{
    init();
}

void AppletBrowserWindow::init() {
    setupUi(this);

    connect(buttonAdd, SIGNAL(clicked()), this, SLOT(buttonAddClicked()));

    QAction* quit = KStandardAction::quit(qApp, SLOT(quit()), this);
    this->addAction(quit);

    // Emblems
    appletList->addEmblem(i18n("Recommended by KDE"), new KIcon("about-kde"), 
            KCategorizedItemsViewModels::Filter("recommended", true));
    appletList->addEmblem(i18n("Used in past"), new KIcon("history"), 
            KCategorizedItemsViewModels::Filter("used", true));

    // Filters: Special
    m_filterModel.addFilter(i18n("All applets"),
        KCategorizedItemsViewModels::Filter(), new KIcon("application-x-plasma"));
    m_filterModel.addFilter(i18n("Recommended by KDE"),
        KCategorizedItemsViewModels::Filter("recommended", true), new KIcon("about-kde"));
    m_filterModel.addFilter(i18n("Favorites"),
        KCategorizedItemsViewModels::Filter("favorite", true), new KIcon("bookmark"));
    m_filterModel.addFilter(i18n("Used in past"),
        KCategorizedItemsViewModels::Filter("used", true), new KIcon("history"));

    m_filterModel.addSeparator(i18n("Categories:"));

    // Filters: Categories
    foreach (const QString& category, Plasma::Applet::knownCategories()) {
        m_filterModel.addFilter(category, 
            KCategorizedItemsViewModels::Filter("category", category));
    }

    appletList->setFilterModel(& m_filterModel);

    // Other models

    appletList->setItemModel(& m_itemModel);

}

AppletBrowserWindow::~AppletBrowserWindow()
{
    /*delete m_itemModel;
    delete m_filterModel;
    delete m_proxyModel;*/
}

void AppletBrowserWindow::buttonAddClicked() {
    kDebug() << "Button ADD clicked\n";

    foreach (AbstractItem * item, appletList->selectedItems()) {
        PlasmaAppletItem * selectedItem = (PlasmaAppletItem *) item;
        kDebug() << "Adding applet " << selectedItem->name();
        if (m_corona) {
            kDebug() << " to corona\n";
            m_corona->addApplet(selectedItem->pluginName());
        } else if (m_containment) {
            kDebug() << " to conatainment\n";
            m_containment->addApplet(selectedItem->pluginName());
        }

    }
}
#include "appletbrowserwindow.moc"
