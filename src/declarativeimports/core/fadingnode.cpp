/*
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "fadingnode_p.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSGMaterial>
#include <QSGMaterialShader>

class FadingMaterial : public QSGMaterial
{
public:
    FadingMaterial();

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QSGMaterialShader *createShader() const override;
#else
    QSGMaterialShader *createShader(QSGRendererInterface::RenderMode) const override;
#endif
    QSGMaterialType *type() const override;
    int compare(const QSGMaterial *other) const override;

    QSGTexture *source = nullptr;
    QSGTexture *target = nullptr;
    float progress = 0.0f;
};

class FadingMaterialShader : public QSGMaterialShader
{
public:
    FadingMaterialShader();

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const char *const *attributeNames() const override;
    void updateState(const QSGMaterialShader::RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial) override;
    void initialize() override;

private:
    QOpenGLFunctions *glFuncs = nullptr;
    int m_matrixId = 0;
    int m_opacityId = 0;
    int m_progressId = 0;
    int m_sourceRectId = 0;
    int m_targetRectId = 0;
#else
    bool updateUniformData(QSGMaterialShader::RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial) override;
    void
    updateSampledImage(QSGMaterialShader::RenderState &state, int binding, QSGTexture **texture, QSGMaterial *newMaterial, QSGMaterial *oldMaterial) override;
#endif
};

FadingMaterial::FadingMaterial()
{
    setFlag(QSGMaterial::Blending);
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
QSGMaterialShader *FadingMaterial::createShader() const
#else
QSGMaterialShader *FadingMaterial::createShader(QSGRendererInterface::RenderMode) const
#endif
{
    return new FadingMaterialShader;
}

QSGMaterialType *FadingMaterial::type() const
{
    static QSGMaterialType type;
    return &type;
}

int FadingMaterial::compare(const QSGMaterial *other) const
{
    auto material = static_cast<const FadingMaterial *>(other);
    if (material->source == source && material->target == target && qFuzzyCompare(material->progress, progress)) {
        return 0;
    }

    return QSGMaterial::compare(other);
}

FadingMaterialShader::FadingMaterialShader()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    setShaderSourceFile(QOpenGLShader::Fragment, QStringLiteral(":/plasma-framework/shaders/fadingmaterial.frag"));
    setShaderSourceFile(QOpenGLShader::Vertex, QStringLiteral(":/plasma-framework/shaders/fadingmaterial.vert"));
#else
    setShaderFileName(QSGMaterialShader::FragmentStage, QStringLiteral(":/plasma-framework/shaders/fadingmaterial.frag.qsb"));
    setShaderFileName(QSGMaterialShader::VertexStage, QStringLiteral(":/plasma-framework/shaders/fadingmaterial.vert.qsb"));
#endif
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
const char *const *FadingMaterialShader::attributeNames() const
{
    static char const *const names[] = {"qt_Vertex", "qt_MultiTexCoord0", nullptr};
    return names;
}

void FadingMaterialShader::updateState(const QSGMaterialShader::RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial)
{
    auto p = program();

    if (state.isMatrixDirty()) {
        p->setUniformValue(m_matrixId, state.combinedMatrix());
    }

    if (state.isOpacityDirty()) {
        p->setUniformValue(m_opacityId, state.opacity());
    }

    if (!oldMaterial || newMaterial->compare(oldMaterial) != 0) {
        auto material = static_cast<FadingMaterial *>(newMaterial);
        glFuncs->glActiveTexture(GL_TEXTURE0);
        material->source->bind();
        QRectF rect = material->source->normalizedTextureSubRect();
        p->setUniformValue(m_sourceRectId, QVector4D(rect.x(), rect.y(), rect.width(), rect.height()));

        glFuncs->glActiveTexture(GL_TEXTURE1);
        material->target->bind();
        rect = material->target->normalizedTextureSubRect();
        p->setUniformValue(m_targetRectId, QVector4D(rect.x(), rect.y(), rect.width(), rect.height()));
        // reset the active texture back to 0 after we changed it to something else
        glFuncs->glActiveTexture(GL_TEXTURE0);

        p->setUniformValue(m_progressId, (GLfloat)material->progress);
    }
}

void FadingMaterialShader::initialize()
{
    if (!program()->isLinked()) {
        // shader not linked, exit otherwise we crash, BUG: 336272
        return;
    }
    QSGMaterialShader::initialize();
    glFuncs = QOpenGLContext::currentContext()->functions();
    program()->bind();
    program()->setUniformValue("u_src", 0);
    program()->setUniformValue("u_target", 1);

    m_matrixId = program()->uniformLocation("qt_Matrix");
    m_opacityId = program()->uniformLocation("qt_Opacity");
    m_progressId = program()->uniformLocation("u_transitionProgress");
    m_sourceRectId = program()->uniformLocation("u_src_rect");
    m_targetRectId = program()->uniformLocation("u_target_rect");
}
#else
bool FadingMaterialShader::updateUniformData(QSGMaterialShader::RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial)
{
    bool changed = false;
    QByteArray *buf = state.uniformData();
    Q_ASSERT(buf->size() >= 104);

    if (state.isMatrixDirty()) {
        const QMatrix4x4 m = state.combinedMatrix();
        memcpy(buf->data(), m.constData(), 64);
        changed = true;
    }

    if (state.isOpacityDirty()) {
        const float opacity = state.opacity();
        memcpy(buf->data() + 100, &opacity, 4);
        changed = true;
    }

    if (!oldMaterial || newMaterial->compare(oldMaterial) != 0) {
        const auto material = static_cast<FadingMaterial *>(newMaterial);

        QRectF rect = material->source->normalizedTextureSubRect();
        QVector4D v(rect.x(), rect.y(), rect.width(), rect.height());
        memcpy(buf->data() + 64, &v, 16);
        rect = material->target->normalizedTextureSubRect();
        v = QVector4D(rect.x(), rect.y(), rect.width(), rect.height());
        memcpy(buf->data() + 80, &v, 16);
        memcpy(buf->data() + 96, &material->progress, 4);
        changed = true;
    }

    return changed;
}

void FadingMaterialShader::updateSampledImage(QSGMaterialShader::RenderState &state,
                                              int binding,
                                              QSGTexture **texture,
                                              QSGMaterial *newMaterial,
                                              QSGMaterial *oldMaterial)
{
    Q_UNUSED(state);
    Q_UNUSED(oldMaterial);
    switch (binding) {
    case 1:
        *texture = static_cast<FadingMaterial *>(newMaterial)->source;
        break;
    case 2:
        *texture = static_cast<FadingMaterial *>(newMaterial)->target;
        break;
    }
}
#endif

FadingNode::FadingNode(QSGTexture *source, QSGTexture *target)
    : m_source(source)
    , m_target(target)
{
    auto *m = new FadingMaterial();
    setMaterial(m);
    setFlag(OwnsMaterial, true);
    setProgress(1.0);

    QSGGeometry *g = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4);
    QSGGeometry::updateTexturedRectGeometry(g, QRect(), QRect());
    setGeometry(g);
    setFlag(QSGNode::OwnsGeometry, true);
}

FadingNode::~FadingNode()
{
}

void FadingNode::setRect(const QRectF &bounds)
{
    QSGGeometry::updateTexturedRectGeometry(geometry(), bounds, QRectF(0, 0, 1, 1));
    markDirty(QSGNode::DirtyGeometry);
}

void FadingNode::setProgress(qreal progress)
{
    auto *m = static_cast<FadingMaterial *>(material());
    m->source = m_source.get();
    m->target = m_target.get();
    m->progress = progress;
    markDirty(QSGNode::DirtyMaterial);
}
