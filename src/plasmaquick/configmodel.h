/*
 *   Copyright 2013 Marco Martin <mart@kde.org>
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

#ifndef CONFIGMODEL_H
#define CONFIGMODEL_H


#include <QQmlListProperty>
#include <QAbstractListModel>

#include <plasmaquick/plasmaquick_export.h>

namespace Plasma {
    class Applet;
}

class ConfigPropertyMap;

class ConfigCategoryPrivate;

class ConfigModelPrivate;
class ConfigCategory;

class PLASMAQUICK_EXPORT ConfigModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<ConfigCategory> categories READ categories CONSTANT)
    Q_CLASSINFO("DefaultProperty", "categories")
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        NameRole = Qt::UserRole+1,
        IconRole,
        SourceRole,
        PluginNameRole
    };
    ConfigModel(QObject *parent = 0);
    ~ConfigModel();

    /**
     * add a new category in the model
     * @param ConfigCategory the new category
     **/
    void appendCategory(ConfigCategory *c);

    /**
     * clears the model
     **/
    void clear();

    void setApplet(Plasma::Applet *interface);
    Plasma::Applet *applet() const;

    int count() {return rowCount();}
    virtual int rowCount(const QModelIndex &index = QModelIndex()) const;
    virtual QVariant data(const QModelIndex&, int) const;

    /**
     * @param row the row for which the data will be returned
     * @raturn the data of the specified row
     **/
    Q_INVOKABLE QVariant get(int row) const;

    /**
     * @return the categories of the model
     **/
    QQmlListProperty<ConfigCategory> categories();

Q_SIGNALS:
    /**
     * emitted when the count is changed
     **/
    void countChanged();

private:
    friend class ConfigModelPrivate;
    ConfigModelPrivate *const d;
};

#endif // multiple inclusion guard
