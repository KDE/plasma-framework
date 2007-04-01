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

#ifndef PLASMA_ENGINE_H
#define PLASMA_ENGINE_H

#include <QAtomic>
#include <QHash>
#include <QObject>
#include <QStringList>

#include <kdemacros.h>

namespace Plasma
{

class DataSource;
class DataVisualization;

class KDE_EXPORT DataSource : public QObject
{
    Q_OBJECT

    public:
        typedef QHash<QString, DataSource*> Dict;
        typedef QHash<QString, QVariant> Data;
        typedef QHash<QString, Dict> Grouping;

        explicit DataSource(QObject* parent = 0);
        virtual ~DataSource();

        QString name();

    signals:
        void data(const DataSource::Data&);

    private:
        class Private;
        Private* d;
};

class KDE_EXPORT DataEngine : public QObject
{
    Q_OBJECT

    public:
        typedef QHash<QString, DataEngine*> Dict;

        DataEngine(QObject* parent);
        virtual ~DataEngine();

        virtual QStringList dataSources();
        void connect(const QString& source, DataVisualization* visualization);
        DataSource::Data query(const QString& source);

        void ref();
        void deref();
        bool isUsed();

    protected:
        virtual void init();
        virtual void cleanup();
        void setDataSource(const QString& source, const QVariant& value);
        void createDataSource(const QString& source,
                              const QString& domain = QString());
        void removeDataSource(const QString& source);
        void clearAllDataSources();

    private:
        QAtomic m_ref;
        class Private;
        Private* d;
};

} // Plasma namespace

#define K_EXPORT_PLASMA_ENGINE(libname, classname)                       \
        K_EXPORT_COMPONENT_FACTORY(                                      \
                        plasmaengine_##libname,                          \
                        KGenericFactory<classname>("libplasmaengine_" #libname))

#endif // multiple inclusion guard

