/*
    SPDX-FileCopyrightText: 2010 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef SVGITEM_P
#define SVGITEM_P

#include <QImage>
#include <QQuickItem>

#include "units.h"

namespace Plasma
{
class Svg;

/**
 * @class SvgItem
 * @short Displays an SVG or an element from an SVG file
 */
class SvgItem : public QQuickItem
{
    Q_OBJECT

    /**
     * The sub element of the svg we want to render. If empty the whole svg document will be painted.
     */
    Q_PROPERTY(QString elementId READ elementId WRITE setElementId NOTIFY elementIdChanged)

    /**
     * Svg class that is the source of the image, use it like that:
     * @code
     * SvgItem {
     *     svg: Svg {imagePath: "widgets/arrows"}
     *     elementId: "arrow-left"
     * }
     * @endcode
     * Instead of a Svg declaration it can also be the id of a Svg declared elsewhere, useful to share Svg instances.
     */
    Q_PROPERTY(Plasma::Svg *svg READ svg WRITE setSvg NOTIFY svgChanged)

    /**
     * The natural, unscaled size of the svg document or the element. useful if a pixel perfect rendering of outlines is needed.
     */
    Q_PROPERTY(QSizeF naturalSize READ naturalSize NOTIFY naturalSizeChanged)

public:
    /// @cond INTERNAL_DOCS

    explicit SvgItem(QQuickItem *parent = nullptr);
    ~SvgItem() override;

    void setElementId(const QString &elementID);
    QString elementId() const;

    void setSvg(Plasma::Svg *svg);
    Plasma::Svg *svg() const;

    QSizeF naturalSize() const;

    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *updatePaintNodeData) override;
    /// @endcond

Q_SIGNALS:
    void elementIdChanged();
    void svgChanged();
    void naturalSizeChanged();

protected Q_SLOTS:
    /// @cond INTERNAL_DOCS
    void updateNeeded();
    void updateDevicePixelRatio();
    /// @endcond

private:
    void scheduleImageUpdate();
    void updatePolish() override;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
#else
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
#endif

    QPointer<Plasma::Svg> m_svg;
    QString m_elementID;
    bool m_textureChanged;
    QImage m_image;
};
}

#endif
