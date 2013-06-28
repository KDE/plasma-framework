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

#ifndef SOLIDX_XLIB_CONNECTION_H
#define SOLIDX_XLIB_CONNECTION_H

#include <QObject>

#include <memory>
#include <functional>

#include <X11/extensions/XI.h>

struct _XDisplay;
union  _XEvent;

namespace solidx {
namespace backends {
namespace xlib {

/**
 * XlibConnection
 */
class XlibConnection: public QObject {
public:
    XlibConnection();
    virtual ~XlibConnection();

    _XDisplay * display() const;

    void handleExtensionEvent(int eventType, XEventClass & eventClass, std::function<void(const _XEvent &)> handler);
    void releaseExtensionEventHandler(int eventType);

private:

    class Private;
    const std::shared_ptr<Private> d;
};


} // namespace xlib
} // namespace backends
} // namespace solidx


#endif /* SOLIDX_XLIB_CONNECTION_H */

