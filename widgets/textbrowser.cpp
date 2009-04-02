/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
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

#include "textbrowser.h"

#include <QPainter>
#include <QScrollBar>

#include <kmimetype.h>
#include <ktextbrowser.h>

#include "plasma/theme.h"
#include "plasma/svg.h"
#include "private/style_p.h"

namespace Plasma
{

class TextBrowserPrivate
{
public:
    TextBrowserPrivate(TextBrowser *browser)
        : q(browser),
          native(0),
          savedMinimumHeight(0),
          savedMaximumHeight(QWIDGETSIZE_MAX),
          wasNotFixed(true)
    {
    }

    void setFixedHeight()
    {
        if (native && native->document() &&
            q->sizePolicy().verticalPolicy() == QSizePolicy::Fixed &&
            native->verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOff) {
            native->document()->setTextWidth(q->size().width());
            QSize s = native->document()->size().toSize();
            if (wasNotFixed) {
                savedMinimumHeight = q->minimumHeight();
                savedMaximumHeight = q->maximumHeight();
                wasNotFixed = false;
            }
            q->setMinimumHeight(s.height());
            q->setMaximumHeight(s.height());
        } else if (!wasNotFixed) {
            q->setMinimumHeight(savedMinimumHeight);
            q->setMaximumHeight(savedMaximumHeight);
            wasNotFixed = true;
        }
    }


    TextBrowser *q;
    KTextBrowser *native;
    Plasma::Style::Ptr style;
    int savedMinimumHeight;
    int savedMaximumHeight;
    bool wasNotFixed;
};

TextBrowser::TextBrowser(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new TextBrowserPrivate(this))
{
    KTextBrowser *native = new KTextBrowser;
    connect(native, SIGNAL(textChanged()), this, SIGNAL(textChanged()));
    connect(native, SIGNAL(textChanged()), this, SLOT(setFixedHeight()));
    setWidget(native);
    d->native = native;
    native->setAttribute(Qt::WA_NoSystemBackground);
    native->setFrameShape(QFrame::NoFrame);
    native->setTextBackgroundColor(Qt::transparent);
    native->viewport()->setAutoFillBackground(false);
    d->style = Plasma::Style::sharedStyle();
    native->verticalScrollBar()->setStyle(d->style.data());
    native->horizontalScrollBar()->setStyle(d->style.data());
}

TextBrowser::~TextBrowser()
{
    delete d;
    Plasma::Style::doneWithSharedStyle();
}

void TextBrowser::setText(const QString &text)
{
    //FIXME I'm not certain about using only the html methods. look at this again later.
    static_cast<KTextBrowser*>(widget())->setHtml(text);
}

QString TextBrowser::text() const
{
    return static_cast<KTextBrowser*>(widget())->toHtml();
}

void TextBrowser::setStyleSheet(const QString &stylesheet)
{
    widget()->setStyleSheet(stylesheet);
}

QString TextBrowser::styleSheet()
{
    return widget()->styleSheet();
}

KTextBrowser *TextBrowser::nativeWidget() const
{
    return static_cast<KTextBrowser*>(widget());
}

void TextBrowser::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(sourceName)

    KTextBrowser *te = nativeWidget();
    te->clear();

    foreach (const QVariant &v, data) {
        if (v.canConvert(QVariant::String)) {
            te->append(v.toString() + '\n');
        }
    }
}

void TextBrowser::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    d->setFixedHeight();
    QGraphicsProxyWidget::resizeEvent(event);
}

} // namespace Plasma

#include <textbrowser.moc>

