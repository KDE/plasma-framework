/*
 *   Copyright 2009 by Alan Alpert <alan.alpert@nokia.com>
 *   Copyright 2010 by Ménard Alexis <menard@kde.org>
 *   Copyright 2010 by Marco Martin <mart@kde.org>

 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "qtextracomponentsplugin.h"

#include <QtDeclarative/qdeclarative.h>

#include "qpixmapitem.h"
#include "qimageitem.h"
#include "qiconitem.h"
#include "mouseeventlistener.h"


void QtExtraComponentsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.qtextracomponents"));

    qmlRegisterType<QPixmapItem>(uri, 0, 1, "QPixmapItem");
    qmlRegisterType<QImageItem>(uri, 0, 1, "QImageItem");
    qmlRegisterType<QIconItem>(uri, 0, 1, "QIconItem");
    qmlRegisterType<MouseEventListener>(uri, 0, 1, "MouseEventListener");
}


#include "qtextracomponentsplugin.moc"

