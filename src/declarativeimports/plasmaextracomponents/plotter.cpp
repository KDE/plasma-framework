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

#include <epoxy/gl.h>
#include "plotter.h"

#include <QGuiApplication>
#include <QWindow>

#include <QOpenGLContext>
#include <QOpenGLShaderProgram>

#include <QPainterPath>
#include <QPolygonF>

#include <QVector2D>
#include <QMatrix4x4>

#include <QSGTexture>
#include <QSGSimpleTextureNode>

#include <QQuickWindow>
#include <QQuickView>
#include <QQuickItem>

#include <QDebug>

//completely arbitrary
static int s_defaultSampleSize = 40;

PlotData::PlotData(QObject *parent)
    : QObject(parent),
      m_min(std::numeric_limits<qreal>::max()),
      m_max(std::numeric_limits<qreal>::min()),
      m_sampleSize(s_defaultSampleSize)
{
    m_values.reserve(s_defaultSampleSize);
    for (int i = 0; i < s_defaultSampleSize; ++i) {
        m_values << 0.0;
    }
}

void PlotData::setColor(const QColor &color)
{
    if (m_color == color) {
        return;
    }

    m_color = color;

    emit colorChanged();
}

QColor PlotData::color() const
{
    return m_color;
}

qreal PlotData::max() const
{
    return m_max;
}

qreal PlotData::min() const
{
    return m_min;
}

void PlotData::setSampleSize(int size)
{
    if (m_sampleSize != size) {
        return;
    }

    m_values.reserve(size);
    if (m_values.size() > size) {
        for (int i = 0; i < (m_values.size() - size); ++i) {
            m_values.pop_front();
        }
    } else if (m_values.size() < size) {
        for (int i = 0; i < (size - m_values.size()); ++i) {
            m_values.prepend(0.0);
        }
    }

    m_sampleSize = size;
}

QString PlotData::label() const
{
    return m_label;
}

void PlotData::setLabel(const QString &label)
{
    if (m_label == label) {
        return;
    }

    m_label = label;
    emit labelChanged();
}

void PlotData::addSample(qreal value)
{

    //assume at this point we'll have to pop a single time to stay in size
    if (m_values.size() >= m_sampleSize) {
        m_values.pop_front();
    }

    m_values.push_back(value);

    m_max = std::numeric_limits<qreal>::min();
    m_min = std::numeric_limits<qreal>::max();
    for (auto v : m_values) {
        if (v > m_max) {
            m_max = v;
        } else if (v < m_min) {
            m_min = v;
        }
    }

    emit valuesChanged();
}

QList<qreal> PlotData::values() const
{
    return m_values;
}

const char *vs_source =
    "attribute vec4 vertex;\n"
    "varying float gradient;\n"

    "uniform mat4 matrix;\n"
    "uniform float yMin;\n"
    "uniform float yMax;\n"

    "void main(void) {\n"
    "    gradient = (vertex.y - yMin) / (yMax - yMin);"
    "    gl_Position = matrix * vertex;\n"
    "}";

const char *fs_source= 
    "uniform vec4 color1;\n"
    "uniform vec4 color2;\n"

    "varying float gradient;\n"

    "void main(void) {\n"
    "    gl_FragColor = mix(color1, color2, gradient);\n"
    "}";




// --------------------------------------------------



class Texture : public QSGTexture
{
public:
    Texture(QOpenGLContext *ctx);
    ~Texture();

    void bind() override final;
    bool hasAlphaChannel() const override final { return true; }
    bool hasMipmaps() const override final { return false; }
    int textureId() const override final { return m_texture; }
    QSize textureSize() const override final { return m_size; }

    void recreate(const QSize &size);
    GLuint fbo() const { return m_fbo; }

private:
    GLuint m_texture = 0;
    GLuint m_fbo = 0;
    GLenum m_internalFormat;
    bool m_haveTexStorage;
    QSize m_size;
};

Texture::Texture(QOpenGLContext *ctx) : QSGTexture()
{
    QPair<int, int> version = ctx->format().version();

    if (ctx->isOpenGLES()) {
        m_haveTexStorage = version >= qMakePair(3, 0) || ctx->hasExtension("GL_EXT_texture_storage");
        m_internalFormat = version >= qMakePair(3, 0) ? GL_RGBA8 : GL_RGBA;
    } else {
        m_haveTexStorage = version >= qMakePair(4, 2) || ctx->hasExtension("GL_ARB_texture_storage");
        m_internalFormat = GL_RGBA8;
    }
 
    glGenFramebuffers(1, &m_fbo);
}

Texture::~Texture()
{
    if (m_texture)
        glDeleteTextures(1, &m_texture);

    glDeleteFramebuffers(1, &m_fbo);
}

void Texture::bind()
{
    glBindTexture(GL_TEXTURE_2D, m_texture);
}

void Texture::recreate(const QSize &size)
{
    if (m_texture)
        glDeleteTextures(1, &m_texture);

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    if (m_haveTexStorage)
        glTexStorage2D(GL_TEXTURE_2D, 1, m_internalFormat, size.width(), size.height());
    else
        glTexImage2D(GL_TEXTURE_2D, 0, m_internalFormat, size.width(), size.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_texture, 0);

    m_size = size;
}



// ----------------------

QOpenGLShaderProgram *Plotter::s_program = nullptr;
int Plotter::u_matrix;
int Plotter::u_color1;
int Plotter::u_color2;
int Plotter::u_yMin;
int Plotter::u_yMax;

Plotter::Plotter(QQuickItem *parent)
    : QQuickItem(parent),
      m_min(0),
      m_max(0),
      m_sampleSize(s_defaultSampleSize),
      m_stacked(true),
      m_autoRange(true)
{
    setFlag(ItemHasContents);
}

Plotter::~Plotter()
{
    delete m_node;
}

qreal Plotter::max() const
{
    return m_max;
}

qreal Plotter::min() const
{
    return m_min;
}

int Plotter::sampleSize() const
{
    return m_sampleSize;
}

void Plotter::setSampleSize(int size)
{
    if (m_sampleSize != size) {
        return;
    }

    m_sampleSize = size;

    for (auto data : m_plotData) {
        data->setSampleSize(size);
    }

    update();
    emit sampleSizeChanged();
}

bool Plotter::isStacked() const
{
    return m_stacked;
}

void Plotter::setStacked(bool stacked)
{
    if (m_stacked == stacked) {
        return;
    }

    m_stacked = stacked;

    emit stackedChanged();
    update();
}

bool Plotter::isAutoRange() const
{
    return m_autoRange;
}

void Plotter::setAutoRange(bool autoRange)
{
    if (m_autoRange == autoRange) {
        return;
    }

    m_autoRange = autoRange;

    emit autoRangeChanged();
    update();
}

void Plotter::addSample(const QList<qreal> &value)
{
    if (value.count() != m_plotData.count()) {
        qWarning() << "Must add a new value per data set";
        return;
    }

    int i = 0;
    for (auto data : m_plotData) {
        data->addSample(value.value(i));
        ++i;
    }

    update();
}

void Plotter::dataSet_append(QQmlListProperty<PlotData> *list, PlotData *item)
{
    Plotter *p = static_cast<Plotter *>(list->object);
    return p->m_plotData.append(item);
}

int Plotter::dataSet_count(QQmlListProperty<PlotData> *list)
{
    Plotter *p = static_cast<Plotter *>(list->object);
    return p->m_plotData.count();
}

PlotData *Plotter::dataSet_at(QQmlListProperty<PlotData> *list, int index)
{
    Plotter *p = static_cast<Plotter *>(list->object);
    return p->m_plotData.at(index);
}

void Plotter::dataSet_clear(QQmlListProperty<PlotData> *list)
{
    Plotter *p = static_cast<Plotter *>(list->object);
    return p->m_plotData.clear();
}


QQmlListProperty<PlotData> Plotter::dataSets()
{
    return QQmlListProperty<PlotData>(this, 0, Plotter::dataSet_append, Plotter::dataSet_count, Plotter::dataSet_at, Plotter::dataSet_clear);
}



// Catmull-Rom interpolation
QPainterPath Plotter::interpolate(const QVector<qreal> &p, qreal x0, qreal x1) const
{
    QPainterPath path;

    const QMatrix4x4 matrix( 0,    1,    0,     0,
                            -1/6., 1,    1/6.,  0,
                             0,    1/6., 1,    -1/6.,
                             0,    0,    1,     0);

    const qreal xDelta = (x1 - x0) / (p.count() - 3);
    qreal x = x0 - xDelta;

    path.moveTo(x0, p[0]);

    for (int i = 1; i < p.count() - 2; i++) {
        const QMatrix4x4 points(x,              p[i-1], 0, 0,
                                x + xDelta * 1, p[i+0], 0, 0,
                                x + xDelta * 2, p[i+1], 0, 0,
                                x + xDelta * 3, p[i+2], 0, 0); 

        const QMatrix4x4 res = matrix * points;

        path.cubicTo(res(1, 0), res(1, 1),
                     res(2, 0), res(2, 1),
                     res(3, 0), res(3, 1));

        x += xDelta;
    }

    return path;
}

void Plotter::render()
{
    GLuint rb;

    if (m_haveMSAA && m_haveFramebufferBlit) {
        // Allocate a temporary MSAA renderbuffer
        glGenRenderbuffers(1, &rb);
        glBindRenderbuffer(GL_RENDERBUFFER, rb);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, m_internalFormat, width(), height());

        // Attach it to the framebuffer object
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb);
    } else {
        // If we don't have MSAA support we render directly into the texture
        glBindFramebuffer(GL_FRAMEBUFFER, static_cast<Texture*>(m_node->texture())->fbo());
    }

    glViewport(0, 0, width(), height());

    // Clear the color buffer
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Add horizontal lines
    int lineCount = height() / 20;

    QVector<QVector2D> vertices;
    for (int i = 0; i < lineCount; i++)
        vertices << QVector2D(0, i * 20) << QVector2D(width(), i * 20);

    // Tessellate
    float min = height();
    float max = height();

    QHash<PlotData *, int> verticesCounts;
    for (auto data : m_plotData) {
        // Interpolate the data set
        const QPainterPath path = interpolate(data->m_normalizedValues, 0, width());

        // Flatten the path
        const QList<QPolygonF> polygons = path.toSubpathPolygons();

        for (const QPolygonF &p : polygons) {
            verticesCounts[data] = 0;
            vertices << QVector2D(p.first().x(), height());

            for (int i = 0; i < p.count()-1; i++) {
                min = qMin<float>(min, height() - p[i].y());
                vertices << QVector2D(p[i].x(), height() - p[i].y());
                vertices << QVector2D((p[i].x() + p[i+1].x()) / 2.0, height());
                verticesCounts[data] += 2;
            }

            min = qMin<float>(min, height() - p.last().y());
            vertices << QVector2D(p.last().x(), height() - p.last().y());
            vertices << QVector2D(p.last().x(), height());
            verticesCounts[data] += 3;
        }
    }

    // Upload vertices
    GLuint vbo;
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.count() * sizeof(QVector2D), vertices.constData(), GL_STATIC_DRAW);

    // Set up the array
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(QVector2D), nullptr);
    glEnableVertexAttribArray(0);

    // Bind the shader program
    s_program->bind();
    s_program->setUniformValue(u_matrix, m_matrix);

    // Draw the lines
    s_program->setUniformValue(u_yMin, (float) 0.0);
    s_program->setUniformValue(u_yMax, (float) height());
    s_program->setUniformValue(u_color1, QColor(230, 230, 230));
    s_program->setUniformValue(u_color2, QColor(250, 250, 250));

    glDrawArrays(GL_LINES, 0, lineCount * 2);

    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int oldCount = 0;
    for (auto data : m_plotData) {
        QColor color2 = data->color();
        color2.setAlphaF(0.60);
        // Draw the graph
        s_program->setUniformValue(u_yMin, min);
        s_program->setUniformValue(u_yMax, max);
        s_program->setUniformValue(u_color1, data->color());
        s_program->setUniformValue(u_color2, color2);

        glDrawArrays(GL_TRIANGLE_STRIP, lineCount * 2 + oldCount, verticesCounts[data]);
        oldCount = verticesCounts[data];
    }

    glDisable(GL_BLEND);

    if (m_haveMSAA && m_haveFramebufferBlit) {
        // Resolve the MSAA buffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<Texture*>(m_node->texture())->fbo());
        glBlitFramebuffer(0, 0, width(), height(), 0, 0, width(), height(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

        // Delete the render buffer
        glDeleteRenderbuffers(1, &rb);
    }

    // Delete the VBO
    glDeleteBuffers(1, &vbo);
}

QSGNode *Plotter::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *updatePaintNodeData)
{
    Q_UNUSED(updatePaintNodeData)

    QSGSimpleTextureNode *n = static_cast<QSGSimpleTextureNode *>(oldNode);

    if (!n) {
        n = new QSGSimpleTextureNode();
        n->setTexture(new Texture(window()->openglContext()));
        n->setFiltering(QSGTexture::Linear);

        m_node = n;
    }

    if (!m_initialized) {
        glGenFramebuffers(1, &m_fbo);
        connect(window(), &QQuickWindow::beforeRendering, this, &Plotter::render, Qt::DirectConnection);

        QOpenGLContext *ctx = window()->openglContext();
        QPair<int, int> version = ctx->format().version();

        if (ctx->isOpenGLES()) {
            m_haveMSAA = version >= qMakePair(3, 0) || ctx->hasExtension("GL_NV_framebuffer_multisample"); 
            m_haveFramebufferBlit = version >= qMakePair(3, 0) || ctx->hasExtension("GL_NV_framebuffer_blit");
            m_haveInternalFormatQuery = version >= qMakePair(3, 0);
            m_internalFormat = version >= qMakePair(3, 0) ? GL_RGBA8 : GL_RGBA;
        } else {
            m_haveMSAA = version >= qMakePair(3, 2) || ctx->hasExtension("GL_ARB_framebuffer_object") ||
                ctx->hasExtension("GL_EXT_framebuffer_multisample"); 
            m_haveFramebufferBlit = version >= qMakePair(3, 0) || ctx->hasExtension("GL_ARB_framebuffer_object") ||
                ctx->hasExtension("GL_EXT_framebuffer_blit");
            m_haveInternalFormatQuery = version >= qMakePair(4, 2) || ctx->hasExtension("GL_ARB_internalformat_query");
            m_internalFormat = GL_RGBA8;
        }

        // Query the maximum sample count for the internal format
        if (m_haveInternalFormatQuery) {
            int count = 0;
            glGetInternalformativ(GL_RENDERBUFFER, m_internalFormat, GL_NUM_SAMPLE_COUNTS, 1, &count);

            if (count > 0) {
                QVector<int> samples(count);
                glGetInternalformativ(GL_RENDERBUFFER, m_internalFormat, GL_SAMPLES, count, samples.data());

                // The samples are returned in descending order. Choose the highest value.
                m_samples = samples.at(0);
            } else {
                m_samples = 0;
            }
        } else if (m_haveMSAA) {
            glGetIntegerv(GL_MAX_SAMPLES, &m_samples);
        } else {
            m_samples = 0;
        }

        m_initialized = true;
    }

    if (!s_program) {
        s_program = new QOpenGLShaderProgram;
        s_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vs_source);
        s_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fs_source);
        s_program->bindAttributeLocation("vertex", 0);
        s_program->link();

        u_yMin = s_program->uniformLocation("yMin");
        u_yMax = s_program->uniformLocation("yMax");
        u_color1 = s_program->uniformLocation("color1");
        u_color2 = s_program->uniformLocation("color2");
        u_matrix = s_program->uniformLocation("matrix");
    }

    if (n->texture()->textureSize() != boundingRect().size()) {
        static_cast<Texture *>(n->texture())->recreate(boundingRect().size().toSize());
        m_matrix = QMatrix4x4();
        m_matrix.ortho(0, width(), 0, height(), -1, 1);
    }

    //normalize data
    m_max = std::numeric_limits<qreal>::min();
    m_min = std::numeric_limits<qreal>::max();
    qreal adjustedMax = m_max;
    qreal adjustedMin = m_min;
    if (m_stacked) {
        PlotData *previousData = 0;
        QList<PlotData *>::const_iterator i = m_plotData.constEnd();
        do {
            --i;
            PlotData *data = *i;
            data->m_normalizedValues.clear();
            data->m_normalizedValues.resize(data->values().count());
            if (previousData) {
                for (int i = 0; i < data->values().count(); ++i) {
                    data->m_normalizedValues[i] = data->values().value(i) + previousData->m_normalizedValues.value(i);

                    if (data->m_normalizedValues[i] > adjustedMax) {
                        adjustedMax = data->m_normalizedValues[i];
                    }
                    if (data->m_normalizedValues[i] < adjustedMin) {
                        adjustedMin = data->m_normalizedValues[i];
                    }
                }
            } else {
                data->m_normalizedValues = data->values().toVector();
                if (data->max() > adjustedMax) {
                    adjustedMax = data->max();
                }
                if (data->min() < adjustedMin) {
                    adjustedMin = data->min();
                }
            }
            previousData = data;

            //global max and global min
            if (data->max() > m_max) {
                m_max = data->max();
            }
            if (data->min() < m_min) {
                m_min = data->min();
            }
        } while (i != m_plotData.constBegin());

    } else {
        for (auto data : m_plotData) {
            data->m_normalizedValues.clear();
            data->m_normalizedValues = data->values().toVector();
            //global max and global min
            if (data->max() > m_max) {
                adjustedMax = m_max = data->max();
            }
            if (data->min() < m_min) {
                adjustedMin = m_min = data->min();
            }
        }
    }

    if (m_autoRange) {
        //leave some empty space (of a line) top and bottom
        adjustedMax += height()/20;
        adjustedMin -= height()/20;
        qreal adjust;
        //this should never happen, remove?
        if (qFuzzyCompare(adjustedMax - adjustedMin, 0)) {
            adjust = 1;
        } else {
            adjust = (height() / (adjustedMax - adjustedMin));
        }
        //normalizebased on global max and min
        for (auto data : m_plotData) {
            for (int i = 0; i < data->values().count(); ++i) {
                data->m_normalizedValues[i] = (data->m_normalizedValues.value(i) - adjustedMin) * adjust;
            }
        }
    }

    if (window()) {
        window()->update();
    }

    n->setRect(boundingRect());
    return n;
}


#include "plotter.moc"
