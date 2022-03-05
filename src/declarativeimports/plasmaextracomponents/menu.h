#pragma once

#include <QQmlEngine>
#include <QMenu>

#include "plasma.h"

class QQuickItem;

class MenuItem : public QObject
{
    Q_OBJECT

    QML_NAMED_ELEMENT(MenuItem)

    /**
     * The parent object
     */
    Q_PROPERTY(QObject *parent READ parent WRITE setParent)

    /**
     * If true, the menu item will behave like a separator
     */
    Q_PROPERTY(bool separator READ separator WRITE setSeparator NOTIFY separatorChanged)
    /**
     * If true, the menu item will behave like a section
     */
    Q_PROPERTY(bool section READ section WRITE setSection NOTIFY sectionChanged)
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QVariant icon READ icon WRITE setIcon NOTIFY iconChanged)
    Q_PROPERTY(QAction *action READ action WRITE setAction NOTIFY actionChanged)
    Q_PROPERTY(bool checkable READ checkable WRITE setCheckable NOTIFY checkableChanged)
    Q_PROPERTY(bool checked READ checked WRITE setChecked NOTIFY toggled)

public:
    explicit MenuItem(QObject *parent = nullptr);

    QAction *action() const;
    void setAction(QAction *a);
    QVariant icon() const;
    void setIcon(const QVariant &i);
    bool separator() const;
    void setSeparator(bool s);
    bool section() const;
    void setSection(bool s);
    QString text() const;
    void setText(const QString &t);

    bool visible() const;
    void setVisible(bool newValue);
    Q_SIGNAL void visibleChanged();

    bool enabled() const;
    void setEnabled(bool newValue);
    Q_SIGNAL void enabledChanged();

    bool checkable() const;
    void setCheckable(bool checkable);

    bool checked() const;
    void setChecked(bool checked);

Q_SIGNALS:
    void clicked();

    void actionChanged();
    void iconChanged();
    void separatorChanged();
    void sectionChanged();
    void textChanged();
    void toggled(bool checked);
    void checkableChanged();

private:
    void updateAction();
    QAction *m_action;
    QVariant m_icon;
    bool m_section;
    bool m_visible = true;
    bool m_enabled = true;
};

class MenuExtension : public QObject
{

    Q_OBJECT

    Q_PROPERTY(QQmlListProperty<MenuItem> content READ content CONSTANT)
    Q_CLASSINFO("DefaultProperty", "content")

    /**
     * This is a hint to the window manager that this window is a dialog or pop-up on
     * behalf of the given window.
     */
    Q_PROPERTY(QWindow *transientParent READ transientParent WRITE setTransientParent NOTIFY transientParentChanged)

    /**
     * The visualParent is used to position the menu. it can be an item on the
     * scene, like a button (that will open the menu on clicked) or another menuitem
     * (in this case this will be a submenu).
     */
    Q_PROPERTY(QObject *visualParent READ visualParent WRITE setVisualParent NOTIFY visualParentChanged())
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)

    /**
     * The default placement for the menu.
     */
    Q_PROPERTY(Plasma::Types::PopupPlacement placement READ placement WRITE setPlacement NOTIFY placementChanged)

    /**
     * A minimum width for the menu.
     */
    Q_PROPERTY(int minimumWidth READ minimumWidth WRITE setMinimumWidth NOTIFY minimumWidthChanged)

    /**
     * A maximum width for the menu.
     *
     * @since 5.31
     */
    Q_PROPERTY(int maximumWidth READ maximumWidth WRITE setMaximumWidth RESET resetMaximumWidth NOTIFY maximumWidthChanged)

public:
    enum Status {
        Opening,
        Open,
        Closing,
        Closed,
    };
    Q_ENUM(Status)

    explicit MenuExtension(QObject *parent = nullptr);
    ~MenuExtension() override;

    QQmlListProperty<MenuItem> content();
    int actionCount() const;
    MenuItem *action(int) const;
    Status status() const;

    QObject *visualParent() const;
    void setVisualParent(QObject *parent);

    QWindow *transientParent();
    void setTransientParent(QWindow *parent);

    Plasma::Types::PopupPlacement placement() const;
    void setPlacement(Plasma::Types::PopupPlacement placement);

    int minimumWidth() const;
    void setMinimumWidth(int width);

    int maximumWidth() const;
    void setMaximumWidth(int maximumWidth);
    void resetMaximumWidth();

    /**
     * This opens the menu at position x,y on the given visualParent. By default x and y are set to 0
     */
    Q_INVOKABLE void open(int x = 0, int y = 0);
    /**
     * This opens the menu at the specified placement relative to the visualParent.
     */
    Q_INVOKABLE int openRelative();
    /**
     * This closes the menu
     */
    Q_INVOKABLE void close();
    /**
     * This removes all menuItems inside the menu
     */
    Q_INVOKABLE void clearMenuItems();
    /**
     * This adds a menu item from a String
     */
    Q_INVOKABLE void addMenuItem(const QString &text);
    /**
     * This adds MenuItem 'item' to the menu before MenuItem 'before'.
     * If MenuItem 'before' is 0 or does not exist in the menu, 'item'
     * is appended to the menu instead.
     * If MenuItem 'item' already exists in the menu, it is removed and
     * inserted at the new position.
     */
    Q_INVOKABLE void addMenuItem(MenuItem *item, MenuItem *before = nullptr);
    /**
     * This adds a section header with a string used as name for the section
     */
    Q_INVOKABLE void addSection(const QString &text);

    /**
     * This removes MenuItem 'item'
     *
     * @since 5.27
     */
    Q_INVOKABLE void removeMenuItem(MenuItem *item);

protected:
    bool event(QEvent *event) override;

Q_SIGNALS:
    void statusChanged();
    void visualParentChanged();
    void transientParentChanged();
    void placementChanged();
    void minimumWidthChanged();
    void maximumWidthChanged();
    void triggered(MenuItem *item);
    void triggeredIndex(int index);

private Q_SLOTS:
    void itemTriggered(QAction *item);

private:
    void rebuildMenu();
    void openInternal(QPoint pos);
    QQuickItem *parentItem() const;

    QList<MenuItem *> m_items;
    QMenu *m_menu;
    Status m_status;
    QPointer<QObject> m_visualParent;
    Plasma::Types::PopupPlacement m_placement;

};

struct ForeignMenu
{

    Q_GADGET
    QML_NAMED_ELEMENT(Menu)
    QML_FOREIGN(QMenu)
    QML_EXTENDED(MenuExtension)

};
