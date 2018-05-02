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
#include <QQuickView>

#include <Plasma/Applet>
#include <Plasma/Theme>

#include <appletquickitem.h>
#include "declarativeappletscript.h"

class QAction;
class QmlAppletScript;
class QSizeF;

namespace KDeclarative
{
class ConfigPropertyMap;
class QmlObject;
}

namespace Plasma
{
class ConfigLoader;
} // namespace Plasma

/**
 * @class AppletInterface
 *
 * @short This class is exposed to applets in QML as the attached property Plasmoid
 *
 * \@import org.kde.plasma.Plasmoid
 */
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
     * how to handle the text format of the tooltip subtext:
     * * Text.AutoText (default)
     * * Text.PlainText
     * * Text.StyledText
     * * Text.RichText
     * Note: in the default implementation the main text is always plain text
     */
    Q_PROPERTY(int toolTipTextFormat READ toolTipTextFormat WRITE setToolTipTextFormat NOTIFY toolTipTextFormatChanged)

    /**
     * This allows to set custom full QML items as the tooltip.
     * It will ignore all texts set by setToolTipMainText or setToolTipSubText
     *
     * @since: 5.19
     */
    Q_PROPERTY(QQuickItem *toolTipItem READ toolTipItem WRITE setToolTipItem NOTIFY toolTipItemChanged)


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
    Q_PROPERTY(QObject *configuration READ configuration CONSTANT)

    /**
     * When true the plasmoid is busy. The containment may graphically indicate that drawing for instance a spinner busy widget over it
     */
    Q_PROPERTY(bool busy WRITE setBusy READ isBusy NOTIFY busyChanged)

    /**
     * How the applet wants its background to be drawn. The containment may chose to ignore this hint.
     */
    Q_PROPERTY(Plasma::Types::BackgroundHints backgroundHints WRITE setBackgroundHints READ backgroundHints NOTIFY backgroundHintsChanged)

    /**
     * Whether the Corona is immutable. The plasmoid implementation should avoid allowing "dangerous" modifications from the user when in an immutable mode
     *
     * This is true when immutability is not Mutable
     */
    Q_PROPERTY(bool immutable READ immutable NOTIFY immutabilityChanged)

    /**
     * The immutability of the Corona.
     *
     * Use this if you need more granular control than just using the immutable property
     *
     * @see immutable
     * @since 5.23
     */
    Q_PROPERTY(Plasma::Types::ImmutabilityType immutability READ immutability NOTIFY immutabilityChanged)

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
     */
    Q_PROPERTY(QString associatedApplication WRITE setAssociatedApplication READ associatedApplication NOTIFY associatedApplicationChanged)

    /**
     * Sets the associated application of this plasmoid, if the plasmoid is representing the "compact" view for some application or for some document type.
     */
    Q_PROPERTY(QList<QUrl> associatedApplicationUrls WRITE setAssociatedApplicationUrls READ associatedApplicationUrls NOTIFY associatedApplicationUrlsChanged)

    // TODO: This was moved up from ContainmentInterface because it is required by the
    // Task Manager applet (for "Show only tasks from this screen") and no Qt API exposes
    // screen numbering. An alternate solution that doesn't extend the applet interface
    // would be preferable if found.
    Q_PROPERTY(int screen READ screen NOTIFY screenChanged)

    /**
     * Provides access to the geometry of the applet is in.
     * Can be useful to figure out what's the absolute position of the applet.
     */
    Q_PROPERTY(QRect screenGeometry READ screenGeometry NOTIFY screenGeometryChanged)

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

    /**
     * An interface to the native C++ plasmoid, if implemented
     */
    Q_PROPERTY(QObject *nativeInterface READ nativeInterface CONSTANT)

    /**
     * If true the applet requires manual configuration from the user
     */
    Q_PROPERTY(bool configurationRequired READ configurationRequired WRITE setConfigurationRequiredProperty NOTIFY configurationRequiredChanged)

    /**
     * Reason why the manual user configuration is required
     */
    Q_PROPERTY(QString configurationRequiredReason READ configurationRequiredReason WRITE setConfigurationRequiredReason NOTIFY configurationRequiredReasonChanged)

    /**
     * screen area free of panels: the coordinates are relative to the containment,
     * it's independent from the screen position
     * For more precise available geometry use availableScreenRegion()
     */
    Q_PROPERTY(QRect availableScreenRect READ availableScreenRect NOTIFY availableScreenRectChanged)

    /**
     * The available region of this screen, panels excluded. It's a list of rectangles
     */
    Q_PROPERTY(QVariantList availableScreenRegion READ availableScreenRegion NOTIFY availableScreenRegionChanged)

public:
    AppletInterface(DeclarativeAppletScript *script, const QVariantList &args = QVariantList(), QQuickItem *parent = nullptr);
    ~AppletInterface() Q_DECL_OVERRIDE;

//API not intended for the QML part

    DeclarativeAppletScript *appletScript() const;

    QList<QAction *> contextualActions() const;

    void executeAction(const QString &name);

//QML API-------------------------------------------------------------------

    /**
     * Set this to true if the plasmoid needs to be configured in order to work. The containment will display reason as a message to ask the user to configure.
     * @param needsConfiguring If the plasmoid needs configuration
     * @param reason The user readable (and localized) reason the plasmoid needs
     */
    Q_INVOKABLE void setConfigurationRequired(bool needsConfiguring, const QString &reason = QString());

    Q_INVOKABLE void setActionSeparator(const QString &name);
    /**
     * Add an action to the Plasmoid contextual menu.
     * When the action is triggered a function called action_<name> will be called, if there is no function with that name actionTriggered(name) will be called instead.
     * @param: action name
     * @text: user visible displayed text
     * @icon: user visible optional displayed icon
     * @shortcut: shortcut to trigger this action
     */
    Q_INVOKABLE void setAction(const QString &name, const QString &text,
                               const QString &icon = QString(), const QString &shortcut = QString());

    Q_INVOKABLE void removeAction(const QString &name);

    Q_INVOKABLE void clearActions();

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
     * @returns A path where it is safe to write on disk downloaded files.
     * @since 5.23
     */
    Q_INVOKABLE QString downloadPath() const;

    /**
     * @returns A path where it is safe to write on disk downloaded files.
     * @param file that name of the file to download (unused).
     * @deprecated Use downloadPath() instead.
     */
    Q_INVOKABLE PLASMA_DEPRECATED QString downloadPath(const QString &file);

    /**
     * @returns The list of files that have been downloaded
     */
    Q_INVOKABLE QStringList downloadedFiles() const;

    QVariantList availableScreenRegion() const;

    QRect availableScreenRect() const;

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

    int toolTipTextFormat() const;
    void setToolTipTextFormat(int format);

    QQuickItem *toolTipItem() const;
    void setToolTipItem(QQuickItem *toolTipItem);

    uint id() const;

    Plasma::Types::FormFactor formFactor() const;

    Plasma::Types::Location location() const;

    QString currentActivity() const;

    QObject *configuration() const;

    bool isBusy() const;
    void setBusy(bool busy);

    Plasma::Types::BackgroundHints backgroundHints() const;
    void setBackgroundHints(Plasma::Types::BackgroundHints hint);

    void setAssociatedApplication(const QString &string);
    QString associatedApplication() const;

    void setAssociatedApplicationUrls(const QList<QUrl> &urls);
    QList<QUrl> associatedApplicationUrls() const;

    void setStatus(const Plasma::Types::ItemStatus &status);
    Plasma::Types::ItemStatus status() const;

    int screen() const;
    QRect screenGeometry() const;

    bool immutable() const;
    Plasma::Types::ImmutabilityType immutability() const;
    bool userConfiguring() const;
    int apiVersion() const;

    bool hideOnWindowDeactivate() const;
    void setHideOnWindowDeactivate(bool hide);

    QKeySequence globalShortcut() const;
    void setGlobalShortcut(const QKeySequence &keySequence);

    QObject *nativeInterface();

    //NOTE: setConfigurationRequiredProperty because ambiguous with the
    // setConfigurationRequired invokable
    bool configurationRequired() const;
    void setConfigurationRequiredProperty(bool required);

    QString configurationRequiredReason() const;
    void setConfigurationRequiredReason(const QString &reason);

Q_SIGNALS:
    /**
     * somebody else, usually the containment sent some data to the applet
     * @param mimetype the mime type of the data such as text/plain
     * @param data either the actual data or an URL representing it
     */
    void externalData(const QString &mimetype, const QVariant &data);

    void configNeedsSaving();

    /**
     * Emitted when the applet's activation action is triggered
     */
    void activated();

    /**
     * Emitted just before the contextual actions are about to show
     * For instance just before the context menu containing the actions
     * added with setAction() is shown
     */
    void contextualActionsAboutToShow();

//PROPERTY change notifiers--------------
    void iconChanged();
    void titleChanged();
    void toolTipMainTextChanged();
    void toolTipSubTextChanged();
    void toolTipTextFormatChanged();
    void toolTipItemChanged();
    void formFactorChanged();
    void locationChanged();
    void contextChanged();
    void immutabilityChanged();
    void statusChanged();
    void backgroundHintsChanged();
    void busyChanged();
    void screenChanged();
    void screenGeometryChanged();
    void hideOnWindowDeactivateChanged();
    void associatedApplicationChanged();
    void associatedApplicationUrlsChanged();
    void availableScreenRegionChanged();
    void availableScreenRectChanged();

    void userConfiguringChanged();
    void globalShortcutChanged();
    void configurationRequiredChanged();
    void configurationRequiredReasonChanged();

protected Q_SLOTS:
    void init() Q_DECL_OVERRIDE;

protected:
    bool event(QEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void destroyedChanged(bool destroyed);

private:

    QStringList m_actions;

    KDeclarative::ConfigPropertyMap *m_configuration;
    DeclarativeAppletScript *m_appletScriptEngine;

//UI-specific members ------------------

    QString m_toolTipMainText;
    QString m_toolTipSubText;
    int m_toolTipTextFormat;
    QPointer<QQuickItem> m_toolTipItem;
    QVariantList m_args;
    Plasma::Types::BackgroundHints m_backgroundHints;
    bool m_hideOnDeactivate : 1;
    //this is used to build an emacs style shortcut
    int m_oldKeyboardShortcut;
    QObject *m_dummyNativeInterface;

    friend class ContainmentInterface;
    //This is used by ContainmentInterface
    QPointF m_positionBeforeRemoval;
};

QML_DECLARE_TYPEINFO(AppletInterface, QML_HAS_ATTACHED_PROPERTIES)

#endif
