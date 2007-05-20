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

#ifndef PLASMA_DATAENGINE_H
#define PLASMA_DATAENGINE_H

#include <QAtomic>
#include <QHash>
#include <QObject>
#include <QStringList>

#include <plasma_export.h>

namespace Plasma
{

class DataVisualization;

class PLASMA_EXPORT DataEngine : public QObject
{
    Q_OBJECT

    public:
        typedef QHash<QString, DataEngine*> Dict;
        typedef QHash<QString, QVariant> Data;

        DataEngine(QObject* parent);
        virtual ~DataEngine();

        virtual QStringList dataSources();
        void connectSource(const QString& source, DataVisualization* visualization);
        Data query(const QString& source);

        void ref();
        void deref();
        bool isUsed();

    protected:
        virtual void init();
        void setData(const QString& source, const QVariant& value);
        void setData(const QString& source, const QString& key, const QVariant& value);
/*        void createDataSource(const QString& source,
                              const QString& domain = QString());*/
        void removeDataSource(const QString& source);
        void clearAllDataSources();

    protected slots:
        void checkForUpdates();

    private:
        class Private;
        Private* const d;
};

} // Plasma namespace

#define K_EXPORT_PLASMA_DATAENGINE(libname, classname) \
        K_EXPORT_COMPONENT_FACTORY(                    \
                        plasma_##libname##_engine,     \
                        KGenericFactory<classname>("plasma_" #libname "_engine"))

#endif // multiple inclusion guard

