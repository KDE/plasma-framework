/*
 * Copyright (C) 2014  David Edmundson <davidedmundson@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef FADINGNODE_H
#define FADINGNODE_H

#include <QSGGeometryNode>
#include <QSGTexture>
#include <QRectF>

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
    ~FadingNode();

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
