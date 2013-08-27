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

#include "currentcontainmentactionsmodel.h"

#include <QMouseEvent>

#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>

#include <KAboutData>
#include <KAboutApplicationDialog>
#include <KLocalizedString>
#include <KConfigGroup>

#include <Plasma/Corona>
#include <Plasma/Containment>
#include <Plasma/ContainmentActions>
#include <Plasma/PluginLoader>

class CurrentContainmentActionsModelPrivate
{
public:
    CurrentContainmentActionsModelPrivate(CurrentContainmentActionsModel *currentContainmentActionsModel);
    ~CurrentContainmentActionsModelPrivate();

    void init();

    QString mouseEventString(int mouseButtons, int modifiers);
    QString wheelEventString(const QPointF &delta, int mouseButtons, int modifiers);
    bool append(const QString &action, const QString &plugin);
    void update(int row, const QString &action, const QString &plugin);
    void remove(int row);
    void showConfiguration(int row);
    void showAbout(int row);
    void save();

    CurrentContainmentActionsModel *q;
    Plasma::Containment *containment;
    QHash<QString, Plasma::ContainmentActions *> plugins;
    KConfigGroup baseCfg;
};

CurrentContainmentActionsModelPrivate::CurrentContainmentActionsModelPrivate(CurrentContainmentActionsModel *currentContainmentActionsModel)
      : q(currentContainmentActionsModel)
{
}

CurrentContainmentActionsModelPrivate::~CurrentContainmentActionsModelPrivate()
{
    baseCfg = KConfigGroup(containment->corona()->config(), "ActionPlugins");

    QHash<QString, Plasma::ContainmentActions*> actions = containment->containmentActions();

    QHashIterator<QString, Plasma::ContainmentActions*> i(actions);
    while (i.hasNext()) {
        i.next();

        QStandardItem *item = new QStandardItem();
        item->setData(i.key(), CurrentContainmentActionsModel::ActionRole);
        item->setData(i.value()->pluginInfo().pluginName(), CurrentContainmentActionsModel::PluginRole);
        q->appendRow(item);
        plugins[i.key()] = Plasma::PluginLoader::self()->loadContainmentActions(containment, i.value()->pluginInfo().pluginName());
        plugins[i.key()]->setContainment(containment);
        KConfigGroup cfg(&baseCfg, i.key());
        plugins[i.key()]->restore(cfg);
    }
}

void CurrentContainmentActionsModelPrivate::init()
{
    baseCfg = KConfigGroup(containment->corona()->config(), "ActionPlugins");

    QHash<QString, Plasma::ContainmentActions*> actions = containment->containmentActions();

    QHashIterator<QString, Plasma::ContainmentActions*> i(actions);
    while (i.hasNext()) {
        i.next();

        QStandardItem *item = new QStandardItem();
        item->setData(i.key(), CurrentContainmentActionsModel::ActionRole);
        item->setData(i.value()->pluginInfo().pluginName(), CurrentContainmentActionsModel::PluginRole);
        q->appendRow(item);
        plugins[i.key()] = Plasma::PluginLoader::self()->loadContainmentActions(containment, i.value()->pluginInfo().pluginName());
        plugins[i.key()]->setContainment(containment);
        KConfigGroup cfg(&baseCfg, i.key());
        plugins[i.key()]->restore(cfg);
    }

}

QString CurrentContainmentActionsModelPrivate::mouseEventString(int mouseButton, int modifiers)
{
    QMouseEvent *mouse = new QMouseEvent(QEvent::MouseButtonRelease, QPoint(), (Qt::MouseButton)mouseButton, (Qt::MouseButton)mouseButton, (Qt::KeyboardModifiers) modifiers);

    QString string = Plasma::ContainmentActions::eventToString(mouse);

    delete mouse;

    return string;
}

QString CurrentContainmentActionsModelPrivate::wheelEventString(const QPointF &delta, int mouseButtons, int modifiers)
{
    QWheelEvent *wheel = new QWheelEvent(QPointF(), QPointF(), delta.toPoint(), QPoint(), 0, Qt::Vertical, (Qt::MouseButtons)mouseButtons, (Qt::KeyboardModifiers) modifiers);

    QString string = Plasma::ContainmentActions::eventToString(wheel);

    delete wheel;

    return string;
}

bool CurrentContainmentActionsModelPrivate::append(const QString &action, const QString &plugin)
{
    if (plugins.contains(action)) {
        return false;
    }

    QStandardItem *item = new QStandardItem();
    item->setData(action, CurrentContainmentActionsModel::ActionRole);
    item->setData(plugin, CurrentContainmentActionsModel::PluginRole);
    q->appendRow(item);
    plugins[action] = Plasma::PluginLoader::self()->loadContainmentActions(containment, plugin);
    KConfigGroup cfg(&baseCfg, action);
    plugins[action]->setContainment(containment);
    plugins[action]->restore(cfg);
    return true;
}

void CurrentContainmentActionsModelPrivate::update(int row, const QString &action, const QString &plugin)
{
    const QString oldPlugin = q->itemData(q->index(row, 0)).value(CurrentContainmentActionsModel::PluginRole).toString();

    QModelIndex idx = q->index(row, 0);

    if (idx.isValid()) {
        q->setData(idx, action, CurrentContainmentActionsModel::ActionRole);
        q->setData(idx, plugin, CurrentContainmentActionsModel::PluginRole);

        if (plugins.contains(action) && oldPlugin != plugin) {
            delete plugins[action];
            plugins[action] = Plasma::PluginLoader::self()->loadContainmentActions(containment, plugin);
        }
    }
}

void CurrentContainmentActionsModelPrivate::remove(int row)
{
    const QString action = q->itemData(q->index(row, 0)).value(CurrentContainmentActionsModel::ActionRole).toString();
    q->removeRows(row, 1);

    if (plugins.contains(action)) {
        delete plugins[action];
        plugins.remove(action);
    }
}

void CurrentContainmentActionsModelPrivate::showConfiguration(int row)
{
    const QString action = q->itemData(q->index(row, 0)).value(CurrentContainmentActionsModel::ActionRole).toString();

    if (!plugins.contains(action)) {
        return;
    }

    QDialog *configDlg = new QDialog();
    configDlg->setAttribute(Qt::WA_DeleteOnClose);
    QLayout *lay = new QVBoxLayout(configDlg);
    configDlg->setLayout(lay);
    configDlg->setWindowModality(Qt::WindowModal);

    //put the config in the dialog
    QWidget *w = plugins[action]->createConfigurationInterface(configDlg);
    QString title;
    if (w) {
        lay->addWidget(w);
        title = w->windowTitle();
    }

    configDlg->setWindowTitle(title.isEmpty() ? i18n("Configure Plugin") :title);
    //put buttons below
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                        Qt::Horizontal, configDlg);
    lay->addWidget(buttons);

    QObject::connect(buttons, SIGNAL(accepted()), q, SLOT(acceptConfig()));
    QObject::connect(buttons, SIGNAL(rejected()), q, SLOT(rejectConfig()));


    configDlg->show();
}

void CurrentContainmentActionsModelPrivate::showAbout(int row)
{
    const QString action = q->itemData(q->index(row, 0)).value(CurrentContainmentActionsModel::ActionRole).toString();

    if (!plugins.contains(action)) {
        return;
    }

    KPluginInfo info = plugins[action]->pluginInfo();

    KAboutData aboutData(info.name().toUtf8(),
            info.name().toUtf8(),
            ki18n(info.name().toUtf8()).toString(),
            info.version().toUtf8(), ki18n(info.comment().toUtf8()).toString(),
            KAboutLicense::byKeyword(info.license()).key(), ki18n(QByteArray()).toString(), ki18n(QByteArray()).toString(), info.website().toLatin1(),
            info.email().toLatin1());

    aboutData.setProgramIconName(info.icon());

    aboutData.addAuthor(ki18n(info.author().toUtf8()).toString(), ki18n(QByteArray()).toString(), info.email().toLatin1());

    KAboutApplicationDialog *aboutDialog = new KAboutApplicationDialog(aboutData, qobject_cast<QWidget*>(q->parent()));
    aboutDialog->show();
}

void CurrentContainmentActionsModelPrivate::save()
{

    //TODO: this configuration save is still a stub, not completely "correct" yet
    //clean old config, just i case
    foreach (const QString &group, baseCfg.groupList()) {
        KConfigGroup cfg = KConfigGroup(&baseCfg, group);
        cfg.deleteGroup();

        if (plugins.contains(group)) {
            containment->setContainmentActions(group, QString());
        }
    }

    QHashIterator<QString, Plasma::ContainmentActions*> i(plugins);
    while (i.hasNext()) {
        containment->setContainmentActions(i.key(), i.value()->pluginInfo().pluginName());
        i.next();
        KConfigGroup cfg(&baseCfg, i.key());
        i.value()->save(cfg);
    }
}


/////////////////////CurrentContainmentActionsModel
CurrentContainmentActionsModel::CurrentContainmentActionsModel(Plasma::Containment *cont, QObject *parent)
    : QStandardItemModel(parent),
      d(new CurrentContainmentActionsModelPrivate(this))
{
    QHash<int, QByteArray> roleNames;
    roleNames[ActionRole] = "action";
    roleNames[PluginRole] = "plugin";

    setRoleNames(roleNames);

    //WARNING!!!!
    //WARNING!!!!
    //containment must have a valid value before init is being called
    d->containment = cont;
    d->init();
}

CurrentContainmentActionsModel::~CurrentContainmentActionsModel()
{
    delete d;
}

QString CurrentContainmentActionsModel::mouseEventString(int mouseButton, int modifiers)
{
    return d->mouseEventString(mouseButton, modifiers);
}

QString CurrentContainmentActionsModel::wheelEventString(const QPointF &delta, int mouseButtons, int modifiers)
{
    return d->wheelEventString(delta, mouseButtons, modifiers);
}

bool CurrentContainmentActionsModel::append(const QString &action, const QString &plugin)
{
    return d->append(action, plugin);
}

void CurrentContainmentActionsModel::update(int row, const QString &action, const QString &plugin)
{
    d->update(row, action, plugin);
}

void CurrentContainmentActionsModel::remove(int row)
{
    d->remove(row);
}

void CurrentContainmentActionsModel::showConfiguration(int row)
{
    d->showConfiguration(row);
}

void CurrentContainmentActionsModel::showAbout(int row)
{
    d->showAbout(row);
}

void CurrentContainmentActionsModel::save()
{
    d->save();
}

#include "moc_currentcontainmentactionsmodel.cpp"
