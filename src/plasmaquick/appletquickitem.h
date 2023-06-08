/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef APPLETQUICKITEM_H
#define APPLETQUICKITEM_H

#include <QQmlComponent>
#include <QQmlEngine>
#include <QQmlParserStatus>
#include <QQuickItem>
#include <QTimer>

#include <plasmaquick/plasmaquick_export.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the public Plasma API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace Plasma
{
class Applet;
}

namespace PlasmaQuick
{
class AppletQuickItemPrivate;
class SharedQmlEngine;

class PLASMAQUICK_EXPORT AppletQuickItem : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(int switchWidth READ switchWidth WRITE setSwitchWidth NOTIFY switchWidthChanged)
    Q_PROPERTY(int switchHeight READ switchHeight WRITE setSwitchHeight NOTIFY switchHeightChanged)

    Q_PROPERTY(QQmlComponent *compactRepresentation READ compactRepresentation WRITE setCompactRepresentation NOTIFY compactRepresentationChanged)
    Q_PROPERTY(QQuickItem *compactRepresentationItem READ compactRepresentationItem NOTIFY compactRepresentationItemChanged)

    Q_PROPERTY(QQmlComponent *fullRepresentation READ fullRepresentation WRITE setFullRepresentation NOTIFY fullRepresentationChanged)
    Q_PROPERTY(QQuickItem *fullRepresentationItem READ fullRepresentationItem NOTIFY fullRepresentationItemChanged)

    /**
     * When true the full representation will be loaded immediately together with the main plasmoid.
     * Note that this will have a negative impact on plasmoid loading times
     * This is needed only when some important logic has to live inside the full representation and
     * needs to be accessed from the outside. Use with care
     * TODO: remove? we whould find a better way to fix folderview and Notes
     */
    Q_PROPERTY(bool preloadFullRepresentation READ preloadFullRepresentation WRITE setPreloadFullRepresentation NOTIFY preloadFullRepresentationChanged)

    /**
     * this is supposed to be either one between compactRepresentation or fullRepresentation
     */
    Q_PROPERTY(QQmlComponent *preferredRepresentation READ preferredRepresentation WRITE setPreferredRepresentation NOTIFY preferredRepresentationChanged)

    /**
     * Hint set to true if the applet should be siplayed as expanded, such as the main popup open
     */
    Q_PROPERTY(bool expanded READ isExpanded WRITE setExpanded NOTIFY expandedChanged)

    /**
     * True when the applet wants the activation signal act in toggle mode, i.e. while being expanded
     * the signal shrinks the applet to its not expanded state instead of reexpanding it.
     */
    Q_PROPERTY(bool activationTogglesExpanded WRITE setActivationTogglesExpanded READ isActivationTogglesExpanded NOTIFY activationTogglesExpandedChanged)

    /**
     * Whether the dialog should be hidden when the dialog loses focus.
     *
     * The default value is @c false.
     **/
    Q_PROPERTY(bool hideOnWindowDeactivate READ hideOnWindowDeactivate WRITE setHideOnWindowDeactivate NOTIFY hideOnWindowDeactivateChanged)

    /**
     * Gives compatibility to the old plasmoid.* api
     */
    Q_PROPERTY(QObject *plasmoid READ applet CONSTANT)

public:
    AppletQuickItem(QQuickItem *parent = nullptr);
    ~AppletQuickItem() override;

    ////API NOT SUPPOSED TO BE USED BY QML
    Plasma::Applet *applet() const;

    void classBegin() override;

    QQuickItem *compactRepresentationItem();
    QQuickItem *fullRepresentationItem();
    QObject *testItem();

    ////PROPERTY ACCESSORS
    int switchWidth() const;
    void setSwitchWidth(int width);

    int switchHeight() const;
    void setSwitchHeight(int width);

    QQmlComponent *compactRepresentation();
    void setCompactRepresentation(QQmlComponent *component);

    QQmlComponent *fullRepresentation();
    void setFullRepresentation(QQmlComponent *component);

    QQmlComponent *preferredRepresentation();
    void setPreferredRepresentation(QQmlComponent *component);

    bool isExpanded() const;
    void setExpanded(bool expanded);

    bool isActivationTogglesExpanded() const;
    void setActivationTogglesExpanded(bool activationTogglesExpanded);

    bool hideOnWindowDeactivate() const;
    void setHideOnWindowDeactivate(bool hide);

    bool preloadFullRepresentation() const;
    void setPreloadFullRepresentation(bool preload);

    static bool hasItemForApplet(Plasma::Applet *applet);
    static AppletQuickItem *itemForApplet(Plasma::Applet *applet);

Q_SIGNALS:
    // Property signals
    void switchWidthChanged(int width);
    void switchHeightChanged(int height);

    void expandedChanged(bool expanded);

    void activationTogglesExpandedChanged(bool activationTogglesExpanded);
    void hideOnWindowDeactivateChanged(bool hide);

    void compactRepresentationChanged(QQmlComponent *compactRepresentation);
    void fullRepresentationChanged(QQmlComponent *fullRepresentation);
    void preferredRepresentationChanged(QQmlComponent *preferredRepresentation);

    void compactRepresentationItemChanged(QObject *compactRepresentationItem);
    void fullRepresentationItemChanged(QObject *fullRepresentationItem);

    void preloadFullRepresentationChanged(bool preload);

protected:
    // Initializations that need to be executed after classBegin()
    virtual void init();
    PlasmaQuick::SharedQmlEngine *qmlObject();

    // Reimplementation
    void childEvent(QChildEvent *event) override;
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void itemChange(ItemChange change, const ItemChangeData &value) override;

private:
    AppletQuickItemPrivate *const d;

    Q_PRIVATE_SLOT(d, void minimumWidthChanged())
    Q_PRIVATE_SLOT(d, void minimumHeightChanged())
    Q_PRIVATE_SLOT(d, void preferredWidthChanged())
    Q_PRIVATE_SLOT(d, void preferredHeightChanged())
    Q_PRIVATE_SLOT(d, void maximumWidthChanged())
    Q_PRIVATE_SLOT(d, void maximumHeightChanged())
    Q_PRIVATE_SLOT(d, void fillWidthChanged())
    Q_PRIVATE_SLOT(d, void fillHeightChanged())
};

}

#endif
