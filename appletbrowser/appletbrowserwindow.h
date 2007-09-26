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

#ifndef APPLETBROWSERWINDOW_H_
#define APPLETBROWSERWINDOW_H_

#include <QtGui>
#include <QtCore>
#include <iostream>
#include "ui_appletbrowserwindowbase.h"

#include "plasmaappletitemmodel.h"
#include "kcategorizeditemsview.h"

#include "../corona.h"
#include "../containment.h"

class AppletBrowserWindow: public QDialog, public Ui::AppletBrowserWindowBase
{
    //Q_OBJECT
public:
    explicit AppletBrowserWindow(Plasma::Corona * corona, QWidget * parent = 0, Qt::WindowFlags f = 0);
    explicit AppletBrowserWindow(Plasma::Containment * containment, QWidget * parent = 0, Qt::WindowFlags f = 0);
    virtual ~AppletBrowserWindow();

private:
    void init();
    Plasma::Corona * m_corona;
    Plasma::Containment * m_containment;
    PlasmaAppletItemModel m_itemModel;
    KCategorizedItemsViewModels::DefaultFilterModel m_filterModel;
};

#endif /*APPLETBROWSERWINDOW_H_*/
