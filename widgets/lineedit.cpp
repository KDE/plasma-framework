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

#include "lineedit.h"

#include <QPainter>

#include <klineedit.h>
#include <kmimetype.h>

#include <plasma/private/style_p.h>

#include "theme.h"
#include "svg.h"

namespace Plasma
{

class LineEditPrivate
{
public:
    LineEditPrivate(LineEdit *lineEdit)
        :q(lineEdit)
    {
    }

    ~LineEditPrivate()
    {
    }

    void setPalette()
    {
        KLineEdit *native = q->nativeWidget();
        QColor color = Theme::defaultTheme()->color(Theme::TextColor);
        QPalette p = native->palette();

        p.setColor(QPalette::Normal, QPalette::Text, color);
        p.setColor(QPalette::Inactive, QPalette::Text, color);
        native->setPalette(p);
        native->setFont(Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont));
    }

    LineEdit *q;
    Plasma::Style::Ptr style;
};

LineEdit::LineEdit(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new LineEditPrivate(this))
{
    KLineEdit *native = new KLineEdit;
    connect(native, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
    connect(native, SIGNAL(returnPressed()), this, SIGNAL(returnPressed()));
    connect(native, SIGNAL(textEdited(const QString&)), this, SIGNAL(textEdited(const QString&)));
    setWidget(native);
    native->setAttribute(Qt::WA_NoSystemBackground);
    d->style = Plasma::Style::sharedStyle();
    native->setStyle(d->style.data());

    d->setPalette();
    connect(Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(setPalette()));
}

LineEdit::~LineEdit()
{
    delete d;
    Plasma::Style::doneWithSharedStyle();
}

void LineEdit::setText(const QString &text)
{
    static_cast<KLineEdit*>(widget())->setText(text);
}

QString LineEdit::text() const
{
    return static_cast<KLineEdit*>(widget())->text();
}

void LineEdit::setStyleSheet(const QString &stylesheet)
{
    widget()->setStyleSheet(stylesheet);
}

QString LineEdit::styleSheet()
{
    return widget()->styleSheet();
}

KLineEdit *LineEdit::nativeWidget() const
{
    return static_cast<KLineEdit*>(widget());
}

} // namespace Plasma

#include <lineedit.moc>

