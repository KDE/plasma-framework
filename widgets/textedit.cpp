/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#include "textedit.h"

#include <QPainter>
#include <QScrollBar>

#include <kmimetype.h>
#include <ktextedit.h>

#include "theme.h"
#include "svg.h"
#include "private/style_p.h"

namespace Plasma
{

class TextEditPrivate
{
public:
    TextEditPrivate(TextEdit *textEdit)
        : q(textEdit),
          customFont(false)
    {
    }

    ~TextEditPrivate()
    {
    }

    void setPalette()
    {
        KTextEdit *native = q->nativeWidget();
        QColor color = Theme::defaultTheme()->color(Theme::TextColor);
        QPalette p = native->palette();

        p.setColor(QPalette::Normal, QPalette::Text, color);
        p.setColor(QPalette::Inactive, QPalette::Text, color);
        p.setColor(QPalette::Normal, QPalette::ButtonText, color);
        p.setColor(QPalette::Inactive, QPalette::ButtonText, color);
        p.setColor(QPalette::Normal, QPalette::Base, QColor(0,0,0,0));
        p.setColor(QPalette::Inactive, QPalette::Base, QColor(0,0,0,0));
        native->setPalette(p);

        if (!customFont) {
            q->nativeWidget()->setFont(Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont));
        }
    }

    TextEdit *q;
    Plasma::Style::Ptr style;
    bool customFont;
};

TextEdit::TextEdit(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new TextEditPrivate(this))
{
    d->style = Plasma::Style::sharedStyle();

    setNativeWidget(new KTextEdit);
    connect(Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(setPalette()));
}

TextEdit::~TextEdit()
{
    delete d;
    Plasma::Style::doneWithSharedStyle();
}

void TextEdit::setText(const QString &text)
{
    //FIXME I'm not certain about using only the html methods. look at this again later.
    static_cast<KTextEdit*>(widget())->setHtml(text);
}

QString TextEdit::text() const
{
    return static_cast<KTextEdit*>(widget())->toHtml();
}

void TextEdit::setStyleSheet(const QString &stylesheet)
{
    widget()->setStyleSheet(stylesheet);
}

QString TextEdit::styleSheet()
{
    return widget()->styleSheet();
}

void TextEdit::setNativeWidget(KTextEdit *nativeWidget)
{
    if (widget()) {
        widget()->deleteLater();
    }

    connect(nativeWidget, SIGNAL(textChanged()), this, SIGNAL(textChanged()));

    setWidget(nativeWidget);

    nativeWidget->setAttribute(Qt::WA_NoSystemBackground);
    nativeWidget->setFrameShape(QFrame::NoFrame);
    nativeWidget->setTextBackgroundColor(Qt::transparent);
    nativeWidget->viewport()->setAutoFillBackground(false);
    nativeWidget->verticalScrollBar()->setStyle(d->style.data());
    nativeWidget->horizontalScrollBar()->setStyle(d->style.data());
}

KTextEdit *TextEdit::nativeWidget() const
{
    return static_cast<KTextEdit*>(widget());
}

void TextEdit::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(sourceName)

    KTextEdit *te = nativeWidget();
    te->clear();

    foreach (const QVariant &v, data) {
        if (v.canConvert(QVariant::String)) {
            te->append(v.toString() + '\n');
        }
    }
}

void TextEdit::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    QGraphicsProxyWidget::resizeEvent(event);
}

void TextEdit::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        d->customFont = true;
        nativeWidget()->setFont(font());
    }

    QGraphicsProxyWidget::changeEvent(event);
}

} // namespace Plasma

#include <textedit.moc>

