/*
 * This file is part of the KDE project
 *
 * Copyright © 2014 Fredrik Höglund <fredrik@kde.org>
 * Copyright © 2014 Marco Martin <mart@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef PLASMA_PLOTTER_H
#define PLASMA_PLOTTER_H

#include <QSGTexture>
#include <QSGSimpleTextureNode>
#include <QQuickItem>


class Plotter : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor USER true)
    Q_PROPERTY(QVariantList values READ values WRITE setValues USER true)

public:
    Plotter(QQuickItem *parent = 0);
    ~Plotter();

    void setColor(const QColor &color);
    QColor color() const;

    void setValues(const QVariantList &values);
    QVariantList values() const;

    Q_INVOKABLE void addValue(qreal value);
private:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *updatePaintNodeData) override final;
    QPainterPath interpolate(const QVector<float> &p, float x0, float x1) const;

private Q_SLOTS:
    void render();

private:
    GLuint m_fbo = 0;
    QSGSimpleTextureNode *m_node = nullptr;
    QColor m_color = QColor::fromRgbF(0.30, 0.7, 1.0);
    struct {
        QVariantList values;
        QColor color;
        bool dirty = true;
    } properties;
    QVector<float> m_data;
    qreal m_min;
    qreal m_max;
    QMatrix4x4 m_matrix;
    bool m_valuesDirty = true;
    bool m_initialized = false;
    bool m_haveMSAA;
    bool m_haveFramebufferBlit;
    bool m_haveInternalFormatQuery;
    GLenum m_internalFormat;
    int m_samples;

    static QOpenGLShaderProgram *s_program;
    static int u_matrix;
    static int u_color1;
    static int u_color2;
    static int u_yMin;
    static int u_yMax;
};

#endif
