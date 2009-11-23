/*
 *   Copyright 2007 by André Duffeck <duffeck@kde.org>
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

#ifndef PLASMA_FLASHINGLABEL_H
#define PLASMA_FLASHINGLABEL_H

#include <QtGui/QGraphicsWidget>
#include <QtGui/QTextOption>

#include <plasma/plasma_export.h>

namespace Plasma
{

class FlashingLabelPrivate;

/**
 * @class FlashingLabel plasma/widgets/flashinglabel.h <Plasma/Widgets/FlashingLabel>
 *
 * @short Provides flashing text or icons inside Plasma
 */
class PLASMA_EXPORT FlashingLabel : public QGraphicsWidget
{
    Q_OBJECT
    Q_PROPERTY(bool autohide READ autohide WRITE setAutohide)
    Q_PROPERTY(QFont font READ font WRITE setFont)
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(int duration READ duration WRITE setDuration)

public:
    explicit FlashingLabel(QGraphicsItem *parent = 0);
    virtual ~FlashingLabel();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    QFont font() const;
    void setFont(const QFont &);

    QColor color() const;
    void setColor(const QColor &);

    int duration() const;
    void setDuration(int duration);

    Q_INVOKABLE void flash(const QString &text, int duration = 0,
                           const QTextOption &option = QTextOption(Qt::AlignCenter));
    Q_INVOKABLE void flash(const QPixmap &pixmap, int duration = 0,
                           Qt::Alignment align = Qt::AlignCenter);

    void setAutohide(bool autohide);
    bool autohide() const;

protected:
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const;

public Q_SLOTS:
    void kill();

protected Q_SLOTS:
    void fadeIn();
    void fadeOut();

private:
    Q_PRIVATE_SLOT(d, void elementAnimationFinished(int))
    Q_PRIVATE_SLOT(d, void setPalette())
    FlashingLabelPrivate *const d;
};

}

#endif
