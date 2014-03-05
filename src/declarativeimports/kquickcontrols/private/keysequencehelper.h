/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  David Edmundson <davidedmundson@kde.org>
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

#ifndef KEYSEQUENCEHELPER_H
#define KEYSEQUENCEHELPER_H

#include <QObject>
#include <QKeySequence>

/**
 * This class is a clone of Key from XMLGUI
 * It performs only the logic of building shortcuts
 * It is a private class to be used by KeySequenceItem
 *
 */

class KeySequenceHelperPrivate;

class KeySequenceHelper : public QObject
{
    Q_OBJECT

    Q_FLAGS(ShortcutTypes)

    Q_PROPERTY(
        QKeySequence keySequence
        READ keySequence
        WRITE setKeySequence
        NOTIFY keySequenceChanged)

    Q_PROPERTY(
        bool multiKeyShortcutsAllowed
        READ multiKeyShortcutsAllowed
        WRITE setMultiKeyShortcutsAllowed)

    Q_PROPERTY(
        QString shortcutDisplay
        READ shortcutDisplay
        NOTIFY shortcutDisplayChanged)

    Q_PROPERTY(
        bool modifierlessAllowed
        READ isModifierlessAllowed
        WRITE setModifierlessAllowed)

public:

    enum ShortcutType {
        None           = 0x00,      //!< No checking for conflicts
        StandardShortcuts = 0x01,   //!< Check against standard shortcuts. @see KStandardShortcut
        GlobalShortcuts = 0x02      //!< Check against global shortcuts. @see KGlobalAccel
    };
    Q_DECLARE_FLAGS(ShortcutTypes, ShortcutType)

    /**
    * Constructor.
    */
    explicit KeySequenceHelper(QObject* parent = 0);

    /**
    * Destructs the widget.
    */
    virtual ~KeySequenceHelper();

    /**
     * Allow multikey shortcuts?
     */
    void setMultiKeyShortcutsAllowed(bool);
    bool multiKeyShortcutsAllowed() const;

    /**
     * This only applies to user input, not to setShortcut().
     * Set whether to accept "plain" keys without modifiers (like Ctrl, Alt, Meta).
     * Plain keys by our definition include letter and symbol keys and
     * text editing keys (Return, Space, Tab, Backspace, Delete).
     * "Special" keys like F1, Cursor keys, Insert, PageDown will always work.
     */
    void setModifierlessAllowed(bool allow);

    /**
     * @see setModifierlessAllowed()
     */
    bool isModifierlessAllowed();


    bool isRecording() const;
    void setShortcut(bool recording);

    /**
     * Set the default key sequence from a string
     */
    void setKeySequence(const QKeySequence &sequence);

    /**
     * Return the currently selected key sequence as a string
     */
    QKeySequence keySequence() const;
    QString shortcutDisplay() const;

    bool isKeySequenceAvailable(const QKeySequence &keySequence) const;


Q_SIGNALS:
    void keySequenceChanged(const QKeySequence &seq);
    void shortcutDisplayChanged(const QString &string);
    void captureFinished();

public Q_SLOTS:
    void captureKeySequence();
    void keyPressed(int key, int modifiers);
    void keyReleased(int key, int modifiers);

   /**
    * Clear the key sequence.
    */
    void clearKeySequence();

private Q_SLOTS:
    void doneRecording();

private:
    friend class KeySequenceHelperPrivate;
    KeySequenceHelperPrivate *const d;

    Q_DISABLE_COPY(KeySequenceHelper)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KeySequenceHelper::ShortcutTypes)


#endif // KEYSEQUENCEHELPER_H
