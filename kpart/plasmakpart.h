/*
 *   Copyright 2006-2008 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
 *   Copyright 2010 Ryan Rix <ry@n.rix.si>
 *   Copyright 2010 Siddharth Sharma <siddharth.kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
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

#ifndef PLASMAKPART_H
#define PLASMAKPART_H

class PlasmaKPartView;
class PlasmaKPartCorona;

#include <Plasma/Containment>
Q_DECLARE_METATYPE(Plasma::Containment *)
namespace Plasma
{
    class Applet;
    class PluginLoader;
}

#include <KDE/KParts/Part>
#include <KDE/KService>

#include <QtCore/QHash>
class QVariant;
class QVBoxLayout;

class PlasmaKPart : public KParts::ReadOnlyPart
{
    Q_OBJECT
    Q_PROPERTY(Plasma::Applet::List activeApplets READ listActiveApplets)

public:
    /**
     * The default constructor.
     * The args may contain a pointer to a Plasma::PluginLoader as the first parameter.
     * Note that only one Plasma::PluginLoader can be active at a time, and that the
     * prefered mechanism for registering the plugin loader is via
     * Plasma::PluginLoader::setPluginLoader
     */
    PlasmaKPart(QWidget *parentWidget, QObject *parent, const QVariantList &args);
    ~PlasmaKPart();

    void notifyStartup(bool completed);

    PlasmaKPartCorona *corona() const;
    Plasma::Containment *containment() const;

    /**
     * Returns a list of active applets in the containment.
     *
     * @return A list of the containment's Applets
     **/
    Plasma::Applet::List listActiveApplets() const;

public Q_SLOTS:
    /**
     * Add an applet to the currently running containment.
     *
     * @short Add an applet to the containment
     * @author Ryan Rix <ry@n.rix.si>
     * @version 0.01
     **/
    void addApplet(const QString &pluginName, const QVariantList &args = QVariantList(), const QRectF &dimensions = QRectF());

Q_SIGNALS:
    void viewCreated();

private:
    void initCorona();

private Q_SLOTS:
    void cleanup();
    void syncConfig();
    void createView(Plasma::Containment* containment);
    void setThemeDefaults();

private:
    PlasmaKPartCorona* m_corona;
    PlasmaKPartView* m_view;
    KService::Ptr m_service;
    QHash<QString,Plasma::Applet*>* m_appletList;
    QVBoxLayout* m_configLayout;
};

#endif // multiple inclusion guard

