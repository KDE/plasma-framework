/*
 *   Copyright 2009 Aaron J. Seigo
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

#include "filedialogproxy.h"

FileDialogProxy::FileDialogProxy(KFileDialog::OperationMode mode, QObject *parent)
    : QObject(parent),
      m_dialog(new KFileDialog(KUrl("~"), QString(), 0))
{
    m_dialog->setOperationMode(mode);
    connect(m_dialog, SIGNAL(okClicked()), this, SIGNAL(finished()));
}

FileDialogProxy::~FileDialogProxy()
{
    delete m_dialog;
}

KUrl FileDialogProxy::selectedUrl() const
{
    return m_dialog->selectedUrl();
}

void FileDialogProxy::setUrl(const KUrl &url)
{
    m_dialog->setUrl(url);
}

KUrl::List FileDialogProxy::selectedUrls() const
{
    return m_dialog->selectedUrls();
}

KUrl FileDialogProxy::baseUrl() const
{
    return m_dialog->baseUrl();
}

QString FileDialogProxy::selectedFile() const
{
    return m_dialog->selectedFile();
}

QStringList FileDialogProxy::selectedFiles() const
{
    return m_dialog->selectedFiles();
}

QString FileDialogProxy::filter() const
{
    return m_dialog->currentFilter();
}

void FileDialogProxy::setFilter(const QString &filter)
{
    m_dialog->setFilter(filter);
}

bool FileDialogProxy::localOnly() const
{
    return m_dialog->mode() & KFile::LocalOnly;
}

void FileDialogProxy::setLocalOnly(bool localOnly)
{
    if (localOnly) {
        m_dialog->setMode(m_dialog->mode() ^ KFile::LocalOnly);
    } else {
        m_dialog->setMode(m_dialog->mode() | KFile::LocalOnly);
    }
}

bool FileDialogProxy::directoriesOnly() const
{
    return m_dialog->mode() & KFile::Directory;
}

void FileDialogProxy::setDirectoriesOnly(bool directoriesOnly)
{
    if (directoriesOnly) {
        m_dialog->setMode(m_dialog->mode() ^ KFile::Directory);
    } else {
        m_dialog->setMode(m_dialog->mode() | KFile::Directory);
    }
}

bool FileDialogProxy::existingOnly() const
{
    return m_dialog->mode() & KFile::ExistingOnly;
}

void FileDialogProxy::setExistingOnly(bool existingOnly)
{
    if (existingOnly) {
        m_dialog->setMode(m_dialog->mode() ^ KFile::ExistingOnly);
    } else {
        m_dialog->setMode(m_dialog->mode() | KFile::ExistingOnly);
    }
}

void FileDialogProxy::show()
{
    m_dialog->show();
}

#include "filedialogproxy.moc"

