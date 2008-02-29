/*
 *   Copyright (C) 2008 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "plasma/appletbrowser/openwidgetassistant_p.h"

#include <QLabel>
#include <QVBoxLayout>

#include <KDebug>
#include <kfilewidget.h>
#include <KListWidget>
#include <KService>
#include <KServiceTypeTrader>
#include <KStandardDirs>

#include "plasma/packagestructure.h"

namespace Plasma
{

OpenWidgetAssistant::OpenWidgetAssistant(QWidget *parent)
    : KAssistantDialog(parent),
      m_fileDialog(0),
      m_filePageWidget(0)
{
    QWidget *selectWidget = new QWidget(this);
    QVBoxLayout *selectLayout = new QVBoxLayout(selectWidget);
    QLabel *selectLabel = new QLabel(selectWidget);
    selectLabel->setText(i18n("Select what sort of widget to install from the list below."));
    m_widgetTypeList = new KListWidget(selectWidget);
    m_widgetTypeList->setSelectionMode(QAbstractItemView::SingleSelection);
    //m_widgetTypeList->setSelectionBehavior(QAbstractItemView::SelectItems);

    QString constraint("'Applet' in [X-Plasma-ComponentTypes] and exist [X-Plasma-PackageFormat]");
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/ScriptEngine", constraint);

    QListWidgetItem * item = new QListWidgetItem(KIcon("plasma"), i18n("Plasmoid: Native plasma widget"), m_widgetTypeList);
    item->setSelected(true);
    m_widgetTypeList->setCurrentItem(item);

    foreach (const KService::Ptr &offer, offers) {
        QString text(offer->name());
        if (!offer->comment().isEmpty()) {
            text.append(": ").append(offer->comment());
        }

        item = new QListWidgetItem(text, m_widgetTypeList);
        item->setData(PackageStructureRole, offer->property("X-KDE-PluginInfo-Name"));

        if (!offer->icon().isEmpty()) {
            item->setIcon(KIcon(offer->icon()));
        }
    }

    selectLayout->addWidget(selectLabel);
    selectLayout->addWidget(m_widgetTypeList);

    m_typePage = new KPageWidgetItem(selectWidget, i18n("Install New Widget From File"));
    m_typePage->setIcon(KIcon("plasma"));
    addPage(m_typePage);

    m_filePageWidget = new QWidget(this);
    m_filePage = new KPageWidgetItem(m_filePageWidget, i18n("Select File"));
    addPage(m_filePage);

    connect(this, SIGNAL(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)), SLOT(prepPage(KPageWidgetItem*,KPageWidgetItem*)));
    connect(this, SIGNAL(user1Clicked()), this, SLOT(finished()));
}

void OpenWidgetAssistant::prepPage(KPageWidgetItem *current, KPageWidgetItem *before)
{
    Q_UNUSED(before)

    if (current != m_filePage) {
        return;
    }

    if (!m_fileDialog) {
        QVBoxLayout *layout = new QVBoxLayout(m_filePageWidget);
        m_fileDialog = new KFileWidget(KUrl(), m_filePageWidget);
        m_fileDialog->setOperationMode(KFileWidget::Opening);
        m_fileDialog->setMode(KFile::File | KFile::ExistingOnly);
        //m_fileDialog->setWindowFlags(Qt::Widget);
        layout->addWidget(m_fileDialog);
    }

    QListWidgetItem *item = m_widgetTypeList->selectedItems().first();
    Q_ASSERT(item);

    QString type = item->data(PackageStructureRole).toString();

    m_fileDialog->setFilter(QString());
    if (!type.isEmpty()) {
        QString constraint = QString("'%1' == [X-KDE-PluginInfo-Name]").arg(type);
        KService::List offers = KServiceTypeTrader::self()->query("Plasma/PackageStructure", constraint);

        kDebug() << "looking for a Plasma/PackageStructure with" << constraint << type;
        Q_ASSERT(offers.count() > 0);

        m_packageStructureService = offers.first();
        QStringList mimes = m_packageStructureService->property("X-Plasma-PackageFileMimetypes").toStringList();

        if (mimes.count() > 0) {
            m_fileDialog->setMimeFilter(mimes);
        } else {
            QString filter = m_packageStructureService->property("X-Plasma-PackageFileFilter").toString();
            if (!filter.isEmpty()) {
                m_fileDialog->setFilter( + "|" + m_packageStructureService->name());
            }
        }
    } else {
        m_fileDialog->setFilter("*.plasmoid|Plasma Widget");
    }
}

void OpenWidgetAssistant::finished()
{
    m_fileDialog->slotOk();
    m_fileDialog->accept(); // how interesting .. accept() must be called before the state is set

    if (m_fileDialog->selectedFile().isEmpty()) {
        //TODO: user visible error handling
        return;
    }

    //kDebug() << "selected uri is" << m_fileDialog->selectedFile() << "of type" << m_fileDialog->currentFilter();
    QString error;
    PackageStructure *installer = m_packageStructureService->createInstance<Plasma::PackageStructure>(0, QVariantList(), &error);
    if (!installer) {
        kDebug() << "Could not load requested PackageStructure installer "
                 << m_packageStructureService << ". Error given: " << error;
        return;
    }

    QString root = KStandardDirs::locateLocal("data", "plasma/plasmoids/");
    //kDebug() << "installing to root dir of" << root;
    bool success = installer->installPackage(m_fileDialog->selectedFile(), root);

    kDebug() << "install returned. were we successful?" << success;
    //TODO: user visible feedback
}

} // Plasma namespace

#include "openwidgetassistant_p.moc"
