/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef APPLETBROWSERWINDOW_H_
#define APPLETBROWSERWINDOW_H_

#include <KDE/KDialog>

#include <plasma/plasma_export.h>

namespace Plasma
{

class Corona;
class Containment;
class Applet;
class AppletBrowserPrivate;
class AppletBrowserWidgetPrivate;

class PLASMA_EXPORT AppletBrowserWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AppletBrowserWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~AppletBrowserWidget();

    void setApplication(const QString& application = QString());
    QString application();

    /**
     * Changes the current default containment to add applets to
     *
     * @arg containment the new default
     */
    void setContainment(Plasma::Containment *containment);

    /**
     * @return the current default containment to add applets to
     */
    Containment* containment() const;

public Q_SLOTS:
    /**
     * Adds currently selected applets
     */
    void addApplet();

    /**
     * Destroy all applets with this name
     */
    void destroyApplets(const QString &name);

    /**
     * Launches a download dialog to retrieve new applets from the Internet
     */
    void downloadWidgets();

    /**
     * Opens a file dialog to open a widget from a local file
     */
    void openWidgetFile();

private:
    Q_PRIVATE_SLOT(d, void appletAdded(Plasma::Applet*))
    Q_PRIVATE_SLOT(d, void appletRemoved(Plasma::Applet*))

    AppletBrowserWidgetPrivate * const d;
};

class PLASMA_EXPORT AppletBrowser: public KDialog
{
    Q_OBJECT
public:
    explicit AppletBrowser(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~AppletBrowser();

    void setApplication(const QString& application = QString());
    QString application();

    /**
     * Changes the current default containment to add applets to
     *
     * @arg containment the new default
     */
    void setContainment(Plasma::Containment *containment);

    /**
     * @return the current default containment to add applets to
     */
    Containment* containment() const;

private:
    AppletBrowserPrivate * const d;
};

} // namespace Plasma

#endif /*APPLETBROWSERWINDOW_H_*/
