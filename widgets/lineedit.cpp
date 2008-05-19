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

#include "lineedit.h"

#include <QLineEdit>
#include <QPainter>

#include <KMimeType>

#include "theme.h"
#include "svg.h"

namespace Plasma
{

class LineEdit::Private
{
public:
    Private()
        : svg(0)
    {
    }

    ~Private()
    {
        delete svg;
    }

    void setPixmap(LineEdit *q)
    {
        if (imagePath.isEmpty()) {
            return;
        }

        KMimeType::Ptr mime = KMimeType::findByPath(absImagePath);
        QPixmap pm(q->size().toSize());

        if (mime->is("image/svg+xml")) {
            svg = new Svg();
            QPainter p(&pm);
            svg->paint(&p, pm.rect());
        } else {
            pm = QPixmap(absImagePath);
        }

        //TODO: load image into widget
        //static_cast<QLineEdit*>(widget())->setPixmap();
        //static_cast<QLineEdit*>(widget())->setIcon(QIcon();
    }

    QString imagePath;
    QString absImagePath;
    Svg *svg;
};

LineEdit::LineEdit(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new Private)
{
    QLineEdit* native = new QLineEdit;
    //TODO: forward signals
    setWidget(native);
    native->setAttribute(Qt::WA_NoSystemBackground);
}

LineEdit::~LineEdit()
{
    delete d;
}

void LineEdit::setText(const QString &text)
{
    static_cast<QLineEdit*>(widget())->setText(text);
}

QString LineEdit::text() const
{
    return static_cast<QLineEdit*>(widget())->text();
}

void LineEdit::setImage(const QString &path)
{
    if (d->imagePath == path) {
        return;
    }

    delete d->svg;
    d->svg = 0;
    d->imagePath = path;

    bool absolutePath = !path.isEmpty() && 
                        #ifdef Q_WS_WIN
                            !QDir::isRelativePath(path)
                        #else
                            path[0] == '/'
                        #endif
        ;

    if (absolutePath) {
        d->absImagePath = path;
    } else {
        //TODO: package support
        d->absImagePath = Theme::defaultTheme()->imagePath(path);
    }

    d->setPixmap(this);
}

QString LineEdit::image() const
{
    return d->imagePath;
}

void LineEdit::setStylesheet(const QString &stylesheet)
{
    widget()->setStyleSheet(stylesheet);
}

QString LineEdit::stylesheet()
{
    return widget()->styleSheet();
}

QLineEdit* LineEdit::nativeWidget() const
{
    return static_cast<QLineEdit*>(widget());
}

void LineEdit::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_UNUSED(event)
    d->setPixmap(this);
}

} // namespace Plasma

#include <lineedit.moc>

