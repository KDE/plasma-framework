 /*
 *   Copyright 2005 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 by Riccardo Iaconelli <riccardo@kde.org>
 *   Copyright 2008 by Ménard Alexis <darktears31@gmail.com>
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

#ifndef PLASMA_APPLET_P_H
#define PLASMA_APPLET_P_H

#include <QBasicTimer>

#include <kactioncollection.h>
#include <kconfigdialog.h>
#include <kconfigskeleton.h>
#include <kservice.h>
#include <kplugininfo.h>

#include "plasma/applet.h"

class KKeySequenceWidget;

namespace Plasma
{

class AppletScript;
class Service;

class AppletConfigDialog : public KConfigDialog
{
    Q_OBJECT

public:
    AppletConfigDialog(QWidget* parent, const QString &id, KConfigSkeleton *s)
        : KConfigDialog(parent, id, s),
          m_changed(false)
    {
    }

public Q_SLOTS:
    void settingsModified(bool modified = true)
    {
        m_changed = modified;
        updateButtons();
    }

protected:
    bool hasChanged()
    {
        return m_changed || KConfigDialog::hasChanged();
    }

private:
    bool m_changed;
};

class PLASMA_EXPORT AppletPrivate
{
public:
    AppletPrivate(KService::Ptr service, const KPluginInfo *info, int uniqueID, Applet *applet);
    virtual ~AppletPrivate();

    void init(const QString &packagePath = QString());

    // the interface
    virtual void showConfigurationRequiredMessage(bool show, const QString &reason);
    virtual void cleanUpAndDelete();

    // put all setup routines for script here. at this point we can assume that
    // package exists and that we have a script engin
    void setupPackage();

    /**
     * Sets whether or not this Applet is acting as a Containment
     */
    void setIsContainment(bool isContainment, bool forceUpdate = false);

    QString globalName() const;
    void scheduleConstraintsUpdate(Plasma::Constraints c);
    void scheduleModificationNotification();
    KConfigGroup *mainConfigGroup();
    void resetConfigurationObject();
    void addGlobalShortcutsPage(KConfigDialog *dialog);
    void configDialogFinished();
    KConfigDialog *generateGenericConfigDialog();
    void addStandardConfigurationPages(KConfigDialog *dialog);
    QString configDialogId() const;
    QString configWindowTitle() const;
    void updateShortcuts();
    void globalShortcutChanged();
    void propagateConfigChanged();

    static KActionCollection* defaultActions(QObject *parent);


    static uint s_maxAppletId;

    //      number of members at this point.
    uint appletId;
    Applet *q;

    // applet attributes
    ImmutabilityType immutability;
    QString launchErrorMessage;

    // applet info we keep around in case its needed
    KPluginInfo appletDescription;
    QString customTitle;

    // bookkeeping
    KConfigGroup *mainConfig;
    Plasma::Constraints pendingConstraints;

    // sripting and package stuff
    AppletScript *script;
    Package *package;
    ConfigLoader *configLoader;

    // actions stuff; put activationAction into actions?
    KActionCollection *actions;
    KAction *activationAction;

    // configuration
    QWeakPointer<KKeySequenceWidget> shortcutEditor; //TODO: subclass KConfigDialog and encapsulate this in there

    ItemStatus itemStatus;

    // timerEvent bookkeeping
    QBasicTimer constraintsTimer;
    QBasicTimer *modificationsTimer;

    // a great green field of booleans :)
    bool hasConfigurationInterface : 1;
    bool isContainment : 1;
    bool failed : 1;
    bool transient : 1;
    bool needsConfig : 1;
    bool started : 1;
};

} // Plasma namespace

#endif
