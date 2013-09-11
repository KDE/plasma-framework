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

#include "configview_p.h"
#include "configview.h"
#include "configmodel.h"
#include "Plasma/Applet"
#include "Plasma/Containment"
//#include "plasmoid/wallpaperinterface.h"
#include "kdeclarative/configpropertymap.h"

#include <QDebug>
#include <QDir>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickItem>

#include <KGlobal>
#include <KLocalizedString>
#include <kdeclarative/kdeclarative.h>

#include <Plasma/Corona>
#include <Plasma/PluginLoader>

///////////////////////ConfigCategory

ConfigCategory::ConfigCategory(QObject *parent)
    : QObject(parent)
{
}

ConfigCategory::~ConfigCategory()
{}

QString ConfigCategory::name() const
{
    return m_name;
}

void ConfigCategory::setName(const QString &name)
{
    if (m_name == name) {
        return;
    }

    m_name = name;
    emit nameChanged();
}


QString ConfigCategory::icon() const
{
    return m_icon;
}

void ConfigCategory::setIcon(const QString &icon)
{
    if (m_icon == icon) {
        return;
    }

    m_icon = icon;
    emit iconChanged();
}


QString ConfigCategory::source() const
{
    return m_source;
}

void ConfigCategory::setSource(const QString &source)
{
    if (m_source == source) {
        return;
    }

    m_source = source;
    emit sourceChanged();
}

QString ConfigCategory::pluginName() const
{
    return m_pluginName;
}

void ConfigCategory::setPluginName(const QString &name)
{
    if (m_pluginName == name) {
        return;
    }

    m_pluginName = name;
    emit pluginNameChanged();
}



//////////////////////////////ConfigModel

class ConfigModelPrivate
{
public:
    ConfigModelPrivate(ConfigModel *model);
    ~ConfigModelPrivate();

    ConfigModel *q;
    QList<ConfigCategory*> categories;
    QWeakPointer<Plasma::Applet> appletInterface;

    void appendCategory(ConfigCategory *c);
    void clear();
    QVariant get(int row) const;

    static ConfigCategory *categories_at(QQmlListProperty<ConfigCategory> *prop, int index);
    static void categories_append(QQmlListProperty<ConfigCategory> *prop, ConfigCategory *o);
    static int categories_count(QQmlListProperty<ConfigCategory> *prop);
    static void categories_clear(QQmlListProperty<ConfigCategory> *prop);
};

ConfigModelPrivate::ConfigModelPrivate(ConfigModel *model)
    : q(model)
{
}

ConfigModelPrivate::~ConfigModelPrivate()
{
}

ConfigCategory *ConfigModelPrivate::categories_at(QQmlListProperty<ConfigCategory> *prop, int index)
{
    ConfigModel *model = qobject_cast<ConfigModel *>(prop->object);
    if (!model || index >= model->d->categories.count() || index < 0) {
        return 0;
    }  else {
        return model->d->categories.at(index);
    }
}

void ConfigModelPrivate::categories_append(QQmlListProperty<ConfigCategory> *prop, ConfigCategory *o)
{
    ConfigModel *model = qobject_cast<ConfigModel *>(prop->object);
    if (!o || !model) {
        return;
    }

    if (o->parent() == prop->object) {
        o->setParent(0);
    }

    o->setParent(prop->object);
    model->appendCategory(o);
}

int ConfigModelPrivate::categories_count(QQmlListProperty<ConfigCategory> *prop)
{
    ConfigModel *model = qobject_cast<ConfigModel *>(prop->object);
    if (model) {
        return model->d->categories.count();
    } else {
        return 0;
    }
}

void ConfigModelPrivate::categories_clear(QQmlListProperty<ConfigCategory> *prop)
{
    ConfigModel *model = qobject_cast<ConfigModel *>(prop->object);
    if (!model) {
        return;
    }

    model->clear();
}

void ConfigModelPrivate::clear()
{
    q->beginResetModel();
    while (!categories.isEmpty()) {
        categories.first()->setParent(0);
        categories.pop_front();
    }
    q->endResetModel();
    emit q->countChanged();
}

void ConfigModelPrivate::appendCategory(ConfigCategory *c)
{
    q->beginInsertRows(QModelIndex(), categories.size(), categories.size());
    categories.append(c);
    q->endInsertRows();
    emit q->countChanged();
}

QVariant ConfigModelPrivate::get(int row) const
{
    QVariantMap value;
    if (row < 0 || row >= categories.count()) {
        return value;
    }

    value["name"] = categories.at(row)->name();
    value["icon"] = categories.at(row)->icon();
    value["pluginName"] = categories.at(row)->pluginName();
    if (appletInterface) {
        value["source"] = QUrl::fromLocalFile(appletInterface.data()->package().filePath("ui", categories.at(row)->source()));
    } else {
        value["source"] = categories.at(row)->source();
    }
    return value;
}


ConfigModel::ConfigModel(QObject *parent)
    : QAbstractListModel(parent),
      d(new ConfigModelPrivate(this))
{
    QHash<int, QByteArray> roleNames;
    roleNames[NameRole] = "name";
    roleNames[IconRole] = "icon";
    roleNames[SourceRole] = "source";
    roleNames[PluginNameRole] = "pluginName";

    setRoleNames(roleNames);
}

ConfigModel::~ConfigModel()
{
    delete d;
}

int ConfigModel::rowCount(const QModelIndex &index) const
{
    if (index.column() > 0) {
        return 0;
    }
    return d->categories.count();
}

QVariant ConfigModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= d->categories.count()) {
        return QVariant();
    }
    switch (role) {
    case NameRole:
        return d->categories.at(index.row())->name();
    case IconRole:
        return d->categories.at(index.row())->icon();
    case SourceRole:
        if (d->appletInterface) {
            return QUrl::fromLocalFile(d->appletInterface.data()->package().filePath("ui", d->categories.at(index.row())->source()));
        } else {
            return d->categories.at(index.row())->source();
        }
    case PluginNameRole:
        return d->categories.at(index.row())->pluginName();
    default:
        return QVariant();
    }
}

QVariant ConfigModel::get(int row) const
{
    return d->get(row);
}

void ConfigModel::appendCategory(ConfigCategory *c)
{
    d->appendCategory(c);
}

void ConfigModel::clear()
{
    d->clear();
}

void ConfigModel::setApplet(Plasma::Applet *interface)
{
    d->appletInterface = interface;
}

Plasma::Applet *ConfigModel::applet() const
{
    return d->appletInterface.data();
}

QQmlListProperty<ConfigCategory> ConfigModel::categories()
{
    return QQmlListProperty<ConfigCategory>(this, 0, ConfigModel::categories_append,
                                             ConfigModel::categories_count,
                                             ConfigModel::categories_at,
                                             ConfigModel::categories_clear);

}

ConfigCategory *ConfigModel::categories_at(QQmlListProperty<ConfigCategory> *prop, int index)
{
    return ConfigModelPrivate::categories_at(prop, index);
}

void ConfigModel::categories_append(QQmlListProperty<ConfigCategory> *prop, ConfigCategory *o)
{
    ConfigModelPrivate::categories_append(prop, o);
}

int ConfigModel::categories_count(QQmlListProperty<ConfigCategory> *prop)
{
    return ConfigModelPrivate::categories_count(prop);
}

void ConfigModel::categories_clear(QQmlListProperty<ConfigCategory> *prop)
{
    ConfigModelPrivate::categories_clear(prop);
}


#include "moc_configmodel.cpp"
