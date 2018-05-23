/***************************************************************************
 *   Copyright 2010 Marco Martin <mart@kde.org>                            *
 *   Copyright 2014 David Edmundson <davidedmundson@kde.org>               *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/
#ifndef FRAMESVGITEM_P
#define FRAMESVGITEM_P

#include <QQuickItem>
#include <QQmlParserStatus>

#include <Plasma/FrameSvg>

#include "units.h"

namespace Plasma
{

class FrameSvg;

/**
 * @class FrameSvgItemMargins
 *
 * @short The sizes of a frame's margins.
 */
class FrameSvgItemMargins : public QObject
{
    Q_OBJECT

    /**
     * Width in pixels of the left margin.
     */
    Q_PROPERTY(qreal left READ left NOTIFY marginsChanged)

    /**
     * Height in pixels of the top margin.
     */
    Q_PROPERTY(qreal top READ top NOTIFY marginsChanged)

    /**
     * Width in pixels of the right margin.
     */
    Q_PROPERTY(qreal right READ right NOTIFY marginsChanged)

    /**
     * Height in pixels of the bottom margin.
     */
    Q_PROPERTY(qreal bottom READ bottom NOTIFY marginsChanged)

    /**
     * Width in pixels of the left and right margins combined.
     */
    Q_PROPERTY(qreal horizontal READ horizontal NOTIFY marginsChanged)

    /**
     * Height in pixels of the top and bottom margins combined.
     */
    Q_PROPERTY(qreal vertical READ vertical NOTIFY marginsChanged)


public:
    FrameSvgItemMargins(Plasma::FrameSvg *frameSvg, QObject *parent = nullptr);

    qreal left() const;
    qreal top() const;
    qreal right() const;
    qreal bottom() const;
    qreal horizontal() const;
    qreal vertical() const;

    /// returns a vector with left, top, right, bottom
    QVector<qreal> margins() const;

    void setFixed(bool fixed);
    bool isFixed() const;

public Q_SLOTS:
    void update();

Q_SIGNALS:
    void marginsChanged();

private:
    FrameSvg *m_frameSvg;
    bool m_fixed;
};


/**
 * @class FrameSvgItem
 *
 * @short Provides an SVG with borders.
 *
 * It is exposed as org.kde.plasma.core.FrameSvgItem
 */
class FrameSvgItem : public QQuickItem
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    /**
     * Theme relative path of the svg, like "widgets/background"
     */
    Q_PROPERTY(QString imagePath READ imagePath WRITE setImagePath NOTIFY imagePathChanged)

    /**
     * prefix for the 9 piece svg, like "pushed" or "normal" for the button
     * see http://techbase.kde.org/Development/Tutorials/Plasma/ThemeDetails
     * for a list of paths and prefixes
     * It can also be an array of strings, specifying a fallback chain in case
     * the first element isn't found in the theme, eg ["toolbutton-normal", "normal"]
     * so it's easy to keep backwards compatibility with old themes
     * (Note: fallback chain is supported only @since 5.32)
     */
    Q_PROPERTY(QVariant prefix READ prefix WRITE setPrefix NOTIFY prefixChanged)

    /**
     * the actual prefix that was used, if a fallback chain array was set as "prefix"
     * @since 5.34
     */
    Q_PROPERTY(QString usedPrefix READ usedPrefix NOTIFY usedPrefixChanged)

    /**
     * The margins of the frame, read only
     * @see FrameSvgItemMargins
     */
    Q_PROPERTY(QObject *margins READ margins CONSTANT)

    /**
     * The margins of the frame, regardless if they are enabled or not
     * read only
     * @see FrameSvgItemMargins
     */
    Q_PROPERTY(QObject *fixedMargins READ fixedMargins CONSTANT)

    /**
     * The borders that will be rendered, it's a flag combination of:
     *  NoBorder
     *  TopBorder
     *  BottomBorder
     *  LeftBorder
     *  RightBorder
     */
    Q_PROPERTY(Plasma::FrameSvg::EnabledBorders enabledBorders READ enabledBorders WRITE setEnabledBorders NOTIFY enabledBordersChanged)

    /**
     * Holds whether the current svg is present in the current theme and NO fallback is involved
     */
    Q_PROPERTY(bool fromCurrentTheme READ fromCurrentTheme NOTIFY fromCurrentThemeChanged)

    /**
     * Set a color group for the FrameSvgItem.
     * if the Svg uses stylesheets and has elements
     * that are eithe TextColor or BackgroundColor class,
     * make them use ButtonTextColor/ButtonBackgroundColor
     * or ViewTextColor/ViewBackgroundColor, ComplementaryTextColor etc.
     */
    Q_PROPERTY(Plasma::Theme::ColorGroup colorGroup READ colorGroup WRITE setColorGroup NOTIFY colorGroupChanged)

    /**
     * Sets the image in a selected status.
     * Svgs can be colored with system color themes, if the status is selected,
     * the TextColor will become HighlightedText color and BackgroundColor
     * will become HighlightColor, making the svg graphics (for instance an icon)
     * will look correct together selected text
     * @see Plasma::Svg::status
     * @since 5.23
     */
    Q_PROPERTY(Plasma::Svg::Status status READ status WRITE setStatus NOTIFY statusChanged)

public:
    /**
     * @return true if the svg has the necessary elements with the given prefix
     * to draw a frame
     * @param prefix the given prefix we want to check if drawable
     */
    Q_INVOKABLE bool hasElementPrefix(const QString &prefix) const;

    /// @cond INTERNAL_DOCS
    FrameSvgItem(QQuickItem *parent = nullptr);
    ~FrameSvgItem();

    void setImagePath(const QString &path);
    QString imagePath() const;

    void setPrefix(const QVariant &prefix);
    QVariant prefix() const;

    QString usedPrefix() const;

    void setEnabledBorders(const Plasma::FrameSvg::EnabledBorders borders);
    Plasma::FrameSvg::EnabledBorders enabledBorders() const;

    FrameSvgItemMargins *margins();
    FrameSvgItemMargins *fixedMargins();

    void setColorGroup(Plasma::Theme::ColorGroup group);
    Plasma::Theme::ColorGroup colorGroup() const;

    bool fromCurrentTheme() const;

    void setStatus(Plasma::Svg::Status status);
    Plasma::Svg::Status status() const;

    void geometryChanged(const QRectF &newGeometry,
                         const QRectF &oldGeometry) override;

    /**
     * Only to be used from inside this library, is not intended to be invokable
     */
    Plasma::FrameSvg *frameSvg() const;

    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) override;

    void itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData & data) override;

protected:
    void classBegin() override;
    void componentComplete() override;

/// @endcond

Q_SIGNALS:
    void imagePathChanged();
    void prefixChanged();
    void enabledBordersChanged();
    void fromCurrentThemeChanged();
    void colorGroupChanged();
    void repaintNeeded();
    void statusChanged();
    void usedPrefixChanged();

private Q_SLOTS:
    void doUpdate();
    void updateDevicePixelRatio();

private:
    void applyPrefixes();

    Plasma::FrameSvg *m_frameSvg;
    FrameSvgItemMargins *m_margins;
    FrameSvgItemMargins *m_fixedMargins;
    QStringList m_prefixes;
    bool m_textureChanged;
    bool m_sizeChanged;
    bool m_fastPath;
};

}

#endif
