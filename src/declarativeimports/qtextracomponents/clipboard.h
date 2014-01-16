/*
 *    Copyright 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Library General Public
 *    License as published by the Free Software Foundation; either
 *    version 2 of the License, or (at your option) any later version.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Library General Public License for more details.
 *
 *    You should have received a copy of the GNU Library General Public License
 *    along with this library; see the file COPYING.LIB.  If not, write to
 *    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA 02110-1301, USA.
 */

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <QClipboard>
#include <QVariant>
#include <QMimeData>

class ClipboardPrivate;

class Clipboard : public QObject
{
    Q_OBJECT
    /**
     * Controls the state this object will be monitoring and extracting its contents from.
     */
    Q_PROPERTY(QClipboard::Mode mode READ mode WRITE setMode NOTIFY modeChanged);

    /**
     * Provides the contents currently in the clipboard and lets modify them.
     */
    Q_PROPERTY(QVariant content READ content WRITE setContent NOTIFY contentChanged);

    /**
     * Figure out the nature of the contents in the clipboard.
     */
    Q_PROPERTY(QStringList formats READ formats NOTIFY contentChanged);

    public:
        explicit Clipboard(QObject* parent = 0);

        QClipboard::Mode mode() const;
        void setMode(QClipboard::Mode mode);

        QVariant content() const;
        void setContent(const QVariant &content);

        QStringList formats() const;

        Q_SCRIPTABLE void clear();

    Q_SIGNALS:
        void modeChanged(QClipboard::Mode mode);
        void contentChanged();

    private Q_SLOTS:
        void clipboardChanged(QClipboard::Mode m);

    private:
        QClipboard* m_clipboard;
        QClipboard::Mode m_mode;
};

#endif
