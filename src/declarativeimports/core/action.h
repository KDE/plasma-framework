/*
    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMAACTION_H
#define PLASMAACTION_H

#include <QAction>
#include <QActionGroup>
#include <QObject>
#include <qqml.h>
#include <qtclasshelpermacros.h>
#include <qtmetamacros.h>

class QQuickItem;

namespace PlasmaQuick
{
class SharedQmlEngine;
}

class ActionExtension;

class IconGroup : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon NOTIFY iconChanged)
public:
    explicit IconGroup(ActionExtension *parent = nullptr);
    ~IconGroup();

    void setName(const QString &name);
    QString name() const;

    QIcon icon() const;
    void setIcon(const QIcon &icon);

Q_SIGNALS:
    void nameChanged();
    void iconChanged();

private:
    QAction *m_action;
    QString m_name;
};

class ActionExtension : public QObject
{
    Q_OBJECT
    Q_PROPERTY(IconGroup *icon MEMBER m_iconGroup CONSTANT)
    Q_PROPERTY(bool isSeparator READ isSeparator WRITE setSeparator NOTIFY isSeparatorChanged)
    Q_PROPERTY(QActionGroup *actionGroup READ actionGroup WRITE setActionGroup NOTIFY actionGroupChanged)
    Q_PROPERTY(QVariant shortcut READ shortcut WRITE setShortcut NOTIFY shortcutChanged)
    Q_PROPERTY(QMenu *menu READ menu WRITE setMenu NOTIFY menuChanged)

public:
    explicit ActionExtension(QObject *parent = nullptr);
    ~ActionExtension() override;

    bool isSeparator() const;
    void setSeparator(bool setSeparator);

    void setActionGroup(QActionGroup *group);
    QActionGroup *actionGroup() const;

    void setShortcut(const QVariant &var);
    QVariant shortcut() const;

    void setMenu(QMenu *menu);
    QMenu *menu();

    QAction *action() const;

Q_SIGNALS:
    void isSeparatorChanged();
    void actionGroupChanged();
    void shortcutChanged();
    void menuChanged();

private:
    QAction *m_action;
    IconGroup *m_iconGroup;
    QString m_icon;
};

class ActionGroup : public QActionGroup
{
    Q_OBJECT
public:
    ActionGroup(QObject *parent = nullptr)
        : QActionGroup(parent)
    {
    }
};

#endif
