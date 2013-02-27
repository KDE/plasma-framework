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
#include "plasmoid/appletinterface.h"

#include <QDebug>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlContext>

#include <KLocalizedString>

#include <Plasma/Corona>

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


//////////////////////////////ConfigModel
ConfigModel::ConfigModel(QObject *parent)
    : QAbstractListModel(parent)
{
    QHash<int, QByteArray> roleNames;
    roleNames[NameRole] = "name";
    roleNames[IconRole] = "icon";
    roleNames[SourceRole] = "source";

    setRoleNames(roleNames);
}

ConfigModel::~ConfigModel()
{}

int ConfigModel::rowCount(const QModelIndex &index) const
{
    if (index.column() > 0) {
        return 0;
    }
    return m_categories.count();
}

QVariant ConfigModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= m_categories.count()) {
        return QVariant();
    }
    switch (role) {
    case NameRole:
        return m_categories.at(index.row())->name();
    case IconRole:
        return m_categories.at(index.row())->icon();
    case SourceRole:
        return m_categories.at(index.row())->source();
    default:
        return QVariant();
    }
}

void ConfigModel::appendCategory(ConfigCategory *c)
{
    beginInsertRows(QModelIndex(), m_categories.size(), m_categories.size());
    m_categories.append(c);
    endInsertRows();
}

void ConfigModel::clear()
{
    beginResetModel();
    while (!m_categories.isEmpty()) {
        m_categories.first()->setParent(0);
        m_categories.pop_front();
    }
    endResetModel();
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
    ConfigModel *model = qobject_cast<ConfigModel *>(prop->object);
    if (!model || index >= model->m_categories.count() || index < 0)
        return 0;
    else
        return model->m_categories.at(index);
}

void ConfigModel::categories_append(QQmlListProperty<ConfigCategory> *prop, ConfigCategory *o)
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

int ConfigModel::categories_count(QQmlListProperty<ConfigCategory> *prop)
{
    ConfigModel *model = qobject_cast<ConfigModel *>(prop->object);
    if (model) {
        return model->m_categories.count();
    } else {
        return 0;
    }
}

void ConfigModel::categories_clear(QQmlListProperty<ConfigCategory> *prop)
{
    ConfigModel *model = qobject_cast<ConfigModel *>(prop->object);
    if (!model) {
        return;
    }

    model->clear();
}



//////////////////////////////ConfigView
ConfigView::ConfigView(AppletInterface *interface, QWindow *parent)
    : QQuickView(parent),
      m_appletInterface(interface)
{
    qmlRegisterType<ConfigModel>("org.kde.plasma.configuration", 0, 1, "ConfigModel");
    qmlRegisterType<ConfigCategory>("org.kde.plasma.configuration", 0, 1, "ConfigCategory");

    //FIXME: problem on nvidia, all windows should be transparent or won't show
    setColor(Qt::transparent);
    setTitle(i18n("%1 Settings", m_appletInterface->applet()->title()));


    if (!m_appletInterface->applet()->containment()->corona()->package().isValid()) {
        qWarning() << "Invalid home screen package";
    }

    setResizeMode(QQuickView::SizeViewToRootObject);


    QQmlComponent *component = new QQmlComponent(engine(), QUrl::fromLocalFile(m_appletInterface->applet()->package().filePath("ui", "config.qml")), this);

    QObject *configObject = component->create(engine()->rootContext());
    if (configObject) {
        m_configPages = configObject->property("modules").value<QQmlListProperty<QObject> >();
    }

    delete component;

    engine()->rootContext()->setContextProperty("plasmoid", interface);
    engine()->rootContext()->setContextProperty("configDialog", this);
    setSource(QUrl::fromLocalFile(m_appletInterface->applet()->containment()->corona()->package().filePath("ui", "Configuration.qml")));
}

ConfigView::~ConfigView()
{
}


QQmlListProperty<QObject> ConfigView::configPages() const
{
    return m_configPages;
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
