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

#include "clipboard.h"
#include <QGuiApplication>
#include <QMimeData>
#include <QUrl>
#include <QDebug>

Clipboard::Clipboard(QObject* parent)
    : QObject(parent)
    , m_clipboard(QGuiApplication::clipboard())
    , m_mode(QClipboard::Clipboard)
{
    connect(m_clipboard, SIGNAL(changed(QClipboard::Mode)), SLOT(clipboardChanged(QClipboard::Mode)));
}

void Clipboard::setMode(QClipboard::Mode mode)
{
    m_mode = mode;
    emit modeChanged(m_mode);
}

void Clipboard::clipboardChanged(QClipboard::Mode m)
{
    if (m == m_mode) {
        emit contentChanged();
    }
}

void Clipboard::clear()
{
    m_clipboard->clear(m_mode);
}

QClipboard::Mode Clipboard::mode() const
{
    return m_mode;
}

QVariant Clipboard::content() const
{
    const QMimeData* data = m_clipboard->mimeData(m_mode);

    return data->data(data->formats().first());
}

void Clipboard::setContent(const QVariant &content)
{
    QMimeData* mimeData = new QMimeData;
    switch(content.type())
    {
        case QVariant::String:
            mimeData->setText(content.toString());
            break;
        case QVariant::Color:
            mimeData->setColorData(content.toString());
            break;
        case QVariant::Pixmap:
        case QVariant::Image:
            mimeData->setImageData(content);
            break;
        default:
            if (content.type() == QVariant::List) {
                QVariantList list = content.toList();
                QList<QUrl> urls;
                bool wasUrlList = true;
                foreach (const QVariant& url, list) {
                    if (url.type() != QVariant::Url) {
                        wasUrlList = true;
                        break;
                    }
                    urls += url.toUrl();
                }
                if(wasUrlList) {
                    mimeData->setUrls(urls);
                    break;
                }
            }

            if (content.canConvert(QVariant::String)) {
                mimeData->setText(content.toString());
            } else {
                delete mimeData;
                mimeData = nullptr;
                qWarning() << "Couldn't manage to set the content to the clipboard";
            }
            break;
    }
    qDebug() << "seeeeeeeeet" << mimeData->formats();
    m_clipboard->setMimeData(mimeData, m_mode);
}

QStringList Clipboard::formats() const
{
    return m_clipboard->mimeData(m_mode)->formats();
}
