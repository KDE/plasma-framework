#include <QTreeView>
#include "kcategorizeditemsviewmodels_p.h"

class KCategorizedItemsView;

class CustomDragTreeView: public QTreeView
{
public:
    CustomDragTreeView(QWidget * parent = 0);

protected:
    void startDrag ( Qt::DropActions supportedActions );

private:
    KCategorizedItemsView * m_view;

    friend class KCategorizedItemsView;
};
