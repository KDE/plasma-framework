/*
    SPDX-FileCopyrightText: 2011 Viranch Mehta <viranch.mehta@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "menu.h"

#include <QApplication>
#include <QDebug>
#include <QQuickItem>
#include <QQuickWindow>
#include <QScreen>
#include <QTimer>
#include <QVersionNumber>

#include <KAcceleratorManager>


MenuItem::MenuItem(QObject *parent)
    : QObject(parent)
    , m_action(nullptr)
    , m_section(false)
{
    setAction(new QAction(this));
}

QAction *MenuItem::action() const
{
    return m_action;
}

void MenuItem::setAction(QAction *a)
{
    if (m_action != a) {
        if (m_action) {
            disconnect(m_action, nullptr, this, nullptr);

            if (m_action->parent() == this) {
                delete m_action;
                m_action = nullptr;
            }
        }

        if (a) {
            m_action = a;
        } else {
            // don't end up with no action, create an invisible one instead
            m_action = new QAction(this);
            m_action->setVisible(false);
        }

        setVisible(m_action->isVisible());
        setEnabled(m_action->isEnabled());

        connect(m_action, &QAction::changed, this, &MenuItem::textChanged);
        connect(m_action, &QAction::changed, this, &MenuItem::checkableChanged);
        connect(m_action, &QAction::toggled, this, &MenuItem::toggled);
        connect(m_action, &QAction::triggered, this, &MenuItem::clicked);
        // HACK MenuItem doesn't delete other people's QAction (see m_action->parent() check above)
        // but it does not take kindly to the QAction being deleted under it
        // as a workaround for crashing when this happens, replace it by a dummy action again
        // TODO this entire ownership handling in QMenu(Item) needs to be refactored...
        connect(m_action, &QObject::destroyed, this, [this] {
            if (m_action->parent() != this) {
                m_action = new QAction(this);
                m_action->setVisible(false);
                Q_EMIT actionChanged();
            }
        });

        connect(this, &MenuItem::visibleChanged, this, &MenuItem::updateAction);
        connect(this, &MenuItem::enabledChanged, this, &MenuItem::updateAction);
        connect(this, &QObject::destroyed, this, &MenuItem::deleteLater);

        Q_EMIT actionChanged();
    }
}

QVariant MenuItem::icon() const
{
    return m_icon;
}

void MenuItem::setIcon(const QVariant &i)
{
    m_icon = i;
    if (i.canConvert<QIcon>()) {
        m_action->setIcon(i.value<QIcon>());
    } else if (i.canConvert<QString>()) {
        m_action->setIcon(QIcon::fromTheme(i.toString()));
    }

    Q_EMIT iconChanged();
}

bool MenuItem::separator() const
{
    return m_action->isSeparator();
}

void MenuItem::setSeparator(bool s)
{
    m_action->setSeparator(s);
}

bool MenuItem::section() const
{
    return m_section;
}

void MenuItem::setSection(bool s)
{
    m_section = s;
}

QString MenuItem::text() const
{
    return m_action->text();
}

void MenuItem::setText(const QString &t)
{
    if (m_action->text() != t) {
        m_action->setText(t);
        // signal comes from m_action
    }
}

bool MenuItem::checkable() const
{
    return m_action->isCheckable();
}

void MenuItem::setCheckable(bool checkable)
{
    m_action->setCheckable(checkable);
}

bool MenuItem::checked() const
{
    return m_action->isChecked();
}

void MenuItem::setChecked(bool checked)
{
    m_action->setChecked(checked);
}

void MenuItem::updateAction()
{
    m_action->setVisible(visible());
    m_action->setEnabled(enabled());
}

bool MenuItem::visible() const
{
    return m_visible;
}

void MenuItem::setVisible(bool newValue)
{
    if (m_visible == newValue) {
        return;
    }

    m_visible = newValue;
    Q_EMIT visibleChanged();
}

bool MenuItem::enabled() const
{
    return m_enabled;
}

void MenuItem::setEnabled(bool newValue)
{
    if (m_enabled == newValue) {
        return;
    }

    m_enabled = newValue;
    Q_EMIT enabledChanged();
}

MenuExtension::MenuExtension(QObject *parent)
    : QObject(parent)
    , m_menu(qobject_cast<QMenu*>(parent))
    , m_status(Closed)
    , m_placement(Plasma::Types::LeftPosedTopAlignedPopup)
{
    if (m_menu) {
        // Breeze and Oxygen have rounded corners on menus. They set this attribute in polish()
        // but at that time the underlying surface has already been created where setting this
        // flag makes no difference anymore (Bug 385311)
        m_menu->setAttribute(Qt::WA_TranslucentBackground);

        KAcceleratorManager::manage(m_menu);
        connect(m_menu, &QMenu::triggered, this, &MenuExtension::itemTriggered);
        connect(m_menu, &QMenu::aboutToHide, this, [=]() {
            m_status = Closed;
            Q_EMIT statusChanged();
        });
    }
}

MenuExtension::~MenuExtension()
{
    delete m_menu;
}

QQmlListProperty<MenuItem> MenuExtension::content()
{
    return QQmlListProperty<MenuItem>(this, &m_items);
}

int MenuExtension::actionCount() const
{
    return m_items.count();
}

MenuItem *MenuExtension::action(int index) const
{
    return m_items.at(index);
}

MenuExtension::Status MenuExtension::status() const
{
    return m_status;
}

QObject *MenuExtension::visualParent() const
{
    return m_visualParent.data();
}

void MenuExtension::setVisualParent(QObject *parent)
{
    if (m_visualParent.data() == parent) {
        return;
    }

    // if the old parent was a QAction, disconnect the menu from it
    QAction *action = qobject_cast<QAction *>(m_visualParent.data());
    if (action) {
        action->setMenu(nullptr);
        m_menu->clear();
    }
    // if parent is a QAction, become a submenu
    action = qobject_cast<QAction *>(parent);
    if (action) {
        action->setMenu(m_menu);
        m_menu->clear();
        for (MenuItem *item : std::as_const(m_items)) {
            if (item->section()) {
                if (!item->visible()) {
                    continue;
                }

                m_menu->addSection(item->text());
            } else {
                m_menu->addAction(item->action());
            }
        }
        m_menu->updateGeometry();
    }

    m_visualParent = parent;
    Q_EMIT visualParentChanged();
}

QWindow *MenuExtension::transientParent()
{
    if (!m_menu) {
        return nullptr;
    }
    return m_menu->windowHandle()->transientParent();
}

void MenuExtension::setTransientParent(QWindow *parent)
{
    if (parent == m_menu->windowHandle()->transientParent()) {
        return;
    }

    m_menu->windowHandle()->setTransientParent(parent);
    Q_EMIT transientParentChanged();
}

Plasma::Types::PopupPlacement MenuExtension::placement() const
{
    return m_placement;
}

void MenuExtension::setPlacement(Plasma::Types::PopupPlacement placement)
{
    if (m_placement != placement) {
        m_placement = placement;

        Q_EMIT placementChanged();
    }
}

int MenuExtension::minimumWidth() const
{
    return m_menu->minimumWidth();
}

void MenuExtension::setMinimumWidth(int width)
{
    if (m_menu->minimumWidth() != width) {
        m_menu->setMinimumWidth(width);

        Q_EMIT minimumWidthChanged();
    }
}

int MenuExtension::maximumWidth() const
{
    return m_menu->maximumWidth();
}

void MenuExtension::setMaximumWidth(int width)
{
    if (m_menu->maximumWidth() != width) {
        m_menu->setMaximumWidth(width);

        Q_EMIT maximumWidthChanged();
    }
}

void MenuExtension::resetMaximumWidth()
{
    setMaximumWidth(QWIDGETSIZE_MAX);
}

bool MenuExtension::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::ChildAdded: {
        QChildEvent *ce = static_cast<QChildEvent *>(event);
        MenuItem *mi = qobject_cast<MenuItem *>(ce->child());
        // FIXME: linear complexity here
        if (mi && !m_items.contains(mi)) {
            if (mi->separator()) {
                m_menu->addSection(mi->text());
            } else {
                m_menu->addAction(mi->action());
            }
            m_items << mi;
        }
        break;
    }

    case QEvent::ChildRemoved: {
        QChildEvent *ce = static_cast<QChildEvent *>(event);
        MenuItem *mi = qobject_cast<MenuItem *>(ce->child());

        // FIXME: linear complexity here
        if (mi) {
            m_menu->removeAction(mi->action());
            m_items.removeAll(mi);
        }
        break;
    }

    default:
        break;
    }

    return QObject::event(event);
}

void MenuExtension::clearMenuItems()
{
    qDeleteAll(m_items);
    m_items.clear();
}

void MenuExtension::addMenuItem(const QString &text)
{
    MenuItem *item = new MenuItem();
    item->setText(text);
    m_menu->addAction(item->action());
    m_items << item;
}

void MenuExtension::addMenuItem(MenuItem *item, MenuItem *before)
{
    if (before) {
        if (m_items.contains(item)) {
            m_menu->removeAction(item->action());
            m_items.removeAll(item);
        }

        m_menu->insertAction(before->action(), item->action());

        const int index = m_items.indexOf(before);

        if (index != -1) {
            m_items.insert(index, item);
        } else {
            m_items << item;
        }

    } else if (!m_items.contains(item)) {
        m_menu->addAction(item->action());
        m_items << item;
    }
    connect(item, &MenuItem::destroyed, this, [this, item]() {
        removeMenuItem(item);
    });
}

void MenuExtension::addSection(const QString &text)
{
    m_menu->addSection(text);
}

void MenuExtension::removeMenuItem(MenuItem *item)
{
    if (!item) {
        return;
    }

    m_menu->removeAction(item->action());
    m_items.removeOne(item);
}

void MenuExtension::itemTriggered(QAction *action)
{
    for (int i = 0; i < m_items.count(); ++i) {
        MenuItem *item = m_items.at(i);
        if (item->action() == action) {
            Q_EMIT triggered(item);
            Q_EMIT triggeredIndex(i);
            break;
        }
    }
}

void MenuExtension::rebuildMenu()
{
    m_menu->clear();

    for (MenuItem *item : std::as_const(m_items)) {
        if (item->section()) {
            if (!item->visible()) {
                continue;
            }

            m_menu->addSection(item->text());
        } else {
            m_menu->addAction(item->action());
            if (item->action()->menu()) {
                // This ensures existence of the QWindow
                m_menu->winId();
                item->action()->menu()->winId();
                item->action()->menu()->windowHandle()->setTransientParent(m_menu->windowHandle());
            }
        }
    }

    m_menu->adjustSize();
}

void MenuExtension::open(int x, int y)
{
    qDebug() << "Opening menu at" << x << y;

    QQuickItem *parentItem = nullptr;

    if (m_visualParent) {
        parentItem = qobject_cast<QQuickItem *>(m_visualParent.data());
    } else {
        parentItem = qobject_cast<QQuickItem *>(parent());
    }

    if (!parentItem) {
        return;
    }

    rebuildMenu();

    QPointF pos = parentItem->mapToScene(QPointF(x, y));

    if (parentItem->window() && parentItem->window()->screen()) {
        pos = parentItem->window()->mapToGlobal(pos.toPoint());
    }

    openInternal(pos.toPoint());
}

int MenuExtension::openRelative()
{
    qWarning() << "open relative";

    QQuickItem *parentItem = nullptr;

    if (m_visualParent) {
        parentItem = qobject_cast<QQuickItem *>(m_visualParent.data());
    } else {
        parentItem = qobject_cast<QQuickItem *>(parent());
    }

    if (!parentItem) {
        return 5;
    }

    rebuildMenu();

    QPointF pos;

    using namespace Plasma;

    auto boundaryCorrection = [&pos, this, parentItem](int hDelta, int vDelta) {
        if (!parentItem->window()) {
            return;
        }
        QScreen *screen = parentItem->window()->screen();
        if (!screen) {
            return;
        }
        QRect geo = screen->geometry();
        pos = parentItem->window()->mapToGlobal(pos.toPoint());

        if (pos.x() < geo.x()) {
            pos.setX(pos.x() + hDelta);
        }
        if (pos.y() < geo.y()) {
            pos.setY(pos.y() + vDelta);
        }

        if (geo.x() + geo.width() < pos.x() + this->m_menu->width()) {
            pos.setX(pos.x() + hDelta);
        }
        if (geo.y() + geo.height() < pos.y() + this->m_menu->height()) {
            pos.setY(pos.y() + vDelta);
        }
    };

    switch (m_placement) {
    case Types::TopPosedLeftAlignedPopup: {
        pos = parentItem->mapToScene(QPointF(0, -m_menu->height()));
        boundaryCorrection(-m_menu->width() + parentItem->width(), m_menu->height() + parentItem->height());
        break;
    }
    case Types::LeftPosedTopAlignedPopup: {
        pos = parentItem->mapToScene(QPointF(-m_menu->width(), 0));
        boundaryCorrection(m_menu->width() + parentItem->width(), -m_menu->height() + parentItem->height());
        break;
    }
    case Types::TopPosedRightAlignedPopup:
        pos = parentItem->mapToScene(QPointF(parentItem->width() - m_menu->width(), -m_menu->height()));
        boundaryCorrection(m_menu->width() - parentItem->width(), m_menu->height() + parentItem->height());
        break;
    case Types::RightPosedTopAlignedPopup: {
        pos = parentItem->mapToScene(QPointF(parentItem->width(), 0));
        boundaryCorrection(-m_menu->width() - parentItem->width(), -m_menu->height() + parentItem->height());
        break;
    }
    case Types::LeftPosedBottomAlignedPopup:
        pos = parentItem->mapToScene(QPointF(-m_menu->width(), -m_menu->height() + parentItem->height()));
        boundaryCorrection(m_menu->width() + parentItem->width(), m_menu->height() - parentItem->height());
        break;
    case Types::BottomPosedLeftAlignedPopup: {
        pos = parentItem->mapToScene(QPointF(0, parentItem->height()));
        boundaryCorrection(-m_menu->width() + parentItem->width(), -m_menu->height() - parentItem->height());
        break;
    }
    case Types::BottomPosedRightAlignedPopup: {
        pos = parentItem->mapToScene(QPointF(parentItem->width() - m_menu->width(), parentItem->height()));
        boundaryCorrection(m_menu->width() - parentItem->width(), -m_menu->height() - parentItem->height());
        break;
    }
    case Types::RightPosedBottomAlignedPopup: {
        pos = parentItem->mapToScene(QPointF(parentItem->width(), -m_menu->height() + parentItem->height()));
        boundaryCorrection(-m_menu->width() - parentItem->width(), m_menu->height() - parentItem->height());
        break;
    }
    default:
        open();
        return 3;
    }

    openInternal(pos.toPoint());
    return 1;
}

void MenuExtension::openInternal(QPoint pos)
{
    QQuickItem *parentItem = this->parentItem();

    if (parentItem && parentItem->window()) {
        // create the QWindow
        m_menu->winId();
        m_menu->windowHandle()->setTransientParent(parentItem->window());

        // Workaround for QTBUG-59044
        auto ungrabMouseHack = [this]() {
            QQuickItem *parentItem = this->parentItem();
            if (parentItem && parentItem->window() && parentItem->window()->mouseGrabberItem()) {
                parentItem->window()->mouseGrabberItem()->ungrabMouse();
            }
        };

        // pre 5.8.0 QQuickWindow code is "item->grabMouse(); sendEvent(item, mouseEvent)"
        // post 5.8.0 QQuickWindow code is sendEvent(item, mouseEvent); item->grabMouse()
        if (QVersionNumber::fromString(QString::fromLatin1(qVersion())) > QVersionNumber(5, 8, 0)) {
            QTimer::singleShot(0, this, ungrabMouseHack);
        } else {
            ungrabMouseHack();
        }
        // end workaround
    }

    m_menu->popup(pos);
    m_status = Open;
    Q_EMIT statusChanged();
}

QQuickItem *MenuExtension::parentItem() const
{
    if (m_visualParent) {
        return qobject_cast<QQuickItem *>(m_visualParent.data());
    }

    return qobject_cast<QQuickItem *>(parent());
}

void MenuExtension::close()
{
    m_menu->hide();
}
