#ifndef APPLETCOMPOSITOR_H
#define APPLETCOMPOSITOR_H

#include <kdelibs_export.h>

#include <QWidget>

namespace Plasma
{

/**
 * This is the visual representation of our applet container.
 * Together with AppletLayout handles all GUI aspect of displaying
 * applets. AppletCompositor is responsible for all the effects
 * and animations that happen between applets.
 */
class KDE_EXPORT AppletCompositor : public QWidget
{
        Q_OBJECT
    public:
        AppletCompositor(QWidget *parent);

    protected slots:
        virtual void mergeTwoApplets();

    private:
        class Private;
        Private *d;
};

}

#endif
