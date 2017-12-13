/***************************************************************************
 *   Copyright 2014 Marco Martin <mart@kde.org>                            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef COLORSCOPE_H
#define COLORSCOPE_H

#include <QQuickItem>
#include <QWeakPointer>
#include <QtCore/QVariant>
#include <Plasma/Plasma>
#include <Plasma/Theme>

class QQuickItem;

/**
 * @class ColorScope
 *
 * @short Sets the colour scheme to be used by all child items
 *
 * It is exposed as org.kde.plasma.core.ColorScope
 */
class ColorScope : public QQuickItem
{
    Q_OBJECT

    /**
     * Specifies the color group to use for this ColorScope
     */
    Q_PROPERTY(Plasma::Theme::ColorGroup colorGroup READ colorGroup WRITE setColorGroup NOTIFY colorGroupChanged)

    /**
     * The main foreground color within this colorscope
     */
    Q_PROPERTY(QColor textColor READ textColor NOTIFY colorsChanged)

    /**
     * The highlight color within this colorscope
     */
    Q_PROPERTY(QColor highlightColor READ highlightColor NOTIFY colorsChanged)

    /**
     * The highlighted text color within this colorscope
     */
    Q_PROPERTY(QColor highlightedTextColor READ highlightedTextColor NOTIFY colorsChanged)

    /**
     * The background color that should be used within this colorscope
     */
    Q_PROPERTY(QColor backgroundColor READ backgroundColor NOTIFY colorsChanged)

    /**
     * Color of foreground objects with a "positive message" connotation (usually green)
     */
    Q_PROPERTY(QColor positiveTextColor READ positiveTextColor NOTIFY colorsChanged)

    /**
     * Color of foreground objects with a "neutral message" connotation (usually yellow)
     */
    Q_PROPERTY(QColor neutralTextColor READ neutralTextColor NOTIFY colorsChanged)
 
    /**
     * Color of foreground objects with a "negative message" connotation (usually red)
     */
    Q_PROPERTY(QColor negativeTextColor READ negativeTextColor NOTIFY colorsChanged)

    /**
     * true if the scope inherits from its parent scope
     * @since 5.39
     */
    Q_PROPERTY(bool inherit READ inherit WRITE setInherit NOTIFY inheritChanged)

public:
/// @cond INTERNAL_DOCS
    explicit ColorScope(QQuickItem *parent = nullptr, QObject *parentObject = nullptr);
    ~ColorScope();

    void setColorGroup(Plasma::Theme::ColorGroup group);
    Plasma::Theme::ColorGroup colorGroup() const;

    QColor textColor() const;
    QColor highlightColor() const;
    QColor highlightedTextColor() const;
    QColor backgroundColor() const;
    QColor positiveTextColor() const;
    QColor neutralTextColor() const;
    QColor negativeTextColor() const;

    bool inherit() const;
    void setInherit(bool inherit);

    ////NEEDED BY QML TO CREATE ATTACHED PROPERTIES
    static ColorScope *qmlAttachedProperties(QObject *object);

/// @endcond

    ColorScope *findParentScope();
    void itemChange(ItemChange change, const ItemChangeData &value) Q_DECL_OVERRIDE;

    bool eventFilter(QObject * watched, QEvent * event) override;

Q_SIGNALS:
    void colorGroupChanged();
    void colorsChanged();
    void inheritChanged();

private:
    void checkColorGroupChanged();
    void setParentScope(ColorScope * parentScope);

    bool m_inherit;
    Plasma::Theme m_theme;
    Plasma::Theme::ColorGroup m_group;
    QPointer<ColorScope> m_parentScope;
    QObject *const m_parent;
    Plasma::Theme::ColorGroup m_actualGroup;

    static QHash<QObject *, ColorScope *> s_attachedScopes;
};

QML_DECLARE_TYPEINFO(ColorScope, QML_HAS_ATTACHED_PROPERTIES)

#endif
