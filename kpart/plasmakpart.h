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
Q_DECLARE_METATYPE( Plasma::Containment* );
namespace Plasma
{
    class Applet;
    class PluginLoader;
}

#include <KDE/KParts/Part>
#include <KDE/KService>
class KDialog;

#include <QtCore/QHash>
class QVariant;
class QVBoxLayout;

class PlasmaKPart : public KParts::ReadOnlyPart
{
Q_OBJECT
Q_PROPERTY( Plasma::Applet::List activeApplets READ listActiveApplets );
public:
    PlasmaKPart(QWidget* parentWidget, QObject* parent, const QVariantList&);
    ~PlasmaKPart();

    void notifyStartup(bool completed);

    PlasmaKPartCorona* corona();
    Plasma::Containment* containment();
    QString category();

public Q_SLOTS:
    /**
     * Add an applet to the currently running containment.
     *
     * @short Add an applet to the containment
     * @author Ryan Rix <ry@n.rix.si>
     * @version 0.01
     **/
    void addApplet(const QString& pluginName, const QVariantList& args = QVariantList(), const QRectF& dimensions = QRectF());

    /**
     * Sets the application-specific plugin loader. This allows
     * applications which need to add internal applets (such
     * as existing QWidget-based dashboard plugins), services or
     * data engines to the Plasma dashboard. The preferred way to
     * set this loader is by passing it to the KPart wrapped in a
     * QVariant in the @p args parameter of the KPart constructor.
     * This method is provided for applications which cannot set
     * the loader in this method.
     * The method will return false if Plasma already has a
     * PluginLoader in memory, and will return true if the PluginLoader
     * is successfully set.
     *
     * @param loader The loader which you want Plasma to query for
     *               new Applets, Data Engines and Services.
     * @return True if the loader was successfully set, false otherwise
     *         (If Plasma already has a PluginLoader in memory)
     *
     * @short Set application-specific plugin loader
     **/
    bool setPluginLoader( Plasma::PluginLoader* loader );

    /**
     * Returns a list of active applets in the containment.
     *
     * @return A list of the containment's Applets
     **/
    Plasma::Applet::List listActiveApplets( );

private Q_SLOTS:
    void cleanup();
    void syncConfig();
    void createView(Plasma::Containment* containment);
    void setThemeDefaults();

private:
    PlasmaKPartCorona* m_corona;
    PlasmaKPartView* m_view;
    QString m_category;
    KService::Ptr m_service;
    QHash<QString,Plasma::Applet*>* m_appletList;
    QVBoxLayout* m_configLayout;

Q_SIGNALS:
    void viewCreated();
};

#endif // multiple inclusion guard

