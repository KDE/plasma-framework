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
#include < * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
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
#include <QQmlListProperty>

class PlotData : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QList<qreal> values READ values NOTIFY valuesChanged)
    Q_PROPERTY(qreal max READ max NOTIFY maxChanged)
    Q_PROPERTY(qreal min READ min NOTIFY minChanged)

public:
    PlotData(QObject *parent = 0);

    void setColor(const QColor &color);
    QColor color() const;

    void addValue(qreal value);

    QList<qreal> values() const;

    QVector<qreal> m_normalizedValues;

    qreal max() const;
    qreal min() const;

Q_SIGNALS:
    void colorChanged();
    void valuesChanged();
    void maxChanged();
    void minChanged();

private:
    QColor m_color;
    QList<qreal> m_values;

    qreal m_min;
    qreal m_max;
};

class Plotter : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<PlotData> dataSets READ dataSets)
    Q_PROPERTY(qreal max READ max NOTIFY maxChanged)
    Q_PROPERTY(qreal min READ min NOTIFY minChanged)

    //Q_CLASSINFO("DefaultProperty", "dataSets")

public:
    Plotter(QQuickItem *parent = 0);
    ~Plotter();

    qreal max() const;
    qreal min() const;

    QQmlListProperty<PlotData> dataSets();
    static void dataSet_append(QQmlListProperty<PlotData> *list, PlotData *item);
    static int dataSet_count(QQmlListProperty<PlotData> *list);
    static PlotData *dataSet_at(QQmlListProperty<PlotData> *list, int pos);
    static void dataSet_clear(QQmlListProperty<PlotData> *list);

    Q_INVOKABLE void addValue(const QList<qreal> &value);
private:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *updatePaintNodeData) override final;
    QPainterPath interpolate(const QVector<qreal> &p, qreal x0, qreal x1) const;

Q_SIGNALS:
    void maxChanged();
    void minChanged();

private Q_SLOTS:
    void render();

private:
    QList<PlotData *> m_plotData;

    GLuint m_fbo = 0;
    QSGSimpleTextureNode *m_node = nullptr;
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
