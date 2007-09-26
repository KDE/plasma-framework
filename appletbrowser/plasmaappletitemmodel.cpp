/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 (or,
 *   at your option, any later version) as published by the Free Software
 *   Foundation
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

#include "plasmaappletitemmodel.h"

PlasmaAppletItem::PlasmaAppletItem(QObject * parent, QString name,
        QString pluginName, QString description, QString category, QIcon icon,
        FilterFlags flags) :
    QObject(parent)
{
    QMap<QString, QVariant> attrs;
    attrs.insert("name", QVariant(name));
    attrs.insert("pluginName", QVariant(pluginName));
    attrs.insert("description", QVariant(description));
    attrs.insert("category", QVariant(category));
    attrs.insert("favorite", flags & Favorite ? true : false);
    attrs.insert("used", flags & Used ? true : false);
    attrs.insert("recommended", flags & Recommended ? true : false);
    setText(name + " - "+ description);
    setData(QVariant(attrs));
    setIcon(icon);
}

PlasmaAppletItem::PlasmaAppletItem(QObject *parent, const KPluginInfo& info,
        FilterFlags flags) :
    QObject(parent)
{
    QMap<QString, QVariant> attrs;
    attrs.insert("name", info.name());
    attrs.insert("pluginName", info.pluginName());
    attrs.insert("description", info.comment());
    attrs.insert("category", info.category());
    attrs.insert("favorite", flags & Favorite ? true : false);
    attrs.insert("used", flags & Used ? true : false);
    attrs.insert("recommended", flags & Recommended ? true : false);
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
}

bool PlasmaAppletItem::passesFiltering(
        const KCategorizedItemsViewModels::Filter & filter) const
{
    return data().toMap()[filter.first] == filter.second;
}

PlasmaAppletItemModel::PlasmaAppletItemModel(QObject * parent) :
    KCategorizedItemsViewModels::DefaultItemModel(parent)
{
    // some test items that do have FilterFlags as well as overly long text
    appendRow(new PlasmaAppletItem(this, 
        i18n("This is a very long name for an applet, isn't it?"), "null", 
        i18n("This is a Graphics applet whose description is even longer than it's title. And it was hard to achieve!"), 
        QString("graph"),
        QIcon("/usr/share/icons/oxygen/64x64/apps/k3b.png"), PlasmaAppletItem::Used));
    appendRow(new PlasmaAppletItem(this, 
        i18n("This is a very long name for another applet, isn't it?"), "null", 
        i18n("This is a Graphics applet whose description is even longer than it's title. And it was hard to achieve!"), 
        QString("graph"),
        QIcon("/usr/share/icons/oxygen/64x64/apps/kfind.png"), PlasmaAppletItem::Used | PlasmaAppletItem::Recommended));
    appendRow(new PlasmaAppletItem(this, 
        i18n("This is my favorite item!"), "null",
        i18n("This is a Graphics applet whose description is even longer than it's title. And it was hard to achieve!"), 
        QString("graph"),
        QIcon("/usr/share/icons/oxygen/64x64/apps/okular.png"), PlasmaAppletItem::Favorite | PlasmaAppletItem::Recommended));

    //TODO: get recommended, favorit, used, etc out of knownApplets()
    foreach (const KPluginInfo& info, Plasma::Applet::knownApplets()) {
        appendRow(new PlasmaAppletItem(this, info));
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
