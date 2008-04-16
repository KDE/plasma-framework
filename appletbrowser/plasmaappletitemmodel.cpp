/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "plasmaappletitemmodel_p.h"

PlasmaAppletItem::PlasmaAppletItem(PlasmaAppletItemModel * model, const QMap<QString, QVariant>& info,
        FilterFlags flags, QMap<QString, QVariant> * extraAttrs) :
    QObject(model), m_model(model)
{
    QMap<QString, QVariant> attrs(info);

    attrs.insert("favorite", flags & Favorite ? true : false);
    attrs.insert("used", flags & Used ? true : false);
    //attrs.insert("recommended", flags & Recommended ? true : false);
    if (extraAttrs) attrs.unite(* extraAttrs);
    setText(info["name"].toString() + " - "+ info["category"].toString());
    setData(attrs);
    setIcon(qvariant_cast<QIcon>(info["icon"]));
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

int PlasmaAppletItem::running() const
{
    return data().toMap()["runningCount"].toInt();
}

void PlasmaAppletItem::setFavorite(bool favorite)
{
    QMap<QString, QVariant> attrs = data().toMap();
    attrs.insert("favorite", favorite ? true : false);
    setData(QVariant(attrs));

    QString pluginName = attrs["pluginName"].toString();
    m_model->setFavorite(pluginName, favorite);
}

void PlasmaAppletItem::setRunning(int count)
{
    QMap<QString, QVariant> attrs = data().toMap();
    attrs.insert("running", count > 0); //bool for the filter
    attrs.insert("runningCount", count);
    setData(QVariant(attrs));
}

bool PlasmaAppletItem::passesFiltering(
        const KCategorizedItemsViewModels::Filter & filter) const
{
    return data().toMap()[filter.first] == filter.second;
}

QVariantList PlasmaAppletItem::arguments() const
{
    return qvariant_cast<QVariantList>(data().toMap()["arguments"]);
}

PlasmaAppletItemModel::PlasmaAppletItemModel(KConfigGroup configGroup, QObject * parent) :
    KCategorizedItemsViewModels::DefaultItemModel(parent),
    m_configGroup(configGroup)
{
    m_used = m_configGroup.readEntry("used").split(",");
    m_favorites = m_configGroup.readEntry("favorites").split(",");
}

void PlasmaAppletItemModel::populateModel()
{
    clear();
    //kDebug() << "populating model, our application is" << m_application;

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

    //TODO: get recommended, favorite, used, etc out of listAppletInfo()
    //kDebug() << "number of applets is" <<  Plasma::Applet::listAppletInfo(QString(), m_application).count();
    foreach (const KPluginInfo& info, Plasma::Applet::listAppletInfo(QString(), m_application)) {
        //kDebug() << info.pluginName() << "NoDisplay" << info.property("NoDisplay").toBool();
        if (info.property("NoDisplay").toBool()) {
            // we don't want to show the hidden category
            continue;
        }
        //kDebug() << info.pluginName() << " is the name of the plugin\n";

        QMap<QString, QVariant> attrs;
        attrs.insert("name", info.name());
        attrs.insert("pluginName", info.pluginName());
        attrs.insert("description", info.comment());
        attrs.insert("category", info.category());
        attrs.insert("icon", static_cast<QIcon>(KIcon(info.icon().isEmpty()?"application-x-plasma":info.icon())));

        appendRow(new PlasmaAppletItem(this, attrs,
                    ((m_favorites.contains(info.pluginName())) ? PlasmaAppletItem::Favorite : PlasmaAppletItem::NoFilter) |
                    ((m_used.contains(info.pluginName())) ? PlasmaAppletItem::Used : PlasmaAppletItem::NoFilter)
                    , &(extraPluginAttrs[info.pluginName()])));
    }
}

void PlasmaAppletItemModel::setRunningApplets(const QHash<QString, int> &apps)
{
    //foreach item, find that string and set the count
    for (int r=0; r<rowCount(); ++r) {
        QStandardItem *i = item(r);
        PlasmaAppletItem *p = dynamic_cast<PlasmaAppletItem *>(i);
        if (p) {
            p->setRunning(apps.value(p->name()));
        }
    }
}

void PlasmaAppletItemModel::setRunningApplets(const QString &name, int count)
{
    for (int r=0; r<rowCount(); ++r) {
        QStandardItem *i = item(r);
        PlasmaAppletItem *p = dynamic_cast<PlasmaAppletItem *>(i);
        if (p && p->name() == name) {
            p->setRunning(count);
        }
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

void PlasmaAppletItemModel::setFavorite(const QString &plugin, bool favorite)
{
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

void PlasmaAppletItemModel::setApplication(const QString& app)
{
    m_application = app;
    populateModel();
}

QString& PlasmaAppletItemModel::Application()
{
    return m_application;
}
