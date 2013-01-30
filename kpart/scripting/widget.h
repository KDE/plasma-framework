/*
 *   Copyright 2010 Aaron Seigo <aseigo@kde.org>
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

#ifndef APPLET
#define APPLET

#include <QObject>
#include <QRectF>
#include <QVariant>

namespace Plasma
{
    class Applet;
} // namespace Plasma

namespace PlasmaKPartScripting
{

class Widget : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString type READ type)
    Q_PROPERTY(QString version READ version)
    Q_PROPERTY(int id READ id)
    Q_PROPERTY(QStringList configKeys READ configKeys)
    Q_PROPERTY(QStringList configGroups READ configGroups)
    Q_PROPERTY(QStringList globalConfigKeys READ globalConfigKeys)
    Q_PROPERTY(QStringList globalConfigGroups READ globalConfigGroups)
    //Q_PROPERTY(int index WRITE setIndex READ index)
    Q_PROPERTY(QRectF geometry WRITE setGeometry READ geometry)
    Q_PROPERTY(QStringList currentConfigGroup WRITE setCurrentConfigGroup READ currentConfigGroup)

public:
    explicit Widget(Plasma::Applet *applet, QObject *parent = 0);
    ~Widget();

    QStringList configKeys() const;
    QStringList configGroups() const;

    void setCurrentConfigGroup(const QStringList &groupNames);
    QStringList currentConfigGroup() const;

    QStringList globalConfigKeys() const;
    QStringList globalConfigGroups() const;

    void setCurrentGlobalConfigGroup(const QStringList &groupNames);
    QStringList currentGlobalConfigGroup() const;

    QRectF geometry() const;
    void setGeometry(const QRectF &geometry);

    QString version() const;

    uint id() const;
    QString type() const;

    /**
FIXME: what should the index(es?) be given that we're in the newspaper containment
    int index() const;
    void setIndex(int index);
    */

    virtual Plasma::Applet *applet() const;

public Q_SLOTS:
    virtual QVariant readConfig(const QString &key, const QVariant &def = QString()) const;
    virtual void writeConfig(const QString &key, const QVariant &value);
    virtual QVariant readGlobalConfig(const QString &key, const QVariant &def = QString()) const;
    virtual void writeGlobalConfig(const QString &key, const QVariant &value);
    virtual void reloadConfig();
    void remove();
    void showConfigurationInterface();

private:
    class Private;
    Private * const d;
};

}

#endif

