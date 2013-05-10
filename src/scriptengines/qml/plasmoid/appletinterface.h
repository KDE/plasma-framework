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

#include "declarativeappletscript.h"

class QAction;
class QmlAppletScript;
class QSignalMapper;
class QSizeF;

class ConfigPropertyMap;
class ConfigView;

class QmlObject;

namespace Plasma
{
    class ConfigLoader;
} // namespace Plasma

class AppletInterface : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    //TODO: writable icon
    Q_PROPERTY(QString icon READ icon CONSTANT)
    Q_PROPERTY(uint id READ id CONSTANT)
    Q_PROPERTY(Plasma::Types::FormFactor formFactor READ formFactor NOTIFY formFactorChanged)
    Q_PROPERTY(Plasma::Types::Location location READ location NOTIFY locationChanged)
    Q_PROPERTY(QString currentActivity READ currentActivity NOTIFY contextChanged)
    Q_PROPERTY(QObject* configuration READ configuration CONSTANT)
    Q_PROPERTY(QString activeConfig WRITE setActiveConfig READ activeConfig)
    Q_PROPERTY(bool busy WRITE setBusy READ isBusy NOTIFY busyChanged)
    Q_PROPERTY(bool expanded WRITE setExpanded READ isExpanded NOTIFY expandedChanged)
    Q_PROPERTY(Plasma::Types::BackgroundHints backgroundHints WRITE setBackgroundHints READ backgroundHints NOTIFY backgroundHintsChanged)
    Q_PROPERTY(bool immutable READ immutable NOTIFY immutableChanged)
    Q_PROPERTY(bool userConfiguring READ userConfiguring) // @since 4.5
    Q_PROPERTY(int apiVersion READ apiVersion CONSTANT)
    Q_PROPERTY(Plasma::Types::ItemStatus status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(QString associatedApplication WRITE setAssociatedApplication READ associatedApplication)

public:
    AppletInterface(DeclarativeAppletScript *script, QQuickItem *parent = 0);
    ~AppletInterface();

//API not intended for the QML part
    QmlObject *qmlObject();

//QML API-------------------------------------------------------------------

    Q_INVOKABLE void setConfigurationRequired(bool needsConfiguring, const QString &reason = QString());

    Q_INVOKABLE void setActionSeparator(const QString &name);
    Q_INVOKABLE void setAction(const QString &name, const QString &text,
                               const QString &icon = QString(), const QString &shortcut = QString());

    Q_INVOKABLE void removeAction(const QString &name);

    Q_INVOKABLE QAction *action(QString name) const;

    Q_INVOKABLE QVariant readConfig(const QString &entry) const;

    Q_INVOKABLE void writeConfig(const QString &entry, const QVariant &value);

    Q_INVOKABLE QString file(const QString &fileType);
    Q_INVOKABLE QString file(const QString &fileType, const QString &filePath);

    Q_INVOKABLE void debug(const QString &msg);

    QList<QAction*> contextualActions() const;

    inline Plasma::Applet *applet() const { return m_appletScriptEngine->applet(); }

    Q_INVOKABLE QString downloadPath(const QString &file);
    Q_INVOKABLE QStringList downloadedFiles() const;


//PROPERTY ACCESSORS-------------------------------------------------------------------
    QString icon() const;

    QString title() const;
    void setTitle(const QString &title);

    uint id() const;

    Plasma::Types::FormFactor formFactor() const;

    Plasma::Types::Location location() const;

    QString currentActivity() const;

    QObject* configuration() const;

    bool isBusy() const;
    void setBusy(bool busy);

    bool isExpanded() const;
    void setExpanded(bool expanded);

    Plasma::Types::BackgroundHints backgroundHints() const;
    void setBackgroundHints(Plasma::Types::BackgroundHints hint);

    void setAssociatedApplication(const QString &string);
    QString associatedApplication() const;

    void setStatus(const Plasma::Types::ItemStatus &status);
    Plasma::Types::ItemStatus status() const;

    QString activeConfig() const;
    void setActiveConfig(const QString &name);

    bool immutable() const;
    bool userConfiguring() const;
    int apiVersion() const;

Q_SIGNALS:
    void releaseVisualFocus();
    void configNeedsSaving();

//PROPERTY change notifiers--------------
    void titleChanged();
    void formFactorChanged();
    void locationChanged();
    void contextChanged();
    void immutableChanged();
    void statusChanged();
    void backgroundHintsChanged();
    void busyChanged();
    void expandedChanged();

protected:
    virtual void init();
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);
    void itemChange(ItemChange change, const ItemChangeData &value);

    DeclarativeAppletScript *m_appletScriptEngine;

private:
    QStringList m_actions;
    QSignalMapper *m_actionSignals;
    QString m_currentConfig;
    QMap<QString, Plasma::ConfigLoader*> m_configs;


    ConfigPropertyMap *m_configuration;

//UI-specific members ------------------
    QmlObject *m_qmlObject;
    QWeakPointer<QObject> m_compactUiObject;

    QTimer *m_creationTimer;

    Plasma::Types::BackgroundHints m_backgroundHints;
    bool m_busy : 1;
    bool m_expanded : 1;
};

#endif
