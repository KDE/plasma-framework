/*
 *   Copyright © 2009 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include "pinpairingdialog_p.h"

#include "authorizationrule.h"
#include "clientpinrequest.h"
#include "ui_pinpairing.h"

#include <kdebug.h>
#include <kdialog.h>

namespace Plasma
{

class PinPairingDialogPrivate
{
public:
    PinPairingDialogPrivate(AuthorizationRule *rule, ClientPinRequest *request, PinPairingDialog *q)
        : q(q),
          rule(rule),
          request(request),
          dialog(new KDialog(0))
    {
        QWidget *widget = new QWidget(dialog);
        pairingUI.setupUi(widget);
        dialog->setMainWidget(widget);
        if (rule) {
            dialog->setCaption(i18n("Incoming connection request"));
            pairingUI.descriptionLabel->setText(rule->description());
        }

        if (request) {
            dialog->setCaption(i18n("Connect with remote widget"));
            pairingUI.persistentCheckbox->setVisible(false);
            pairingUI.allServicesCheckbox->setVisible(false);
            pairingUI.descriptionLabel->setText(request->description());
        }

        dialog->setButtons(KDialog::Ok | KDialog::Cancel);
        dialog->show();

        q->connect(dialog, SIGNAL(okClicked()), q, SLOT(slotAccept()));
        q->connect(dialog, SIGNAL(cancelClicked()), q, SLOT(slotReject()));
    }

    ~PinPairingDialogPrivate()
    {
    }

    void slotAccept()
    {
        kDebug();
        if (rule) {
            rule->setPin(pairingUI.password->text());
            if (pairingUI.allServicesCheckbox->isChecked()) {
                rule->setPolicy(AuthorizationRule::PinRequired);
                rule->setTargets(AuthorizationRule::AllServices);
            } else {
                rule->setPolicy(AuthorizationRule::PinRequired);
            }
            if (pairingUI.persistentCheckbox->isChecked()) {
                rule->setPersistence(AuthorizationRule::Persistent);
            }
        }

        if (request) {
            request->setPin(pairingUI.password->text());
        }

        q->deleteLater();
    }

    void slotReject()
    {
        q->deleteLater();
    }

    PinPairingDialog *q;
    AuthorizationRule *rule;
    ClientPinRequest *request;
    Ui::pairingDialog pairingUI;
    KDialog *dialog;
};

PinPairingDialog::PinPairingDialog(AuthorizationRule &rule, QObject *parent)
    : QObject(parent),
      d(new PinPairingDialogPrivate(&rule, 0, this))
{
}

PinPairingDialog::PinPairingDialog(ClientPinRequest &request, QObject *parent)
    : QObject(parent),
      d(new PinPairingDialogPrivate(0, &request, this))
{
}

PinPairingDialog::~PinPairingDialog()
{
}

}

#include "pinpairingdialog_p.moc"
