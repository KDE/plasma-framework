/*
 *    Copyright 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Library General Public
 *    License as published by the Free Software Foundation; either
 *    version 2 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Library General Public License for more details.
 *
 *    You should have received a copy of the GNU Library General Public License
 *    along with this library; see the file COPYING.LIB.  If not, write to
 *    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA 02110-1301, USA.
 */

#ifndef MIMEDATABASE_H
#define MIMEDATABASE_H

#include <QObject>
#include <QMimeDatabase>

class MimeDatabase : public QObject
{
    Q_OBJECT
    public:
        MimeDatabase(QObject* parent = 0);

        Q_SCRIPTABLE QVariantMap mimeTypeForUrl(const QUrl & url) const;
        Q_SCRIPTABLE QVariantMap mimeTypeForName(const QString & name) const;

    private:
        QMimeDatabase m_db;
};

#endif // MIMEDATABASE_H
