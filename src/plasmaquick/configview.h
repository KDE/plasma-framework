/*
 *   Copyright 2013 Marco Martin <mart@kde.org>
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

#ifndef CONFIGVIEW_H
#define CONFIGVIEW_H


#include <QQuickView>

#include <plasmaquick/plasmaquick_export.h>

namespace Plasma {
    class Applet;
}

class ConfigViewPrivate;

class ConfigModel;

class PLASMAQUICK_EXPORT ConfigView : public QQuickView
{
    Q_OBJECT
    Q_PROPERTY(ConfigModel *configModel READ configModel CONSTANT)
    Q_PROPERTY(QString appletGlobalShortcut READ appletGlobalShortcut WRITE setAppletGlobalShortcut NOTIFY appletGlobalShortcutChanged)

public:
    /**
     * @param applet the applet of this ConfigView
     * @param parent the QWindow in which this ConfigView is parented to
     **/
    ConfigView(Plasma::Applet *applet, QWindow *parent = 0);
    virtual ~ConfigView();

    virtual void init();

    QString appletGlobalShortcut() const;
    void setAppletGlobalShortcut(const QString &shortcut);

    /**
     * @return the ConfigModel of the ConfigView
     **/
    ConfigModel *configModel() const;

Q_SIGNALS:
    void appletGlobalShortcutChanged();

protected:
     void hideEvent(QHideEvent *ev);
     void resizeEvent(QResizeEvent *re);

private:
    ConfigViewPrivate *const d;
};

#endif // multiple inclusion guard
