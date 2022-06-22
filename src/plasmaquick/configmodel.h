/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CONFIGMODEL_H
#define CONFIGMODEL_H

#include <QAbstractListModel>
#include <QQmlListProperty>

#include <plasmaquick/plasmaquick_export.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the public Plasma API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace Plasma
{
class Applet;
}

namespace PlasmaQuick
{
class ConfigPropertyMap;

class ConfigCategoryPrivate;

class ConfigModelPrivate;
class ConfigCategory;

/**
 * This model contains all the possible config categories for a dialog,
 * such as categories of the config dialog for an Applet
 * TODO: it should probably become an import instead of a library?
 */
class PLASMAQUICK_EXPORT ConfigModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<PlasmaQuick::ConfigCategory> categories READ categories CONSTANT)
    Q_CLASSINFO("DefaultProperty", "categories")
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        IconRole,
        SourceRole,
        PluginNameRole,
        VisibleRole,
        KCMRole,
    };
    Q_ENUM(Roles)

    explicit ConfigModel(QObject *parent = nullptr);
    ~ConfigModel() override;

    /**
     * add a new category in the model
     * @param ConfigCategory the new category
     **/
    void appendCategory(const QString &iconName, const QString &name, const QString &path, const QString &pluginName);

    Q_INVOKABLE void appendCategory(const QString &iconName, const QString &name, const QString &path, const QString &pluginName, bool visible);
    // QML Engine isn't particularly smart resolving namespaces, hence fully qualified signature
    Q_INVOKABLE void appendCategory(PlasmaQuick::ConfigCategory *category);

    Q_INVOKABLE void removeCategory(PlasmaQuick::ConfigCategory *category);
    Q_INVOKABLE void removeCategoryAt(int index);

    /**
     * clears the model
     **/
    void clear();

    void setApplet(Plasma::Applet *interface);
    Plasma::Applet *applet() const;

    int count()
    {
        return rowCount();
    }
    int rowCount(const QModelIndex &index = QModelIndex()) const override;
    QVariant data(const QModelIndex &, int) const override;
    QHash<int, QByteArray> roleNames() const override;

    /**
     * @param row the row for which the data will be returned
     * @return the data of the specified row
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

}

#endif // multiple inclusion guard
