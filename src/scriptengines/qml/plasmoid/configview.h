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

#ifndef CONFIGUILOADER_H
#define CONFIGUILOADER_H


#include <QQuickView>
#include <QJSValue>
#include <QQmlListProperty>
#include <QStandardItemModel>

class AppletInterface;


class ConfigCategory : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY iconChanged)
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)

public:
    ConfigCategory(QObject *parent = 0);
    ~ConfigCategory();

    QString name() const;
    void setName(const QString &name);

    QString icon() const;
    void setIcon(const QString &icon);

    QString source() const;
    void setSource(const QString &source);

Q_SIGNALS:
    void nameChanged();
    void iconChanged();
    void sourceChanged();

private:
    QString m_name;
    QString m_icon;
    QString m_source;
};

class ConfigModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<ConfigCategory> categories READ categories CONSTANT)
    Q_CLASSINFO("DefaultProperty", "categories")
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Roles {
        NameRole = Qt::UserRole+1,
        IconRole,
        SourceRole
    };
    ConfigModel(QObject *parent = 0);
    ~ConfigModel();

    void appendCategory(ConfigCategory *c);
    void clear();

    void setAppletInterface(AppletInterface *interface);
    AppletInterface *appletInterface() const;

    int count() {return rowCount();}
    virtual int rowCount(const QModelIndex &index = QModelIndex()) const;
    virtual QVariant data(const QModelIndex&, int) const;
    Q_INVOKABLE QVariant get(int row) const;

    QQmlListProperty<ConfigCategory> categories();
    
    static ConfigCategory *categories_at(QQmlListProperty<ConfigCategory> *prop, int index);
    static void categories_append(QQmlListProperty<ConfigCategory> *prop, ConfigCategory *o);
    static int categories_count(QQmlListProperty<ConfigCategory> *prop);
    static void categories_clear(QQmlListProperty<ConfigCategory> *prop);

Q_SIGNALS:
    void countChanged();

private:
    QList<ConfigCategory*>m_categories;
    QWeakPointer<AppletInterface> m_appletInterface;
};

class ConfigView : public QQuickView
{
    Q_OBJECT
    Q_PROPERTY(QObject *configModel READ configModel CONSTANT)

public:
    ConfigView(AppletInterface *scriptEngine, QWindow *parent = 0);
    virtual ~ConfigView();

    QObject *configModel() const;

protected:
     void hideEvent(QHideEvent *ev);
     void resizeEvent(QResizeEvent *re);

private:
    AppletInterface *m_appletInterface;
    ConfigModel *m_configModel;
};

#endif // multiple inclusion guard
