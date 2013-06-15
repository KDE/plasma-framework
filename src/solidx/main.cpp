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

#include <xcb/xcb.h>

#include <iostream>
#include <unistd.h>

#include <X11/extensions/XInput.h>

int main(int argc, char *argv[])
{
    int screenNum = -1;
    xcb_connection_t * connection = xcb_connect(NULL, &screenNum);

    if (!connection) {
        std::cerr << "Error: Can not open the display" << std::endl;
    }

    auto setup = xcb_get_setup(connection);



    auto screen = xcb_setup_roots_iterator(xcb_get_setup(connection));

    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t values[] = { screen.data->white_pixel, XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS };

    auto window = xcb_generate_id(connection);

    xcb_create_window(
            connection, XCB_COPY_FROM_PARENT, window, screen.data->root,
            10, 10, 100, 100, 1,
            XCB_WINDOW_CLASS_INPUT_OUTPUT,
            screen.data->root_visual,
            mask, values);

    std::cerr << "Successfully connected to the X server" << std::endl;
    std::cerr << "connection:        " << connection << std::endl;
    std::cerr << "number of screens: " << screenNum << std::endl;
    std::cerr << "number of screens: " << (int)setup->roots_len << std::endl;

    //xcb_map_window(connection, window);
    //xcb_flush(connection);

    for (int i = 0; i < setup->roots_len; i++) {
        xcb_screen_next(&screen);
        auto data = screen.data;

        std::cerr << "Informations of screen: " << data->root << std::endl;
        std::cerr << "  width.........: " << data->width_in_pixels << std::endl;
        std::cerr << "  height........: " << data->height_in_pixels << std::endl;
        std::cerr << "  white pixel...: " << data->white_pixel << std::endl;
        std::cerr << "  black pixel...: " << data->black_pixel << std::endl;
    }

    std::cerr << "-------------------------------------" << std::endl;

    xcb_disconnect(connection);

    // XLib opening connection
    int  device_count = -1;
    auto display = XOpenDisplay(nullptr);

    if (!display) return EXIT_FAILURE;

    // Listing screens
    auto screen_count = XScreenCount(display);

    std::cerr << "Display has screens:" << screen_count << std::endl;

    for (int i = 0; i < screen_count; i++) {
        auto height = XDisplayHeight(display, i);
        auto width  = XDisplayWidth(display, i);

        std::cerr << "size is " << width << "x" << height << std::endl;
    }

    // Listing input devices
    auto devices = XListInputDevices(display, &device_count);

    if (devices) for (int i = 0; i < device_count; i++) {
        auto device = devices[i];

        std::cerr
            << device.name << " "
            << device.use << " "
            << device.id << std::endl;
    }


    XFreeDeviceList(devices);
    XCloseDisplay(display);

    std::cerr << "out" << std::endl;

    return 0;
}


