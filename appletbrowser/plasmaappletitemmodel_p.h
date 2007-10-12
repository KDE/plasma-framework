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

#ifndef PLASMAAPPLETSMODEL_H_
#define PLASMAAPPLETSMODEL_H_

#include <KPluginInfo>

#include <plasma/applet.h>

#include "kcategorizeditemsview_p.h"

class PlasmaAppletItemModel;

/**
 * Implementation of the KCategorizedItemsViewModels::AbstractItem
 */
class PlasmaAppletItem : public KCategorizedItemsViewModels::AbstractItem,
        QObject
{
public:
    enum FilterFlag {NoFilter = 0,
        Favorite = 1,
        Used = 2};

    Q_DECLARE_FLAGS(FilterFlags, FilterFlag)

    PlasmaAppletItem(PlasmaAppletItemModel * model, const KPluginInfo& info,
            FilterFlags flags = NoFilter, QMap<QString, QVariant> * extraAttrs = NULL);

    virtual QString name() const;
    QString pluginName() const;
    virtual QString description() const;
    virtual void setFavorite(bool favorite);
    virtual bool passesFiltering(
            const KCategorizedItemsViewModels::Filter & filter) const;
private:
    PlasmaAppletItemModel * m_model;
};

class PlasmaAppletItemModel :
    public KCategorizedItemsViewModels::DefaultItemModel
{
public:
    PlasmaAppletItemModel(KConfigGroup configGroup, QObject * parent = 0);

    QStringList mimeTypes() const;

    QMimeData* mimeData(const QModelIndexList & indexes) const;
    
    void setFavorite(QString plugin, bool favorite);

private:
    QStringList m_favorites;
    KConfigGroup m_configGroup;
    
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PlasmaAppletItem::FilterFlags)

#endif /*PLASMAAPPLETSMODEL_H_*/
