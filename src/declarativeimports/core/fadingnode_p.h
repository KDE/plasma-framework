/*
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef FADINGNODE_H
#define FADINGNODE_H

#include <QRectF>
#include <QSGGeometryNode>
#include <QSGTexture>

/**
 * This node fades between two textures using a shader
 */

class FadingNode : public QSGGeometryNode
{
public:
    /**
     * Ownership of the textures is transferred to the node
     */
    FadingNode(QSGTexture *source, QSGTexture *target);
    ~FadingNode() override;

    /**
     * Set the progress fading between source and target
     */
    void setProgress(qreal progress);
    void setRect(const QRectF &bounds);

private:
    QScopedPointer<QSGTexture> m_source;
    QScopedPointer<QSGTexture> m_target;
};

#endif // PLASMAFADINGNODE_H
