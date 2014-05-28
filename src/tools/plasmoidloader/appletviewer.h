/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  David Edmundson <david@davidedmundson.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef APPLETVIEWER_H
#define APPLETVIEWER_H

#include <QQuickWindow>

namespace PlasmaQuick
{
class AppletQuickItem;
}

class AppletViewer : public QQuickWindow
{
    Q_OBJECT

public:
    enum OperationMode {
        FrameCount,
        StartupBenchmark,
        ErrorCount
    };

    enum Representation {
        FullRepresentation,
        CompactRepresentation,
        PreferredRepresentation
    };

    AppletViewer(const QString &appletName, Representation mode);

private Q_SLOTS:
    void onSwapBuffers();
    void onTimeout();

private:
    PlasmaQuick::AppletQuickItem *m_appletItem;
    int m_frameCount;
    qint64 m_startTime;
    OperationMode m_mode;
};

#endif // APPLETVIEWER_H
