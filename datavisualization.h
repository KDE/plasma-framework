/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
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

#ifndef PLASMA_DATAVISUALIZATION_H
#define PLASMA_DATAVISUALIZATION_H

#include <QObject>

#include "dataengine.h"

namespace Plasma
{

// this will end up being multiple-inherited?
class DataVisualization : public QObject
{
    Q_OBJECT

    public:
        DataVisualization(QObject* parent = 0);
        virtual ~DataVisualization();

    public slots:
        virtual void data(const DataSource::Data&) = 0;

    private:
        class Private;
        Private* d;
};

} // Plasma namespace

#endif // multiple inclusion guard
