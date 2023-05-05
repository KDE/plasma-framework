#include "containmentactions.h"
#include <KPluginFactory>

class DummyContainmentAction : public Plasma::ContainmentActions
{
    Q_OBJECT

public:
    explicit DummyContainmentAction(QObject *parent)
        : Plasma::ContainmentActions(parent)
    {
    }
};

K_PLUGIN_CLASS_WITH_JSON(DummyContainmentAction, "dummycontainmentaction.json")

#include "dummycontainmentaction.moc"
