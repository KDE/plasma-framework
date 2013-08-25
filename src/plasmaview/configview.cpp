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

#include "configview.h"
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

#include <Plasma/Corona>
#include <Plasma/PluginLoader>

///////////////////////ConfigCategory

class ConfigCategoryPrivate
{
public:
    ConfigCategoryPrivate();
    ~ConfigCategoryPrivate();
    QString name;
    QString icon;
    QString source;
    QString pluginName;
};

ConfigCategoryPrivate::ConfigCategoryPrivate()
{
}

ConfigCategoryPrivate::~ConfigCategoryPrivate()
{
}

ConfigCategory::ConfigCategory(QObject *parent)
    : QObject(parent),
      d(new ConfigCategoryPrivate())
{
}

ConfigCategory::~ConfigCategory()
{}

QString ConfigCategory::name() const
{
    return d->name;
}

void ConfigCategory::setName(const QString &name)
{
    if (d->name == name) {
        return;
    }

    d->name = name;
    emit nameChanged();
}


QString ConfigCategory::icon() const
{
    return d->icon;
}

void ConfigCategory::setIcon(const QString &icon)
{
    if (d->icon == icon) {
        return;
    }

    d->icon = icon;
    emit iconChanged();
}


QString ConfigCategory::source() const
{
    return d->source;
}

void ConfigCategory::setSource(const QString &source)
{
    if (d->source == source) {
        return;
    }

    d->source = source;
    emit sourceChanged();
}

QString ConfigCategory::pluginName() const
{
    return d->pluginName;
}

void ConfigCategory::setPluginName(const QString &name)
{
    if (d->pluginName == name) {
        return;
    }

    d->pluginName = name;
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
    qDebug() << " XXX loaded QALM";
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



//////////////////////////////ConfigView

class ConfigViewPrivate
{
public:
    ConfigViewPrivate(Plasma::Applet *appl, ConfigView *view);
    ~ConfigViewPrivate();

    void init();
    ConfigView *q;
    Plasma::Applet *applet;
    ConfigModel *configModel;
};

ConfigViewPrivate::ConfigViewPrivate(Plasma::Applet *appl, ConfigView *view)
    : q(view),
      applet(appl)
{
}

void ConfigViewPrivate::init()
{
    applet->setUserConfiguring(true);

    //FIXME: problem on nvidia, all windows should be transparent or won't show
    q->setColor(Qt::transparent);
    q->setTitle(i18n("%1 Settings", applet->title()));


    if (!applet->containment()->corona()->package().isValid()) {
        qWarning() << "Invalid home screen package";
    }

    q->setResizeMode(QQuickView::SizeViewToRootObject);


    //config model local of the applet
    QQmlComponent *component = new QQmlComponent(q->engine(), QUrl::fromLocalFile(applet->package().filePath("configmodel")), q);
    QObject *object = component->beginCreate(q->engine()->rootContext());
    configModel = qobject_cast<ConfigModel *>(object);
    if (configModel) {
        configModel->setApplet(applet);
    } else {
        delete object;
    }

    Plasma::Containment *cont = qobject_cast<Plasma::Containment *>(applet);

    q->engine()->rootContext()->setContextProperty("plasmoid", applet->property("graphicObject").value<QObject*>());
    q->engine()->rootContext()->setContextProperty("configDialog", q);
    component->completeCreate();
    delete component;
}



ConfigView::ConfigView(Plasma::Applet *applet, QWindow *parent)
    : QQuickView(parent),
      d(new ConfigViewPrivate(applet, this))
{
    d->init();
    qmlRegisterType<ConfigModel>("org.kde.plasma.configuration", 2, 0, "ConfigModel");
    qmlRegisterType<ConfigCategory>("org.kde.plasma.configuration", 2, 0, "ConfigCategory");
}

ConfigView::~ConfigView()
{
    d->applet->setUserConfiguring(false);
}

void ConfigView::init()
{
    setSource(QUrl::fromLocalFile(d->applet->containment()->corona()->package().filePath("appletconfigurationui")));
}

ConfigModel *ConfigView::configModel() const
{
    return d->configModel;
}

//To emulate Qt::WA_DeleteOnClose that QWindow doesn't have
void ConfigView::hideEvent(QHideEvent *ev)
{
    QQuickWindow::hideEvent(ev);
    deleteLater();
}

void ConfigView::resizeEvent(QResizeEvent *re)
{
    if (!rootObject()) {
        return;
    }
    rootObject()->setWidth(re->size().width());
    rootObject()->setHeight(re->size().height());
    QQuickWindow::resizeEvent(re);
}


#include "moc_configview.cpp"
