/*
    SPDX-FileCopyrightText: 2023 David Edmundson <davidedmundson@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QMargins>
#include <QObject>
#include <memory>

#include <plasmaquick/plasmaquick_export.h>

class QWindow;

class EdgeEventForwarderPrivate;

namespace PlasmaQuick
{

/**
 * @brief The EdgeEventForwarder class
 * This class forwards edge events to be replayed within the given margin
 * This is useful if children do not touch the edge of a window, but want to get input events
 */
class PLASMAQUICK_EXPORT EdgeEventForwarder : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief EdgeEventForwarder constructor
     * @param window The window to intercept and filter
     * The event forwarder is parented to the window
     */
    EdgeEventForwarder(QWindow *parent);
    ~EdgeEventForwarder();

    /**
     * @brief setMargins sets the margins to use for the event forwarding
     */
    void setMargins(const QMargins &margins);
    QMargins margins();

    /**
     * @brief setActiveEdges sets which margins should be active for edge forwarding
     * typically this should match edges touching a screen edge
     */
    void setActiveEdges(Qt::Edges edges);
    Qt::Edges activeEdges();

    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    std::unique_ptr<EdgeEventForwarderPrivate> d;
};

}
