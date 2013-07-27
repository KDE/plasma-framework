/*
 *   Copyright (C) 2013 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SHELLHANDLER_H
#define SHELLHANDLER_H

#include <utils/d_ptr.h>

#include <QObject>

class ShellHandler: public QObject {
    Q_OBJECT
    Q_PROPERTY(bool willing READ willing WRITE setWilling NOTIFY willingnessChanged)
    Q_PROPERTY(unsigned short priority READ priority WRITE setPriority NOTIFY priorityChanged)

public:
    ShellHandler();
    ~ShellHandler();

public Q_SLOTS:
    bool willing() const;
    unsigned short priority() const;

    void setWilling(bool willing);
    void setPriority(unsigned short priority);

    virtual void load() = 0;
    virtual void unload() = 0;

Q_SIGNALS:
    void willingnessChanged(bool willing);
    void priorityChanged(unsigned short priority);

private:
    D_PTR;

    typedef std::unique_ptr<ShellHandler> Ptr;
    friend class ShellManager;
};

class ShellManager: public QObject {
    Q_OBJECT
public:
    static ShellManager * instance();
    ~ShellManager();

    void loadHandlers();

protected:
    void registerHandler(ShellHandler * handler);

public Q_SLOTS:
    void updateShell();

private:
    ShellManager();

    D_PTR;

    friend class ShellHandler;
};

#endif /* SHELLHANDLER_H */

