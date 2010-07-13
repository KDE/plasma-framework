/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#include "pushbutton.h"

#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <QDir>
#include <QApplication>

#include <QWeakPointer>
#include <QPropertyAnimation>

#include <kicon.h>
#include <kiconeffect.h>
#include <kmimetype.h>
#include <kpushbutton.h>

#include "theme.h"
#include "svg.h"
#include "framesvg.h"
#include "animator.h"
#include "paintutils.h"
#include "private/actionwidgetinterface_p.h"
#include <plasma/private/focusindicator_p.h>
#include "animations/animation.h"

namespace Plasma
{

class PushButtonPrivate : public ActionWidgetInterface<PushButton>
{
public:
    PushButtonPrivate(PushButton *pushButton)
        : ActionWidgetInterface<PushButton>(pushButton),
          q(pushButton),
          background(0),
          fadeIn(false),
          svg(0),
          customFont(false)
    {
    }

    ~PushButtonPrivate()
    {
        delete svg;
    }

    void setPixmap()
    {
        if (imagePath.isEmpty()) {
            delete svg;
            svg = 0;
            return;
        }

        KMimeType::Ptr mime = KMimeType::findByPath(absImagePath);
        QPixmap pm;

        if (mime->is("image/svg+xml") || mime->is("image/svg+xml-compressed")) {
            if (!svg || svg->imagePath() != absImagePath) {
                delete svg;
                svg = new Svg();
                svg->setImagePath(imagePath);
                QObject::connect(svg, SIGNAL(repaintNeeded()), q, SLOT(setPixmap()));
                if (!svgElement.isNull()) {
                    svg->setContainsMultipleImages(true);
                }
            }

            //QPainter p(&pm);

            if (!svgElement.isEmpty() && svg->hasElement(svgElement)) {
                svg->resize();
                QSizeF elementSize = svg->elementSize(svgElement);
                float scale = q->nativeWidget()->iconSize().width() / qMax(elementSize.width(), elementSize.height());

                svg->resize(elementSize * scale);
                pm = svg->pixmap(svgElement);
            } else {
                svg->resize(q->nativeWidget()->iconSize());
                pm = svg->pixmap();
            }
        } else {
            delete svg;
            svg = 0;
            pm = QPixmap(absImagePath);
        }

        static_cast<KPushButton*>(q->widget())->setIcon(KIcon(pm));
    }

    void syncActiveRect();
    void syncBorders();

    PushButton *q;

    FrameSvg *background;
    bool fadeIn;
    qreal opacity;
    QRectF activeRect;

    Animation *hoverAnimation;

    QString imagePath;
    QString absImagePath;
    Svg *svg;
    QString svgElement;
    bool customFont;
};

void PushButtonPrivate::syncActiveRect()
{
    background->setElementPrefix("normal");

    qreal left, top, right, bottom;
    background->getMargins(left, top, right, bottom);

    background->setElementPrefix("active");
    qreal activeLeft, activeTop, activeRight, activeBottom;
    background->getMargins(activeLeft, activeTop, activeRight, activeBottom);

    activeRect = QRectF(QPointF(0, 0), q->size());
    activeRect.adjust(left - activeLeft, top - activeTop,
                      -(right - activeRight), -(bottom - activeBottom));

    background->setElementPrefix("normal");
}

void PushButtonPrivate::syncBorders()
{
    //set margins from the normal element
    qreal left, top, right, bottom;

    background->setElementPrefix("normal");
    background->getMargins(left, top, right, bottom);
    q->setContentsMargins(left, top, right, bottom);

    //calc the rect for the over effect
    syncActiveRect();
}


PushButton::PushButton(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new PushButtonPrivate(this))
{
    d->background = new FrameSvg(this);
    d->background->setImagePath("widgets/button");
    d->background->setCacheAllRenderedFrames(true);

    d->background->setElementPrefix("normal");

    d->hoverAnimation = Animator::create(Animator::PixmapTransitionAnimation);
    d->hoverAnimation->setTargetWidget(this);
    d->hoverAnimation->setProperty("startPixmap", d->background->framePixmap());

    KPushButton *native = new KPushButton;
    connect(native, SIGNAL(pressed()), this, SIGNAL(pressed()));
    connect(native, SIGNAL(released()), this, SIGNAL(released()));
    connect(native, SIGNAL(clicked()), this, SIGNAL(clicked()));
    connect(native, SIGNAL(toggled(bool)), this, SIGNAL(toggled(bool)));
    setWidget(native);
    native->setAttribute(Qt::WA_NoSystemBackground);
    native->setWindowIcon(QIcon());

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    FocusIndicator *focusIndicator = new FocusIndicator(this, "widgets/button");

    d->syncBorders();
    setAcceptHoverEvents(true);

    connect(d->background, SIGNAL(repaintNeeded()), SLOT(syncBorders()));
}

PushButton::~PushButton()
{
    delete d;
}

void PushButton::setText(const QString &text)
{
    static_cast<KPushButton*>(widget())->setText(text);
    updateGeometry();
}

QString PushButton::text() const
{
    return static_cast<KPushButton*>(widget())->text();
}

void PushButton::setImage(const QString &path)
{
    if (d->imagePath == path) {
        return;
    }

    delete d->svg;
    d->svg = 0;
    d->imagePath = path;

    bool absolutePath = !path.isEmpty() &&
                        #ifdef Q_WS_WIN
                            !QDir::isRelativePath(path)
                        #else
                            (path[0] == '/' || path.startsWith(QLatin1String(":/")))
                        #endif
        ;

    if (absolutePath) {
        d->absImagePath = path;
    } else {
        //TODO: package support
        d->absImagePath = Theme::defaultTheme()->imagePath(path);
    }

    d->setPixmap();
}

void PushButton::setImage(const QString &path, const QString &elementid)
{
    d->svgElement = elementid;
    setImage(path);
}

QString PushButton::image() const
{
    return d->imagePath;
}

void PushButton::setStyleSheet(const QString &stylesheet)
{
    widget()->setStyleSheet(stylesheet);
}

QString PushButton::styleSheet()
{
    return widget()->styleSheet();
}

void PushButton::setAction(QAction *action)
{
    d->setAction(action);
}

QAction *PushButton::action() const
{
    return d->action;
}

void PushButton::setIcon(const KIcon &icon)
{
    nativeWidget()->setIcon(icon);
}

void PushButton::setIcon(const QIcon &icon)
{
    setIcon(KIcon(icon));
}

QIcon PushButton::icon() const
{
    return nativeWidget()->icon();
}

void PushButton::setCheckable(bool checkable)
{
    nativeWidget()->setCheckable(checkable);
}

bool PushButton::isCheckable() const
{
    return nativeWidget()->isCheckable();
}

void PushButton::setChecked(bool checked)
{
    nativeWidget()->setChecked(checked);
}

bool PushButton::isChecked() const
{
    return nativeWidget()->isChecked();
}

bool PushButton::isDown() const
{
    return nativeWidget()->isDown();
}

KPushButton *PushButton::nativeWidget() const
{
    return static_cast<KPushButton*>(widget());
}

void PushButton::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    d->setPixmap();

   if (d->background) {
        //resize all four panels
        d->background->setElementPrefix("pressed");
        d->background->resizeFrame(size());

        d->syncActiveRect();

        d->background->setElementPrefix("active");
        d->background->resizeFrame(d->activeRect.size());
        d->hoverAnimation->setProperty("targetPixmap", d->background->framePixmap());
        d->background->setElementPrefix("focus");
        d->background->resizeFrame(d->activeRect.size());

        d->background->setElementPrefix("normal");
        d->background->resizeFrame(size());
        d->hoverAnimation->setProperty("startPixmap", d->background->framePixmap());
   }

   QGraphicsProxyWidget::resizeEvent(event);
}

void PushButton::paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget)
{
    if (!styleSheet().isNull() || Theme::defaultTheme()->useNativeWidgetStyle()) {
        QGraphicsProxyWidget::paint(painter, option, widget);
        return;
    }

    QPixmap bufferPixmap;

    //Normal button, pressed or not
    if (isEnabled()) {
        if (nativeWidget()->isDown() || nativeWidget()->isChecked()) {
            d->background->setElementPrefix("pressed");
        } else {
            d->background->setElementPrefix("normal");
        }

    //flat or disabled
    } else if (!isEnabled() || nativeWidget()->isFlat()) {
        bufferPixmap = QPixmap(rect().size().toSize());
        bufferPixmap.fill(Qt::transparent);

        QPainter buffPainter(&bufferPixmap);
        d->background->paintFrame(&buffPainter);
        buffPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        buffPainter.fillRect(bufferPixmap.rect(), QColor(0, 0, 0, 128));

        painter->drawPixmap(0, 0, bufferPixmap);
    }

    //if is under mouse draw the animated glow overlay
    if (!nativeWidget()->isDown() && !nativeWidget()->isChecked() && isEnabled() && acceptHoverEvents() && d->background->hasElementPrefix("active")) {
        if (d->hoverAnimation->state() == QAbstractAnimation::Running && !isUnderMouse() && !nativeWidget()->isDefault()) {
            d->background->setElementPrefix("active");
            d->background->paintFrame(painter, d->activeRect.topLeft());
        } else {
            painter->drawPixmap(
                d->activeRect.topLeft(),
                d->hoverAnimation->property("currentPixmap").value<QPixmap>());
        }
    } else if (isEnabled()) {
        d->background->paintFrame(painter);
    }


    painter->setPen(Plasma::Theme::defaultTheme()->color(Theme::ButtonTextColor));

    if (nativeWidget()->isDown()) {
        painter->translate(QPoint(1, 1));
    }

    QRectF rect = contentsRect();

    if (!nativeWidget()->icon().isNull()) {
        const qreal iconSize = qMin(rect.width(), rect.height());
        QPixmap iconPix = nativeWidget()->icon().pixmap(iconSize);
        if (!isEnabled()) {
            KIconEffect *effect = KIconLoader::global()->iconEffect();
            iconPix = effect->apply(iconPix, KIconLoader::Toolbar, KIconLoader::DisabledState);
        }

        QRect pixmapRect;
        if (nativeWidget()->text().isEmpty()) {
            pixmapRect = nativeWidget()->style()->alignedRect(option->direction, Qt::AlignCenter, iconPix.size(), rect.toRect());
        } else {
            pixmapRect = nativeWidget()->style()->alignedRect(option->direction, Qt::AlignLeft|Qt::AlignVCenter, iconPix.size(), rect.toRect());
        }
        painter->drawPixmap(pixmapRect.topLeft(), iconPix);

        if (option->direction == Qt::LeftToRight) {
            rect.adjust(rect.height(), 0, 0, 0);
        } else {
            rect.adjust(0, 0, -rect.height(), 0);
        }
    }

    QFont widgetFont;
    if (d->customFont) {
        widgetFont = font();
    } else {
        widgetFont = Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    }

    QFontMetricsF fm(widgetFont);
    // If the height is too small increase the Height of the button to shall the whole text #192988
    if (rect.height() < fm.height()) {
        rect.setHeight(fm.height());
        rect.moveTop(boundingRect().center().y() - rect.height() / 2);
    }

    // If there is not enough room for the text make it to fade out
    if (rect.width() < fm.width(nativeWidget()->text())) {
        if (bufferPixmap.isNull()) {
            bufferPixmap = QPixmap(rect.size().toSize());
        }
        bufferPixmap.fill(Qt::transparent);

        QPainter p(&bufferPixmap);
        p.setPen(painter->pen());
        p.setFont(widgetFont);

        // Create the alpha gradient for the fade out effect
        QLinearGradient alphaGradient(0, 0, 1, 0);
        alphaGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        if (option->direction == Qt::LeftToRight) {
            alphaGradient.setColorAt(0, QColor(0, 0, 0, 255));
            alphaGradient.setColorAt(1, QColor(0, 0, 0, 0));
            p.drawText(bufferPixmap.rect(), Qt::AlignLeft|Qt::AlignVCenter,
                       nativeWidget()->text());
        } else {
            alphaGradient.setColorAt(0, QColor(0, 0, 0, 0));
            alphaGradient.setColorAt(1, QColor(0, 0, 0, 255));
            p.drawText(bufferPixmap.rect(), Qt::AlignRight|Qt::AlignVCenter,
                       nativeWidget()->text());
        }

        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(bufferPixmap.rect(), alphaGradient);

        painter->drawPixmap(rect.topLeft(), bufferPixmap);
    } else {
        painter->setFont(widgetFont);
        painter->drawText(rect, Qt::AlignCenter, nativeWidget()->text());
    }
}

void PushButton::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (nativeWidget()->isDown() || d->background->hasElementPrefix("hover")) {
        return;
    }

    d->hoverAnimation->setProperty("duration", 75);

    d->background->setElementPrefix("normal");
    d->hoverAnimation->setProperty("startPixmap", d->background->framePixmap());

    d->background->setElementPrefix("active");
    d->hoverAnimation->setProperty("targetPixmap", d->background->framePixmap());

    d->hoverAnimation->start();

    QGraphicsProxyWidget::hoverEnterEvent(event);
}

void PushButton::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        d->customFont = true;
    }

    QGraphicsProxyWidget::changeEvent(event);
}

void PushButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (nativeWidget()->isDown() || d->background->hasElementPrefix("hover")) {
        return;
    }

    d->hoverAnimation->setProperty("duration", 150);

    d->background->setElementPrefix("active");
    d->hoverAnimation->setProperty("startPixmap", d->background->framePixmap());

    d->background->setElementPrefix("normal");
    d->hoverAnimation->setProperty("targetPixmap", d->background->framePixmap());

    d->hoverAnimation->start();


    QGraphicsProxyWidget::hoverLeaveEvent(event);
}


QSizeF PushButton::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    QSizeF hint = QGraphicsProxyWidget::sizeHint(which, constraint);

    if (hint.isEmpty()) {
        return hint;
    }

    //replace the native margin with the Svg one
    QStyleOption option;
    option.initFrom(nativeWidget());
    int nativeMargin = nativeWidget()->style()->pixelMetric(QStyle::PM_ButtonMargin, &option, nativeWidget());
    qreal left, top, right, bottom;
    d->background->getMargins(left, top, right, bottom);
    hint = hint - QSize(nativeMargin, nativeMargin) + QSize(left+right, top+bottom);
    return hint;
}

} // namespace Plasma

#include <pushbutton.moc>

