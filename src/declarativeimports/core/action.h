/*
    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PLASMAACTION_H
#define PLASMAACTION_H

#include <QAction>
#include <QObject>
#include <qqml.h>
#include <qtclasshelpermacros.h>

class QQuickItem;

namespace PlasmaQuick
{
class SharedQmlEngine;
}

class IconGroup : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name MEMBER m_name NOTIFY nameChanged)
public:
    explicit IconGroup(QObject *parent = nullptr);
Q_SIGNALS:
    void nameChanged();

private:
    QString m_name;
};

class ActionExtension : public QObject
{
    Q_OBJECT
    Q_PROPERTY(IconGroup *icon MEMBER m_iconGroup CONSTANT)

public:
    explicit ActionExtension(QObject *parent = nullptr);
    ~ActionExtension() override;

Q_SIGNALS:
    void iconAChanged();

private:
    QAction *m_action;
    IconGroup *m_iconGroup;
    QString m_icon;
};

struct QActionForeign {
    Q_GADGET
    QML_FOREIGN(QAction)
    QML_ELEMENT
    QML_EXTENDED(ActionExtension)
};

#endif
