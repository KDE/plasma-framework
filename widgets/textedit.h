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

#ifndef PLASMA_TEXTEDIT_H
#define PLASMA_TEXTEDIT_H

#include <QtGui/QGraphicsProxyWidget>

class KTextEdit;

#include <plasma/plasma_export.h>
#include <plasma/dataengine.h>

namespace Plasma
{

class TextEditPrivate;

/**
 * @class TextEdit plasma/widgets/textedit.h <Plasma/Widgets/TextEdit>
 *
 * @short Provides a plasma-themed KTextEdit.
 */
class PLASMA_EXPORT TextEdit : public QGraphicsProxyWidget
{
    Q_OBJECT

    Q_PROPERTY(QGraphicsWidget *parentWidget READ parentWidget)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE setStyleSheet)
    Q_PROPERTY(KTextEdit *nativeWidget READ nativeWidget WRITE setNativeWidget)
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly)

public:
    explicit TextEdit(QGraphicsWidget *parent = 0);
    ~TextEdit();

    /**
     * Sets the display text for this TextEdit
     *
     * @arg text the text to display; should be translated.
     */
    void setText(const QString &text);

    /**
     * @return the display text
     */
    QString text() const;

    /**
     * Sets the text area to be read only or interactive
     * @arg true to make it read only, false for interactive
     * @since 4.4
     */
    void setReadOnly(bool readOnly);

    /**
     * @return true if the text area is non-interacive
     */
    bool isReadOnly() const;

    /**
     * Sets the stylesheet used to control the visual display of this TextEdit
     *
     * @arg stylesheet a CSS string
     */
    void setStyleSheet(const QString &stylesheet);

    /**
     * @return the stylesheet currently used with this widget
     */
    QString styleSheet();

    /**
     * Sets the text edit wrapped by this TextEdit (widget must inherit KTextEdit), ownership is transferred to the TextEdit
     *
     * @arg text edit that will be wrapped by this TextEdit
     * @since KDE4.4
     */
    void setNativeWidget(KTextEdit *nativeWidget);

    /**
     * @return the native widget wrapped by this TextEdit
     */
    KTextEdit *nativeWidget() const;

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
    void changeEvent(QEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private:
    TextEditPrivate * const d;
    Q_PRIVATE_SLOT(d, void setPalette())
};

} // namespace Plasma

#endif // multiple inclusion guard
