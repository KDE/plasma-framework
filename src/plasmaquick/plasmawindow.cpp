/*
    SPDX-FileCopyrightText: 2023 David Edmundson <davidedmundson@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plasmawindow.h"

#include "../declarativeimports/core/framesvgitem.h"
#include "dialogshadows_p.h"

#include <QMarginsF>
#include <QQuickItem>

#include <KWindowEffects>
#include <KX11Extras>

using namespace Plasma;

namespace PlasmaQuick
{
class PlasmaWindowPrivate
{
public:
    PlasmaWindowPrivate(PlasmaWindow *_q)
        : q(_q)
    {
    }
    void handleFrameChanged();
    void updateMainItemGeometry();
    PlasmaWindow *q;
    QPointer<QQuickItem> mainItem;
    Plasma::FrameSvgItem *frameSvgItem;
};

PlasmaWindow::PlasmaWindow(QWindow *parent)
    : QQuickWindow(parent)
    , d(new PlasmaWindowPrivate(this))
{
    setColor(QColor(Qt::transparent));
    setFlags(Qt::FramelessWindowHint);

    d->frameSvgItem = new Plasma::FrameSvgItem(contentItem());
    d->frameSvgItem->setImagePath(QStringLiteral("dialogs/background"));
    connect(d->frameSvgItem->fixedMargins(), &Plasma::FrameSvgItemMargins::marginsChanged, this, [this]() {
        d->updateMainItemGeometry();
        Q_EMIT marginsChanged();
    });
    connect(d->frameSvgItem, &Plasma::FrameSvgItem::maskChanged, this, [this]() {
        d->handleFrameChanged();
    });

    DialogShadows::self()->addWindow(this, d->frameSvgItem->enabledBorders());
}

PlasmaWindow::~PlasmaWindow()
{
}

void PlasmaWindow::setMainItem(QQuickItem *mainItem)
{
    if (d->mainItem == mainItem)
        return;

    d->mainItem = mainItem;

    if (d->mainItem) {
        mainItem->setParentItem(contentItem());
        d->updateMainItemGeometry();
    }
    Q_EMIT mainItemChanged();
}

QQuickItem *PlasmaWindow::mainItem() const
{
    return d->mainItem;
}

static FrameSvg::EnabledBorders edgeToBorder(Qt::Edges edges)
{
    FrameSvg::EnabledBorders borders = FrameSvg::EnabledBorders(0);

    if (edges & Qt::Edge::TopEdge) {
        borders |= FrameSvg::EnabledBorder::TopBorder;
    }
    if (edges & Qt::Edge::BottomEdge) {
        borders |= FrameSvg::EnabledBorder::BottomBorder;
    }
    if (edges & Qt::Edge::LeftEdge) {
        borders |= FrameSvg::EnabledBorder::LeftBorder;
    }
    if (edges & Qt::Edge::RightEdge) {
        borders |= FrameSvg::EnabledBorder::RightBorder;
    }

    return borders;
}

void PlasmaWindow::setBorders(Qt::Edges bordersToShow)
{
    d->frameSvgItem->setEnabledBorders(edgeToBorder(bordersToShow));
    DialogShadows::self()->setEnabledBorders(this, d->frameSvgItem->enabledBorders());
}

void PlasmaWindow::resizeEvent(QResizeEvent *e)
{
    QQuickWindow::resizeEvent(e);

    const QSize windowSize = e->size();
    d->frameSvgItem->setSize(windowSize);
    if (d->mainItem) {
        const QSize contentSize = windowSize.shrunkBy(margins());
        d->mainItem->setSize(contentSize);
    }
}

void PlasmaWindowPrivate::handleFrameChanged()
{
    auto theme = Plasma::Theme();
    const QRegion mask = frameSvgItem->frameSvg()->mask();
    KWindowEffects::enableBlurBehind(q, theme.blurBehindEnabled(), mask);
    KWindowEffects::enableBackgroundContrast(q,
                                             theme.backgroundContrastEnabled(),
                                             theme.backgroundContrast(),
                                             theme.backgroundIntensity(),
                                             theme.backgroundSaturation(),
                                             mask);

    if (KX11Extras::compositingActive()) {
        q->setMask(QRegion());
    } else {
        q->setMask(mask);
    }
}

void PlasmaWindowPrivate::updateMainItemGeometry()
{
    if (!mainItem) {
        return;
    }
    const QMargins frameMargins = q->margins();
    const QSize windowSize = q->size();

    mainItem->setX(frameMargins.left());
    mainItem->setY(frameMargins.top());

    const QSize contentSize = windowSize.shrunkBy(frameMargins);
    mainItem->setSize(contentSize);
}

QMargins PlasmaWindow::margins() const
{
    return QMargins(d->frameSvgItem->fixedMargins()->left(),
                    d->frameSvgItem->fixedMargins()->top(),
                    d->frameSvgItem->fixedMargins()->right(),
                    d->frameSvgItem->fixedMargins()->bottom());
}
}
