#include "appletLayout.h"

#include <QLayoutItem>
#include <QSizePolicy>
#include <QRect>

namespace Plasma
{

struct AppletLayoutItem
{
    public:
        AppletLayoutItem(QLayoutItem *it);

    private:
        QLayoutItem *item;
};

class AppletLayout::Private
{
    public:
        ~Private()
        {
            QLayoutItem *item;
            while ((item = takeAt(0)))
                delete item;
        }
        QLayoutItem *takeAt(int index)
        {
            if (index >= 0 && index < itemList.size())
                return itemList.takeAt(index);
            else
                return 0;
        }
        QList<QLayoutItem *> itemList;
};

AppletLayout::AppletLayout(QWidget *parent)
    : QLayout(parent),
      d(new Private)
{
}

AppletLayout::~AppletLayout()
{
    delete d; d = 0;
}


void AppletLayout::addItem(QLayoutItem *item)
{
    d->itemList.append(item);
}

Qt::Orientations AppletLayout::expandingDirections() const
{
    return 0;
}

bool AppletLayout::hasHeightForWidth() const
{
    return true;
}

int AppletLayout::heightForWidth(int width) const
{
    int height = layoutApplets(QRect(0, 0, width, 0), true);
    return height;
}

int AppletLayout::count() const
{
    return d->itemList.size();
}

QLayoutItem *AppletLayout::itemAt(int index) const
{
    return d->itemList.value(index);
}

QSize AppletLayout::minimumSize() const
{
    QSize size;
    QLayoutItem *item;
    foreach (item, d->itemList)
        size = size.expandedTo(item->minimumSize());

    size += QSize(2*margin(), 2*margin());
    return size;
}

void AppletLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    layoutApplets(rect, false);
}

QSize AppletLayout::sizeHint() const
{
    return minimumSize();
}

QLayoutItem * AppletLayout::takeAt(int index)
{
    return d->takeAt(index);
}

int AppletLayout::layoutApplets(const QRect &rect, bool computeHeightOnly) const
{
    int x = rect.x();
    int y = rect.y();
    int lineHeight = 0;

    QLayoutItem *item;
    foreach (item, d->itemList) {
        int nextX = x + item->sizeHint().width() + spacing();
        if (nextX - spacing() > rect.right() && lineHeight > 0) {
            x = rect.x();
            y = y + lineHeight + spacing();
            nextX = x + item->sizeHint().width() + spacing();
            lineHeight = 0;
        }

        if (!computeHeightOnly)
            item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }
    return y + lineHeight - rect.y();
}

} //end namespace Plasma


#include "appletLayout.moc"
