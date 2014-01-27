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
     * Conflicts the key sequence @a seq with a current local
     * shortcut?
     */
    bool conflictWithLocalShortcuts(const QKeySequence &seq);

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

    bool checkAgainstLocalShortcuts() const
    {
        return checkAgainstShortcutTypes & KeySequenceHelper::LocalShortcuts;
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


    bool promptStealShortcutSystemwide(
        QWidget *parent,
        const QHash<QKeySequence, QList<KGlobalShortcutInfo> > &shortcuts,
        const QKeySequence &sequence)
    {
        if (shortcuts.isEmpty()) {
            // Usage error. Just say no
            return false;
        }

        QString clashingKeys;
        Q_FOREACH (const QKeySequence &seq, shortcuts.keys()) {
            Q_FOREACH (const KGlobalShortcutInfo &info, shortcuts[seq]) {
                clashingKeys += i18n("Shortcut '%1' in Application %2 for action %3\n",
                                     seq.toString(),
                                     info.componentFriendlyName(),
                                     info.friendlyName());
            }
        }

        const int hashSize = shortcuts.size();

        QString message = i18ncp("%1 is the number of conflicts (hidden), %2 is the key sequence of the shortcut that is problematic",
                                 "The shortcut '%2' conflicts with the following key combination:\n",
                                 "The shortcut '%2' conflicts with the following key combinations:\n",
                                 hashSize, sequence.toString());
        message += clashingKeys;

        QString title = i18ncp("%1 is the number of shortcuts with which there is a conflict",
                               "Conflict with Registered Global Shortcut", "Conflict with Registered Global Shortcuts", hashSize);

        return KMessageBox::warningContinueCancel(parent, message, title, KGuiItem(i18n("Reassign")))
               == KMessageBox::Continue;
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
, checkAgainstShortcutTypes(KeySequenceHelper::LocalShortcuts | KeySequenceHelper::GlobalShortcuts)
, stealActions()
{

}

//
// bool KeySequenceHelperPrivate::stealShortcuts(
//     const QList<QAction *> &actions,
//     const QKeySequence &seq)
// {
//
//     const int listSize = actions.size();
//
//     QString title = i18ncp("%1 is the number of conflicts", "Shortcut Conflict", "Shortcut Conflicts", listSize);
//
//     QString conflictingShortcuts;
//     Q_FOREACH (const QAction *action, actions) {
//         conflictingShortcuts += i18n("Shortcut '%1' for action '%2'\n",
//                                      action->shortcut().toString(QKeySequence::NativeText),
//                                      KLocalizedString::removeAcceleratorMarker(action->text()));
//     }
//     QString message = i18ncp("%1 is the number of ambigious shortcut clashes (hidden)",
//                              "The \"%2\" shortcut is ambiguous with the following shortcut.\n"
//                              "Do you want to assign an empty shortcut to this action?\n"
//                              "%3",
//                              "The \"%2\" shortcut is ambiguous with the following shortcuts.\n"
//                              "Do you want to assign an empty shortcut to these actions?\n"
//                              "%3",
//                              listSize,
//                              seq.toString(QKeySequence::NativeText),
//                              conflictingShortcuts);
//
//     if (KMessageBox::warningContinueCancel(q, message, title, KGuiItem(i18n("Reassign"))) != KMessageBox::Continue) {
//         return false;
//     }
//
//     return true;
// }
//
// void KeySequenceHelperPrivate::wontStealShortcut(QAction *item, const QKeySequence &seq)
// {
//     QString title(i18n("Shortcut conflict"));
//     QString msg(i18n("<qt>The '%1' key combination is already used by the <b>%2</b> action.<br>"
//                      "Please select a different one.</qt>", seq.toString(QKeySequence::NativeText),
//                      KLocalizedString::removeAcceleratorMarker(item->text())));
//     KMessageBox::sorry(q, msg);
// }
//

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
    doneRecording();
}


void KeySequenceHelper::clearKeySequence()
{
    setKeySequence(QKeySequence());
}

// //slot
// void KeySequenceHelper::applyStealShortcut()
// {
//     QSet<KActionCollection *> changedCollections;
//
//     Q_FOREACH (QAction *stealAction, d->stealActions) {
//
//         // Stealing a shortcut means setting it to an empty one.
//         stealAction->setShortcuts(QList<QKeySequence>());
//
//         // The following code will find the action we are about to
//         // steal from and save it's actioncollection.
//         KActionCollection *parentCollection = 0;
//         foreach (KActionCollection *collection, d->checkActionCollections) {
//             if (collection->actions().contains(stealAction)) {
//                 parentCollection = collection;
//                 break;
//             }
//         }
//
//         // Remember the changed collection
//         if (parentCollection) {
//             changedCollections.insert(parentCollection);
//         }
//     }
//
//     Q_FOREACH (KActionCollection *col, changedCollections) {
//         col->writeSettings();
//     }
//
//     d->stealActions.clear();
// }
//
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
    QHash<QKeySequence, QList<KGlobalShortcutInfo> > others;
    for (int i = 0; i < keySequence.count(); ++i) {
        QKeySequence tmp(keySequence[i]);

        if (!KGlobalAccel::isGlobalShortcutAvailable(tmp, componentName)) {
            others.insert(tmp, KGlobalAccel::getGlobalShortcutsByKey(tmp));
        }
    }

    if (!others.isEmpty()
            && !promptStealShortcutSystemwide(0, others, keySequence)) {
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
//
// bool shortcutsConflictWith(const QList<QKeySequence> &shortcuts, const QKeySequence &needle)
// {
//     if (needle.isEmpty()) {
//         return false;
//     }
//
//     foreach (const QKeySequence &sequence, shortcuts) {
//         if (sequence.isEmpty()) {
//             continue;
//         }
//
//         if (sequence.matches(needle) != QKeySequence::NoMatch
//                 || needle.matches(sequence) != QKeySequence::NoMatch) {
//             return true;
//         }
//     }
//
//     return false;
// }
//
// bool KeySequenceHelperPrivate::conflictWithLocalShortcuts(const QKeySequence &keySequence)
// {
//     if (!(checkAgainstShortcutTypes & KeySequenceHelper::LocalShortcuts)) {
//         return false;
//     }
//
//     // We have actions both in the deprecated checkList and the
//     // checkActionCollections list. Add all the actions to a single list to
//     // be able to process them in a single loop below.
//     // Note that this can't be done in setCheckActionCollections(), because we
//     // keep pointers to the action collections, and between the call to
//     // setCheckActionCollections() and this function some actions might already be
//     // removed from the collection again.
//     QList<QAction *> allActions;
//     allActions += checkList;
//     foreach (KActionCollection *collection, checkActionCollections) {
//         allActions += collection->actions();
//     }
//
//     // Because of multikey shortcuts we can have clashes with many shortcuts.
//     //
//     // Example 1:
//     //
//     // Application currently uses 'CTRL-X,a', 'CTRL-X,f' and 'CTRL-X,CTRL-F'
//     // and the user wants to use 'CTRL-X'. 'CTRL-X' will only trigger as
//     // 'activatedAmbiguously()' for obvious reasons.
//     //
//     // Example 2:
//     //
//     // Application currently uses 'CTRL-X'. User wants to use 'CTRL-X,CTRL-F'.
//     // This will shadow 'CTRL-X' for the same reason as above.
//     //
//     // Example 3:
//     //
//     // Some weird combination of Example 1 and 2 with three shortcuts using
//     // 1/2/3 key shortcuts. I think you can imagine.
//     QList<QAction *> conflictingActions;
//
//     //find conflicting shortcuts with existing actions
//     foreach (QAction *qaction, allActions) {
//         if (shortcutsConflictWith(qaction->shortcuts(), keySequence)) {
//             // A conflict with a KAction. If that action is configurable
//             // ask the user what to do. If not reject this keySequence.
//             if (checkActionCollections.first()->isShortcutsConfigurable(qaction)) {
//                 conflictingActions.append(qaction);
//             } else {
//                 wontStealShortcut(qaction, keySequence);
//                 return true;
//             }
//         }
//     }
//
//     if (conflictingActions.isEmpty()) {
//         // No conflicting shortcuts found.
//         return false;
//     }
//
//     if (stealShortcuts(conflictingActions, keySequence)) {
//         stealActions = conflictingActions;
//         // Announce that the user
//         // agreed
//         Q_FOREACH (QAction *stealAction, stealActions) {
//             emit q->stealShortcut(
//                 keySequence,
//                 stealAction);
//         }
//         return false;
//     } else {
//         return true;
//     }
// }
//
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
                key |= (d->modifierKeys & ~Qt::SHIFT);
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
