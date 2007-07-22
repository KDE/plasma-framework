#include "rectangle.h"

#include <QPainter>

namespace Plasma {

Rectangle::Rectangle(Widget *parent)
    : Widget(parent)
{
    resize(400.0f, 400.0f);
    setFlag(QGraphicsItem::ItemIsMovable);
}

Rectangle::~Rectangle()
{
}

Qt::Orientations Rectangle::expandingDirections() const
{
    return Qt::Horizontal | Qt::Vertical;
}

void Rectangle::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    p->setBrush(Qt::white);
    p->setPen(Qt::black);
    p->setOpacity(0.5f);
    p->drawRect(localGeometry());
}

}
