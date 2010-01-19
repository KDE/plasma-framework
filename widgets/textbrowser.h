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

#ifndef PLASMA_TEXTBROWSER_H
#define PLASMA_TEXTBROWSER_H

#include <QtGui/QGraphicsProxyWidget>

class KTextBrowser;

#include <plasma/plasma_export.h>
#include <plasma/dataengine.h>

namespace Plasma
{

class TextBrowserPrivate;

/**
 * @class TextBrowser plasma/widgets/TextBrowser.h <Plasma/Widgets/TextBrowser>
 *
 * @short Provides a plasma-themed KTextBrowser.
 *
 * @since 4.3
 */
class PLASMA_EXPORT TextBrowser : public QGraphicsProxyWidget
{
    Q_OBJECT

    Q_PROPERTY(QGraphicsWidget *parentWidget READ parentWidget)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE setStyleSheet)
    Q_PROPERTY(KTextBrowser *nativeWidget READ nativeWidget)

public:
    explicit TextBrowser(QGraphicsWidget *parent = 0);
    ~TextBrowser();

    /**
     * Sets the display text for this TextBrowser
     *
     * @arg text the text to display; should be translated.
     */
    void setText(const QString &text);

    /**
     * @return the display text
     */
    QString text() const;

    /**
     * Sets the policy used to show/hide the horizontal scrollbar
     */
    void setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy);

    /**
     * Sets the policy used to show/hide the vertical scrollbar
     */
    void setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy);

    /**
     * Sets the stylesheet used to control the visual display of this TextBrowser
     *
     * @arg stylesheet a CSS string
     */
    void setStyleSheet(const QString &stylesheet);

    /**
     * @return the stylesheet currently used with this widget
     */
    QString styleSheet();

    /**
     * @return the native widget wrapped by this TextBrowser
     */
    KTextBrowser *nativeWidget() const;

public Q_SLOTS:
    /**
     * Allows appending text to the text browser
     * @since 4.4
     */
    void append(const QString &text);

    void dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data);

Q_SIGNALS:
    void textChanged();

protected:
    void resizeEvent(QGraphicsSceneResizeEvent *event);
    void wheelEvent(QGraphicsSceneWheelEvent *event);
    void changeEvent(QEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private:
    TextBrowserPrivate * const d;

    Q_PRIVATE_SLOT(d, void setFixedHeight())
    Q_PRIVATE_SLOT(d, void setPalette())
};

} // namespace Plasma

#endif // multiple inclusion guard
