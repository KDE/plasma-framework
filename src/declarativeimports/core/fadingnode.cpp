/*
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "fadingnode_p.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QSGSimpleMaterialShader>

struct FadingMaterialState {
    QSGTexture *source = nullptr;
    QSGTexture *target = nullptr;
    qreal progress;
};

class FadingMaterialShader : public QSGSimpleMaterialShader<FadingMaterialState>
{
    QSG_DECLARE_SIMPLE_SHADER(FadingMaterialShader, FadingMaterialState)
public:
    FadingMaterialShader();
    using QSGSimpleMaterialShader<FadingMaterialState>::updateState;
    void updateState(const FadingMaterialState *newState, const FadingMaterialState *oldState) override;
    QList<QByteArray> attributes() const override;

    void initialize() override;

private:
    QOpenGLFunctions *glFuncs = nullptr;
    int m_progressId = 0;
    int m_sourceRectId = 0;
    int m_targetRectId = 0;
};

FadingMaterialShader::FadingMaterialShader()
{
    setShaderSourceFile(QOpenGLShader::Fragment, QStringLiteral(":/plasma-framework/shaders/fadingmaterial.frag"));
    setShaderSourceFile(QOpenGLShader::Vertex, QStringLiteral(":/plasma-framework/shaders/fadingmaterial.vert"));
}

QList<QByteArray> FadingMaterialShader::attributes() const
{
    return {QByteArrayLiteral("qt_Vertex"), QByteArrayLiteral("qt_MultiTexCoord0")};
}

void FadingMaterialShader::updateState(const FadingMaterialState *newState, const FadingMaterialState *oldState)
{
    if (!oldState || oldState->source != newState->source) {
        glFuncs->glActiveTexture(GL_TEXTURE0);
        newState->source->bind();
        QRectF rect = newState->source->normalizedTextureSubRect();
        program()->setUniformValue(m_sourceRectId, QVector4D(rect.x(), rect.y(), rect.width(), rect.height()));
    }

    if (!oldState || oldState->target != newState->target) {
        glFuncs->glActiveTexture(GL_TEXTURE1);
        newState->target->bind();
        QRectF rect = newState->target->normalizedTextureSubRect();
        program()->setUniformValue(m_targetRectId, QVector4D(rect.x(), rect.y(), rect.width(), rect.height()));
        // reset the active texture back to 0 after we changed it to something else
        glFuncs->glActiveTexture(GL_TEXTURE0);
    }

    if (!oldState || oldState->progress != newState->progress) {
        program()->setUniformValue(m_progressId, (GLfloat)newState->progress);
    }
}

void FadingMaterialShader::initialize()
{
    if (!program()->isLinked()) {
        // shader not linked, exit otherwise we crash, BUG: 336272
        return;
    }
    QSGSimpleMaterialShader<FadingMaterialState>::initialize();
    glFuncs = QOpenGLContext::currentContext()->functions();
    program()->bind();
    program()->setUniformValue("u_src", 0);
    program()->setUniformValue("u_target", 1);

    m_progressId = program()->uniformLocation("u_transitionProgress");
    m_sourceRectId = program()->uniformLocation("u_src_rect");
    m_targetRectId = program()->uniformLocation("u_target_rect");
}

FadingNode::FadingNode(QSGTexture *source, QSGTexture *target)
    : m_source(source)
    , m_target(target)
{
    QSGSimpleMaterial<FadingMaterialState> *m = FadingMaterialShader::createMaterial();
    m->setFlag(QSGMaterial::Blending);
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
    QSGSimpleMaterial<FadingMaterialState> *m = static_cast<QSGSimpleMaterial<FadingMaterialState> *>(material());
    m->state()->source = m_source.data();
    m->state()->target = m_target.data();
    m->state()->progress = progress;
    markDirty(QSGNode::DirtyMaterial);
}
