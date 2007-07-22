#include "label.h"

#include <QPainter>
#include <QFontMetricsF>

namespace Plasma {

class Label::Private
{
    public:
        Private() {}

        QString text;
};

Label::Label(Widget *parent)
    : Widget(parent),
      d(new Private)
{
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
    return 0;
}

QSizeF Label::sizeHint() const
{
    QFontMetricsF m(QFont("Arial", 12));

    return m.boundingRect(d->text).size();
}

void Label::setText(const QString& text)
{
    d->text = text;
}

QString Label::text() const
{
    return d->text;
}

void Label::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    /* TODO:  Add config parameters, like text color, text alignment, and brush. */
    p->setPen(QPen(Qt::black, 1));
    p->drawText(localGeometry(), Qt::AlignCenter | Qt::TextWordWrap, d->text);
}

}
