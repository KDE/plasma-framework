#include <plasma/widgets/widget.h>

namespace Plasma {

class PLASMA_EXPORT Rectangle : public Plasma::Widget
{
    public:
        Rectangle(Widget *parent);
        virtual ~Rectangle();

        Qt::Orientations expandingDirections() const;

        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    private:
        class Private;
        Private const *d;
};

}
