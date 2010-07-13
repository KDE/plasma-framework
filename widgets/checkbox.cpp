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

#include "checkbox.h"

#include <QCheckBox>
#include <QPainter>
#include <QDir>

#include <kmimetype.h>

#include "theme.h"
#include "svg.h"

namespace Plasma
{

class CheckBoxPrivate
{
public:
    CheckBoxPrivate(CheckBox *c)
        : q(c),
          svg(0),
          customFont(false)
    {
    }

    ~CheckBoxPrivate()
    {
        delete svg;
    }

    void setPixmap()
    {
        if (imagePath.isEmpty()) {
            delete svg;
            svg = 0;
            return;
        }

        KMimeType::Ptr mime = KMimeType::findByPath(absImagePath);
        QPixmap pm(q->size().toSize());

        if (mime->is("image/svg+xml") || mime->is("image/svg+xml-compressed")) {
            if (!svg || svg->imagePath() != imagePath) {
                delete svg;
                svg = new Svg();
                svg->setImagePath(imagePath);
                QObject::connect(svg, SIGNAL(repaintNeeded()), q, SLOT(setPixmap()));
            }

            QPainter p(&pm);
            svg->paint(&p, pm.rect());
        } else {
            delete svg;
            svg = 0;
            pm = QPixmap(absImagePath);
        }

        static_cast<QCheckBox*>(q->widget())->setIcon(QIcon(pm));
    }

    void setPalette()
    {
        QCheckBox *native = q->nativeWidget();
        QColor color = Theme::defaultTheme()->color(Theme::TextColor);
        QPalette p = native->palette();
        p.setColor(QPalette::Normal, QPalette::WindowText, color);
        p.setColor(QPalette::Inactive, QPalette::WindowText, color);
        native->setPalette(p);
        if (!customFont) {
            q->nativeWidget()->setFont(Plasma::Theme::defaultTheme()->font(Plasma::Theme::DefaultFont));
        }
    }

    CheckBox *q;
    QString imagePath;
    QString absImagePath;
    Svg *svg;
    bool customFont;
};

CheckBox::CheckBox(QGraphicsWidget *parent)
    : QGraphicsProxyWidget(parent),
      d(new CheckBoxPrivate(this))
{
    QCheckBox *native = new QCheckBox;
    connect(native, SIGNAL(toggled(bool)), this, SIGNAL(toggled(bool)));
    setWidget(native);
    native->setWindowIcon(QIcon());
    d->setPalette();
    native->setAttribute(Qt::WA_NoSystemBackground);
    connect(Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(setPalette()));
}

CheckBox::~CheckBox()
{
    delete d;
}

void CheckBox::setText(const QString &text)
{
    static_cast<QCheckBox*>(widget())->setText(text);
}

QString CheckBox::text() const
{
    return static_cast<QCheckBox*>(widget())->text();
}

void CheckBox::setImage(const QString &path)
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
                            (path[0] == '/' || path.startsWith(QLatin1String(":/")))
                        #endif
        ;

    if (absolutePath) {
        d->absImagePath = path;
    } else {
        //TODO: package support
        d->absImagePath = Theme::defaultTheme()->imagePath(path);
    }

    d->setPixmap();
}

QString CheckBox::image() const
{
    return d->imagePath;
}

void CheckBox::setStyleSheet(const QString &stylesheet)
{
    widget()->setStyleSheet(stylesheet);
}

QString CheckBox::styleSheet()
{
    return widget()->styleSheet();
}

QCheckBox *CheckBox::nativeWidget() const
{
    return static_cast<QCheckBox*>(widget());
}

void CheckBox::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    d->setPixmap();
    QGraphicsProxyWidget::resizeEvent(event);
}

void CheckBox::setChecked(bool checked)
{
    static_cast<QCheckBox*>(widget())->setChecked(checked);
}

bool CheckBox::isChecked() const
{
    return static_cast<QCheckBox*>(widget())->isChecked();
}

void CheckBox::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        d->customFont = true;
        nativeWidget()->setFont(font());
    }

    QGraphicsProxyWidget::changeEvent(event);
}

} // namespace Plasma

#include <checkbox.moc>

