/*
 *   Copyright (C) 2013 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

// #include "pointer.h"

#include <QDebug>

#include <QListView>
#include <QApplication>

#include "inputdevicemodel.h"

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    // XLib opening connection
    // backends::xlib::Connection connection;

    // // Listing screens
    // auto screen_count = XScreenCount(connection.display());

    // std::cerr << "Display has screens:" << screen_count << std::endl;

    // for (int i = 0; i < screen_count; i++) {
    //     auto height = XDisplayHeight(connection.display(), i);
    //     auto width  = XDisplayWidth(connection.display(), i);

    //     std::cerr << "size is " << width << "x" << height << std::endl;
    // }

    // std::cerr << "out" << std::endl;

    // backends::xlib::Input input;
    // input.test();

    // application.installNativeEventFilter(new MyXcbEventFilter());

    // solidx::backends::xlib::InputDeviceBackend xlibbackend;

    auto model = new solidx::InputDeviceModel();
    auto list  = new QListView();

    list->setModel(model);
    list->show();

    return application.exec();
}


