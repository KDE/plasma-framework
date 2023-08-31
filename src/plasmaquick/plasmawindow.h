/*
    SPDX-FileCopyrightText: 2023 David Edmundson <davidedmundson@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMAWINDOW_H
#define PLASMAWINDOW_H

#include <QQuickWindow>

#include <plasmaquick/plasmaquick_export.h>

namespace Plasma
{
class FrameSvgItem;
}

namespace PlasmaQuick
{
class PlasmaWindowPrivate;

/*
 * Creates a QQuickWindow themed in a Plasma style with background
 */
class PLASMAQUICK_EXPORT PlasmaWindow : public QQuickWindow
{
    Q_OBJECT

    /**
     * The main QML item that will be displayed in the Dialog
     */
    Q_PROPERTY(QQuickItem *mainItem READ mainItem WRITE setMainItem NOTIFY mainItemChanged)

public:
    PlasmaWindow(QWindow *parent = nullptr);
    ~PlasmaWindow() override;

    /**
     * The main QML item that will be displayed in the Dialog
     */
    void setMainItem(QQuickItem *mainItem);

    QQuickItem *mainItem() const;

    /**
     * Changes which rounded corners are shown on the window.
     * Margins remain the same
     * The default is all borders
     */
    void setBorders(Qt::Edges bordersToShow);

    Qt::Edges borders();

    /**
     * Returns the margins that are placed around the mainItem
     * When setting size hints on the window this should be factored in.
     */
    QMargins margins() const;

Q_SIGNALS:
    void mainItemChanged();
    void marginsChanged();
    void bordersChanged();

protected:
    void resizeEvent(QResizeEvent *e) override;

private:
    const std::unique_ptr<PlasmaWindowPrivate> d;
};
}

#endif
