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

namespace Plasma
{

class PanelSvg;
class AppletScript;
class ShadowItem;

class AppletOverlayWidget : public QGraphicsWidget
{
public:
    AppletOverlayWidget(QGraphicsWidget *parent);

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
};

class Applet::Private
{
public:
    Private(KService::Ptr service, int uniqueID, Applet *applet);
    ~Private();

    void init();

    // put all setup routines for script here. at this point we can assume that
    // package exists and that we have a script engin
    void setupScriptSupport();

    QString globalName() const;
    QString instanceName();
    void scheduleConstraintsUpdate(Plasma::Constraints c);
    KConfigGroup* mainConfigGroup();
    void copyEntries(KConfigGroup *source, KConfigGroup *destination);
    QString visibleFailureText(const QString& reason);
    void checkImmutability();
    void themeChanged();
    void resetConfigurationObject();

    static uint s_maxAppletId;
    static uint s_maxZValue;
    static uint s_minZValue;
    static PackageStructure::Ptr packageStructure;

    //TODO: examine the usage of memory here; there's a pretty large
    //      number of members at this point.
    uint appletId;
    Applet *q;
    BackgroundHints backgroundHints;
    KPluginInfo appletDescription;
    Package* package;
    AppletOverlayWidget *needsConfigOverlay;
    QList<QGraphicsItem*> registeredAsDragHandle;
    QStringList loadedEngines;
    Plasma::PanelSvg *background;
    //Plasma::LineEdit *failureText;
    AppletScript *script;
    ConfigXml* configXml;
    ShadowItem* shadow;
    QPixmap* cachedBackground;
    KConfigGroup *mainConfig;
    Plasma::Constraints pendingConstraints;
    Plasma::AspectRatioMode aspectRatioMode;
    ImmutabilityType immutability;
    int constraintsTimerId;
    bool hasConfigurationInterface : 1;
    bool failed : 1;
    bool isContainment : 1;
    bool square : 1;
    bool transient : 1;
};

} // Plasma namespace

#endif
