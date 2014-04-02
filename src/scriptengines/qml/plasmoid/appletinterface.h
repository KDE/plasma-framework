/*
 *   Copyright 2008 Chani Armitage <chani@kde.org>
 *   Copyright 2008, 2009 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2010 Marco Martin <mart@kde.org>
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

#ifndef APPLETINTERFACE_H
#define APPLETINTERFACE_H

#include <QQuickItem>
#include <QScriptValue>
#include <QQuickView>

#include <Plasma/Applet>
#include <Plasma/Theme>

#include <appletquickitem.h>
#include "declarativeappletscript.h"

class QAction;
class QmlAppletScript;
class QSignalMapper;
class QSizeF;

class ConfigView;

namespace KDeclarative {
    class ConfigPropertyMap;
    class QmlObject;
}

namespace Plasma
{
    class ConfigLoader;
} // namespace Plasma

class AppletInterface : public PlasmaQuick::AppletQuickItem
{
    Q_OBJECT

    /**
     * The QML root object defined in the applet main.qml will be direct child of an AppletInterface instance
     */

    /**
     * Version of the QML2 script engine
     */
    Q_PROPERTY(int apiVersion READ apiVersion CONSTANT)

    /**
     * Plugin name of the plasmoid
     */
    Q_PROPERTY(QString pluginName READ pluginName CONSTANT)

    /**
     * User friendly title for the plasmoid: it's the localized applet name by default
     */
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)

    /**
     * Main title for the plasmoid tooltip or other means of quick information:
     * it's the same as the title property by default, but it can be personalized
     */
    Q_PROPERTY(QString toolTipMainText READ toolTipMainText WRITE setToolTipMainText NOTIFY toolTipMainTextChanged)

    /**
     * Description for the plasmoid tooltip or other means of quick information:
     * it comes from the pluginifo comment by default, but it can be personalized
     */
    Q_PROPERTY(QString toolTipSubText READ toolTipSubText WRITE setToolTipSubText NOTIFY toolTipSubTextChanged)

    /**
     * Icon to represent the plasmoid
     */
    Q_PROPERTY(QString icon READ icon WRITE setIcon NOTIFY iconChanged)

    /**
     * Applet id: is unique in the whole Plasma session and will never change across restarts
     */
    Q_PROPERTY(uint id READ id CONSTANT)

    /**
     * FormFactor for the plasmoid
     */
    Q_PROPERTY(Plasma::Types::FormFactor formFactor READ formFactor NOTIFY formFactorChanged)

    /**
     * Location for the plasmoid
     */
    Q_PROPERTY(Plasma::Types::Location location READ location NOTIFY locationChanged)

    /**
     * Current activity name the plasmoid is in
     */
    Q_PROPERTY(QString currentActivity READ currentActivity NOTIFY contextChanged)

    /**
     * Configuration object: each config key will be a writable property of this object. property bindings work.
     */
    Q_PROPERTY(QObject* configuration READ configuration CONSTANT)

    /**
     * When true the plasmoid is busy. The containment may graphically indicate that drawing for instance a spinner busy widget over it
     */
    Q_PROPERTY(bool busy WRITE setBusy READ isBusy NOTIFY busyChanged)

    /**
     * How the applet wants its background to be drawn. The containment may chose to ignore this hint.
     */
    Q_PROPERTY(Plasma::Types::BackgroundHints backgroundHints WRITE setBackgroundHints READ backgroundHints NOTIFY backgroundHintsChanged)

    /**
     * The immutability state of the Corona. the plasmoid implementation should avoid allowing "dangerous" modifications from the user when in an immutable mode
     */
    Q_PROPERTY(bool immutable READ immutable NOTIFY immutableChanged)

    /**
     * True when the user is configuring, for instance when the configuration dialog is open.
     */
    Q_PROPERTY(bool userConfiguring READ userConfiguring NOTIFY userConfiguringChanged)

    /**
     * Status of the plasmoid: useful to instruct the shell if this plasmoid is requesting attention, if is accepting input, or if is in an idle, inactive state
     */
    Q_PROPERTY(Plasma::Types::ItemStatus status READ status WRITE setStatus NOTIFY statusChanged)

    /**
     * Sets the associated application of this plasmoid, if the plasmoid is representing the "compact" view for some application or for some document type. 
     * TODO: a way to set associated application urls.
     */
    Q_PROPERTY(QString associatedApplication WRITE setAssociatedApplication READ associatedApplication)

    // TODO: This was moved up from ContainmentInterface because it is required by the
    // Task Manager applet (for "Show only tasks from this screen") and no Qt API exposes
    // screen numbering. An alternate solution that doesn't extend the applet interface
    // would be preferrable if found.
    Q_PROPERTY(int screen READ screen NOTIFY screenChanged)

    /**
     * Whether the dialog should be hidden when the dialog loses focus.
     *
     * The default value is @c false.
     **/
    Q_PROPERTY(bool hideOnWindowDeactivate READ hideOnWindowDeactivate WRITE setHideOnWindowDeactivate NOTIFY hideOnWindowDeactivateChanged)

    /**
     * The global shortcut to activate the plasmoid
     *
     * This is typically only used by the default configuration module
     *
     */
    Q_PROPERTY(QKeySequence globalShortcut READ globalShortcut WRITE setGlobalShortcut NOTIFY globalShortcutChanged)

public:
    AppletInterface(DeclarativeAppletScript *script, QQuickItem *parent = 0);
    AppletInterface(Plasma::Applet *applet, QQuickItem *parent = 0);
    ~AppletInterface();

//API not intended for the QML part

    DeclarativeAppletScript *appletScript() const;

    QList<QAction*> contextualActions() const;

    void executeAction(const QString &name);

//QML API-------------------------------------------------------------------

    /**
     * Set this to true if the plasmoid needs to be configured in order to work. The containment will display reason as a message to ask the user to configure.
     * @param needsConfiguring If the plasmoid needs configuration
     * @param reason The user readable (and localized) reason the plasmoid needs
     */
    Q_INVOKABLE void setConfigurationRequired(bool needsConfiguring, const QString &reason = QString());

    Q_INVOKABLE void setActionSeparator(const QString &name);
    Q_INVOKABLE void setAction(const QString &name, const QString &text,
                               const QString &icon = QString(), const QString &shortcut = QString());

    Q_INVOKABLE void removeAction(const QString &name);

    Q_INVOKABLE QAction *action(QString name) const;

    /**
     * FIXME: remove?
     * Retrieve the path of a file from the Plasmoid package
     * @param fileName the package-recognized name, such as "mainscript"
     * @returns the full absolute path of the file, if found, an empty string if not
     */
    Q_INVOKABLE QString file(const QString &fileName);

    /**
     * FIXME: remove?
     * Retrieve the path of a file from the Plasmoid package
     * @param fileType the type supported from the package, such as "ui", "config" or "image"
     * @param filePath the name of the file, such as "foo.qml" or "bar.png"
     * @returns the full absolute path of the file, if found, an empty string if not
     */
    Q_INVOKABLE QString file(const QString &fileType, const QString &filePath);

    /**
     * @returns A path where is safe to write on disk downloaded files
     */
    Q_INVOKABLE QString downloadPath(const QString &file);

    /**
     * @returns The list of files that have been downloaded
     */
    Q_INVOKABLE QStringList downloadedFiles() const;

    static AppletInterface *qmlAttachedProperties(QObject *object)
    {
        return qobject_cast<AppletInterface *>(AppletQuickItem::qmlAttachedProperties(object));
    }

//PROPERTY ACCESSORS-------------------------------------------------------------------
    QString pluginName() const;

    QString icon() const;
    void setIcon(const QString &icon);

    QString title() const;
    void setTitle(const QString &title);

    QString toolTipMainText() const;
    void setToolTipMainText(const QString &text);

    QString toolTipSubText() const;
    void setToolTipSubText(const QString &text);

    uint id() const;

    Plasma::Types::FormFactor formFactor() const;

    Plasma::Types::Location location() const;

    QString currentActivity() const;

    QObject* configuration() const;

    bool isBusy() const;
    void setBusy(bool busy);

    Plasma::Types::BackgroundHints backgroundHints() const;
    void setBackgroundHints(Plasma::Types::BackgroundHints hint);

    void setAssociatedApplication(const QString &string);
    QString associatedApplication() const;

    void setStatus(const Plasma::Types::ItemStatus &status);
    Plasma::Types::ItemStatus status() const;

    int screen() const;

    bool immutable() const;
    bool userConfiguring() const;
    int apiVersion() const;

    bool hideOnWindowDeactivate() const;
    void setHideOnWindowDeactivate(bool hide);

    QKeySequence globalShortcut() const;
    void setGlobalShortcut(const QKeySequence &keySequence);

Q_SIGNALS:
    /**
     * somebody else, usually the containment sent some data to the applet
     * @param mimetype the mime type of the data such as text/plain
     * @param data either the actual data or an URL representing it
     */
    void externalData(const QString &mimetype, const QVariant &data);

    void configNeedsSaving();


//PROPERTY change notifiers--------------
    void iconChanged();
    void titleChanged();
    void toolTipMainTextChanged();
    void toolTipSubTextChanged();
    void formFactorChanged();
    void locationChanged();
    void contextChanged();
    void immutableChanged();
    void statusChanged();
    void backgroundHintsChanged();
    void busyChanged();
    void screenChanged();
    void hideOnWindowDeactivateChanged();

    void userConfiguringChanged();
    void globalShortcutChanged();

protected Q_SLOTS:
    virtual void init();

private:

    QStringList m_actions;
    QSignalMapper *m_actionSignals;

    KDeclarative::ConfigPropertyMap *m_configuration;
    DeclarativeAppletScript *m_appletScriptEngine;

//UI-specific members ------------------


    QString m_toolTipMainText;
    QString m_toolTipSubText;
    Plasma::Types::BackgroundHints m_backgroundHints;
    bool m_busy : 1;
    bool m_hideOnDeactivate : 1;
    friend class ContainmentInterface;
};

QML_DECLARE_TYPEINFO(AppletInterface, QML_HAS_ATTACHED_PROPERTIES)

#endif
