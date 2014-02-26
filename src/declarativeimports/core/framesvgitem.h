/***************************************************************************
 *   Copyright 2010 Marco Martin <mart@kde.org>                            *
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

#include <QQuickPaintedItem>

#include <Plasma/FrameSvg>

#include "units.h"

namespace Plasma {

    class FrameSvg;

class FrameSvgItemMargins : public QObject
{
    Q_OBJECT

    /**
     * width in pixels of the left margin
     */
    Q_PROPERTY(qreal left READ left NOTIFY marginsChanged)

    /**
     * height in pixels of the top margin
     */
    Q_PROPERTY(qreal top READ top NOTIFY marginsChanged)

    /**
     * width in pixels of the right margin
     */
    Q_PROPERTY(qreal right READ right NOTIFY marginsChanged)

    /**
     * height in pixels of the bottom margin
     */
    Q_PROPERTY(qreal bottom READ bottom NOTIFY marginsChanged)

public:
    FrameSvgItemMargins(Plasma::FrameSvg *frameSvg, QObject *parent = 0);

    qreal left() const;
    qreal top() const;
    qreal right() const;
    qreal bottom() const;

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

class FrameSvgItem : public QQuickPaintedItem
{
    Q_OBJECT

    /**
     * Theme relative path of the svg, like "widgets/background"
     */
    Q_PROPERTY(QString imagePath READ imagePath WRITE setImagePath NOTIFY imagePathChanged)

    /**
     * prefix for the 9 piece svg, like "pushed" or "normal" for the button
     * see http://techbase.kde.org/Development/Tutorials/Plasma/ThemeDetails
     * for a list of paths and prefixes
     */
    Q_PROPERTY(QString prefix READ prefix WRITE setPrefix NOTIFY prefixChanged)

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

    Q_FLAGS(Plasma::FrameSvg::EnabledBorders)
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
     * suggested default size hint for width
     */
    Q_PROPERTY(qreal implicitWidth READ implicitWidth WRITE setImplicitWidth NOTIFY implicitWidthChanged)

    /**
     * suggested default size hint for height
     */
    Q_PROPERTY(qreal implicitHeight READ implicitHeight WRITE setImplicitHeight NOTIFY implicitHeightChanged)

public:
    FrameSvgItem(QQuickItem *parent=0);
    ~FrameSvgItem();

    void setImagePath(const QString &path);
    QString imagePath() const;

    void setPrefix(const QString &prefix);
    QString prefix() const;

    void setEnabledBorders(const Plasma::FrameSvg::EnabledBorders borders);
    Plasma::FrameSvg::EnabledBorders enabledBorders() const;

    FrameSvgItemMargins *margins() const;
    FrameSvgItemMargins *fixedMargins() const;

    void paint(QPainter *painter);

    void geometryChanged(const QRectF &newGeometry,
                              const QRectF &oldGeometry);

    void setImplicitWidth(qreal width);
    qreal implicitWidth() const;

    void setImplicitHeight(qreal height);
    qreal implicitHeight() const;

    /**
     * Only to be used from inside this library, is not intended to be invokable
     */
    Plasma::FrameSvg *frameSvg() const;

protected:
    virtual void componentComplete();

Q_SIGNALS:
    void imagePathChanged();
    void prefixChanged();
    void enabledBordersChanged();
    void implicitWidthChanged();
    void implicitHeightChanged();

private Q_SLOTS:
    void doUpdate();
    void updateDevicePixelRatio();

private:
    Plasma::FrameSvg *m_frameSvg;
    FrameSvgItemMargins *m_margins;
    FrameSvgItemMargins *m_fixedMargins;
    QString m_prefix;
    Units m_units;
};

}

#endif
