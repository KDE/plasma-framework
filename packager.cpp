/******************************************************************************
*   Copyright (C) 2007 by Riccardo Iaconelli  <ruphy@fsfe.org>                *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#include <QtGui>
#include <KTemporaryFile>
#include <KDebug>
#include <KZip>
#include <KConfig>
#include <KConfigGroup>
#include <packagemetadata.h>
#include <packager.h>

namespace Plasma
{

class Packager::Private
{
    public:
        PackageMetadata* metadata;
};

Packager::Packager(PackageMetadata *metadata)
    : d(new Private)
{
    d->metadata = metadata;
}

Packager::~Packager()
{
    delete d;
}

const PackageMetadata* Packager::metadata()
{
    return d->metadata;
}

void Packager::setMetadata(PackageMetadata *metadata)
{
    d->metadata = metadata;
}

bool Packager::createPackage(const QString& destination, const QString& source)
{
    if (!d->metadata->isComplete()) {
        kWarning(550) << "Metadata file is not complete" << endl;
        return false;
    }

    KTemporaryFile metadataFile;
    metadataFile.open();
    d->metadata->write(metadataFile.fileName());

    KTemporaryFile releaseNotes;
    //We just write the content of the QString containing the metadata in an
    //empty temporary file that we will package with the name metadata.desktop
    if (releaseNotes.open()) {
        QTextStream out(&releaseNotes);
        if (d->metadata->releaseNotes().isEmpty()) {
            out << d->metadata->releaseNotes();
        } else {
            out << "NO_RELEASE_NOTES";
        }
    }

    //OK, we've got the temporary file with the metadata in it.
    //Now we just need to put everything into a zip archive.
    KZip creation(destination);
    creation.setCompression(KZip::NoCompression);

    if (!creation.open(QIODevice::WriteOnly)) {
        return false;
    }

    creation.addLocalFile(metadataFile.fileName(), "metadata.desktop");
    creation.addLocalFile(releaseNotes.fileName(), "notes.txt");

    if (!d->metadata->icon().isEmpty()) {
        //TODO: just one icon?
        creation.addLocalFile(d->metadata->icon(), "icon.png");
    }

    if (!d->metadata->preview().isEmpty()) {
        //TODO: just one icon?
        creation.addLocalFile(d->metadata->preview(), "preview.png");
    }

    creation.addLocalDirectory(source, "contents/");
    creation.close();
    return true;
}

} // Plasma namespace
