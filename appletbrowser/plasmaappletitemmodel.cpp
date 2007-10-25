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

void PlasmaAppletItem::setFavorite(bool favorite)
{
    QMap<QString, QVariant> attrs = data().toMap();
    attrs.insert("favorite", favorite ? true : false);
    setData(QVariant(attrs));

    QString pluginName = attrs["pluginName"].toString();

    if (pluginName == "skapplet" && attrs.contains("arguments")) {
        // skapplet can be used with all SuperKaramba themes,
        // so when setting skapplet as favorite it is also
        // necessary to know which theme is meant
        QString themePath = qvariant_cast<QVariantList>(attrs["arguments"])[0].toString();

        m_model->setFavorite(pluginName + " - " + themePath, favorite);
    } else {
        m_model->setFavorite(pluginName, favorite);
    }
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
    m_used = m_configGroup.readEntry("used").split(",");

    //TODO: get recommended, favorit, used, etc out of knownApplets()
    foreach (const KPluginInfo& info, Plasma::Applet::knownApplets()) {
        //kDebug() << info.pluginName() << "NoDisplay" << info.property("NoDisplay").toBool();
        if (info.property("NoDisplay").toBool()) {
            // we don't want to show the hidden category
            continue;
        }
        //kDebug() << info.pluginName() << " is the name of the plugin\n";

        if (info.pluginName() == "skapplet") {
            // If there is the SuperKaramba applet,
            // add SuperKaramba themes to the
            // model too
            loadSuperKarambaThemes(info);
        } else {
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

void PlasmaAppletItemModel::setFavorite(QString plugin, bool favorite)
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

/*
 * Define function type to get the SuperKaramba themes
 * from skapplet (see skapplet.cpp in kdeutils/superkaramba)
 */
extern "C" {
    typedef QList<QMap<QString, QVariant> > (*installedThemes)();
}

void PlasmaAppletItemModel::loadSuperKarambaThemes(const KPluginInfo &info)
{
    KService::Ptr service = info.service();
    QString libName = service->library();

    // Load the Plugin as library to get access
    // to installedThemes() in skapplet
    KLibrary *lib = KLibLoader::self()->library(libName);
    if (lib) {
        installedThemes loadThemes = 0;

        loadThemes = (installedThemes)lib->resolveFunction("installedThemes");

        if (loadThemes) {
            // loadThemes() returns the name, description, the icon
            // and one argument (file path) from the theme
            QList<QMap<QString, QVariant> > themeMetadata = loadThemes();

            QMap <QString, QVariant> metadata;
            foreach (metadata, themeMetadata) {
                metadata.insert("pluginName", "skapplet");
                metadata.insert("category", "SuperKaramba");

                QString favorite = info.pluginName() + " - " + qvariant_cast<QVariantList>(metadata["arguments"])[0].toString();

                appendRow(new PlasmaAppletItem(this, metadata,
                    ((m_favorites.contains(favorite)) ? PlasmaAppletItem::Favorite : PlasmaAppletItem::NoFilter) |
                    ((m_used.contains(info.pluginName())) ? PlasmaAppletItem::Used : PlasmaAppletItem::NoFilter)));
            }
        }
    } else {
        kWarning() << "Could not load" << libName;
    }
}

