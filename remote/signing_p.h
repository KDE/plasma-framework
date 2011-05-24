/*
 *  Copyright (C) 2010 by Diego '[Po]lentino' Casella <polentino911@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation
 *  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef SIGNING_P_H
#define SIGNING_P_H

#include <QtCore/QObject>
#include <QtCore/QStringList>

#include <KDE/KDirWatch>

namespace GpgME {
    class Context;
}

namespace Plasma
{

/**
  * @brief Encapsulates all the nasty GpgME stuff and shows a more friendly a simple implementation
  * for our goal.
  */
class SigningPrivate
{

public:
    SigningPrivate(Signing *auth, const QString &keystorePath);
    ~SigningPrivate();

    Signing *q;
    // Save all the keys in a single object, splitted by their SigningLevel
    // Note: keys[CompletelyUntrusted] refers to an empty list
    QMap<TrustLevel, QSet<QByteArray> > keys;

    QString ultimateKeyStoragePath() const;
    void registerUltimateTrustKeys();
    void splitKeysByTrustLevel();
    Plasma::TrustLevel addKeyToCache(const QByteArray &fingerprint);
    void dumpKeysToDebug();
    QStringList keysID(const bool returnPrivate) const;
    QString signerOf(const QString &messagePath, const QString &signaturePath) const;

    void processKeystore(const QString &path);
    void keyAdded(const QString &path);
    void keyRemoved(const QString &path);

    QStringList signersOf(const QString id) const;

    KDirWatch *m_keystoreDir; // used to be notified for changes in the gnupg folders
    GpgME::Context *m_gpgContext;
    QString m_keystorePath;
};

}

#endif // SIGNING_P_H
