/*
    SPDX-FileCopyrightText: 2009 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLASMA_ASSOCIATEDAPPLICATIONMANAGER_P_H
#define PLASMA_ASSOCIATEDAPPLICATIONMANAGER_P_H

#include <QObject>
#include <QUrl>

namespace Plasma
{
class Applet;

class AssociatedApplicationManagerPrivate;

class AssociatedApplicationManager : public QObject
{
    Q_OBJECT

public:
    static AssociatedApplicationManager *self();

    // set an application name for an applet
    void setApplication(Plasma::Applet *applet, const QString &application);
    // returns the application name associated to an applet
    QString application(const Plasma::Applet *applet) const;

    // sets the urls associated to an applet
    void setUrls(Plasma::Applet *applet, const QList<QUrl> &urls);
    // returns the urls associated to an applet
    QList<QUrl> urls(const Plasma::Applet *applet) const;

    // run the associated application or the urls if no app is associated
    void run(Plasma::Applet *applet);
    // returns true if the applet has a valid associated application or urls
    bool appletHasValidAssociatedApplication(const Plasma::Applet *applet) const;

private:
    AssociatedApplicationManager(QObject *parent = nullptr);
    ~AssociatedApplicationManager() override;

    AssociatedApplicationManagerPrivate *const d;
    friend class AssociatedApplicationManagerSingleton;

    Q_PRIVATE_SLOT(d, void cleanupApplet(QObject *obj))
    Q_PRIVATE_SLOT(d, void updateActionNames())
};

} // namespace Plasma

#endif // multiple inclusion guard
