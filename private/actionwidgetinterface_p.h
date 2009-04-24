#ifndef ACTIONWIDGETINTERFACE_P_H
    /**
     * update the icon's text, icon, etc. to reflect the properties of its associated action.
     */
#include <QAction>

namespace Plasma
{

template <class T>
class ActionWidgetInterface
{
public:
    T *t;
    QAction *action;

    ActionWidgetInterface(T *publicClass)
        : t(publicClass),
          action(0)
    {
    }

    virtual ~ActionWidgetInterface()
    {
    }

    virtual void changed()
    {
    }

    void clearAction()
    {
        action = 0;
        syncToAction();
        changed();
    }

    void syncToAction()
    {
        if (!action) {
            t->setIcon(QIcon());
            t->setText(QString());
            t->setEnabled(false);
            return;
        }
        //we don't get told *what* changed, just that something changed
        //so we update everything we care about
        t->setIcon(action->icon());
        t->setText(action->iconText());
        t->setEnabled(action->isEnabled());
        t->setVisible(action->isVisible());

        if (!t->toolTip().isEmpty()) {
            t->setToolTip(action->text());
        }

        changed();
    }

    void setAction(QAction *a)
    {
        if (action) {
            QObject::disconnect(action, 0, t, 0);
            QObject::disconnect(t, 0, action, 0);
        }

        action = a;

        if (action) {
            QObject::connect(action, SIGNAL(changed()), t, SLOT(syncToAction()));
            QObject::connect(action, SIGNAL(destroyed(QObject*)), t, SLOT(clearAction()));
            QObject::connect(t, SIGNAL(clicked()), action, SLOT(trigger()));
            syncToAction();
        }
    }
};

} // namespace Plasma
#endif

