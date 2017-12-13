/*
 *   Copyright 2013 Marco Martin <mart@kde.org>
 *   Copyright 2015 Eike Hein <hein@kde.org>
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

#ifndef CONFIGCATEGORY_P_H
#define CONFIGCATEGORY_P_H

#include <QObject>

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

namespace PlasmaQuick
{

//This class represents a single row item of the ConfigModel model in a QML friendly manner.
//the properties contains all the data needed to represent an icon in the sidebar of a configuration dialog, of applets or containments
class ConfigCategory : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY iconChanged)
    Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QString pluginName READ pluginName WRITE setPluginName NOTIFY pluginNameChanged)
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)

public:
    ConfigCategory(QObject *parent = nullptr);
    ~ConfigCategory();

    QString name() const;
    void setName(const QString &name);

    QString icon() const;
    void setIcon(const QString &icon);

    QString source() const;
    void setSource(const QString &source);

    QString pluginName() const;
    void setPluginName(const QString &pluginName);

    bool visible() const;
    void setVisible(bool visible);

Q_SIGNALS:
    void nameChanged();
    void iconChanged();
    void sourceChanged();
    void pluginNameChanged();
    void visibleChanged();

private:
    QString m_name;
    QString m_icon;
    QString m_source;
    QString m_pluginName;
    bool m_visible;
};

}

#endif // multiple inclusion guard
