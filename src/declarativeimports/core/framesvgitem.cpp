/*
    SPDX-FileCopyrightText: 2010 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "framesvgitem.h"

#include <QQuickWindow>
#include <QSGGeometry>
#include <QSGTexture>

#include <QDebug>
#include <QPainter>

#include <plasma/private/framesvg_helpers.h>
#include <plasma/private/framesvg_p.h>

#include <QuickAddons/ImageTexturesCache>
#include <QuickAddons/ManagedTextureNode>

#include <cmath> //floor()

namespace Plasma
{
Q_GLOBAL_STATIC(ImageTexturesCache, s_cache)

class FrameNode : public QSGNode
{
public:
    FrameNode(const QString &prefix, FrameSvg *svg)
        : QSGNode()
        , leftWidth(0)
        , rightWidth(0)
        , topHeight(0)
        , bottomHeight(0)
    {
        if (svg->enabledBorders() & FrameSvg::LeftBorder) {
            leftWidth = svg->elementSize(prefix % QLatin1String("left")).width();
        }
        if (svg->enabledBorders() & FrameSvg::RightBorder) {
            rightWidth = svg->elementSize(prefix % QLatin1String("right")).width();
        }
        if (svg->enabledBorders() & FrameSvg::TopBorder) {
            topHeight = svg->elementSize(prefix % QLatin1String("top")).height();
        }
        if (svg->enabledBorders() & FrameSvg::BottomBorder) {
            bottomHeight = svg->elementSize(prefix % QLatin1String("bottom")).height();
        }
    }

    QRect contentsRect(const QSize &size) const
    {
        const QSize contentSize(size.width() - leftWidth - rightWidth, size.height() - topHeight - bottomHeight);

        return QRect(QPoint(leftWidth, topHeight), contentSize);
    }

private:
    int leftWidth;
    int rightWidth;
    int topHeight;
    int bottomHeight;
};

class FrameItemNode : public ManagedTextureNode
{
public:
    enum FitMode {
        // render SVG at native resolution then stretch it in openGL
        FastStretch,
        // on resize re-render the part of the frame from the SVG
        Stretch,
        Tile,
    };

    FrameItemNode(FrameSvgItem *frameSvg, FrameSvg::EnabledBorders borders, FitMode fitMode, QSGNode *parent)
        : ManagedTextureNode()
        , m_frameSvg(frameSvg)
        , m_border(borders)
        , m_lastParent(parent)
        , m_fitMode(fitMode)
    {
        m_lastParent->appendChildNode(this);

        if (m_fitMode == Tile) {
            if (m_border == FrameSvg::TopBorder || m_border == FrameSvg::BottomBorder || m_border == FrameSvg::NoBorder) {
                static_cast<QSGTextureMaterial *>(material())->setHorizontalWrapMode(QSGTexture::Repeat);
                static_cast<QSGOpaqueTextureMaterial *>(opaqueMaterial())->setHorizontalWrapMode(QSGTexture::Repeat);
            }
            if (m_border == FrameSvg::LeftBorder || m_border == FrameSvg::RightBorder || m_border == FrameSvg::NoBorder) {
                static_cast<QSGTextureMaterial *>(material())->setVerticalWrapMode(QSGTexture::Repeat);
                static_cast<QSGOpaqueTextureMaterial *>(opaqueMaterial())->setVerticalWrapMode(QSGTexture::Repeat);
            }
        }

        if (m_fitMode == Tile || m_fitMode == FastStretch) {
            QString elementId = m_frameSvg->frameSvg()->actualPrefix() + FrameSvgHelpers::borderToElementId(m_border);
            m_elementNativeSize = m_frameSvg->frameSvg()->elementSize(elementId);

            if (m_elementNativeSize.isEmpty()) {
                // if the default element is empty, we can avoid the slower tiling path
                // this also avoids a divide by 0 error
                m_fitMode = FastStretch;
            }

            updateTexture(m_elementNativeSize, elementId);
        }
    }

    void updateTexture(const QSize &size, const QString &elementId)
    {
        QQuickWindow::CreateTextureOptions options;
        if (m_fitMode != Tile) {
            options = QQuickWindow::TextureCanUseAtlas;
        }
        setTexture(s_cache->loadTexture(m_frameSvg->window(), m_frameSvg->frameSvg()->image(size, elementId), options));
    }

    void reposition(const QRect &frameGeometry, QSize &fullSize)
    {
        QRect nodeRect = FrameSvgHelpers::sectionRect(m_border, frameGeometry, fullSize);

        // ensure we're not passing a weird rectangle to updateTexturedRectGeometry
        if (!nodeRect.isValid() || nodeRect.isEmpty()) {
            nodeRect = QRect();
        }

        // the position of the relevant texture within this texture ID.
        // for atlas' this will only be a small part of the texture
        QRectF textureRect;

        if (m_fitMode == Tile) {
            textureRect = QRectF(0, 0, 1, 1); // we can never be in an atlas for tiled images.

            // if tiling horizontally
            if (m_border == FrameSvg::TopBorder || m_border == FrameSvg::BottomBorder || m_border == FrameSvg::NoBorder) {
                // cmp. CSS3's border-image-repeat: "repeat", though with first tile not centered, but aligned to left
                textureRect.setWidth((qreal)nodeRect.width() / m_elementNativeSize.width());
            }
            // if tiling vertically
            if (m_border == FrameSvg::LeftBorder || m_border == FrameSvg::RightBorder || m_border == FrameSvg::NoBorder) {
                // cmp. CSS3's border-image-repeat: "repeat", though with first tile not centered, but aligned to top
                textureRect.setHeight((qreal)nodeRect.height() / m_elementNativeSize.height());
            }
        } else if (m_fitMode == Stretch) {
            QString prefix = m_frameSvg->frameSvg()->actualPrefix();

            QString elementId = prefix + FrameSvgHelpers::borderToElementId(m_border);

            // re-render the SVG at new size
            updateTexture(nodeRect.size(), elementId);
            textureRect = texture()->normalizedTextureSubRect();
        } else if (texture()) { // for fast stretch.
            textureRect = texture()->normalizedTextureSubRect();
        }

        QSGGeometry::updateTexturedRectGeometry(geometry(), nodeRect, textureRect);
        markDirty(QSGNode::DirtyGeometry);
    }

private:
    FrameSvgItem *m_frameSvg;
    FrameSvg::EnabledBorders m_border;
    QSGNode *m_lastParent;
    QSize m_elementNativeSize;
    FitMode m_fitMode;
};

FrameSvgItemMargins::FrameSvgItemMargins(Plasma::FrameSvg *frameSvg, QObject *parent)
    : QObject(parent)
    , m_frameSvg(frameSvg)
    , m_fixed(false)
    , m_inset(false)
{
    // qDebug() << "margins at: " << left() << top() << right() << bottom();
}

qreal FrameSvgItemMargins::left() const
{
    if (m_fixed) {
        return m_frameSvg->fixedMarginSize(Types::LeftMargin);
    } else if (m_inset) {
        return m_frameSvg->insetSize(Types::LeftMargin);
    } else {
        return m_frameSvg->marginSize(Types::LeftMargin);
    }
}

qreal FrameSvgItemMargins::top() const
{
    if (m_fixed) {
        return m_frameSvg->fixedMarginSize(Types::TopMargin);
    } else if (m_inset) {
        return m_frameSvg->insetSize(Types::TopMargin);
    } else {
        return m_frameSvg->marginSize(Types::TopMargin);
    }
}

qreal FrameSvgItemMargins::right() const
{
    if (m_fixed) {
        return m_frameSvg->fixedMarginSize(Types::RightMargin);
    } else if (m_inset) {
        return m_frameSvg->insetSize(Types::RightMargin);
    } else {
        return m_frameSvg->marginSize(Types::RightMargin);
    }
}

qreal FrameSvgItemMargins::bottom() const
{
    if (m_fixed) {
        return m_frameSvg->fixedMarginSize(Types::BottomMargin);
    } else if (m_inset) {
        return m_frameSvg->insetSize(Types::BottomMargin);
    } else {
        return m_frameSvg->marginSize(Types::BottomMargin);
    }
}

qreal FrameSvgItemMargins::horizontal() const
{
    return left() + right();
}

qreal FrameSvgItemMargins::vertical() const
{
    return top() + bottom();
}

QVector<qreal> FrameSvgItemMargins::margins() const
{
    qreal left;
    qreal top;
    qreal right;
    qreal bottom;
    m_frameSvg->getMargins(left, top, right, bottom);
    return {left, top, right, bottom};
}

void FrameSvgItemMargins::update()
{
    Q_EMIT marginsChanged();
}

void FrameSvgItemMargins::setFixed(bool fixed)
{
    if (fixed == m_fixed) {
        return;
    }

    m_fixed = fixed;
    Q_EMIT marginsChanged();
}

bool FrameSvgItemMargins::isFixed() const
{
    return m_fixed;
}

void FrameSvgItemMargins::setInset(bool inset)
{
    if (inset == m_inset) {
        return;
    }

    m_inset = inset;
    Q_EMIT marginsChanged();
}

bool FrameSvgItemMargins::isInset() const
{
    return m_inset;
}

FrameSvgItem::FrameSvgItem(QQuickItem *parent)
    : QQuickItem(parent)
    , m_margins(nullptr)
    , m_fixedMargins(nullptr)
    , m_insetMargins(nullptr)
    , m_textureChanged(false)
    , m_sizeChanged(false)
    , m_fastPath(true)
{
    m_frameSvg = new Plasma::FrameSvg(this);
    setFlag(ItemHasContents, true);
    connect(m_frameSvg, &FrameSvg::repaintNeeded, this, &FrameSvgItem::doUpdate);
    connect(&Units::instance(), &Units::devicePixelRatioChanged, this, &FrameSvgItem::updateDevicePixelRatio);
    connect(m_frameSvg, &Svg::fromCurrentThemeChanged, this, &FrameSvgItem::fromCurrentThemeChanged);
    connect(m_frameSvg, &Svg::statusChanged, this, &FrameSvgItem::statusChanged);
}

FrameSvgItem::~FrameSvgItem()
{
}

class CheckMarginsChange
{
public:
    CheckMarginsChange(QVector<qreal> &oldMargins, FrameSvgItemMargins *marginsObject)
        : m_oldMargins(oldMargins)
        , m_marginsObject(marginsObject)
    {
    }

    ~CheckMarginsChange()
    {
        const QVector<qreal> oldMarginsBefore = m_oldMargins;
        m_oldMargins = m_marginsObject ? m_marginsObject->margins() : QVector<qreal>();

        if (oldMarginsBefore != m_oldMargins) {
            m_marginsObject->update();
        }
    }

private:
    QVector<qreal> &m_oldMargins;
    FrameSvgItemMargins *const m_marginsObject;
};

void FrameSvgItem::setImagePath(const QString &path)
{
    if (m_frameSvg->imagePath() == path) {
        return;
    }

    CheckMarginsChange checkMargins(m_oldMargins, m_margins);
    CheckMarginsChange checkFixedMargins(m_oldFixedMargins, m_fixedMargins);
    CheckMarginsChange checkInsetMargins(m_oldInsetMargins, m_insetMargins);

    updateDevicePixelRatio();
    m_frameSvg->setImagePath(path);

    if (implicitWidth() <= 0) {
        setImplicitWidth(m_frameSvg->marginSize(Plasma::Types::LeftMargin) + m_frameSvg->marginSize(Plasma::Types::RightMargin));
    }

    if (implicitHeight() <= 0) {
        setImplicitHeight(m_frameSvg->marginSize(Plasma::Types::TopMargin) + m_frameSvg->marginSize(Plasma::Types::BottomMargin));
    }

    Q_EMIT imagePathChanged();

    if (isComponentComplete()) {
        applyPrefixes();

        m_frameSvg->resizeFrame(QSizeF(width(), height()));
        m_textureChanged = true;
        update();
    }
}

QString FrameSvgItem::imagePath() const
{
    return m_frameSvg->imagePath();
}

void FrameSvgItem::setPrefix(const QVariant &prefixes)
{
    QStringList prefixList;
    // is this a simple string?
    if (prefixes.canConvert<QString>()) {
        prefixList << prefixes.toString();
    } else if (prefixes.canConvert<QStringList>()) {
        prefixList = prefixes.toStringList();
    }

    if (m_prefixes == prefixList) {
        return;
    }

    CheckMarginsChange checkMargins(m_oldMargins, m_margins);
    CheckMarginsChange checkFixedMargins(m_oldFixedMargins, m_fixedMargins);
    CheckMarginsChange checkInsetMargins(m_oldInsetMargins, m_insetMargins);

    m_prefixes = prefixList;
    applyPrefixes();

    if (implicitWidth() <= 0) {
        setImplicitWidth(m_frameSvg->marginSize(Plasma::Types::LeftMargin) + m_frameSvg->marginSize(Plasma::Types::RightMargin));
    }

    if (implicitHeight() <= 0) {
        setImplicitHeight(m_frameSvg->marginSize(Plasma::Types::TopMargin) + m_frameSvg->marginSize(Plasma::Types::BottomMargin));
    }

    Q_EMIT prefixChanged();

    if (isComponentComplete()) {
        m_frameSvg->resizeFrame(QSizeF(width(), height()));
        m_textureChanged = true;
        update();
    }
}

QVariant FrameSvgItem::prefix() const
{
    return m_prefixes;
}

QString FrameSvgItem::usedPrefix() const
{
    return m_frameSvg->prefix();
}

FrameSvgItemMargins *FrameSvgItem::margins()
{
    if (!m_margins) {
        m_margins = new FrameSvgItemMargins(m_frameSvg, this);
    }
    return m_margins;
}

FrameSvgItemMargins *FrameSvgItem::fixedMargins()
{
    if (!m_fixedMargins) {
        m_fixedMargins = new FrameSvgItemMargins(m_frameSvg, this);
        m_fixedMargins->setFixed(true);
    }
    return m_fixedMargins;
}

FrameSvgItemMargins *FrameSvgItem::inset()
{
    if (!m_insetMargins) {
        m_insetMargins = new FrameSvgItemMargins(m_frameSvg, this);
        m_insetMargins->setInset(true);
    }
    return m_insetMargins;
}

void FrameSvgItem::setColorGroup(Plasma::Theme::ColorGroup group)
{
    if (m_frameSvg->colorGroup() == group) {
        return;
    }

    m_frameSvg->setColorGroup(group);

    Q_EMIT colorGroupChanged();
}

Plasma::Theme::ColorGroup FrameSvgItem::colorGroup() const
{
    return m_frameSvg->colorGroup();
}

bool FrameSvgItem::fromCurrentTheme() const
{
    return m_frameSvg->fromCurrentTheme();
}

void FrameSvgItem::setStatus(Plasma::Svg::Status status)
{
    m_frameSvg->setStatus(status);
}

Plasma::Svg::Status FrameSvgItem::status() const
{
    return m_frameSvg->status();
}

void FrameSvgItem::setEnabledBorders(const Plasma::FrameSvg::EnabledBorders borders)
{
    if (m_frameSvg->enabledBorders() == borders) {
        return;
    }

    CheckMarginsChange checkMargins(m_oldMargins, m_margins);

    m_frameSvg->setEnabledBorders(borders);
    Q_EMIT enabledBordersChanged();
    m_textureChanged = true;
    update();
}

Plasma::FrameSvg::EnabledBorders FrameSvgItem::enabledBorders() const
{
    return m_frameSvg->enabledBorders();
}

bool FrameSvgItem::hasElementPrefix(const QString &prefix) const
{
    return m_frameSvg->hasElementPrefix(prefix);
}

QRegion FrameSvgItem::mask() const
{
    return m_frameSvg->mask();
}

void FrameSvgItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    const bool isComponentComplete = this->isComponentComplete();
    if (isComponentComplete) {
        m_frameSvg->resizeFrame(newGeometry.size());
        m_sizeChanged = true;
    }

    QQuickItem::geometryChanged(newGeometry, oldGeometry);

    // the above only triggers updatePaintNode, so we have to inform subscribers
    // about the potential change of the mask explicitly here
    if (isComponentComplete) {
        Q_EMIT maskChanged();
    }
}

void FrameSvgItem::doUpdate()
{
    if (m_frameSvg->isRepaintBlocked()) {
        return;
    }

    CheckMarginsChange checkMargins(m_oldMargins, m_margins);
    CheckMarginsChange checkFixedMargins(m_oldFixedMargins, m_fixedMargins);
    CheckMarginsChange checkInsetMargins(m_oldInsetMargins, m_insetMargins);

    // if the theme changed, the available prefix may have changed as well
    applyPrefixes();

    if (implicitWidth() <= 0) {
        setImplicitWidth(m_frameSvg->marginSize(Plasma::Types::LeftMargin) + m_frameSvg->marginSize(Plasma::Types::RightMargin));
    }

    if (implicitHeight() <= 0) {
        setImplicitHeight(m_frameSvg->marginSize(Plasma::Types::TopMargin) + m_frameSvg->marginSize(Plasma::Types::BottomMargin));
    }

    QString prefix = m_frameSvg->actualPrefix();
    bool hasOverlay = (!prefix.startsWith(QLatin1String("mask-")) //
                       && m_frameSvg->hasElement(prefix % QLatin1String("overlay")));
    bool hasComposeOverBorder = m_frameSvg->hasElement(prefix % QLatin1String("hint-compose-over-border"))
        && m_frameSvg->hasElement(QLatin1String("mask-") % prefix % QLatin1String("center"));
    m_fastPath = !hasOverlay && !hasComposeOverBorder;

    // Software rendering (at time of writing Qt5.10) doesn't seem to like our
    // tiling/stretching in the 9-tiles.
    // Also when using QPainter it's arguably faster to create and cache pixmaps
    // of the whole frame, which is what the slow path does
    if (QQuickWindow::sceneGraphBackend() == QLatin1String("software")) {
        m_fastPath = false;
    }
    m_textureChanged = true;

    update();

    Q_EMIT maskChanged();
    Q_EMIT repaintNeeded();
}

Plasma::FrameSvg *FrameSvgItem::frameSvg() const
{
    return m_frameSvg;
}

QSGNode *FrameSvgItem::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *)
{
    if (!window() || !m_frameSvg //
        || (!m_frameSvg->hasElementPrefix(m_frameSvg->actualPrefix()) //
            && !m_frameSvg->hasElementPrefix(m_frameSvg->prefix()))) {
        delete oldNode;
        return nullptr;
    }

    const QSGTexture::Filtering filtering = smooth() ? QSGTexture::Linear : QSGTexture::Nearest;

    if (m_fastPath) {
        if (m_textureChanged) {
            delete oldNode;
            oldNode = nullptr;
        }

        if (!oldNode) {
            QString prefix = m_frameSvg->actualPrefix();
            oldNode = new FrameNode(prefix, m_frameSvg);

            bool tileCenter = (m_frameSvg->hasElement(QStringLiteral("hint-tile-center")) //
                               || m_frameSvg->hasElement(prefix % QLatin1String("hint-tile-center")));
            bool stretchBorders = (m_frameSvg->hasElement(QStringLiteral("hint-stretch-borders")) //
                                   || m_frameSvg->hasElement(prefix % QLatin1String("hint-stretch-borders")));
            FrameItemNode::FitMode borderFitMode = stretchBorders ? FrameItemNode::Stretch : FrameItemNode::Tile;
            FrameItemNode::FitMode centerFitMode = tileCenter ? FrameItemNode::Tile : FrameItemNode::Stretch;

            new FrameItemNode(this, FrameSvg::NoBorder, centerFitMode, oldNode);
            if (enabledBorders() & (FrameSvg::TopBorder | FrameSvg::LeftBorder)) {
                new FrameItemNode(this, FrameSvg::TopBorder | FrameSvg::LeftBorder, FrameItemNode::FastStretch, oldNode);
            }
            if (enabledBorders() & (FrameSvg::TopBorder | FrameSvg::RightBorder)) {
                new FrameItemNode(this, FrameSvg::TopBorder | FrameSvg::RightBorder, FrameItemNode::FastStretch, oldNode);
            }
            if (enabledBorders() & FrameSvg::TopBorder) {
                new FrameItemNode(this, FrameSvg::TopBorder, borderFitMode, oldNode);
            }
            if (enabledBorders() & FrameSvg::BottomBorder) {
                new FrameItemNode(this, FrameSvg::BottomBorder, borderFitMode, oldNode);
            }
            if (enabledBorders() & (FrameSvg::BottomBorder | FrameSvg::LeftBorder)) {
                new FrameItemNode(this, FrameSvg::BottomBorder | FrameSvg::LeftBorder, FrameItemNode::FastStretch, oldNode);
            }
            if (enabledBorders() & (FrameSvg::BottomBorder | FrameSvg::RightBorder)) {
                new FrameItemNode(this, FrameSvg::BottomBorder | FrameSvg::RightBorder, FrameItemNode::FastStretch, oldNode);
            }
            if (enabledBorders() & FrameSvg::LeftBorder) {
                new FrameItemNode(this, FrameSvg::LeftBorder, borderFitMode, oldNode);
            }
            if (enabledBorders() & FrameSvg::RightBorder) {
                new FrameItemNode(this, FrameSvg::RightBorder, borderFitMode, oldNode);
            }

            m_sizeChanged = true;
            m_textureChanged = false;
        }

        QSGNode *node = oldNode->firstChild();
        while (node) {
            static_cast<FrameItemNode *>(node)->setFiltering(filtering);
            node = node->nextSibling();
        }

        if (m_sizeChanged) {
            FrameNode *frameNode = static_cast<FrameNode *>(oldNode);
            QSize frameSize(width(), height());
            QRect geometry = frameNode->contentsRect(frameSize);
            QSGNode *node = oldNode->firstChild();
            while (node) {
                static_cast<FrameItemNode *>(node)->reposition(geometry, frameSize);
                node = node->nextSibling();
            }

            m_sizeChanged = false;
        }
    } else {
        ManagedTextureNode *textureNode = dynamic_cast<ManagedTextureNode *>(oldNode);
        if (!textureNode) {
            delete oldNode;
            textureNode = new ManagedTextureNode;
            m_textureChanged = true; // force updating the texture on our newly created node
            oldNode = textureNode;
        }
        textureNode->setFiltering(filtering);

        if ((m_textureChanged || m_sizeChanged) || textureNode->texture()->textureSize() != m_frameSvg->size()) {
            QImage image = m_frameSvg->framePixmap().toImage();
            textureNode->setTexture(s_cache->loadTexture(window(), image));
            textureNode->setRect(0, 0, width(), height());

            m_textureChanged = false;
            m_sizeChanged = false;
        }
    }

    return oldNode;
}

void FrameSvgItem::classBegin()
{
    QQuickItem::classBegin();
    m_frameSvg->setRepaintBlocked(true);
}

void FrameSvgItem::componentComplete()
{
    CheckMarginsChange checkMargins(m_oldMargins, m_margins);
    CheckMarginsChange checkFixedMargins(m_oldFixedMargins, m_fixedMargins);
    CheckMarginsChange checkInsetMargins(m_oldInsetMargins, m_insetMargins);

    QQuickItem::componentComplete();
    m_frameSvg->resizeFrame(QSize(width(), height()));
    m_frameSvg->setRepaintBlocked(false);
    m_textureChanged = true;
}

void FrameSvgItem::updateDevicePixelRatio()
{
    m_frameSvg->setScaleFactor(qMax<qreal>(1.0, floor(Units::instance().devicePixelRatio())));

    // devicepixelratio is always set integer in the svg, so needs at least 192dpi to double up.
    //(it needs to be integer to have lines contained inside a svg piece to keep being pixel aligned)
    const auto newDevicePixelRation = qMax<qreal>(1.0, floor(window() ? window()->devicePixelRatio() : qApp->devicePixelRatio()));

    if (newDevicePixelRation != m_frameSvg->devicePixelRatio()) {
        m_frameSvg->setDevicePixelRatio(qMax<qreal>(1.0, newDevicePixelRation));
        m_textureChanged = true;
    }
}

void FrameSvgItem::applyPrefixes()
{
    if (m_frameSvg->imagePath().isEmpty()) {
        return;
    }

    const QString oldPrefix = m_frameSvg->prefix();

    if (m_prefixes.isEmpty()) {
        m_frameSvg->setElementPrefix(QString());
        if (oldPrefix != m_frameSvg->prefix()) {
            Q_EMIT usedPrefixChanged();
        }
        return;
    }

    bool found = false;
    for (const QString &prefix : std::as_const(m_prefixes)) {
        if (m_frameSvg->hasElementPrefix(prefix)) {
            m_frameSvg->setElementPrefix(prefix);
            found = true;
            break;
        }
    }
    if (!found) {
        // this setElementPrefix is done to keep the same behavior as before, when it was a simple string
        m_frameSvg->setElementPrefix(m_prefixes.constLast());
    }
    if (oldPrefix != m_frameSvg->prefix()) {
        Q_EMIT usedPrefixChanged();
    }
}

void FrameSvgItem::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &value)
{
    if (change == ItemSceneChange && value.window) {
        updateDevicePixelRatio();
    }

    QQuickItem::itemChange(change, value);
}

} // Plasma namespace
