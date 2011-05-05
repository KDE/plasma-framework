/*
 *   Copyright 2009 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "accessappletjob.h"

#include "service.h"
#include "servicejob.h"
#include "applet.h"

#include "config-plasma.h"

#include <kzip.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <ktempdir.h>
#include <kdesktopfile.h>
#include "package.h"
#include <qtimer.h>
#include "private/applet_p.h"

namespace Plasma
{

class AccessAppletJobPrivate
{
public:
    AccessAppletJobPrivate(const KUrl &location, AccessAppletJob *owner)
        : q(owner),
          location(location),
          applet(0)
    {
    }

    void slotStart()
    {
        q->start();
    }

    void slotServiceReady(Plasma::Service *service)
    {
        KConfigGroup op = service->operationDescription("GetPackage");
        service->startOperationCall(op);
        q->connect(service, SIGNAL(finished(Plasma::ServiceJob*)),
                   q, SLOT(slotPackageDownloaded(Plasma::ServiceJob*)));
    }

    void slotPackageDownloaded(Plasma::ServiceJob *job)
    {
        if (job->error()) {
            kDebug() << "Plasmoid Access Job triggers an error.";
            q->setError(job->error());
            q->setErrorText(job->errorText());
        }

        //TODO: there's some duplication with installPackage, but we don't want to actually install
        //the fetched package. Just extract the archive somewhere in a temp directory.
        if (job->result().type() == QVariant::String) {
            QString pluginName = job->result().toString();
            kDebug() << "Server responded with a pluginname, trying to load: " << pluginName;

            applet = Applet::load(pluginName);
            if (applet) {
                applet->d->remoteLocation = location.prettyUrl();
            } else {
                q->setError(-1);
                q->setErrorText(i18n("The \"%1\" widget is not installed.", pluginName));
            }

            q->emitResult();
        } else {
            kDebug() << "Server responded with a plasmoid package";
            //read, and extract the plasmoid package to a temporary directory
            QByteArray package = job->result().toByteArray();
            QDataStream stream(&package, QIODevice::ReadOnly);

            KZip archive(stream.device());
            if (!archive.open(QIODevice::ReadOnly)) {
                kWarning() << "Could not open package file";
                q->setError(-1);
                q->setErrorText(i18n("Server sent an invalid plasmoid package."));
                q->emitResult();
                return;
            }

            const KArchiveDirectory *source = archive.directory();

            KTempDir tempDir;
            tempDir.setAutoRemove(false);
            QString path = tempDir.name();
            source->copyTo(path);

            KDesktopFile metadata(path + "/metadata.desktop");
            KConfigGroup group = metadata.desktopGroup();

            QString iconName = group.readEntry("Icon");
            QString message = i18n("You are about to open a remote widget on your system.<br>");
            message+= i18n("<table width=\"100%\">");
            message+= i18n("<tr><td align=\"right\"><b>Name:</b></td><td>&nbsp; %1</td></tr>", group.readEntry("Name"));
            message+= i18n("<tr><td align=\"right\"><b>Description:</b></td><td>&nbsp; %1</td></tr>", group.readEntry("Comment"));
            message+= i18n("<tr><td align=\"right\"><b>Author:</b></td><td>&nbsp; %1 &lt;%2&gt;</td></tr>",
                           group.readEntry("X-KDE-PluginInfo-Author"),
                           group.readEntry("X-KDE-PluginInfo-Email"));
            message+= i18n("<tr><td align=\"right\"><b>Server:</b></td><td>&nbsp; %1</td></tr>", location.host());
            message+= i18n("</table>");
            message+= i18n("<br><br>Are you sure you want to open this widget on your system?");

            KDialog *dialog = new KDialog;
            dialog->setWindowTitle(i18n("Remote Widget"));
            dialog->setButtons(KDialog::Yes|KDialog::No);
            dialog->setButtonText(KDialog::Yes, i18n("Open Widget"));
            dialog->setButtonText(KDialog::No, i18n("Reject Widget"));

            int answer = KMessageBox::createKMessageBox(dialog, KIcon(iconName), message,
                                                        QStringList(), QString(), 0,
                                                        KMessageBox::Dangerous);
            //int answer = KMessageBox::questionYesNo(0, message, i18n("Remote Widget"));

            if (answer!=KDialog::Yes) {
                q->setError(-1);
                q->setErrorText(i18n("User rejected"));
                q->emitResult();
                return;
            }

            /**
            QString metadataFilename = path + "/metadata.desktop";
            KDesktopFile cfg(metadataFilename);
            KConfigGroup config = cfg.desktopGroup();
            config.writeEntry("EngineLocation", location.prettyUrl());
            config.sync();
            */

            applet = Applet::loadPlasmoid(path);
            if (applet) {
                applet->d->remoteLocation = location.prettyUrl();
            } else {
                q->setError(-1);
            }

            q->emitResult();
        }
    }

    void slotTimeout()
    {
        kWarning() << "Plasmoid access job timed out";
        q->setError(-1);
        q->setErrorText(i18n("Timeout"));
        q->emitResult();
    }

    AccessAppletJob *q;
    KUrl location;
    Applet *applet;
};

AccessAppletJob::AccessAppletJob(const KUrl &location, QObject *parent)
    : KJob(parent),
      d(new AccessAppletJobPrivate(location, this))
{
    QTimer::singleShot(30000, this, SLOT(slotTimeout()));
}

AccessAppletJob::~AccessAppletJob()
{
    delete d;
}

Applet *AccessAppletJob::applet() const
{
    return d->applet;
}

void AccessAppletJob::start()
{
#ifdef ENABLE_REMOTE_WIDGETS
    kDebug() << "fetching a plasmoid from location = " << d->location.prettyUrl();
    Service *service = Service::access(d->location);
    connect(service, SIGNAL(serviceReady(Plasma::Service*)),
            this, SLOT(slotServiceReady(Plasma::Service*)));
#else
    kWarning() << "libplasma was compiled without support for remote services. Accessing remote applet failed because of that.";
    setError(-1);
    setErrorText(i18n("Your system does not provide support for the 'remote widgets' feature. Access Failed."));
    emitResult();
#endif
}

} // namespace Plasma

#include "accessappletjob.moc"

