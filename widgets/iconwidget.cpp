/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 by Riccardo Iaconelli <riccardo@kde.org>
 *   Copyright 2007 by Matt Broadstone <mbroadst@gmail.com>
 *   Copyright 2006-2007 Fredrik Höglund <fredrik@kde.org>
 *   Copyright 2007 by Marco Martin <notmart@gmail.com>
 *   Copyright 2008 by Alexis Ménard <darktears31@gmail.com>
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

#include "iconwidget.h"
#include "iconwidget_p.h"

#include <QAction>
#include <QApplication>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QMenu>
#include <QStyleOptionGraphicsItem>
#include <QTextLayout>

#include <kglobalsettings.h>
#include <kiconeffect.h>
#include <kiconloader.h>
#include <kicon.h>
#include <kurl.h>
#include <krun.h>
#include <kmimetype.h>
#include <kdebug.h>
#include <kcolorscheme.h>

#include <plasma/animator.h>
#include <plasma/animations/animation.h>
#include <plasma/paintutils.h>
#include <plasma/theme.h>

#include "svg.h"

/*
TODO:
    Add these to a UrlIcon class
    void setUrl(const KUrl& url);
    KUrl url() const;
*/

namespace Plasma
{

IconHoverAnimation::IconHoverAnimation(QObject *parent)
    : QObject(parent), m_value(0), m_fadeIn(false)
{
}

qreal IconHoverAnimation::value() const
{
    return m_value;
}

bool IconHoverAnimation::fadeIn() const
{
    return m_fadeIn;
}

QPropertyAnimation *IconHoverAnimation::animation() const
{
    return m_animation.data();
}

void IconHoverAnimation::setValue(qreal value)
{
    m_value = value;
    QGraphicsWidget *item = static_cast<QGraphicsWidget*>(parent());
    item->update();
}

void IconHoverAnimation::setFadeIn(bool fadeIn)
{
    m_fadeIn = fadeIn;
}

void IconHoverAnimation::setAnimation(QPropertyAnimation *animation)
{
    m_animation = animation;
}

IconWidgetPrivate::IconWidgetPrivate(IconWidget *i)
    : ActionWidgetInterface<IconWidget>(i),
      q(i),
      iconSvg(0),
      hoverAnimation(new IconHoverAnimation(q)),
      iconSize(48, 48),
      preferredIconSize(-1, -1),
      minimumIconSize(-1, -1),
      maximumIconSize(-1, -1),
      states(IconWidgetPrivate::NoState),
      orientation(Qt::Vertical),
      numDisplayLines(2),
      activeMargins(0),
      iconSvgElementChanged(false),
      invertLayout(false),
      drawBg(false),
      textBgCustomized(false),
      customFont(false)
{
}

IconWidgetPrivate::~IconWidgetPrivate()
{
    qDeleteAll(cornerActions);
    delete hoverAnimation;
}

void IconWidgetPrivate::readColors()
{
    textColor = Plasma::Theme::defaultTheme()->color(Theme::TextColor);

    if (qGray(textColor.rgb()) > 192) {
        shadowColor = Qt::black;
    } else {
        shadowColor = Qt::white;
    }

    if (!textBgCustomized) {
        textBgColor = QColor();
    }
}

void IconWidgetPrivate::colorConfigChanged()
{
    readColors();
    if (drawBg) {
        qreal left, top, right, bottom;
        background->getMargins(left, top, right, bottom);
        setVerticalMargin(IconWidgetPrivate::ItemMargin, left, top, right, bottom);
        setHorizontalMargin(IconWidgetPrivate::ItemMargin, left, top, right, bottom);
    }
    q->update();
}

void IconWidgetPrivate::iconConfigChanged()
{
    if (!icon.isNull()) {
        q->update();
    }
}

QFont IconWidgetPrivate::widgetFont() const
{
    if (customFont) {
        return q->font();
    } else {
        return Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    }
}

IconAction::IconAction(IconWidget *icon, QAction *action)
    : m_icon(icon),
      m_action(action),
      m_hovered(false),
      m_pressed(false),
      m_selected(false),
      m_visible(false)
{
}

void IconAction::show()
{
    Animation *animation = m_animation.data();
    if (!animation) {
        animation = Plasma::Animator::create(Plasma::Animator::PixmapTransitionAnimation, m_icon);
        animation->setTargetWidget(m_icon);
        m_animation = animation;
    } else if (animation->state() == QAbstractAnimation::Running) {
        animation->pause();
    }

    rebuildPixmap();
    m_visible = true;

    animation->setProperty("targetPixmap", m_pixmap);
    animation->setDirection(QAbstractAnimation::Forward);
    animation->start();
}

void IconAction::hide()
{
    if (!m_animation) {
        return;
    }

    Animation *animation = m_animation.data();
    if (animation->state() == QAbstractAnimation::Running) {
        animation->pause();
    }

    m_visible = false;

    animation->setDirection(QAbstractAnimation::Backward);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

bool IconAction::isVisible() const
{
    return m_visible;
}

bool IconAction::isAnimating() const
{
    return !m_animation.isNull();
}

bool IconAction::isPressed() const
{
    return m_pressed;
}

bool IconAction::isHovered() const
{
    return m_hovered;
}

void IconAction::setSelected(bool selected)
{
    m_selected = selected;
}

bool IconAction::isSelected() const
{
    return m_selected;
}

void IconAction::setRect(const QRectF &rect)
{
    m_rect = rect;
}

QRectF IconAction::rect() const
{
    return m_rect;
}

void IconAction::rebuildPixmap()
{
    // Determine proper QIcon mode based on selection status
    QIcon::Mode mode = m_selected ? QIcon::Selected : QIcon::Normal;

    // Draw everything
    m_pixmap = QPixmap(26, 26);
    m_pixmap.fill(Qt::transparent);

    int element = IconWidgetPrivate::Minibutton;
    if (m_pressed) {
        element = IconWidgetPrivate::MinibuttonPressed;
    } else if (m_hovered) {
        element = IconWidgetPrivate::MinibuttonHover;
    }

    QPainter painter(&m_pixmap);
    m_icon->drawActionButtonBase(&painter, m_pixmap.size(), element);
    m_action->icon().paint(&painter, 2, 2, 22, 22, Qt::AlignCenter, mode);
}

bool IconAction::event(QEvent::Type type, const QPointF &pos)
{
    if (!m_action->isVisible() || !m_action->isEnabled()) {
        return false;
    }

    if (m_icon->size().width() < m_rect.width() * 2.0 ||
        m_icon->size().height() < m_rect.height() * 2.0) {
        return false;
    }

    switch (type) {
    case QEvent::GraphicsSceneMousePress:
    {
        setSelected(m_rect.contains(pos));
        return isSelected();
    }
    break;

    case QEvent::GraphicsSceneMouseMove:
    {
        bool wasSelected = isSelected();
        bool active = m_rect.contains(pos);
        setSelected(wasSelected && active);
        return (wasSelected != isSelected()) || active;
    }
    break;

    case QEvent::GraphicsSceneMouseRelease:
    {
        // kDebug() << "IconAction::event got a QEvent::MouseButtonRelease, " << isSelected();
        bool wasSelected = isSelected();
        setSelected(false);
        if (wasSelected) {
            m_action->trigger();
        }

        return wasSelected;
    }
    break;

    case QEvent::GraphicsSceneHoverEnter:
        m_pressed = false;
        m_hovered = true;
        break;

    case QEvent::GraphicsSceneHoverLeave:
        m_pressed = false;
        m_hovered = false;
        break;

    default:
        break;
    }

    return false;
}

QAction *IconAction::action() const
{
    return m_action;
}

void IconAction::paint(QPainter *painter) const
{
    if (!m_action->isVisible() || !m_action->isEnabled()) {
        return;
    }

    if (m_icon->size().width() < m_rect.width() * 2.0 ||
        m_icon->size().height() < m_rect.height() * 2.0) {
        return;
    }

    Animation *animation = m_animation.data();
    if (m_visible && !animation) {
        painter->drawPixmap(m_rect.toRect(), m_pixmap);
    } else {
        painter->drawPixmap(m_rect.toRect(),
                animation->property("currentPixmap").value<QPixmap>());
    }
}

IconWidget::IconWidget(QGraphicsItem *parent)
    : QGraphicsWidget(parent),
      d(new IconWidgetPrivate(this))
{
    d->init();
}

IconWidget::IconWidget(const QString &text, QGraphicsItem *parent)
    : QGraphicsWidget(parent),
      d(new IconWidgetPrivate(this))
{
    d->init();
    setText(text);
}

IconWidget::IconWidget(const QIcon &icon, const QString &text, QGraphicsItem *parent)
    : QGraphicsWidget(parent),
      d(new IconWidgetPrivate(this))
{
    d->init();
    setText(text);
    setIcon(icon);
}

IconWidget::~IconWidget()
{
    delete d;
}

void IconWidgetPrivate::init()
{
    readColors();

    iconChangeTimer = new QTimer(q);
    iconChangeTimer->setSingleShot(true);

    QObject::connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), q, SLOT(colorConfigChanged()));
    QObject::connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), q, SLOT(colorConfigChanged()));
    QObject::connect(KGlobalSettings::self(), SIGNAL(iconChanged(int)), q, SLOT(iconConfigChanged()));

    // setAcceptedMouseButtons(Qt::LeftButton);
    q->setAcceptsHoverEvents(true);
    q->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    background = new Plasma::FrameSvg(q);
    background->setImagePath("widgets/viewitem");
    background->setCacheAllRenderedFrames(true);
    background->setElementPrefix("hover");

    // Margins for horizontal mode (list views, tree views, table views)
    setHorizontalMargin(IconWidgetPrivate::TextMargin, 1, 1);
    setHorizontalMargin(IconWidgetPrivate::IconMargin, 1, 1);
    setHorizontalMargin(IconWidgetPrivate::ItemMargin, 0, 0);

    // Margins for vertical mode (icon views)
    setVerticalMargin(IconWidgetPrivate::TextMargin, 6, 2);
    setVerticalMargin(IconWidgetPrivate::IconMargin, 1, 1);
    setVerticalMargin(IconWidgetPrivate::ItemMargin, 0, 0);

    setActiveMargins();
    currentSize = QSizeF(-1, -1);
}

void IconWidget::addIconAction(QAction *action)
{
    int count = d->cornerActions.count();
    if (count >= IconWidgetPrivate::LastIconPosition) {
        kDebug() << "no more room for more actions!";
        // just overlap it with the last item for now. ugly, but there you go.
    }

    IconAction *iconAction = new IconAction(this, action);
    d->cornerActions.append(iconAction);
    connect(action, SIGNAL(destroyed(QObject*)), this, SLOT(actionDestroyed(QObject*)));

    iconAction->setRect(d->actionRect(qMin((IconWidgetPrivate::ActionPosition)count, IconWidgetPrivate::LastIconPosition)));
}

void IconWidget::removeIconAction(QAction *action)
{
    //WARNING: do NOT access the action pointer passed in, as it may already be
    //be destroyed. see IconWidgetPrivate::actionDestroyed(QObject*)
    int count = 0;
    bool found = false;
    foreach (IconAction *iconAction, d->cornerActions) {
        if (found) {
            iconAction->setRect(d->actionRect((IconWidgetPrivate::ActionPosition)count));
        } else if (!action || iconAction->action() == action) {
            delete iconAction;
            d->cornerActions.removeAll(iconAction);
        }

        if (count < IconWidgetPrivate::LastIconPosition) {
            ++count;
        }
    }

    // redraw since an action has been deleted.
    update();
}

void IconWidgetPrivate::actionDestroyed(QObject *action)
{
    q->removeIconAction(static_cast<QAction*>(action));
}

void IconWidget::setAction(QAction *action)
{
    d->setAction(action);
}

QAction *IconWidget::action() const
{
    return d->action;
}

int IconWidget::numDisplayLines()
{
    return d->numDisplayLines;
}

void IconWidget::setNumDisplayLines(int numLines)
{
    if (numLines > d->maxDisplayLines) {
        d->numDisplayLines = d->maxDisplayLines;
    } else {
        d->numDisplayLines = numLines;
    }
}

void IconWidget::setDrawBackground(bool draw)
{
    if (d->drawBg != draw) {
        d->drawBg = draw;

        QStyle *style = QApplication::style();
        int focusHMargin = draw ? style->pixelMetric(QStyle::PM_FocusFrameHMargin) : 1;
        int focusVMargin = draw ? style->pixelMetric(QStyle::PM_FocusFrameVMargin) : 1;
        d->setHorizontalMargin(IconWidgetPrivate::TextMargin, focusHMargin, focusVMargin);
        d->setHorizontalMargin(IconWidgetPrivate::IconMargin, focusHMargin, focusVMargin);
        d->setVerticalMargin(IconWidgetPrivate::IconMargin, focusHMargin, focusVMargin);
        d->currentSize = QSizeF(-1, -1);

        if (draw) {
            qreal left, top, right, bottom;
            d->background->getMargins(left, top, right, bottom);
            d->setHorizontalMargin(IconWidgetPrivate::ItemMargin, left, top, right, bottom);
            d->setVerticalMargin(IconWidgetPrivate::ItemMargin, left, top, right, bottom);
        } else {
            d->setHorizontalMargin(IconWidgetPrivate::ItemMargin, 0, 0);
            d->setVerticalMargin(IconWidgetPrivate::ItemMargin, 0, 0);
        }

        update();
        updateGeometry();
    }
}

bool IconWidget::drawBackground() const
{
    return d->drawBg;
}

QPainterPath IconWidget::shape() const
{
    if (!d->drawBg || d->currentSize.width() < 1) {
        return QGraphicsItem::shape();
    }

    return PaintUtils::roundedRectangle(
        QRectF(QPointF(0.0, 0.0), d->currentSize).adjusted(-2, -2, 2, 2), 10.0);
}

QSizeF IconWidgetPrivate::displaySizeHint(const QStyleOptionGraphicsItem *option, const qreal width) const
{
    if (text.isEmpty() && infoText.isEmpty()) {
      return QSizeF(.0, .0);
    }

    QString label = text;
    // const qreal maxWidth = (orientation == Qt::Vertical) ? iconSize.width() + 10 : 32757;
    // NOTE: find a way to use the other layoutText, it currently returns nominal width, when
    //       we actually need the actual width.

    qreal textWidth = width -
                      horizontalMargin[IconWidgetPrivate::TextMargin].left -
                      horizontalMargin[IconWidgetPrivate::TextMargin].right;

    //allow only five lines of text
    const qreal maxHeight =
        numDisplayLines * QFontMetrics(widgetFont()).lineSpacing();

    // To compute the nominal size for the label + info, we'll just append
    // the information string to the label
    if (!infoText.isEmpty()) {
        label += QString(QChar::LineSeparator) + infoText;
    }

    QTextLayout layout;
    setLayoutOptions(layout, option, q->orientation());
    layout.setFont(widgetFont());
    QSizeF size = layoutText(layout, option, label, QSizeF(textWidth, maxHeight));

    return addMargin(size, TextMargin);
}

void IconWidgetPrivate::layoutIcons(const QStyleOptionGraphicsItem *option)
{
    if (option->rect.size() == currentSize) {
        return;
    }

    currentSize = option->rect.size();
    setActiveMargins();

    //calculate icon size based on the available space
    qreal iconWidth;

    if (orientation == Qt::Vertical) {
        qreal heightAvail;
        //if there is text resize the icon in order to make room for the text
        if (text.isEmpty() && infoText.isEmpty()) {
            heightAvail = currentSize.height();
        } else {
            heightAvail = currentSize.height() -
                          displaySizeHint(option, currentSize.width()).height() -
                          verticalMargin[IconWidgetPrivate::TextMargin].top -
                          verticalMargin[IconWidgetPrivate::TextMargin].bottom;
            //never make a label higher than half the total height
            heightAvail = qMax(heightAvail, currentSize.height() / 2);
        }

        //aspect ratio very "tall"
        if (!text.isEmpty() || !infoText.isEmpty()) {
            if (currentSize.width() < heightAvail) {
                iconWidth = currentSize.width() -
                            verticalMargin[IconWidgetPrivate::IconMargin].left -
                            verticalMargin[IconWidgetPrivate::IconMargin].right;
            } else {
                iconWidth = heightAvail -
                            verticalMargin[IconWidgetPrivate::IconMargin].top -
                            verticalMargin[IconWidgetPrivate::IconMargin].bottom;
            }
        } else {
            iconWidth = qMin(heightAvail, currentSize.width());
        }

        iconWidth -= verticalMargin[IconWidgetPrivate::ItemMargin].left + verticalMargin[IconWidgetPrivate::ItemMargin].right;
    } else {
        //Horizontal layout
        //if there is text resize the icon in order to make room for the text
        if (text.isEmpty() && infoText.isEmpty()) {
            // with no text, we just take up the whole geometry
            iconWidth = qMin(currentSize.height(), currentSize.width());
        } else {
            iconWidth = currentSize.height() -
                        horizontalMargin[IconWidgetPrivate::IconMargin].top -
                        horizontalMargin[IconWidgetPrivate::IconMargin].bottom;
        }
        iconWidth -= horizontalMargin[IconWidgetPrivate::ItemMargin].top + horizontalMargin[IconWidgetPrivate::ItemMargin].bottom;
    }
    iconSize = QSizeF(iconWidth, iconWidth);

    if (maximumIconSize.isValid()) {
        iconSize = iconSize.boundedTo(maximumIconSize);
    }

    int count = 0;
    foreach (IconAction *iconAction, cornerActions) {
        iconAction->setRect(actionRect((IconWidgetPrivate::ActionPosition)count));
        ++count;
    }
}

void IconWidget::setSvg(const QString &svgFilePath, const QString &elementId)
{
    if (svgFilePath.isEmpty()) {
        if (d->iconSvg) {
            d->iconSvg->deleteLater();
            d->iconSvg = 0;
        }
        return;
    }

    if (!d->iconSvg) {
        d->iconSvg = new Plasma::Svg(this);
        connect(d->iconSvg, SIGNAL(repaintNeeded()), this, SLOT(svgChanged()));
    }

    d->iconSvg->setImagePath(svgFilePath);
    d->iconSvg->setContainsMultipleImages(!elementId.isNull());
    d->iconSvgElement = elementId;
    d->iconSvgElementChanged = true;
    d->icon = QIcon();
    updateGeometry();
    update();
}

QString IconWidget::svg() const
{
    if (d->iconSvg) {
        if (d->iconSvg->isValid() && (d->iconSvgElement.isEmpty() || d->iconSvg->hasElement(d->iconSvgElement))) {
            return d->iconSvg->imagePath();
        } else {
            return QString();
        }
    }

    return QString();
}

QSizeF IconWidget::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    if (which == Qt::PreferredSize) {
        if (d->preferredIconSize.isValid()) {
            return sizeFromIconSize(qMax(d->preferredIconSize.height(), d->preferredIconSize.width()));
        }
        int iconSize = KIconLoader::SizeMedium;
        if (d->iconSvg) {
            QSizeF oldSize = d->iconSvg->size();
            d->iconSvg->resize();
            if (d->iconSvgElement.isNull()) {
                iconSize = qMax(d->iconSvg->size().width(), d->iconSvg->size().height());
            } else {
                iconSize = qMax(d->iconSvg->elementSize(d->iconSvgElement).width(), d->iconSvg->elementSize(d->iconSvgElement).height());
            }
            d->iconSvg->resize(oldSize);
        }
        return sizeFromIconSize(iconSize);
    } else if (which == Qt::MinimumSize) {
        if (d->minimumIconSize.isValid()) {
            return sizeFromIconSize(qMax(d->minimumIconSize.height(), d->minimumIconSize.width()));
        }
        return sizeFromIconSize(KIconLoader::SizeSmall);
    } else {
        if (d->maximumIconSize.isValid()) {
            return sizeFromIconSize(qMax(d->maximumIconSize.height(), d->maximumIconSize.width()));
        }
        return QGraphicsWidget::sizeHint(which, constraint);
    }
}

void IconWidgetPrivate::animateMainIcon(bool show, const IconWidgetStates state)
{
    if (show) {
        states = state;
    }

    hoverAnimation->setFadeIn(show);

    QPropertyAnimation *animation = hoverAnimation->animation();
    if (!animation) {
        animation = new QPropertyAnimation(hoverAnimation, "value");
        animation->setProperty("duration", 150);
        animation->setProperty("easingCurve", QEasingCurve::OutQuad);
        animation->setProperty("startValue", 0.0);
        animation->setProperty("endValue", 1.0);
        hoverAnimation->setAnimation(animation);
        q->connect(animation, SIGNAL(finished()), q, SLOT(hoverAnimationFinished()));
    } else if (animation->state() == QAbstractAnimation::Running) {
        animation->pause();
    }

    animation->setProperty("direction", show ?
            QAbstractAnimation::Forward : QAbstractAnimation::Backward);
    animation->start(show ?
            QAbstractAnimation::KeepWhenStopped : QAbstractAnimation::DeleteWhenStopped);
}

void IconWidgetPrivate::hoverAnimationFinished()
{
    if (!hoverAnimation->fadeIn()) {
        states &= ~IconWidgetPrivate::HoverState;
    }
}

void IconWidgetPrivate::drawBackground(QPainter *painter, IconWidgetState state)
{
    if (!drawBg) {
        return;
    }

    if (!(states & IconWidgetPrivate::HoverState) && !(states & IconWidgetPrivate::PressedState)) {
        return;
    }

    if (state == IconWidgetPrivate::PressedState) {
        background->setElementPrefix("selected");
    } else {
        background->setElementPrefix("hover");
    }

    if (qFuzzyCompare(hoverAnimation->value(), 1)) {
        background->resizeFrame(currentSize);
        background->paintFrame(painter);
    } else if (!qFuzzyCompare(hoverAnimation->value()+1, 1)) {
        background->resizeFrame(currentSize);
        QPixmap frame = background->framePixmap();
        QPainter bufferPainter(&frame);
        bufferPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        bufferPainter.fillRect(frame.rect(), QColor(0,0,0, 255*hoverAnimation->value()));
        bufferPainter.end();
        painter->drawPixmap(QPoint(0,0), frame);
    }
}

QPixmap IconWidgetPrivate::decoration(const QStyleOptionGraphicsItem *option, bool useHoverEffect, bool usePressedEffect)
{
    QPixmap result;

    QIcon::Mode mode   = option->state & QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled;
    QIcon::State state = option->state & QStyle::State_Open ? QIcon::On : QIcon::Off;

    if (iconSvg) {
        if (iconSvgElementChanged || iconSvgPixmap.size() != iconSize.toSize()) {
            iconSvg->resize(iconSize);
            iconSvgPixmap = iconSvg->pixmap(iconSvgElement);
            iconSvgElementChanged = false;
        }
        result = iconSvgPixmap;
    } else {
        const QSize size = icon.actualSize(iconSize.toSize(), mode, state);
        result = icon.pixmap(size, mode, state);
    }

    if (usePressedEffect) {
        result = result.scaled(result.size() * 0.9, Qt::KeepAspectRatio);
    }

    if (!result.isNull() && useHoverEffect) {
        KIconEffect *effect = KIconLoader::global()->iconEffect();
        // Note that in KIconLoader terminology, active = hover.
        // We're assuming that the icon group is desktop/filemanager, since this
        // is KFileItemDelegate.
        if (effect->hasEffect(KIconLoader::Desktop, KIconLoader::ActiveState)) {
            if (qFuzzyCompare(qreal(1.0), hoverAnimation->value())) {
                result = effect->apply(result, KIconLoader::Desktop, KIconLoader::ActiveState);
            } else {
                result = PaintUtils::transition(
                    result,
                    effect->apply(result, KIconLoader::Desktop,
                                  KIconLoader::ActiveState), hoverAnimation->value());
            }
        }
    } else if (!result.isNull() && !oldIcon.isNull()) {
        if (qFuzzyCompare(qreal(1.0), hoverAnimation->value())) {
            oldIcon = QIcon();
        } else {
            result = PaintUtils::transition(
                oldIcon.pixmap(result.size(), mode, state),
                result, hoverAnimation->value());
        }
    }

    return result;
}

QPointF IconWidgetPrivate::iconPosition(const QStyleOptionGraphicsItem *option,
                                        const QPixmap &pixmap) const
{
    const QRectF itemRect = subtractMargin(option->rect, IconWidgetPrivate::ItemMargin);

    // Compute the nominal decoration rectangle
    const QSizeF size = addMargin(iconSize, IconWidgetPrivate::IconMargin);


    Qt::LayoutDirection direction = iconDirection(option);

    //alignment depends from orientation and option->direction
    Qt::Alignment alignment;
    if (text.isEmpty() && infoText.isEmpty()) {
        alignment = Qt::AlignCenter;
    } else if (orientation == Qt::Vertical) {
        alignment = Qt::Alignment(Qt::AlignHCenter | Qt::AlignTop);
    //Horizontal
    } else {
        alignment = QStyle::visualAlignment(
            direction, Qt::Alignment(Qt::AlignLeft | Qt::AlignVCenter));
    }

    const QRect iconRect =
        QStyle::alignedRect(direction, alignment, size.toSize(), itemRect.toRect());

    // Position the pixmap in the center of the rectangle
    QRect pixmapRect = pixmap.rect();
    pixmapRect.moveCenter(iconRect.center());

    // add a gimmicky margin of 5px to y, TEMP TEMP TEMP
    // pixmapRect = pixmapRect.adjusted(0, 5, 0, 0);

    return QPointF(pixmapRect.topLeft());
}

QRectF IconWidgetPrivate::labelRectangle(const QStyleOptionGraphicsItem *option,
                                         const QPixmap &icon,
                                         const QString &string) const
{
    Q_UNUSED(string)

    if (icon.isNull()) {
        return option->rect;
    }

    const QSizeF decoSize = addMargin(iconSize, IconWidgetPrivate::IconMargin);
    const QRectF itemRect = subtractMargin(option->rect, IconWidgetPrivate::ItemMargin);
    QRectF textArea(QPointF(0, 0), itemRect.size());

    if (orientation == Qt::Vertical) {
        textArea.setTop(decoSize.height() + 1);
    } else {
        //Horizontal
       textArea.setLeft(decoSize.width() + 1);
    }

    textArea.translate(itemRect.topLeft());
    return QRectF(QStyle::visualRect(iconDirection(option), option->rect, textArea.toRect()));
}

// Lays the text out in a rectangle no larger than constraints, eliding it as necessary
QSizeF IconWidgetPrivate::layoutText(QTextLayout &layout, const QStyleOptionGraphicsItem *option,
                                     const QString &text, const QSizeF &constraints) const
{
    const QSizeF size = layoutText(layout, text, constraints.width());

    if (size.width() > constraints.width() || size.height() > constraints.height()) {
        if (action) {
            q->setToolTip(action->toolTip());
        }
        const QString elided = elidedText(layout, option, constraints);
        return layoutText(layout, elided, constraints.width());
    }
    q->setToolTip(QString());

    return size;
}

// Lays the text out in a rectangle no wider than maxWidth
QSizeF IconWidgetPrivate::layoutText(QTextLayout &layout, const QString &text, qreal maxWidth) const
{
    QFontMetricsF metrics(layout.font());
    qreal leading     = metrics.leading();
    qreal height      = 0.0;
    qreal widthUsed   = 0.0;
    QTextLine line;

    layout.setText(text);

    layout.beginLayout();

    while ((line = layout.createLine()).isValid()) {
        line.setLineWidth(maxWidth);
        height += leading;
        line.setPosition(QPointF(0.0, height));
        height += line.height();
        widthUsed = qMax(widthUsed, line.naturalTextWidth());
    }
    layout.endLayout();

    return QSizeF(widthUsed, height);
}

// Elides the text in the layout, by iterating over each line in the layout, eliding
// or word breaking the line if it's wider than the max width, and finally adding an
// ellipses at the end of the last line, if there are more lines than will fit within
// the vertical size constraints.
QString IconWidgetPrivate::elidedText(QTextLayout &layout, const QStyleOptionGraphicsItem *option,
                                      const QSizeF &size) const
{
    Q_UNUSED(option)

    QFontMetricsF metrics(layout.font());
    const QString text = layout.text();
    qreal maxWidth       = size.width();
    qreal maxHeight      = size.height();
    qreal height         = 0;

    // Elide each line that has already been laid out in the layout.
    QString elided;
    elided.reserve(text.length());

    for (int i = 0; i < layout.lineCount(); i++) {
        QTextLine line = layout.lineAt(i);
        int start  = line.textStart();
        int length = line.textLength();

        height += metrics.leading();
        if (height + line.height() + metrics.lineSpacing() > maxHeight) {
            // Unfortunately, if the line ends because of a line separator,
            // elidedText() will be too clever and keep adding lines until
            // it finds one that's too wide.
            if (line.naturalTextWidth() < maxWidth &&
                start + length > 0 &&
                text[start + length - 1] == QChar::LineSeparator) {
                elided += text.mid(start, length - 1);
            } else {
                elided += metrics.elidedText(text.mid(start), Qt::ElideRight, maxWidth);
            }
            break;
        } else if (line.naturalTextWidth() > maxWidth) {
            elided += metrics.elidedText(text.mid(start, length), Qt::ElideRight, maxWidth);
        } else {
            elided += text.mid(start, length);
        }

        height += line.height();
    }

    return elided;
}

void IconWidgetPrivate::layoutTextItems(const QStyleOptionGraphicsItem *option,
                                        const QPixmap &icon, QTextLayout *labelLayout,
                                        QTextLayout *infoLayout, QRectF *textBoundingRect) const
{
    bool showInformation = false;

    setLayoutOptions(*labelLayout, option, q->orientation());

    QFontMetricsF fm(labelLayout->font());
    const QRectF textArea = labelRectangle(option, icon, text);
    QRectF textRect = subtractMargin(textArea, IconWidgetPrivate::TextMargin);

    //kDebug() << this << "text area" << textArea << "text rect" << textRect;
    // Sizes and constraints for the different text parts
    QSizeF maxLabelSize = textRect.size();
    QSizeF maxInfoSize  = textRect.size();
    QSizeF labelSize;
    QSizeF infoSize;

    // If we have additional info text, and there's space for at least two lines of text,
    // adjust the max label size to make room for at least one line of the info text
    if (!infoText.isEmpty() && textRect.height() >= fm.lineSpacing() * 2) {
        infoLayout->setFont(labelLayout->font());
        infoLayout->setTextOption(labelLayout->textOption());

        maxLabelSize.rheight() -= fm.lineSpacing();
        showInformation = true;
    }

    // Lay out the label text, and adjust the max info size based on the label size
    labelSize = layoutText(*labelLayout, option, text, maxLabelSize);
    maxInfoSize.rheight() -= labelSize.height();

    // Lay out the info text
    if (showInformation) {
        infoSize = layoutText(*infoLayout, option, infoText, maxInfoSize);
    } else {
        infoSize = QSizeF(0, 0);
    }
    // Compute the bounding rect of the text
    const Qt::Alignment alignment = labelLayout->textOption().alignment();
    const QSizeF size(qMax(labelSize.width(), infoSize.width()),
                      labelSize.height() + infoSize.height());
    *textBoundingRect =
        QStyle::alignedRect(iconDirection(option), alignment, size.toSize(), textRect.toRect());

    // Compute the positions where we should draw the layouts
    haloRects.clear();
    labelLayout->setPosition(QPointF(textRect.x(), textBoundingRect->y()));
    QTextLine line;
    for (int i = 0; i < labelLayout->lineCount(); ++i) {
        line = labelLayout->lineAt(i);
        haloRects.append(line.naturalTextRect().translated(labelLayout->position().toPoint()).toRect());
    }
    infoLayout->setPosition(QPointF(textRect.x(), textBoundingRect->y() + labelSize.height()));
    for (int i = 0; i < infoLayout->lineCount(); ++i) {
        line = infoLayout->lineAt(i);
        haloRects.append(line.naturalTextRect().translated(infoLayout->position().toPoint()).toRect());
    }
    //kDebug() << "final position is" << labelLayout->position();
}

QBrush IconWidgetPrivate::foregroundBrush(const QStyleOptionGraphicsItem *option) const
{
    const QPalette::ColorGroup group = option->state & QStyle::State_Enabled ?
            QPalette::Normal : QPalette::Disabled;

    // Always use the highlight color for selected items
    if (option->state & QStyle::State_Selected) {
        return option->palette.brush(group, QPalette::HighlightedText);
    }
    return option->palette.brush(group, QPalette::Text);
}

QBrush IconWidgetPrivate::backgroundBrush(const QStyleOptionGraphicsItem *option) const
{
    const QPalette::ColorGroup group = option->state & QStyle::State_Enabled ?
            QPalette::Normal : QPalette::Disabled;

    QBrush background(Qt::NoBrush);

    // Always use the highlight color for selected items
    if (option->state & QStyle::State_Selected) {
        background = option->palette.brush(group, QPalette::Highlight);
    }
    return background;
}

void IconWidgetPrivate::drawTextItems(QPainter *painter,
                                      const QStyleOptionGraphicsItem *option,
                                      const QTextLayout &labelLayout,
                                      const QTextLayout &infoLayout) const
{
    Q_UNUSED(option)

    painter->save();
    painter->setPen(textColor);

    // the translation prevents odd rounding errors in labelLayout.position()
    // when applied to the canvas
    painter->translate(0.5, 0.5);

    labelLayout.draw(painter, QPointF());

    if (!infoLayout.text().isEmpty()) {
        painter->setPen(textColor);
        infoLayout.draw(painter, QPointF());
    }
    painter->restore();
}

void IconWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    //Lay out the main icon and action icons
    d->layoutIcons(option);

    // Compute the metrics, and lay out the text items
    // ========================================================================
    IconWidgetPrivate::IconWidgetState state = IconWidgetPrivate::NoState;
    if (d->states & IconWidgetPrivate::ManualPressedState) {
        state = IconWidgetPrivate::PressedState;
    } else if (d->states & IconWidgetPrivate::PressedState) {
        if (d->states & IconWidgetPrivate::HoverState) {
            state = IconWidgetPrivate::PressedState;
        }
    } else if (d->states & IconWidgetPrivate::HoverState) {
        state = IconWidgetPrivate::HoverState;
    }

    QPixmap icon = d->decoration(option, state != IconWidgetPrivate::NoState, state & IconWidgetPrivate::PressedState);
    const QPointF iconPos = d->iconPosition(option, icon);

    d->drawBackground(painter, state);

    // draw icon
    if (!icon.isNull()) {
        painter->drawPixmap(iconPos, icon);
    }

    // Draw corner actions
    foreach (const IconAction *action, d->cornerActions) {
        if (action->isAnimating()) {
            action->paint(painter);
        }
    }

    // Draw text last because it is overlayed
    QTextLayout labelLayout, infoLayout;
    QRectF textBoundingRect;

    d->layoutTextItems(option, icon, &labelLayout, &infoLayout, &textBoundingRect);

    if (d->textBgColor != QColor() && d->textBgColor.alpha() > 0 &&
        !(d->text.isEmpty() && d->infoText.isEmpty()) &&
        !textBoundingRect.isEmpty() &&
        !qFuzzyCompare(d->hoverAnimation->value(), (qreal)1.0)) {
        QRectF rect = textBoundingRect.adjusted(-2, -2, 4, 4).toAlignedRect();
        painter->setPen(Qt::transparent);
        QColor color = d->textBgColor;
        color.setAlpha(60 * (1.0 - d->hoverAnimation->value()));
        QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
        gradient.setColorAt(0, color.lighter(120));
        gradient.setColorAt(1, color.darker(120));
        painter->setBrush(gradient);
        gradient.setColorAt(0, color.lighter(130));
        gradient.setColorAt(1, color.darker(130));
        painter->setPen(QPen(gradient, 0));
        painter->setRenderHint(QPainter::Antialiasing);
        painter->drawPath(PaintUtils::roundedRectangle(rect.translated(0.5, 0.5), 4));
    }


    if (d->shadowColor.value() < 128 || textBackgroundColor() != QColor()) {
        QPoint shadowPos;
        if (d->shadowColor.value() < 128) {
            shadowPos = QPoint(1, 2);
        } else {
            shadowPos = QPoint(0, 0);
        }

        QImage shadow(textBoundingRect.size().toSize() + QSize(4, 4),
                    QImage::Format_ARGB32_Premultiplied);
        shadow.fill(Qt::transparent);
        {
            QPainter buffPainter(&shadow);
            buffPainter.translate(-textBoundingRect.x(), -textBoundingRect.y());
            d->drawTextItems(&buffPainter, option, labelLayout, infoLayout);
        }

        PaintUtils::shadowBlur(shadow, 2, d->shadowColor);
        painter->drawImage(textBoundingRect.topLeft() + shadowPos, shadow);
    } else if (!(d->text.isEmpty() && d->infoText.isEmpty()) &&
               !textBoundingRect.isEmpty()) {
        QRect labelRect = d->labelRectangle(option, icon, d->text).toRect();

        foreach (const QRect &rect, d->haloRects) {
            Plasma::PaintUtils::drawHalo(painter, rect);
        }
    }

    d->drawTextItems(painter, option, labelLayout, infoLayout);
}

void IconWidget::setTextBackgroundColor(const QColor &color)
{
    d->textBgCustomized = true;
    d->textBgColor = color;
    update();
}

QColor IconWidget::textBackgroundColor() const
{
    return d->textBgColor;
}

void IconWidget::drawActionButtonBase(QPainter *painter, const QSize &size, int element)
{
    qreal radius = size.width() / 2;
    QRadialGradient gradient(radius, radius, radius, radius, radius);
    int alpha;

    if (element == IconWidgetPrivate::MinibuttonPressed) {
        alpha = 255;
    } else if (element == IconWidgetPrivate::MinibuttonHover) {
        alpha = 200;
    } else {
        alpha = 160;
    }
    gradient.setColorAt(0, QColor::fromRgb(d->textColor.red(),
                                           d->textColor.green(),
                                           d->textColor.blue(), alpha));
    gradient.setColorAt(1, QColor::fromRgb(d->textColor.red(),
                                           d->textColor.green(),
                                           d->textColor.blue(), 0));

    painter->setBrush(gradient);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QRectF(QPointF(.0, .0), size));
}

void IconWidget::setText(const QString &text)
{
    d->text = KGlobal::locale()->removeAcceleratorMarker(text);
    // cause a relayout
    d->currentSize = QSizeF(-1, -1);
    //try to relayout, needed if an icon was never shown before
    if (!isVisible()) {
        QStyleOptionGraphicsItem styleoption;
        d->layoutIcons(&styleoption);
    }
    updateGeometry();
    if (!parentWidget() || !parentWidget()->layout()) {
        resize(preferredSize());
    }
}

QString IconWidget::text() const
{
    return d->text;
}

void IconWidget::setInfoText(const QString &text)
{
    d->infoText = text;
    // cause a relayout
    d->currentSize = QSizeF(-1, -1);
    //try to relayout, needed if an icon was never shown before
    if (!isVisible()) {
        QStyleOptionGraphicsItem styleoption;
        d->layoutIcons(&styleoption);
    }
    updateGeometry();
    if (!parentWidget() || !parentWidget()->layout()) {
        resize(preferredSize());
    }
}

QString IconWidget::infoText() const
{
    return d->infoText;
}

QIcon IconWidget::icon() const
{
    return d->icon;
}

void IconWidget::setIcon(const QString &icon)
{
    if (icon.isEmpty()) {
        setIcon(QIcon());
        return;
    }

    setIcon(KIcon(icon));
}

void IconWidget::setIcon(const QIcon &icon)
{
    setSvg(QString());

    /*fade to the new icon, but to not bee a too big hog, not do that when:
      - the fade animation is already running
      - the icon is under mouse
      - one betwen the old and new icon is null*/
    if (!(d->states & IconWidgetPrivate::HoverState) && !d->iconChangeTimer->isActive() && d->oldIcon.isNull() && !d->icon.isNull() && !icon.isNull()) {
        d->oldIcon = d->icon;
        d->animateMainIcon(true, d->states);
    } else {
        d->oldIcon = QIcon();
    }
    d->iconChangeTimer->start(300);
    d->icon = icon;
    update();
}

QSizeF IconWidget::iconSize() const
{
    return d->iconSize;
}

void IconWidget::setPreferredIconSize(const QSizeF &size)
{
    d->preferredIconSize = size;
    updateGeometry();
}

QSizeF IconWidget::preferredIconSize() const
{
    return d->preferredIconSize;
}

void IconWidget::setMinimumIconSize(const QSizeF &size)
{
    d->minimumIconSize = size;
    updateGeometry();
}

QSizeF IconWidget::minimumIconSize() const
{
    return d->minimumIconSize;
}

void IconWidget::setMaximumIconSize(const QSizeF &size)
{
    d->maximumIconSize = size;
    updateGeometry();
}

QSizeF IconWidget::maximumIconSize() const
{
    return d->maximumIconSize;
}

bool IconWidget::isDown()
{
    return d->states & IconWidgetPrivate::PressedState;
}

void IconWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QGraphicsWidget::mousePressEvent(event);
        return;
    }

    d->states |= IconWidgetPrivate::PressedState;
    d->clickStartPos = scenePos();

    bool handled = false;
    foreach (IconAction *action, d->cornerActions) {
        handled = action->event(event->type(), event->pos());
        if (handled) {
            break;
        }
    }

    if (!handled && boundingRect().contains(event->pos())) {
        emit pressed(true);
    }

    update();
}

void IconWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (~d->states & IconWidgetPrivate::PressedState) {
        QGraphicsWidget::mouseMoveEvent(event);
        return;
    }

    if (boundingRect().contains(event->pos())) {
        if (~d->states & IconWidgetPrivate::HoverState) {
            d->states |= IconWidgetPrivate::HoverState;
            update();
        }
    } else {
        if (d->states & IconWidgetPrivate::HoverState) {
            d->states &= ~IconWidgetPrivate::HoverState;
            update();
        }
    }
}

void IconWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (~d->states & IconWidgetPrivate::PressedState) {
        QGraphicsWidget::mouseMoveEvent(event);
        return;
    }

    d->states &= ~IconWidgetPrivate::PressedState;

    //don't pass click when the mouse was moved
    bool handled = d->clickStartPos != scenePos();
    if (!handled) {
        foreach (IconAction *action, d->cornerActions) {
            if (action->event(event->type(), event->pos())) {
                handled = true;
                break;
            }
        }
    }

    if (!handled) {
        if (boundingRect().contains(event->pos())) {
            emit clicked();
            if (KGlobalSettings::singleClick()) {
               emit activated();
            }

            if (d->action && d->action->menu()) {
                d->action->menu()->popup(event->screenPos());
            }
        }
        emit pressed(false);
    }

    update();
}

void IconWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)

    emit doubleClicked();
    if (!KGlobalSettings::singleClick()) {
        emit activated();
    }
}

void IconWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    //kDebug();
    foreach (IconAction *action, d->cornerActions) {
        action->show();
        action->event(event->type(), event->pos());
    }

    d->oldIcon = QIcon();
    d->animateMainIcon(true, d->states|IconWidgetPrivate::HoverState);
    update();

    QGraphicsWidget::hoverEnterEvent(event);
}

void IconWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    //kDebug() << d->cornerActions;
    foreach (IconAction *action, d->cornerActions) {
        action->hide();
        action->event(event->type(), event->pos());
    }
    // d->states &= ~IconWidgetPrivate::HoverState; // Will be set once progress is zero again ...
    //if an eventfilter stolen the mousereleaseevent remove the pressed state here
    d->states &= ~IconWidgetPrivate::PressedState;

    d->animateMainIcon(false, d->states|IconWidgetPrivate::HoverState);
    update();

    QGraphicsWidget::hoverLeaveEvent(event);
}

bool IconWidget::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    Q_UNUSED(watched)

    if (event->type() == QEvent::GraphicsSceneDragEnter) {
        d->animateMainIcon(true, d->states|IconWidgetPrivate::HoverState);
        update();
    } else if (event->type() == QEvent::GraphicsSceneDragLeave) {
        d->animateMainIcon(false, d->states|IconWidgetPrivate::HoverState);
        update();
    }

    return false;
}

void IconWidget::setPressed(bool pressed)
{
    if (pressed) {
        d->states |= IconWidgetPrivate::ManualPressedState;
        d->states |= IconWidgetPrivate::PressedState;
    } else {
        d->states &= ~IconWidgetPrivate::ManualPressedState;
        d->states &= ~IconWidgetPrivate::PressedState;
    }
    update();
}

void IconWidget::setUnpressed()
{
    setPressed(false);
}

void IconWidgetPrivate::svgChanged()
{
    iconSvgElementChanged = true;
    q->update();
}

void IconWidget::setOrientation(Qt::Orientation orientation)
{
    d->orientation = orientation;
    resize(sizeFromIconSize(d->iconSize.width()));
}

Qt::Orientation IconWidget::orientation() const
{
    return d->orientation;
}

void IconWidget::invertLayout(bool invert)
{
    d->invertLayout = invert;
}

bool IconWidget::invertedLayout() const
{
    return d->invertLayout;
}

QSizeF IconWidget::sizeFromIconSize(const qreal iconWidth) const
{
    d->setActiveMargins();
    if (d->text.isEmpty() && d->infoText.isEmpty()) {
        //no text, just the icon size
        return d->addMargin(QSizeF(iconWidth, iconWidth), IconWidgetPrivate::ItemMargin);
    }

    QFontMetricsF fm(d->widgetFont());
    qreal width = 0;

    if (d->orientation == Qt::Vertical) {
        width = qMax(d->maxWordWidth(d->text),
                     d->maxWordWidth(d->infoText)) +
                     fm.width("xxx") +
                     d->verticalMargin[IconWidgetPrivate::TextMargin].left +
                     d->verticalMargin[IconWidgetPrivate::TextMargin].right;

        width = qMax(width,
                     iconWidth +
                     d->verticalMargin[IconWidgetPrivate::IconMargin].left +
                     d->verticalMargin[IconWidgetPrivate::IconMargin].right);
    } else {
        width = iconWidth +
                d->horizontalMargin[IconWidgetPrivate::IconMargin].left +
                d->horizontalMargin[IconWidgetPrivate::IconMargin].right +
                qMax(fm.width(d->text), fm.width(d->infoText)) + fm.width("xxx") +
                d->horizontalMargin[IconWidgetPrivate::TextMargin].left +
                d->horizontalMargin[IconWidgetPrivate::TextMargin].right;
    }

    qreal height;
    qreal textHeight;

    QStyleOptionGraphicsItem option;
    option.state = QStyle::State_None;
    option.rect = QRect(0, 0, width, QWIDGETSIZE_MAX);
    textHeight = d->displaySizeHint(&option, width).height();

    if (d->orientation == Qt::Vertical) {
        height = iconWidth + textHeight +
                 d->verticalMargin[IconWidgetPrivate::TextMargin].top +
                 d->verticalMargin[IconWidgetPrivate::TextMargin].bottom +
                 d->verticalMargin[IconWidgetPrivate::IconMargin].top +
                 d->verticalMargin[IconWidgetPrivate::IconMargin].bottom;
    } else {
        //Horizontal
        height = qMax(iconWidth +
                      d->verticalMargin[IconWidgetPrivate::IconMargin].top +
                      d->verticalMargin[IconWidgetPrivate::IconMargin].bottom,
                      textHeight +
                      d->verticalMargin[IconWidgetPrivate::TextMargin].top +
                      d->verticalMargin[IconWidgetPrivate::TextMargin].bottom);
    }

    return d->addMargin(QSizeF(width, height), IconWidgetPrivate::ItemMargin);
}

void IconWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        d->customFont = true;
    }

    QGraphicsWidget::changeEvent(event);
}

} // namespace Plasma

#include "iconwidget.moc"
#include "iconwidget_p.moc"
