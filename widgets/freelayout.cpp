
#include "freelayout.h"

using namespace Plasma;

class FreeLayout::Private
{
public:
    QList<LayoutItem*> children;
    QRectF geometry;
};

FreeLayout::FreeLayout(LayoutItem *parent)
    : Layout(parent)
    , d(new Private)
{
}
Qt::Orientations FreeLayout::expandingDirections() const
{
    return Qt::Horizontal | Qt::Vertical;
}
void FreeLayout::addItem(LayoutItem *item)
{
    d->children << item;
    item->setManagingLayout(this);
}
void FreeLayout::removeItem(LayoutItem *item)
{
    d->children.removeAll(item);
}
int FreeLayout::indexOf(LayoutItem *item) const
{
    return d->children.indexOf(item);
}
LayoutItem * FreeLayout::itemAt(int i) const
{
    return d->children[i];
}
int FreeLayout::count() const
{
    return d->children.count();
}
LayoutItem * FreeLayout::takeAt(int i) 
{
    return d->children.takeAt(i);
}
void FreeLayout::setGeometry(const QRectF& geometry)
{
    foreach( LayoutItem *child , d->children ) {
        child->setGeometry( QRectF(child->geometry().topLeft(),child->sizeHint()) );
    }
    d->geometry = geometry;
}
QRectF FreeLayout::geometry() const
{
    return d->geometry;
}
QSizeF FreeLayout::sizeHint() const
{
    return maximumSize();
}

