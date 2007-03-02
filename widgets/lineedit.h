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

#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QTextLayout>
#include <QTimer>
#include <QGraphicsItem>

#include <applet.h>
#include "widget.h"

namespace Plasma
{

class KDE_EXPORT LineEdit : public Widget
{
  Q_OBJECT

  public:
    LineEdit(Plasma::Applet *a, QPointF pos, QSizeF size);
    ~LineEdit();

    void paint(QPainter *painter,
        const QStyleOptionGraphicsItem *option, QWidget *widget);

    void setValue(QString text);
    QString getStringValue() const;

    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

  public Q_SLOTS:
    void data(const DataSource::Data&);

  protected:
    void focusOutEvent(QFocusEvent *event);

  private Q_SLOTS:
    void blinkCursor();

  private:
    class Private;
    Private *const d;
};

} // Plasma namespace

#endif
