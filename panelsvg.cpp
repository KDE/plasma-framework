/*
 *   Copyright 2008 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "panelsvg.h"

#include <QPainter>
#include <QSize>
#include <QBitmap>

#include <KDebug>

#include <plasma/theme.h>

namespace Plasma
{

class PanelData
{
public:
    PanelData()
      : enabledBorders(PanelSvg::AllBorders),
        panelSize(-1,-1)
    {
    }

    PanelData(const PanelData &other)
      : enabledBorders(other.enabledBorders),
        panelSize(other.panelSize)
    {
    }

    ~PanelData()
    {
    }

    PanelSvg::EnabledBorders enabledBorders;
    QPixmap cachedBackground;
    QBitmap cachedMask;
    QSizeF panelSize;

    //measures
    int topHeight;
    int leftWidth;
    int rightWidth;
    int bottomHeight;

    //margins, are equal to the measures by default
    int topMargin;
    int leftMargin;
    int rightMargin;
    int bottomMargin;

    //size of the svg where the size of the "center"
    //element is contentWidth x contentHeight
    bool noBorderPadding : 1;
    bool stretchBorders : 1;
    bool tileCenter : 1;
};

class PanelSvgPrivate
{
public:
    PanelSvgPrivate(PanelSvg *psvg)
      : q(psvg),
        cacheAll(false)
    {
    }

    ~PanelSvgPrivate()
    {
        qDeleteAll(panels);
        panels.clear();
    }

    void generateBackground(PanelData *panel);
    void updateSizes();
    void updateNeeded();
    void updateAndSignalSizes();

    Location location;
    QString prefix;

    PanelSvg *q;

    bool cacheAll : 1;

    QHash<QString, PanelData*> panels;
};

PanelSvg::PanelSvg(QObject *parent)
    : Svg(parent),
      d(new PanelSvgPrivate(this))
{
    connect(this, SIGNAL(repaintNeeded()), this, SLOT(updateNeeded()));
    d->panels.insert(QString(), new PanelData());
}

PanelSvg::~PanelSvg()
{
    delete d;
}

void PanelSvg::setImagePath(const QString &path)
{
    if (path == imagePath()) {
        return;
    }

    Svg::setImagePath(path);
    setContainsMultipleImages(true);

    clearCache();
    d->updateAndSignalSizes();
}

void PanelSvg::setEnabledBorders(const EnabledBorders borders)
{
    if (borders == d->panels[d->prefix]->enabledBorders) {
        return;
    }

    d->panels[d->prefix]->enabledBorders = borders;
    d->updateAndSignalSizes();
}

PanelSvg::EnabledBorders PanelSvg::enabledBorders() const
{
    QHash<QString, PanelData*>::const_iterator it = d->panels.constFind(d->prefix);

    if (it != d->panels.constEnd()) {
        return it.value()->enabledBorders;
    } else {
        return NoBorder;
    }
}

void PanelSvg::setElementPrefix(Plasma::Location location)
{
    switch (location) {
        case TopEdge:
            setElementPrefix("north");
            break;
        case BottomEdge:
            setElementPrefix("south");
            break;
        case LeftEdge:
            setElementPrefix("west");
            break;
        case RightEdge:
            setElementPrefix("east");
            break;
        default:
            setElementPrefix(QString());
            break;
    }
    d->location = location;
}

void PanelSvg::setElementPrefix(const QString & prefix)
{
    const QString oldPrefix(d->prefix);

    if (!hasElement(prefix + "-center")) {
        d->prefix.clear();
    } else {
        d->prefix = prefix;
        if (!d->prefix.isEmpty()) {
            d->prefix += '-';
        }

    }

    if (oldPrefix == d->prefix && d->panels[oldPrefix]) {
        return;
    }

    if (!d->panels.contains(d->prefix)) {
        d->panels.insert(d->prefix, new PanelData(*(d->panels[oldPrefix])));
        d->updateSizes();
    }

    if (!d->cacheAll) {
        delete d->panels[oldPrefix];
        d->panels.remove(oldPrefix);
    }

    d->location = Floating;
}

bool PanelSvg::hasElementPrefix(const QString & prefix) const
{
    //for now it simply checks if a center element exists,
    //because it could make sense for certain themes to not have all the elements
    if (prefix.isEmpty()) {
        return hasElement("center");
    } else {
        return hasElement(prefix + "-center");
    }
}

bool PanelSvg::hasElementPrefix(Plasma::Location location) const
{
    switch (location) {
        case TopEdge:
            return hasElementPrefix("north");
            break;
        case BottomEdge:
            return hasElementPrefix("south");
            break;
        case LeftEdge:
            return hasElementPrefix("west");
            break;
        case RightEdge:
            return hasElementPrefix("east");
            break;
        default:
            return hasElementPrefix(QString());
            break;
    }
}

QString PanelSvg::prefix()
{
    if (d->prefix.isEmpty()) {
        return d->prefix;
    }

    return d->prefix.left(d->prefix.size() - 1);
}

void PanelSvg::resizePanel(const QSizeF &size)
{
    if (size.isEmpty()) {
        kWarning() << "Invalid size" << size;
        return;
    }

    if (size == d->panels[d->prefix]->panelSize) {
        return;
    }

    d->updateSizes();
    d->panels[d->prefix]->panelSize = size;
}

QSizeF PanelSvg::panelSize() const
{
    QHash<QString, PanelData*>::const_iterator it = d->panels.constFind(d->prefix);

    if (it != d->panels.constEnd()) {
        return it.value()->panelSize;
    } else {
        return QSize(-1, -1);
    }
}

qreal PanelSvg::marginSize(const Plasma::MarginEdge edge) const
{
    if (d->panels[d->prefix]->noBorderPadding) {
        return .0;
    }

    switch (edge) {
    case Plasma::TopMargin:
        return d->panels[d->prefix]->topMargin;
    break;

    case Plasma::LeftMargin:
        return d->panels[d->prefix]->leftMargin;
    break;

    case Plasma::RightMargin:
        return d->panels[d->prefix]->rightMargin;
    break;

    //Plasma::BottomMargin
    default:
        return d->panels[d->prefix]->bottomMargin;
    break;
    }
}

void PanelSvg::getMargins(qreal &left, qreal &top, qreal &right, qreal &bottom) const
{
    PanelData *panel = d->panels[d->prefix];

    if (!panel || panel->noBorderPadding) {
        left = top = right = bottom = 0;
        return;
    }

    top = panel->topMargin;
    left = panel->leftMargin;
    right = panel->rightMargin;
    bottom = panel->bottomMargin;
}

QRectF PanelSvg::contentsRect() const
{
    QSizeF size(panelSize());

    if (size.isValid()) {
        QRectF rect(QPointF(0, 0), size);
        PanelData *panel = d->panels[d->prefix];

        return rect.adjusted(panel->leftMargin, panel->topMargin,
                             -panel->rightMargin, -panel->bottomMargin);
    } else {
        return QRectF();
    }
}

QBitmap PanelSvg::mask() const
{
    PanelData *panel = d->panels[d->prefix];

    if (!panel->cachedMask) {
        if (panel->cachedBackground.isNull()) {
            d->generateBackground(panel);
            if (panel->cachedBackground.isNull()) {
                return QBitmap();
            }
        }
        panel->cachedMask = QBitmap(panel->cachedBackground.alphaChannel().createMaskFromColor(Qt::black));
    }
    return panel->cachedMask;
}

void PanelSvg::setCacheAllRenderedPanels(bool cache)
{
    if (d->cacheAll && !cache) {
        clearCache();
    }

    d->cacheAll = cache;
}

bool PanelSvg::cacheAllRenderedPanels() const
{
    return d->cacheAll;
}

void PanelSvg::clearCache()
{
    PanelData *panel = d->panels[d->prefix];

    // delete all the panels that aren't this one
    QMutableHashIterator<QString, PanelData*> it(d->panels);
    while (it.hasNext()) {
        PanelData *p = it.next().value();
        if (panel != p) {
            delete p;
            it.remove();
        }
    }
}

QPixmap PanelSvg::panelPixmap()
{
    PanelData *panel = d->panels[d->prefix];
    if (panel->cachedBackground.isNull()) {
        d->generateBackground(panel);
        if (panel->cachedBackground.isNull()) {
            return QPixmap();
        }
    }

    return panel->cachedBackground;
}

void PanelSvg::paintPanel(QPainter *painter, const QRectF &target, const QRectF &source)
{
    PanelData *panel = d->panels[d->prefix];
    if (panel->cachedBackground.isNull()) {
        d->generateBackground(panel);
        if (panel->cachedBackground.isNull()) {
            return;
        }
    }

    painter->drawPixmap(target, panel->cachedBackground, source.isValid() ? source : target);
}

void PanelSvg::paintPanel(QPainter *painter, const QPointF &pos)
{
    PanelData *panel = d->panels[d->prefix];
    if (panel->cachedBackground.isNull()) {
        d->generateBackground(panel);
        if (panel->cachedBackground.isNull()) {
            return;
        }
    }

    painter->drawPixmap(pos, panel->cachedBackground);
}

void PanelSvgPrivate::generateBackground(PanelData *panel)
{
    if (!panel->cachedBackground.isNull()) {
        return;
    }

    QString id = QString::fromLatin1("%5_%4_%3_%2_%1_").
                         arg(panel->enabledBorders).arg(panel->panelSize.width()).arg(panel->panelSize.height()).arg(prefix).arg(q->imagePath());
    Theme *theme = Theme::defaultTheme();
    if (theme->findInCache(id, panel->cachedBackground) && !panel->cachedBackground.isNull()) {
        return;
    }

    //kDebug() << "generating background";
    const int topWidth = q->elementSize(prefix + "top").width();
    const int leftHeight = q->elementSize(prefix + "left").height();
    const int topOffset = 0;
    const int leftOffset = 0;


    if (!panel->panelSize.isValid()) {
        kWarning() << "Invalid panel size" << panel->panelSize;
        return;
    }

    const int contentWidth = panel->panelSize.width() - panel->leftWidth  - panel->rightWidth;
    const int contentHeight = panel->panelSize.height() - panel->topHeight  - panel->bottomHeight;
    int contentTop = 0;
    int contentLeft = 0;
    int rightOffset = contentWidth;
    int bottomOffset = contentHeight;

    panel->cachedBackground = QPixmap(panel->leftWidth + contentWidth + panel->rightWidth,
                                      panel->topHeight + contentHeight + panel->bottomHeight);
    panel->cachedBackground.fill(Qt::transparent);
    QPainter p(&panel->cachedBackground);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    //if we must stretch the center or the borders we compute how much we will have to stretch
    //the svg to get the desired element sizes
    QSizeF  scaledContentSize(0, 0);
    if (q->elementSize(prefix + "center").width() > 0 &&
        q->elementSize(prefix + "center").height() > 0 &&
        (!panel->tileCenter || panel->stretchBorders)) {
        scaledContentSize = QSizeF(contentWidth * ((qreal)q->size().width() / (qreal)q->elementSize(prefix + "center").width()),
                                   contentHeight * ((qreal)q->size().height() / (qreal)q->elementSize(prefix + "center").height()));
    }

    //CENTER
    if (panel->tileCenter) {
        if (contentHeight > 0 && contentWidth > 0) {
            int centerTileHeight;
            int centerTileWidth;
            centerTileHeight = q->elementSize(prefix + "center").height();
            centerTileWidth = q->elementSize(prefix + "center").width();
            QPixmap center(centerTileWidth, centerTileHeight);
            center.fill(Qt::transparent);

            {
                QPainter centerPainter(&center);
                centerPainter.setCompositionMode(QPainter::CompositionMode_Source);
                q->paint(&centerPainter, QRect(QPoint(0, 0), q->elementSize(prefix + "center")), prefix + "center");
            }

            p.drawTiledPixmap(QRect(panel->leftWidth, panel->topHeight,
                                    contentWidth, contentHeight), center);
        }
    } else {
        if (contentHeight > 0 && contentWidth > 0) {
            q->paint(&p, QRect(panel->leftWidth, panel->topHeight,
                               contentWidth, contentHeight),
                               prefix + "center");
        }
    }

    // Corners
    if (q->hasElement(prefix + "top") && panel->enabledBorders & PanelSvg::TopBorder) {
        contentTop = panel->topHeight;
        bottomOffset += panel->topHeight;

        if (q->hasElement(prefix + "topleft") && panel->enabledBorders & PanelSvg::LeftBorder) {
            q->paint(&p, QRect(leftOffset, topOffset, panel->leftWidth, panel->topHeight), prefix + "topleft");

            contentLeft = panel->leftWidth;
            rightOffset = contentWidth + panel->leftWidth;
        }

        if (q->hasElement(prefix + "topright") && panel->enabledBorders & PanelSvg::RightBorder) {
            q->paint(&p, QRect(rightOffset, topOffset, panel->rightWidth, panel->topHeight), prefix + "topright");
        }
    }

    if (q->hasElement(prefix + "bottom") && panel->enabledBorders & PanelSvg::BottomBorder) {
        if (q->hasElement(prefix + "bottomleft") && panel->enabledBorders & PanelSvg::LeftBorder) {
            q->paint(&p, QRect(leftOffset, bottomOffset, panel->leftWidth, panel->bottomHeight), prefix + "bottomleft");

            contentLeft = panel->leftWidth;
            rightOffset = contentWidth + panel->leftWidth;
        }

        if (q->hasElement(prefix + "bottomright") && panel->enabledBorders & PanelSvg::RightBorder) {
            q->paint(&p, QRect(rightOffset, bottomOffset, panel->rightWidth, panel->bottomHeight), prefix + "bottomright");
        }
    }

    // Sides
    if (panel->stretchBorders) {
        if (panel->enabledBorders & PanelSvg::LeftBorder || panel->enabledBorders & PanelSvg::RightBorder) {

            if (q->hasElement(prefix + "left") &&
                panel->enabledBorders & PanelSvg::LeftBorder) {
                q->paint(&p, QRect(leftOffset, contentTop, panel->leftWidth, contentHeight), prefix + "left");
            }

            if (q->hasElement(prefix + "right") &&
                panel->enabledBorders & PanelSvg::RightBorder) {
                q->paint(&p, QRect(rightOffset, contentTop, panel->rightWidth, contentHeight), prefix + "right");
            }
        }

        if (panel->enabledBorders & PanelSvg::TopBorder ||
            panel->enabledBorders & PanelSvg::BottomBorder) {

            if (q->hasElement(prefix + "top") &&
                panel->enabledBorders & PanelSvg::TopBorder) {
                q->paint(&p, QRect(contentLeft, topOffset, contentWidth, panel->topHeight), prefix + "top");
            }

            if (q->hasElement(prefix + "bottom") &&
                panel->enabledBorders & PanelSvg::BottomBorder) {
                q->paint(&p, QRect(contentLeft, bottomOffset, contentWidth, panel->bottomHeight), prefix + "bottom");
            }

        }
    } else {
        if (q->hasElement(prefix + "left") &&
            panel->enabledBorders & PanelSvg::LeftBorder) {
            QPixmap left(panel->leftWidth, leftHeight);
            left.fill(Qt::transparent);

            QPainter sidePainter(&left);
            sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
            q->paint(&sidePainter, QRect(QPoint(0, 0), left.size()), prefix + "left");

            p.drawTiledPixmap(QRect(leftOffset, contentTop, panel->leftWidth, contentHeight), left);
        }

        if (q->hasElement(prefix + "right") && panel->enabledBorders & PanelSvg::RightBorder) {
            QPixmap right(panel->rightWidth, leftHeight);
            right.fill(Qt::transparent);

            QPainter sidePainter(&right);
            sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
            q->paint(&sidePainter, QRect(QPoint(0, 0), right.size()), prefix + "right");

            p.drawTiledPixmap(QRect(rightOffset, contentTop, panel->rightWidth, contentHeight), right);
        }

        if (q->hasElement(prefix + "top") && panel->enabledBorders & PanelSvg::TopBorder) {
            QPixmap top(topWidth, panel->topHeight);
            top.fill(Qt::transparent);

            QPainter sidePainter(&top);
            sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
            q->paint(&sidePainter, QRect(QPoint(0, 0), top.size()), prefix + "top");

            p.drawTiledPixmap(QRect(contentLeft, topOffset, contentWidth, panel->topHeight), top);
        }

        if (q->hasElement(prefix + "bottom") && panel->enabledBorders & PanelSvg::BottomBorder) {
            QPixmap bottom(topWidth, panel->bottomHeight);
            bottom.fill(Qt::transparent);

            QPainter sidePainter(&bottom);
            sidePainter.setCompositionMode(QPainter::CompositionMode_Source);
            q->paint(&sidePainter, QRect(QPoint(0, 0), bottom.size()), prefix + "bottom");

            p.drawTiledPixmap(QRect(contentLeft, bottomOffset, contentWidth, panel->bottomHeight), bottom);
        }
    }

    theme->insertIntoCache(id, panel->cachedBackground);
}

void PanelSvgPrivate::updateSizes()
{
    //kDebug() << "!!!!!!!!!!!!!!!!!!!!!! updating sizes" << prefix;
    PanelData *panel = panels[prefix];
    Q_ASSERT(panel);

    panel->cachedBackground = QPixmap();
    panel->cachedMask = QPixmap();

    if (panel->enabledBorders & PanelSvg::TopBorder) {
        panel->topHeight = q->elementSize(prefix + "top").height();

        if (q->hasElement(prefix + "hint-top-margin")) {
            panel->topMargin = q->elementSize(prefix + "hint-top-margin").height();
        } else {
            panel->topMargin = panel->topHeight;
        }
    } else {
        panel->topMargin = panel->topHeight = 0;
    }

    if (panel->enabledBorders & PanelSvg::LeftBorder) {
        panel->leftWidth = q->elementSize(prefix + "left").width();

        if (q->hasElement(prefix + "hint-left-margin")) {
            panel->leftMargin = q->elementSize(prefix + "hint-left-margin").height();
        } else {
            panel->leftMargin = panel->leftWidth;
        }
    } else {
        panel->leftMargin = panel->leftWidth = 0;
    }

    if (panel->enabledBorders & PanelSvg::RightBorder) {
        panel->rightWidth = q->elementSize(prefix + "right").width();

        if (q->hasElement(prefix + "hint-right-margin")) {
            panel->rightMargin = q->elementSize(prefix + "hint-right-margin").height();
        } else {
            panel->rightMargin = panel->rightWidth;
        }
    } else {
        panel->rightMargin = panel->rightWidth = 0;
    }

    if (panel->enabledBorders & PanelSvg::BottomBorder) {
        panel->bottomHeight = q->elementSize(prefix + "bottom").height();

        if (q->hasElement(prefix + "hint-bottom-margin")) {
            panel->bottomMargin = q->elementSize(prefix + "hint-bottom-margin").height();
        } else {
            panel->bottomMargin = panel->bottomHeight;
        }
    } else {
        panel->bottomMargin = panel->bottomHeight = 0;
    }

    //since it's rectangular, topWidth and bottomWidth must be the same
    panel->tileCenter = q->hasElement("hint-tile-center");
    panel->noBorderPadding = q->hasElement("hint-no-border-padding");
    panel->stretchBorders = q->hasElement("hint-stretch-borders");
}

void PanelSvgPrivate::updateNeeded()
{
    q->clearCache();
    updateSizes();
}

void PanelSvgPrivate::updateAndSignalSizes()
{
    updateSizes();
    emit q->repaintNeeded();
}

} // Plasma namespace

#include "panelsvg.moc"
