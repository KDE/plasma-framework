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
    /**
     * text Label of the data set: note this is purely a model, it will need a Label somewhere to be actually painted
     */
    Q_PROPERTY(QString label READ label WRITE setLabel NOTIFY labelChanged)

    /**
     * Color to plot this data set
     */
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

    /**
     * All the values currently in this data set
     */
    Q_PROPERTY(QList<qreal> values READ values NOTIFY valuesChanged)

    /**
     * Maximum value of this data set
     */
    Q_PROPERTY(qreal max READ max NOTIFY maxChanged)

    /**
     * Minimum value of this data set
     */
    Q_PROPERTY(qreal min READ min NOTIFY minChanged)

public:
    PlotData(QObject *parent = 0);

    void setColor(const QColor &color);
    QColor color() const;

    void addSample(qreal value);

    QList<qreal> values() const;

    QVector<qreal> m_normalizedValues;

    qreal max() const;
    qreal min() const;

    void setSampleSize(int size);

    QString label() const;
    void setLabel(const QString &label);

Q_SIGNALS:
    void colorChanged();
    void valuesChanged();
    void maxChanged();
    void minChanged();
    void labelChanged();

private:
    QString m_label;
    QColor m_color;
    QList<qreal> m_values;

    qreal m_min;
    qreal m_max;
    int m_sampleSize;
};

class Plotter : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<PlotData> dataSets READ dataSets)

    /**
     * maximum value among all graphs
     */
    Q_PROPERTY(qreal max READ max NOTIFY maxChanged)

    /**
     * minimum value among all graphs
     */
    Q_PROPERTY(qreal min READ min NOTIFY minChanged)

    /**
     * draw at most n samples, if new samples are pushed old values are started to be thrown away
     */
    Q_PROPERTY(int sampleSize READ sampleSize WRITE setSampleSize NOTIFY sampleSizeChanged)

    /**
     * if true stack the graphs one on top of each other instead of just painting one on top of each other
     */
    Q_PROPERTY(bool stacked READ isStacked WRITE setStacked NOTIFY stackedChanged)

    /**
     * If true, the graph is automatically scaled to always fit in the Plotter area
     */
    Q_PROPERTY(bool autoRange READ isAutoRange WRITE setAutoRange NOTIFY autoRangeChanged)

    //Q_CLASSINFO("DefaultProperty", "dataSets")

public:
    Plotter(QQuickItem *parent = 0);
    ~Plotter();

    qreal max() const;
    qreal min() const;

    int sampleSize() const;
    void setSampleSize(int size);

    bool isStacked() const;
    void setStacked(bool stacked);

    bool isAutoRange() const;
    void setAutoRange(bool autorange);

    QQmlListProperty<PlotData> dataSets();
    static void dataSet_append(QQmlListProperty<PlotData> *list, PlotData *item);
    static int dataSet_count(QQmlListProperty<PlotData> *list);
    static PlotData *dataSet_at(QQmlListProperty<PlotData> *list, int pos);
    static void dataSet_clear(QQmlListProperty<PlotData> *list);

    Q_INVOKABLE void addSample(const QList<qreal> &value);
private:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *updatePaintNodeData) override final;
    QPainterPath interpolate(const QVector<qreal> &p, qreal x0, qreal x1) const;

Q_SIGNALS:
    void maxChanged();
    void minChanged();
    void sampleSizeChanged();
    void stackedChanged();
    void autoRangeChanged();

private Q_SLOTS:
    void render();

private:
    QList<PlotData *> m_plotData;

    GLuint m_fbo = 0;
    QSGSimpleTextureNode *m_node = nullptr;
    qreal m_min;
    qreal m_max;
    int m_sampleSize;
    bool m_stacked;
    bool m_autoRange;

    QMatrix4x4 m_matrix;
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
