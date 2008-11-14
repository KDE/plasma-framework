/*
 *   Copyright 2007 by Dan Meltzer <hydrogen@notyetimplemented.com>
 *   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>
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

#include "tooltip_p.h"
#include "windowpreview_p.h"

#include <QBitmap>
#include <QGridLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QTextDocument>
#include <QTimeLine>
#ifdef Q_WS_X11
#include <QX11Info>
#include <netwm.h>
#endif

#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>

#include <plasma/plasma.h>
#include <plasma/theme.h>
#include <plasma/framesvg.h>

namespace Plasma {

class TipTextWidget : public QWidget
{
public:
    TipTextWidget(QWidget *parent)
        : QWidget(parent),
          document(new QTextDocument(this))
    {
        //d->text->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
//        QTextOption op;
//        op.setWrapMode(QTextOption::WordWrap);
//        document->setDefaultTextOption(op);
    }

    void setStyleSheet(const QString &css)
    {
        document->setDefaultStyleSheet(css);
    }

    void setContent(const ToolTipContent &data)
    {
        document->clear();
        data.registerResources(document);
        document->setHtml("<p><b>" + data.mainText() + "</b><br>" + data.subText() + "</p>");
        document->adjustSize();
        update();
    }

    QSize minimumSizeHint() const
    {
        return document->size().toSize();
    }

    void paintEvent(QPaintEvent *event)
    {
        QPainter p(this);
        document->drawContents(&p, event->rect());
    }

private:
    QTextDocument *document;
};

class ToolTipPrivate
{
    public:
        ToolTipPrivate()
        : text(0),
          imageLabel(0),
          preview(0),
          source(0),
          timeline(0),
          autohide(true)
    { }

    TipTextWidget *text;
    QLabel *imageLabel;
    WindowPreview *preview;
    FrameSvg *background;
    QPointer<QObject> source;
    QTimeLine *timeline;
    QPoint to;
    QPoint from;
    bool autohide;
};

void ToolTip::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    d->preview->setInfo();
}

void ToolTip::hideEvent(QHideEvent *e)
{
    QWidget::hideEvent(e);
    if (d->source) {
        QMetaObject::invokeMethod(d->source, "toolTipHidden");
    }
}

void ToolTip::mouseReleaseEvent(QMouseEvent *event)
{
    if (rect().contains(event->pos())) {
        hide();
    }
}

ToolTip::ToolTip(QWidget *parent)
    : QWidget(parent),
      d(new ToolTipPrivate())
{
    setWindowFlags(Qt::ToolTip);
    QGridLayout *l = new QGridLayout;
    d->preview = new WindowPreview(this);
    d->text = new TipTextWidget(this);
    d->imageLabel = new QLabel(this);
    d->imageLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    d->background = new FrameSvg(this);
    d->background->setImagePath("widgets/tooltip");
    d->background->setEnabledBorders(FrameSvg::AllBorders);
    updateTheme();
    connect(d->background, SIGNAL(repaintNeeded()), this, SLOT(updateTheme()));

    l->addWidget(d->preview, 0, 0, 1, 2);
    l->addWidget(d->imageLabel, 1, 0);
    l->addWidget(d->text, 1, 1);
    setLayout(l);
}

ToolTip::~ToolTip()
{
    delete d;
}

void ToolTip::checkSize()
{
    QSize hint = sizeHint();
    QSize current = size();

    if (hint != current) {
        /*
#ifdef Q_WS_X11
        NETRootInfo i(QX11Info::display(), 0);
        int flags = NET::BottomLeft;
        i.moveResizeWindowRequest(winId(), flags,
                                  x(), y() + (current.height() - hint.height()),
                                  hint.width(), hint.height());
#else
        move(x(), y() + (current.height() - hint.height()));
        resize(hint);
#endif
    */
        /*
        kDebug() << "resizing from" << current << "to" << hint
                 << "and moving from" << pos() << "to"
                 << x() << y() + (current.height() - hint.height())
                 << current.height() - hint.height();
                 */
        resize(hint);
        move(x(), y() + (current.height() - hint.height()));
    }
}

void ToolTip::setContent(QObject *tipper, const ToolTipContent &data)
{
    //reset our size
    d->text->setContent(data);
    d->imageLabel->setPixmap(data.image());
    d->preview->setWindowId(data.windowToPreview());
    d->autohide = data.autohide();
    d->source = tipper;

    if (isVisible()) {
        d->preview->setInfo();
        //kDebug() << "about to check size";
        checkSize();
    }
}

void ToolTip::prepareShowing(bool cueUpdate)
{
    if (cueUpdate && d->source) {
        QMetaObject::invokeMethod(d->source, "toolTipAboutToShow");
    }

    if (d->preview->windowId() != 0) {
        // show/hide the preview area
        d->preview->show();
    } else {
        d->preview->hide();
    }

    layout()->activate();
    d->preview->setInfo();
    //kDebug() << "about to check size";
    checkSize();
}

void ToolTip::moveTo(const QPoint &to)
{
    if (!isVisible() ||
        !(KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects)) {
        move(to);
        return;
    }

    d->from = QPoint();
    d->to = to;

    if (!d->timeline) {
        d->timeline = new QTimeLine(250, this);
        d->timeline->setFrameRange(0, 10);
        d->timeline->setCurveShape(QTimeLine::EaseInCurve);
        connect(d->timeline, SIGNAL(valueChanged(qreal)), this, SLOT(animateMove(qreal)));
    }

    d->timeline->stop();
    d->timeline->start();
}

void ToolTip::animateMove(qreal progress)
{
    if (d->from.isNull()) {
        d->from = pos();
    }

    if (qFuzzyCompare(progress, 1.0)) {
        move(d->to);
        return;
    }

    move(d->from.x() + ((d->to.x() - d->from.x()) * progress),
         d->from.y() + ((d->to.y() - d->from.y()) * progress));
}

void ToolTip::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    d->background->resizeFrame(size());
    setMask(d->background->mask());
}

void ToolTip::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipRect(e->rect());
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(rect(), Qt::transparent);

    d->background->paintFrame(&painter);
}

bool ToolTip::autohide() const
{
    return d->autohide;
}

void ToolTip::updateTheme()
{
    const int topHeight = d->background->marginSize(Plasma::TopMargin);
    const int leftWidth = d->background->marginSize(Plasma::LeftMargin);
    const int rightWidth = d->background->marginSize(Plasma::RightMargin);
    const int bottomHeight = d->background->marginSize(Plasma::BottomMargin);
    setContentsMargins(leftWidth, topHeight, rightWidth, bottomHeight);

    // Make the tooltip use Plasma's colorscheme
    QColor textColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    QPalette plasmaPalette = QPalette();
    plasmaPalette.setColor(QPalette::Window,
                           Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
    plasmaPalette.setColor(QPalette::WindowText, textColor);
    setAutoFillBackground(true);
    setPalette(plasmaPalette);
    d->text->setStyleSheet(QString("p { color: %1; }").arg(textColor.name()));
    update();
}

} // namespace Plasma

#include "tooltip_p.moc"
