/*
 *   Copyright 2007 by Alexander Wiedenbruch <mail@wiedenbruch.de>
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

#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QtCore/QObject>
#include <QtGui/QGraphicsTextItem>
#include <QtGui/QGraphicsLayoutItem>

#include <plasma/plasma_export.h>

#include <plasma/dataengine.h>
#include <plasma/plasma.h>

namespace Plasma
{

/**
 * Class that emulates a QLineEdit inside plasma
 */
class PLASMA_EXPORT LineEdit : public QGraphicsTextItem, public QGraphicsLayoutItem
{
    Q_OBJECT
    Q_PROPERTY( bool styled READ styled WRITE setStyled )
    Q_PROPERTY( bool multiLine READ multiLine WRITE setMultiLine )
    Q_PROPERTY( QString defaultText WRITE setDefaultText )
    Q_PROPERTY( QString html READ toHtml )
    Q_PROPERTY( QString plainText READ toPlainText )

    public:
        explicit LineEdit(QGraphicsItem *parent = 0);
        ~LineEdit();

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

        QSizeF minimumSize() const;
        QSizeF maximumSize() const;

        void setStyled(bool style);
        bool styled() const;

        void setMultiLine(bool multi);
        bool multiLine() const;

        bool hasHeightForWidth() const;
        qreal heightForWidth(qreal w) const;

        bool hasWidthForHeight() const;
        qreal widthForHeight(qreal h) const;

        QRectF geometry() const;
        void setGeometry(const QRectF& geometry);
        void updateGeometry();
        QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

        void setDefaultText(const QString &text);
        const QString toHtml();
        const QString toPlainText();

        /**
         * Reimplented from QGraphicsItem
         **/
        enum { Type = Plasma::LineEditType };
        int type() const { return Type; }

    Q_SIGNALS:
        void editingFinished();
        void textChanged(const QString &text);

    public Q_SLOTS:
        void dataUpdated(const QString&, const Plasma::DataEngine::Data&);

    private:
        class Private;
        Private* const d;

    protected:
        void keyPressEvent(QKeyEvent *event);
        void focusInEvent(QFocusEvent *event);
        void focusOutEvent(QFocusEvent *event);
        QPointF position() const;
};

} // namespace Plasma

#endif
