#include "label.h"

#include <QPainter>
#include <QFontMetricsF>
#include <QStyleOptionGraphicsItem>

namespace Plasma {

class Label::Private
{
    public:
        Private() {}

        QString text;
        Qt::Alignment alignment;
        QPen textPen;
};

Label::Label(Widget *parent)
    : Plasma::Widget(parent),
      d(new Private)
{
    setAlignment(Qt::AlignHCenter);
    setPen(QPen(Qt::black, 1));
}

Label::~Label()
{
}

Qt::Orientations Label::expandingDirections() const
{
    return Qt::Horizontal | Qt::Vertical;
}

bool Label::hasHeightForWidth() const
{
    return true;
}

qreal Label::heightForWidth(qreal w) const
{
    Q_UNUSED(w);
    //FIXME: this looks a bit odd?
    return 0;
}

QSizeF Label::sizeHint() const
{
    QFontMetricsF m(QFont("Arial", 12));

    return m.boundingRect(QRectF(0,0,9999,9999), d->alignment | Qt::TextWordWrap, d->text).size();
}

void Label::setText(const QString& text)
{
    d->text = text;
}

QString Label::text() const
{
    return d->text;
}

void Label::setAlignment(Qt::Alignment align)
{
    d->alignment = align;
}

Qt::Alignment Label::alignment() const
{
    return d->alignment;
}

void Label::setPen(const QPen& pen)
{
    d->textPen = pen;
}

QPen Label::pen() const
{
    return d->textPen;
}

void Label::paintWidget(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    p->setPen(d->textPen);
    p->drawText(option->rect, d->alignment | Qt::TextWordWrap, d->text);
}

}
