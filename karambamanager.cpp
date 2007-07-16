/*
 *   Copyright (C) 2007 Alexander Wiedenbruch <wirr01@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include "karambamanager.h"

#include <QFile>

#include <KLibLoader>
#include <KLibrary>
#include <KDebug>

QGraphicsItemGroup* KarambaManager::loadKaramba(const KUrl &themePath, QGraphicsScene *scene)
{
    QString karambaLib = QFile::encodeName(KLibLoader::self()->findLibrary(QLatin1String("libsuperkaramba")));

    QGraphicsItemGroup *karamba = 0;

    KLibrary *lib = KLibLoader::self()->library(karambaLib);
    if (lib) {
        startKaramba createKaramba = 0;
        createKaramba = (startKaramba)lib->resolveFunction("startKaramba");
        if (createKaramba) {
            karamba = createKaramba(themePath, scene->views()[0]);
        }
    } else {
        kWarning() << "Could not load " << karambaLib << endl;
    }

    return karamba;
}

