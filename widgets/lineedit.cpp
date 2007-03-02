/****************************************************************************
 * Copyright (c) 2005 Alexander Wiedenbruch <mail@wiedenbruch.de>
 * Copyright (c) 2007 Matt Broadstone <mbroadst@gmail.com>
 *
 * This file is part of SuperKaramba.
 *
 *  SuperKaramba is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  SuperKaramba is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SuperKaramba; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 ****************************************************************************/

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneMouseEvent>

#include <kdebug.h>

#include "lineedit.h"

namespace Plasma
{

class LineEdit::Private
{
    public:
      Private()
      {
      }

      ~Private()
      {
      }

      QFont m_font;

      QColor m_bgColor;
      QColor m_fgColor;
      QColor m_fontColor;
      QColor m_selectedTextColor;
      QColor m_selectionColor;

      QString m_text;

      QTextLayout m_textLayout;

      double m_hscroll;
      int m_cursorPos;
      bool m_cursorVisible;

      QTimer m_cursorTimer;
};

LineEdit::LineEdit(Plasma::Applet* a, QPointF pos, QSizeF size)
    : Widget(a, pos, size),
    d(new Private)
{
    d->m_bgColor = QColor(200, 200, 200);
    d->m_fgColor = QColor(60, 60, 60);

    setFlags(QGraphicsItem::ItemIsFocusable);
    setAcceptedMouseButtons(Qt::LeftButton);

    connect(&d->m_cursorTimer, SIGNAL(timeout()), (QObject*)this, SLOT(blinkCursor()));
    d->m_cursorTimer.start(1000);
}

LineEdit::~LineEdit()
{
  delete d;
}

void LineEdit::data(const DataSource::Data&)
{
}

/*
  Some code in this method is copied from QLineEdit.
  Copyright (C) 1992-2006 Trolltech ASA. All rights reserved.
*/
void LineEdit::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                  QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setPen(d->m_fgColor);
    QBrush oldBrush = painter->brush();
    painter->setBrush(d->m_bgColor);
    painter->drawRect(boundingRect());
    painter->setBrush(oldBrush);

    if(d->m_textLayout.lineCount() == 0)
      return;

    QTextLine line = d->m_textLayout.lineAt(0);
    if(!line.isValid())
      return;

    int widthUsed = qRound(line.naturalTextWidth()) + 1 + 5;

    QRectF innerRect(boundingRect().x() + 5, boundingRect().y() + 2,
                boundingRect().width() - 10, boundingRect().height() - 4);
    painter->setClipRect(innerRect);

    QPointF topLeft = innerRect.topLeft();

    double curPos = line.cursorToX(d->m_cursorPos);
    if (5 + widthUsed <= innerRect.width())
        d->m_hscroll = 0;
    else if (curPos - d->m_hscroll >= innerRect.width())
        d->m_hscroll = curPos - innerRect.width() + 1;
    else if (curPos - d->m_hscroll < 0)
        d->m_hscroll = curPos;
    else if (widthUsed - d->m_hscroll < innerRect.width())
        d->m_hscroll = widthUsed - innerRect.width();
    topLeft.rx() -= d->m_hscroll;

    painter->setPen(d->m_fontColor);
    d->m_textLayout.draw(painter, topLeft, QVector<QTextLayout::FormatRange>(),
                innerRect);

    if(hasFocus() && d->m_cursorVisible)
        d->m_textLayout.drawCursor(painter, topLeft, d->m_cursorPos);
}

void LineEdit::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QTextLine line = d->m_textLayout.lineAt(0);

    QPoint pos = event->pos().toPoint();
    d->m_cursorPos = line.xToCursor(pos.x() - 5 + d->m_hscroll);
    d->m_cursorVisible = true;

    update();
}

void LineEdit::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);

    d->m_cursorTimer.stop();
    update();
}

void LineEdit::blinkCursor()
{
    d->m_cursorVisible = !d->m_cursorVisible;
    update();
}

void LineEdit::keyPressEvent(QKeyEvent *event)
{
    bool append = true;

    switch(event->key())
    {
    case Qt::Key_Backspace:
            if(d->m_cursorPos > 0)
            {
                d->m_text.remove(d->m_cursorPos-1, 1);
                d->m_cursorPos--;
            }
            append = false;
            break;

    case Qt::Key_Delete:
            d->m_text.remove(d->m_cursorPos, 1);
            append = false;
            break;

    case Qt::Key_Left:
            d->m_cursorPos--;
            append = false;
            break;

    case Qt::Key_Right:
            d->m_cursorPos++;
            append = false;
            break;

    case Qt::Key_Home:
            d->m_cursorPos = 0;
            append = false;
            break;

    case Qt::Key_End:
            d->m_cursorPos = d->m_text.length();
            append = false;
            break;

    case Qt::Key_Enter:
    case Qt::Key_Return:
            append = false;
            break;
    }

    if(append)
    {
        d->m_text.insert(d->m_cursorPos, event->text());
        d->m_cursorPos += event->text().length();
    }

    if(d->m_cursorPos < 0)
        d->m_cursorPos = 0;
    if(d->m_cursorPos > d->m_text.length())
        d->m_cursorPos = d->m_text.length();

    d->m_textLayout.setText(d->m_text);
    d->m_textLayout.setFont(d->m_font);

    d->m_textLayout.beginLayout();
    QTextLine line = d->m_textLayout.createLine();
    line.setLineWidth(m_boundingBox.width());
    line.setPosition(QPointF(0, 0));
    d->m_textLayout.endLayout(); 

    d->m_cursorVisible = true;
    update();
}


void LineEdit::setValue(QString text)
{
    d->m_text = text;
    d->m_textLayout.setText(d->m_text);
    d->m_textLayout.setFont(d->m_font);

    d->m_textLayout.beginLayout();
    QTextLine line = d->m_textLayout.createLine();
    line.setLineWidth(m_boundingBox.width());
    line.setPosition(QPointF(0, 0));
    d->m_textLayout.endLayout();

    update();
}

QString LineEdit::getStringValue() const
{
    return d->m_text;
}

#include "lineedit.moc"

} // Plasma namespace

