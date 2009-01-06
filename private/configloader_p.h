/*
 *   Copyright 2007-2008 Aaron Seigo <aseigo@kde.org>
 *
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

#ifndef PLASMA_CONFIGLOADER_P_H
#define PLASMA_CONFIGLOADER_P_H

namespace Plasma
{

class ConfigLoaderPrivate
{
    public:
        ConfigLoaderPrivate()
            : saveDefaults(false)
        {
        }

        ~ConfigLoaderPrivate()
        {
            qDeleteAll(bools);
            qDeleteAll(strings);
            qDeleteAll(stringlists);
            qDeleteAll(colors);
            qDeleteAll(fonts);
            qDeleteAll(ints);
            qDeleteAll(uints);
            qDeleteAll(urls);
            qDeleteAll(dateTimes);
            qDeleteAll(doubles);
            qDeleteAll(intlists);
            qDeleteAll(longlongs);
            qDeleteAll(points);
            qDeleteAll(rects);
            qDeleteAll(sizes);
            qDeleteAll(ulonglongs);
            qDeleteAll(urllists);
        }

        bool *newBool()
        {
            bool *v = new bool;
            bools.append(v);
            return v;
        }

        QString *newString()
        {
            QString *v = new QString;
            strings.append(v);
            return v;
        }

        QStringList *newStringList()
        {
            QStringList *v = new QStringList;
            stringlists.append(v);
            return v;
        }

        QColor *newColor()
        {
            QColor *v = new QColor;
            colors.append(v);
            return v;
        }

        QFont *newFont()
        {
            QFont *v = new QFont;
            fonts.append(v);
            return v;
        }

        qint32 *newInt()
        {
            qint32 *v = new qint32;
            ints.append(v);
            return v;
        }

        quint32 *newUint()
        {
            quint32 *v = new quint32;
            uints.append(v);
            return v;
        }

        KUrl *newUrl()
        {
            KUrl *v = new KUrl;
            urls.append(v);
            return v;
        }

        QDateTime *newDateTime()
        {
            QDateTime *v = new QDateTime;
            dateTimes.append(v);
            return v;
        }

        double *newDouble()
        {
            double *v = new double;
            doubles.append(v);
            return v;
        }

        QList<qint32>* newIntList()
        {
            QList<qint32> *v = new QList<qint32>;
            intlists.append(v);
            return v;
        }

        qint64 *newLongLong()
        {
            qint64 *v = new qint64;
            longlongs.append(v);
            return v;
        }

        QPoint *newPoint()
        {
            QPoint *v = new QPoint;
            points.append(v);
            return v;
        }

        QRect *newRect()
        {
            QRect *v = new QRect;
            rects.append(v);
            return v;
        }

        QSize *newSize()
        {
            QSize *v = new QSize;
            sizes.append(v);
            return v;
        }

        quint64 *newULongLong()
        {
            quint64 *v = new quint64;
            ulonglongs.append(v);
            return v;
        }

        KUrl::List *newUrlList()
        {
            KUrl::List *v = new KUrl::List;
            urllists.append(v);
            return v;
        }

        void parse(ConfigLoader *loader, QIODevice *xml);

        /**
         * Whether or not to write out default values.
         *
         * @param writeDefaults true if defaults should be written out
         */
        void setWriteDefaults(bool writeDefaults)
        {
            saveDefaults = writeDefaults;
        }

        /**
         * @return true if default values will also be written out
         */
        bool writeDefaults() const
        {
            return saveDefaults;
        }


        QList<bool *> bools;
        QList<QString *> strings;
        QList<QStringList *> stringlists;
        QList<QColor *> colors;
        QList<QFont *> fonts;
        QList<qint32 *> ints;
        QList<quint32 *> uints;
        QList<KUrl *> urls;
        QList<QDateTime *> dateTimes;
        QList<double *> doubles;
        QList<QList<qint32> *> intlists;
        QList<qint64 *> longlongs;
        QList<QPoint *> points;
        QList<QRect *> rects;
        QList<QSize *> sizes;
        QList<quint64 *> ulonglongs;
        QList<KUrl::List *> urllists;
        QString baseGroup;
        QStringList groups;
        QHash<QString, QString> keysToNames;
        bool saveDefaults;
};

} // namespace Plasma

#endif

