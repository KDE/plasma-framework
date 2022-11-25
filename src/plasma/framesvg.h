/*
    SPDX-FileCopyrightText: 2008 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2008 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_FRAMESVG_H
#define PLASMA_FRAMESVG_H

#include <QObject>
#include <QPixmap>

#include <plasma/plasma_export.h>

#include <plasma/plasma.h>
#include <plasma/svg.h>

class QPainter;
class QPoint;
class QPointF;
class QRect;
class QRectF;
class QSize;
class QSizeF;
class QMatrix;

namespace Plasma
{
class FrameSvgPrivate;

/**
 * @class FrameSvg plasma/framesvg.h <Plasma/FrameSvg>
 *
 * @short Provides an SVG with borders.
 *
 * When using SVG images for a background of an object that may change
 * its aspect ratio, such as a dialog, simply scaling a single image
 * may not be enough.
 *
 * FrameSvg allows SVGs to provide several elements for borders as well
 * as a central element, each of which are scaled individually.  These
 * elements should be named
 *
 *  - @c center  - the central element, which will be scaled in both directions
 *  - @c top     - the top border; the height is fixed, but it will be scaled
 *                 horizontally to the same width as @c center
 *  - @c bottom  - the bottom border; scaled in the same way as @c top
 *  - @c left    - the left border; the width is fixed, but it will be scaled
 *                 vertically to the same height as @c center
 *  - @c right   - the right border; scaled in the same way as @c left
 *  - @c topleft - fixed size; must be the same height as @c top and the same
 *                 width as @c left
 *  - @c bottomleft, @c topright, @c bottomright - similar to @c topleft
 *
 * @c center must exist, but all the others are optional.  @c topleft and
 * @c topright will be ignored if @c top does not exist, and similarly for
 * @c bottomleft and @c bottomright.
 *
 * @see Plasma::Svg
 **/
class PLASMA_EXPORT FrameSvg : public Svg
{
    Q_OBJECT

    Q_PROPERTY(EnabledBorders enabledBorders READ enabledBorders WRITE setEnabledBorders)

public:
    /**
     * These flags represents what borders should be drawn
     */
    enum EnabledBorder {
        NoBorder = 0,
        TopBorder = 1,
        BottomBorder = 2,
        LeftBorder = 4,
        RightBorder = 8,
        AllBorders = TopBorder | BottomBorder | LeftBorder | RightBorder,
    };
    Q_DECLARE_FLAGS(EnabledBorders, EnabledBorder)
    Q_FLAG(EnabledBorders)

    /**
     * Constructs a new FrameSvg that paints the proper named subelements
     * as borders. It may also be used as a regular Plasma::Svg object
     * for direct access to elements in the Svg.
     *
     * @param parent options QObject to parent this to
     *
     * @related Plasma::Theme
     */
    explicit FrameSvg(QObject *parent = nullptr);
    ~FrameSvg() override;

    /**
     * Loads a new Svg
     * @param imagePath the new file
     */
    Q_INVOKABLE void setImagePath(const QString &path) override;

    /**
     * Sets what borders should be painted
     * @param flags borders we want to paint
     */
    void setEnabledBorders(const EnabledBorders borders);

    /**
     * Convenience method to get the enabled borders
     * @return what borders are painted
     */
    EnabledBorders enabledBorders() const;

    /**
     * Resize the frame maintaining the same border size
     * @param size the new size of the frame
     */
    Q_INVOKABLE void resizeFrame(const QSizeF &size);

    /**
     * @returns the size of the frame
     */
    Q_INVOKABLE QSizeF frameSize() const;

    /**
     * Returns the margin size given the margin edge we want
     * If the given margin is disabled, it will return 0.
     * If you don't care about the margin being on or off, use fixedMarginSize()
     * @param edge the margin edge we want, top, bottom, left or right
     * @return the margin size
     */
    Q_INVOKABLE qreal marginSize(const Plasma::Types::MarginEdge edge) const;

    /**
     * Convenience method that extracts the size of the four margins
     * in the four output parameters
     * The disabled margins will be 0.
     * If you don't care about the margins being on or off, use getFixedMargins()
     * @param left left margin size
     * @param top top margin size
     * @param right right margin size
     * @param bottom bottom margin size
     */
    Q_INVOKABLE void getMargins(qreal &left, qreal &top, qreal &right, qreal &bottom) const;

    /**
     * Returns the margin size given the margin edge we want.
     * Compared to marginSize(), this doesn't depend whether the margin is enabled or not
     * @param edge the margin edge we want, top, bottom, left or right
     * @return the margin size
     */
    Q_INVOKABLE qreal fixedMarginSize(const Plasma::Types::MarginEdge edge) const;

    /**
     * Convenience method that extracts the size of the four margins
     * in the four output parameters
     * Compared to getMargins(), this doesn't depend whether the margins are enabled or not
     * @param left left margin size
     * @param top top margin size
     * @param right right margin size
     * @param bottom bottom margin size
     */
    Q_INVOKABLE void getFixedMargins(qreal &left, qreal &top, qreal &right, qreal &bottom) const;

    /**
     * Returns the insets margin size given the margin edge we want.
     * @param edge the margin edge we want, top, bottom, left or right
     * @return the margin size
     * @since 5.77
     */
    Q_INVOKABLE qreal insetSize(const Plasma::Types::MarginEdge edge) const;

    /**
     * Convenience method that extracts the size of the four inset margins
     * in the four output parameters
     * @param left left margin size
     * @param top top margin size
     * @param right right margin size
     * @param bottom bottom margin size
     * @since 5.77
     */
    Q_INVOKABLE void getInset(qreal &left, qreal &top, qreal &right, qreal &bottom) const;

    /**
     * @return the rectangle of the center element, taking the margins into account.
     */
    Q_INVOKABLE QRectF contentsRect() const;

    /**
     * Sets the prefix (@see setElementPrefix) to 'north', 'south', 'west' and 'east'
     * when the location is TopEdge, BottomEdge, LeftEdge and RightEdge,
     * respectively. Clears the prefix in other cases.
     *
     * The prefix must exist in the SVG document, which means that this can only be
     * called successfully after setImagePath is called.
     * @param location location in the UI this frame will be drawn
     */
    Q_INVOKABLE void setElementPrefix(Plasma::Types::Location location);

    /**
     * Sets the prefix for the SVG elements to be used for painting. For example,
     * if prefix is 'active', then instead of using the 'top' element of the SVG
     * file to paint the top border, 'active-top' element will be used. The same
     * goes for other SVG elements.
     *
     * If the elements with prefixes are not present, the default ones are used.
     * (for the sake of speed, the test is present only for the 'center' element)
     *
     * Setting the prefix manually resets the location to Floating.
     *
     * The prefix must exist in the SVG document, which means that this can only be
     * called successfully after setImagePath is called.
     *
     * @param prefix prefix for the SVG elements that make up the frame
     */
    Q_INVOKABLE void setElementPrefix(const QString &prefix);

    /**
     * @return true if the svg has the necessary elements with the given prefix
     * to draw a frame
     * @param prefix the given prefix we want to check if drawable (can have trailing '-' since 5.59)
     */
    Q_INVOKABLE bool hasElementPrefix(const QString &prefix) const;

    /**
     * This is an overloaded method provided for convenience equivalent to
     * hasElementPrefix("north"), hasElementPrefix("south")
     * hasElementPrefix("west") and hasElementPrefix("east")
     * @return true if the svg has the necessary elements with the given prefix
     * to draw a frame.
     * @param location the given prefix we want to check if drawable
     */
    Q_INVOKABLE bool hasElementPrefix(Plasma::Types::Location location) const;

    /**
     * Returns the prefix for SVG elements of the FrameSvg (including a '-' at the end if not empty)
     * @return the prefix
     * @sa actualPrefix()
     */
    Q_INVOKABLE QString prefix();

    /**
     * Returns a mask that tightly contains the fully opaque areas of the svg
     * @return a region of opaque areas
     */
    Q_INVOKABLE QRegion mask() const;

    /**
     * @return a pixmap whose alpha channel is the opacity of the frame. It may be the frame itself or a special frame with the mask- prefix
     */
    QPixmap alphaMask() const;

    /**
     * Sets whether saving all the rendered prefixes in a cache or not
     * @param cache if use the cache or not
     */
    Q_INVOKABLE void setCacheAllRenderedFrames(bool cache);

    /**
     * @return if all the different prefixes should be kept in a cache when rendered
     */
    Q_INVOKABLE bool cacheAllRenderedFrames() const;

    /**
     * Deletes the internal cache freeing memory: use this if you want to switch the rendered
     * element and you don't plan to switch back to the previous one for a long time and you
     * used setUsingRenderingCache(true)
     */
    Q_INVOKABLE void clearCache();

    /**
     * Returns a pixmap of the SVG represented by this object.
     *
     * @param elelementId the ID string of the element to render, or an empty
     *                  string for the whole SVG (the default)
     * @return a QPixmap of the rendered SVG
     */
    Q_INVOKABLE QPixmap framePixmap();

    /**
     * Paints the loaded SVG with the elements that represents the border
     * @param painter the QPainter to use
     * @param target the target rectangle on the paint device
     * @param source the portion rectangle of the source image
     */
    Q_INVOKABLE void paintFrame(QPainter *painter, const QRectF &target, const QRectF &source = QRectF());

    /**
     * Paints the loaded SVG with the elements that represents the border
     * This is an overloaded member provided for convenience
     * @param painter the QPainter to use
     * @param pos where to paint the svg
     */
    Q_INVOKABLE void paintFrame(QPainter *painter, const QPointF &pos = QPointF(0, 0));

    /**
     * @returns the prefix that is actually used (including a '-' at the end if not empty)
     * @sa prefix()
     */
    QString actualPrefix() const;

    /**
     * @returns true if we are in a transaction of many changes at once
     * and we don't want to rebuild the generated graphics for each change yet
     * @since 5.31
     */
    bool isRepaintBlocked() const;

    /**
     * If we will do several changes at once in the frame properties,
     * such as prefix, enabled borders and size, in order to not regenerate
     * the graphics for each change, set this property to true, and set
     * it to false again after applying all the changes needed.
     * Note that any change will not be visible in the painted frame while this property is set to true.
     * @since 5.31
     */
    void setRepaintBlocked(bool blocked);

    /**
     * This will return the minimum height required to correctly draw this
     * SVG. 
     * @since 5.101
     */
    Q_INVOKABLE int minimumDrawingHeight();

    /**
     * This will return the minimum width required to correctly draw this
     * SVG. 
     * @since 5.101
     */
    Q_INVOKABLE int minimumDrawingWidth();

private:
    FrameSvgPrivate *const d;
    friend class FrameData;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(FrameSvg::EnabledBorders)

} // Plasma namespace

#endif // multiple inclusion guard
