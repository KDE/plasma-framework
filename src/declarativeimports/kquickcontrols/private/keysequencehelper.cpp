/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  David Edmundson <davidedmundson@kde.org>
 * Copyright (C) 1998 Mark Donohoe <donohoe@kde.org>
 * Copyright (C) 2001 Ellis Whitehead <ellis@kde.org>
 * Copyright (C) 2007 Andreas Hartmetz <ahartmetz@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "keysequencehelper.h"

#include <QAction>
#include <QKeyEvent>
#include <QTimer>
#include <QtCore/QHash>
#include <QToolButton>
#include <QApplication>
#include <QDebug>

#include <KStandardShortcut>
#include <KLocalizedString>
#include <KMessageBox>
#include <KKeyServer>
#include <KGlobalAccel/KGlobalShortcutInfo>
#include <KGlobalAccel/KGlobalAccel>

uint qHash(const QKeySequence &seq)
{
    return qHash(seq.toString());
}

class KeySequenceHelperPrivate
{
public:
    KeySequenceHelperPrivate(KeySequenceHelper *q);

    void init();

    static QKeySequence appendToSequence(const QKeySequence &seq, int keyQt);
    static bool isOkWhenModifierless(int keyQt);

    void updateShortcutDisplay();
    void startRecording();

    /**
     * Conflicts the key sequence @a seq with a current standard
     * shortcut?
     */
    bool conflictWithStandardShortcuts(const QKeySequence &seq);

    /**
     * Conflicts the key sequence @a seq with a current global
     * shortcut?
     */
    bool conflictWithGlobalShortcuts(const QKeySequence &seq);

    /**
     * Get permission to steal the shortcut @seq from the standard shortcut @a std.
     */
    bool stealStandardShortcut(KStandardShortcut::StandardShortcut std, const QKeySequence &seq);

    bool checkAgainstStandardShortcuts() const
    {
        return checkAgainstShortcutTypes & KeySequenceHelper::StandardShortcuts;
    }

    bool checkAgainstGlobalShortcuts() const
    {
        return checkAgainstShortcutTypes & KeySequenceHelper::GlobalShortcuts;
    }

    void controlModifierlessTimout()
    {
        if (nKey != 0 && !modifierKeys) {
            // No modifier key pressed currently. Start the timout
            modifierlessTimeout.start(600);
        } else {
            // A modifier is pressed. Stop the timeout
            modifierlessTimeout.stop();
        }

    }

    void cancelRecording()
    {
        keySequence = oldKeySequence;
        q->doneRecording();
    }


//members
    KeySequenceHelper *const q;
    QToolButton *clearButton;

    QKeySequence keySequence;
    QKeySequence oldKeySequence;
    QTimer modifierlessTimeout;
    bool allowModifierless;
    uint nKey;
    uint modifierKeys;
    bool isRecording;
    bool multiKeyShortcutsAllowed;
    QString componentName;
    QString shortcutDisplay;

    //! Check the key sequence against KStandardShortcut::find()
    KeySequenceHelper::ShortcutTypes checkAgainstShortcutTypes;

    /**
     * The list of action to check against for conflict shortcut
     */
    QList<QAction *> checkList; // deprecated

    /**
     * The list of action collections to check against for conflict shortcut
     */
//     QList<KActionCollection *> checkActionCollections;

    /**
     * The action to steal the shortcut from.
     */
    QList<QAction *> stealActions;

    bool stealShortcuts(const QList<QAction *> &actions, const QKeySequence &seq);
    void wontStealShortcut(QAction *item, const QKeySequence &seq);

};

KeySequenceHelperPrivate::KeySequenceHelperPrivate(KeySequenceHelper *q)
: q(q)
, allowModifierless(false)
, nKey(0)
, modifierKeys(0)
, isRecording(false)
, multiKeyShortcutsAllowed(true)
, componentName()
, checkAgainstShortcutTypes(KeySequenceHelper::StandardShortcuts | KeySequenceHelper::GlobalShortcuts)
, stealActions()
{

}

KeySequenceHelper::KeySequenceHelper(QObject* parent):
    QObject(),
    d(new KeySequenceHelperPrivate(this))
{
    connect(&d->modifierlessTimeout, SIGNAL(timeout()), this, SLOT(doneRecording()));
    d->updateShortcutDisplay();
}


KeySequenceHelper::~KeySequenceHelper()
{
    delete d;
}

bool KeySequenceHelper::multiKeyShortcutsAllowed() const
{
    return d->multiKeyShortcutsAllowed;
}

void KeySequenceHelper::setMultiKeyShortcutsAllowed(bool allowed)
{
    d->multiKeyShortcutsAllowed = allowed;
}

void KeySequenceHelper::setModifierlessAllowed(bool allow)
{
    d->allowModifierless = allow;
}

bool KeySequenceHelper::isModifierlessAllowed()
{
    return d->allowModifierless;
}

bool KeySequenceHelper::isKeySequenceAvailable(const QKeySequence& keySequence) const
{
    if (keySequence.isEmpty()) {
        return true;
    }
    return !(d->conflictWithGlobalShortcuts(keySequence)
             || d->conflictWithStandardShortcuts(keySequence));
}

//
// void KeySequenceHelper::setCheckActionCollections(const QList<KActionCollection *> &actionCollections)
// {
//     d->checkActionCollections = actionCollections;
// }
//

//slot
void KeySequenceHelper::captureKeySequence()
{
    d->startRecording();
}

QKeySequence KeySequenceHelper::keySequence() const
{
    return d->keySequence;
}

void KeySequenceHelper::setKeySequence(const QKeySequence& sequence)
{
    if (!d->isRecording) {
        d->oldKeySequence = d->keySequence;
    }
    d->keySequence = sequence;
    d->updateShortcutDisplay();
    emit keySequenceChanged(d->keySequence);
}


void KeySequenceHelper::clearKeySequence()
{
    setKeySequence(QKeySequence());
}

void KeySequenceHelperPrivate::startRecording()
{
    nKey = 0;
    modifierKeys = 0;
    oldKeySequence = keySequence;
    keySequence = QKeySequence();
    isRecording = true;

    updateShortcutDisplay();
}
//
void KeySequenceHelper::doneRecording()
{
    d->modifierlessTimeout.stop();
    d->isRecording = false;
    d->stealActions.clear();

    if (d->keySequence == d->oldKeySequence) {
//         The sequence hasn't changed
        d->updateShortcutDisplay();
        return;
    }

    if (! isKeySequenceAvailable(d->keySequence)) {
//         The sequence had conflicts and the user said no to stealing it
        d->keySequence = d->oldKeySequence;
    } else {
        emit keySequenceChanged(d->keySequence);
    }

    Q_EMIT captureFinished();

    d->updateShortcutDisplay();
}


bool KeySequenceHelperPrivate::conflictWithGlobalShortcuts(const QKeySequence &keySequence)
{
#ifdef Q_OS_WIN
    //on windows F12 is reserved by the debugger at all times, so we can't use it for a global shortcut
    if (KeySequenceHelper::GlobalShortcuts && keySequence.toString().contains("F12")) {
        QString title = i18n("Reserved Shortcut");
        QString message = i18n("The F12 key is reserved on Windows, so cannot be used for a global shortcut.\n"
                               "Please choose another one.");

        KMessageBox::sorry(q, message, title);
        return false;
    }
#endif

    if (!(checkAgainstShortcutTypes & KeySequenceHelper::GlobalShortcuts)) {
        return false;
    }

    // Global shortcuts are on key+modifier shortcuts. They can clash with
    // each of the keys of a multi key shortcut.
    QList<KGlobalShortcutInfo> others;
    for (int i = 0; i < keySequence.count(); ++i) {
        QKeySequence tmp(keySequence[i]);

        if (!KGlobalAccel::isGlobalShortcutAvailable(tmp, componentName)) {
            others << KGlobalAccel::getGlobalShortcutsByKey(tmp);
        }
    }

    if (!others.isEmpty()
            && !KGlobalAccel::promptStealShortcutSystemwide(0, others, keySequence)) {
        return true;
    }

    // The user approved stealing the shortcut. We have to steal
    // it immediately because KAction::setGlobalShortcut() refuses
    // to set a global shortcut that is already used. There is no
    // error it just silently fails. So be nice because this is
    // most likely the first action that is done in the slot
    // listening to keySequenceChanged().
    for (int i = 0; i < keySequence.count(); ++i) {
        KGlobalAccel::stealShortcutSystemwide(keySequence[i]);
    }
    return false;
}

bool KeySequenceHelperPrivate::conflictWithStandardShortcuts(const QKeySequence &keySequence)
{
    if (!checkAgainstStandardShortcuts()) {
        return false;
    }

    KStandardShortcut::StandardShortcut ssc = KStandardShortcut::find(keySequence);
    if (ssc != KStandardShortcut::AccelNone && !stealStandardShortcut(ssc, keySequence)) {
        qDebug() << "!!!!!!!!!!!!!!";
        return true;
    }
    return false;
}

bool KeySequenceHelperPrivate::stealStandardShortcut(KStandardShortcut::StandardShortcut std, const QKeySequence &seq)
{
    QString title = i18n("Conflict with Standard Application Shortcut");
    QString message = i18n("The '%1' key combination is also used for the standard action "
                           "\"%2\" that some applications use.\n"
                           "Do you really want to use it as a global shortcut as well?",
                           seq.toString(QKeySequence::NativeText), KStandardShortcut::label(std));

    if (KMessageBox::warningContinueCancel(0, message, title, KGuiItem(i18n("Reassign"))) != KMessageBox::Continue) {
        return false;
    }
    return true;
}

void KeySequenceHelperPrivate::updateShortcutDisplay()
{
    //empty string if no non-modifier was pressed
    QString s = keySequence.toString(QKeySequence::NativeText);
    s.replace(QLatin1Char('&'), QStringLiteral("&&"));

    if (isRecording) {
        if (modifierKeys) {
            if (!s.isEmpty()) {
                s.append(QLatin1Char(','));
            }
            if (modifierKeys & Qt::META) {
                s += KKeyServer::modToStringUser(Qt::META) + QLatin1Char('+');
            }
#if defined(Q_OS_MAC)
            if (modifierKeys & Qt::ALT) {
                s += KKeyServer::modToStringUser(Qt::ALT) + QLatin1Char('+');
            }
            if (modifierKeys & Qt::CTRL) {
                s += KKeyServer::modToStringUser(Qt::CTRL) + QLatin1Char('+');
            }
#else
            if (modifierKeys & Qt::CTRL) {
                s += KKeyServer::modToStringUser(Qt::CTRL) + QLatin1Char('+');
            }
            if (modifierKeys & Qt::ALT) {
                s += KKeyServer::modToStringUser(Qt::ALT) + QLatin1Char('+');
            }
#endif
            if (modifierKeys & Qt::SHIFT) {
                s += KKeyServer::modToStringUser(Qt::SHIFT) + QLatin1Char('+');
            }

        } else if (nKey == 0) {
            s = i18nc("What the user inputs now will be taken as the new shortcut", "Input");
        }
        //make it clear that input is still going on
        s.append(QStringLiteral(" ..."));
    }

    if (s.isEmpty()) {
        s = i18nc("No shortcut defined", "None");
    }

    s.prepend(QLatin1Char(' '));
    s.append(QLatin1Char(' '));
    shortcutDisplay = s;
    q->shortcutDisplayChanged(s);
}

QString KeySequenceHelper::shortcutDisplay() const
{
    return d->shortcutDisplay;
}

void KeySequenceHelper::keyPressed(int key, int modifiers)
{
    if (key == -1) {
        // Qt sometimes returns garbage keycodes, I observed -1, if it doesn't know a key.
        // We cannot do anything useful with those (several keys have -1, indistinguishable)
        // and QKeySequence.toString() will also yield a garbage string.
        KMessageBox::sorry(0,
                           i18n("The key you just pressed is not supported by Qt."),
                           i18n("Unsupported Key"));
        return d->cancelRecording();
    }

    //don't have the return or space key appear as first key of the sequence when they
    //were pressed to start editing - catch and them and imitate their effect
    if (!d->isRecording && ((key == Qt::Key_Return || key == Qt::Key_Space))) {
        d->startRecording();
        d->modifierKeys = modifiers;
        d->updateShortcutDisplay();
        return;
    }

    d->modifierKeys = modifiers;

    switch (key) {
    case Qt::Key_AltGr: //or else we get unicode salad
        return;
    case Qt::Key_Shift:
    case Qt::Key_Control:
    case Qt::Key_Alt:
    case Qt::Key_Meta:
    case Qt::Key_Menu: //unused (yes, but why?)
        d->controlModifierlessTimout();
        d->updateShortcutDisplay();
        break;
    default:

        if (d->nKey == 0 && !(d->modifierKeys & ~Qt::SHIFT)) {
            // It's the first key and no modifier pressed. Check if this is
            // allowed
            if (!(KeySequenceHelperPrivate::isOkWhenModifierless(key)
                    || d->allowModifierless)) {
                // No it's not
                return;
            }
        }

        // We now have a valid key press.
        if (key) {
            if ((key == Qt::Key_Backtab) && (d->modifierKeys & Qt::SHIFT)) {
                key = Qt::Key_Tab | d->modifierKeys;
            } else {
                key |= (d->modifierKeys);
            }

            if (d->nKey == 0) {
                d->keySequence = QKeySequence(key);
            } else {
                d->keySequence =
                    KeySequenceHelperPrivate::appendToSequence(d->keySequence, key);
            }

            d->nKey++;
            if ((!d->multiKeyShortcutsAllowed) || (d->nKey >= 4)) {
                doneRecording();
                return;
            }
            d->controlModifierlessTimout();
            d->updateShortcutDisplay();
        }
    }
}
//
void KeySequenceHelper::keyReleased(int key, int modifiers)
{
    if (key == -1) {
        // ignore garbage, see keyPressEvent()
        return;
    }

    //if a modifier that belongs to the shortcut was released...
    if ((modifiers & d->modifierKeys) < d->modifierKeys) {
        d->modifierKeys = modifiers;
        d->controlModifierlessTimout();
        d->updateShortcutDisplay();
    }
}
//
//static
QKeySequence KeySequenceHelperPrivate::appendToSequence(const QKeySequence &seq, int keyQt)
{
    switch (seq.count()) {
    case 0:
        return QKeySequence(keyQt);
    case 1:
        return QKeySequence(seq[0], keyQt);
    case 2:
        return QKeySequence(seq[0], seq[1], keyQt);
    case 3:
        return QKeySequence(seq[0], seq[1], seq[2], keyQt);
    default:
        return seq;
    }
}
//
//static
bool KeySequenceHelperPrivate::isOkWhenModifierless(int keyQt)
{
    //this whole function is a hack, but especially the first line of code
    if (QKeySequence(keyQt).toString().length() == 1) {
        return false;
    }

    switch (keyQt) {
    case Qt::Key_Return:
    case Qt::Key_Space:
    case Qt::Key_Tab:
    case Qt::Key_Backtab: //does this ever happen?
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
        return false;
    default:
        return true;
    }
}
