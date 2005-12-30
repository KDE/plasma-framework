#ifndef APPLEYLAYOUT_H
#define APPLEYLAYOUT_H

#include "kdelibs_export.h"

#include <QLayout>

namespace Plasma
{

/**
 * AppletLayout is our custom layouting engine
 * for applets. It reacts to state transition (removal,addition
 * of applets) by emitting corresponding signals which AppletCompositor
 * can intercept and respond with some nice animation/effect.
 */
class KDE_EXPORT AppletLayout : public QLayout
{
        Q_OBJECT
    public:
        AppletLayout(QWidget *parent);

    signals:
        void mergeTwoApplets();
        void splitTwoApplets();
    private:
        class Private;
        Private *d;
};

}

#endif
