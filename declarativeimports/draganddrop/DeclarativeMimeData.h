/*
	Copyright (C) 2010 by BetterInbox <contact@betterinbox.com>
	Original author: Gregory Schlomoff <greg@betterinbox.com>

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/

#ifndef DECLARATIVEMIMEDATA_H
#define DECLARATIVEMIMEDATA_H

#include <QMimeData>
#include <QColor>
#include <QUrl>
#include <QDeclarativeItem>

class DeclarativeMimeData : public QMimeData
{
	Q_OBJECT

	Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
	Q_PROPERTY(QString html READ html WRITE setHtml NOTIFY htmlChanged)
	Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged) //TODO: use QDeclarativeListProperty<QUrls> to return the whole list instead of only the first url
	Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
	Q_PROPERTY(QDeclarativeItem* source READ source WRITE setSource NOTIFY sourceChanged)
	//TODO: Image property

public:
	DeclarativeMimeData() : QMimeData() {}
	DeclarativeMimeData(const QMimeData* copy);

	QUrl url() const;
	void setUrl(const QUrl &url);

	QColor color() const;
	void setColor(const QColor &color);

	QDeclarativeItem* source() const;
	void setSource(QDeclarativeItem* source);

	/*
	QString text() const;				//TODO: Reimplement this to issue the onChanged signals
	void setText(const QString &text);
	QString html() const;
	void setHtml(const QString &html);
	*/

signals:
	void textChanged();		//FIXME not being used
	void htmlChanged();		//FIXME not being used
	void urlChanged();
	void colorChanged();
	void sourceChanged();

private:
	QDeclarativeItem* m_source;

};

#endif // DECLARATIVEMIMEDATA_H
