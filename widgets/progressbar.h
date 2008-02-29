/*
 *   Copyright (C) 2007  Jon de Andres Frias <jondeandres@gmail.com>
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

#ifndef __PLASMA_PROGRESSBAR__
#define __PLASMA_PROGRESSBAR__

#include <plasma/widgets/widget.h>


namespace Plasma
{

class PLASMA_EXPORT ProgressBar : public Plasma::Widget
{
    Q_OBJECT
    Q_PROPERTY(QString format READ format WRITE setFormat)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(bool invertedAppearance READ invertedAppearance WRITE setInvertedAppearance)
    Q_PROPERTY(bool textVisible READ isTextVisible WRITE setTextVisible)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(int value READ value WRITE setValue)

public:

    /**
     * Constructor
     */
    ProgressBar(Widget *parent);

    /**
     * Virtual Destructor
     */
    virtual ~ProgressBar();

    /**
     * @return alignment of the progressbar
     */
    Qt::Alignment alignment() const;

    /**
     * @return the format for the shown text
     */
    QString format() const;

    /**
     * @return wether the Plasma::ProgressBar is inverted or not.
     */
    bool invertedAppearance();

    /**
     * @return wether the text is visible or not.
     */
    bool isTextVisible() const;

    /**
     * @return the maximum value in the range of Plasma::ProgressBar
     */
    int maximum() const;

    /**
     * @return the minimum value in the range of Plasma:ProgressBar
     */
    int minimum() const;

    /**
     * Changes the alignment of Plasma::ProgressBar
     *
     * %p - is replaced by the percentage completed
     * %v - is replaced by the current value
     * %m - is replaced by the total number of steps
     * @param alignment the alignment for the progressbar.
     */
    void setAlignment(Qt::Alignment alignment);

    /**
     * Sets the format for the shown text in Plasma::ProgressBar;
     * @format the format for the shown text in Plasma:ProgressBar
     */
    void setFormat(const QString &format);

    /**
     * Inverts the appearance of Plasma::ProgressBar
     * @param invert true inverts the appearance
     */
    void setInvertedAppearance(bool invert);

    /**
     * Change the text to be visible or not.
     * @param visible if true then visible
     */
    void setTextVisible(bool visible);

    /**
     * @return the text to be shown in Plasma::ProgressBar
     */
    QString text() const;

    /**
     * @return the actual value of the progress
     */
    int value() const;

    void paintWidget(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

public Q_SLOTS:
    void reset();
    void setRange(int minimum, int maximum);
    void setMinimum(int minimum);
    void setMaximum(int maximum);
    void setValue(int value);

Q_SIGNALS:
    void valueChanged(int value);

private:
    void init();

    class Private;
    Private * const d;
};

}

#endif /*__PLASMA_PROGRESSBAR__*/

