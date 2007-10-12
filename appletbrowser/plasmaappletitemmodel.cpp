/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free 
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "plasmaappletitemmodel_p.h"

PlasmaAppletItem::PlasmaAppletItem(PlasmaAppletItemModel * model, const KPluginInfo& info,
        FilterFlags flags, QMap<QString, QVariant> * extraAttrs) :
    QObject(model), m_model(model)
{
    QMap<QString, QVariant> attrs;
    attrs.insert("name", info.name());
    attrs.insert("pluginName", info.pluginName());
    attrs.insert("description", info.comment());
    attrs.insert("category", info.category());
    attrs.insert("favorite", flags & Favorite ? true : false);
    attrs.insert("used", flags & Used ? true : false);
    //attrs.insert("recommended", flags & Recommended ? true : false);
    if (extraAttrs) attrs.unite(* extraAttrs);
    setText(info.name() + " - "+ info.category());
    setData(attrs);
    setIcon(KIcon(info.icon().isEmpty()?"application-x-plasma":info.icon()));
}

QString PlasmaAppletItem::name() const
{
    return data().toMap()["name"].toString();
}

QString PlasmaAppletItem::pluginName() const
{
    return data().toMap()["pluginName"].toString();
}

QString PlasmaAppletItem::description() const
{
    return data().toMap()["description"].toString();
}

void PlasmaAppletItem::setFavorite(bool favorite)
{
    QMap<QString, QVariant> attrs = data().toMap();
    attrs.insert("favorite", favorite ? true : false);
    setData(QVariant(attrs));
    m_model->setFavorite(attrs["pluginName"].toString(), favorite);
}

bool PlasmaAppletItem::passesFiltering(
        const KCategorizedItemsViewModels::Filter & filter) const
{
    return data().toMap()[filter.first] == filter.second;
}

PlasmaAppletItemModel::PlasmaAppletItemModel(KConfigGroup configGroup, QObject * parent) :
    KCategorizedItemsViewModels::DefaultItemModel(parent),
    m_configGroup(configGroup)
{

    // Recommended emblems and filters
    QRegExp rx("recommended[.]([0-9A-Za-z]+)[.]plugins");
    QMapIterator<QString, QString> i(m_configGroup.entryMap());
    QMap < QString, QMap < QString, QVariant > > extraPluginAttrs;
    while (i.hasNext()) {
        i.next();
        if (!rx.exactMatch(i.key())) continue;
        QString id = rx.cap(1);
        
        foreach (QString plugin, i.value().split(",")) {
            extraPluginAttrs[plugin]["recommended." + id] = true;
        }
    }

    m_favorites = m_configGroup.readEntry("favorites").split(",");
    QStringList m_used = m_configGroup.readEntry("used").split(",");
    
    //TODO: get recommended, favorit, used, etc out of knownApplets()
    foreach (const KPluginInfo& info, Plasma::Applet::knownApplets()) {
        kDebug() << info.pluginName() << " is the name of the plugin\n";
        
        appendRow(new PlasmaAppletItem(this, info,
                ((m_favorites.contains(info.pluginName())) ? PlasmaAppletItem::Favorite : PlasmaAppletItem::NoFilter) |
                ((m_used.contains(info.pluginName())) ? PlasmaAppletItem::Used : PlasmaAppletItem::NoFilter)
                , &(extraPluginAttrs[info.pluginName()])));
    }
}

QStringList PlasmaAppletItemModel::mimeTypes() const
{
    QStringList types;
    types << QLatin1String("text/x-plasmoidservicename");
    return types;
}

QMimeData* PlasmaAppletItemModel::mimeData(const QModelIndexList & indexes) const
{
    kDebug() << "GETTING MIME DATA\n";
    if (indexes.count() <= 0) {
        return 0;
    }

    QStringList types = mimeTypes();

    if (types.isEmpty()) {
        return 0;
    }

    QMimeData * data = new QMimeData();

    QString format = types.at(0);

    PlasmaAppletItem
            * selectedItem = (PlasmaAppletItem *) itemFromIndex(indexes[0]);
    QByteArray appletName(selectedItem->pluginName().toUtf8());

    data->setData(format, appletName);

    return data;
}

void PlasmaAppletItemModel::setFavorite(QString plugin, bool favorite) {
    if (favorite) {
        if (!m_favorites.contains(plugin)) {
            m_favorites.append(plugin);
        }
    } else {
        if (m_favorites.contains(plugin)) {
            m_favorites.removeAll(plugin);
        }
    }
    m_configGroup.writeEntry("favorites", m_favorites.join(","));
    m_configGroup.sync();

}
