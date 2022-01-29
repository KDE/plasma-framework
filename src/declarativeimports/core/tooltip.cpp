/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2011 Artur Duque de Souza <asouza@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <cmath>

#include "tooltip.h"
#include "tooltipdialog.h"

#include <QDebug>
#include <QQmlEngine>

#include "framesvgitem.h"
#include <KDirWatch>
#include <KWindowEffects>

ToolTipDialog *ToolTip::s_dialog = nullptr;
int ToolTip::s_dialogUsers = 0;

ToolTip::ToolTip(QQuickItem *parent)
    : QQuickItem(parent)
    , m_tooltipsEnabledGlobally(false)
    , m_containsMouse(false)
    , m_location(Plasma::Types::Floating)
    , m_textFormat(Qt::AutoText)
    , m_active(true)
    , m_interactive(false)
    , m_timeout(4000)
    , m_triangleMouseFiltering(false)
    , m_triangleMouseStartPoint(0, 0)
    , m_triangleMouseStartAreaItem(nullptr)
    , m_usingDialog(false)
{
    setAcceptHoverEvents(true);
    setFiltersChildMouseEvents(true);

    m_showTimer = new QTimer(this);
    m_showTimer->setSingleShot(true);
    connect(m_showTimer, &QTimer::timeout, this, &ToolTip::showToolTip);

    loadSettings();

    const QString configFile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/plasmarc");
    KDirWatch::self()->addFile(configFile);
    QObject::connect(KDirWatch::self(), &KDirWatch::created, this, &ToolTip::settingsChanged);
    QObject::connect(KDirWatch::self(), &KDirWatch::dirty, this, &ToolTip::settingsChanged);
}

ToolTip::~ToolTip()
{
    if (s_dialog && s_dialog->owner() == this) {
        s_dialog->setVisible(false);
    }

    if (m_usingDialog) {
        --s_dialogUsers;
    }

    if (s_dialogUsers == 0) {
        delete s_dialog;
        s_dialog = nullptr;
    }
}

void ToolTip::settingsChanged(const QString &file)
{
    if (!file.endsWith(QLatin1String("plasmarc"))) {
        return;
    }

    KSharedConfig::openConfig(QStringLiteral("plasmarc"))->reparseConfiguration();
    loadSettings();
}

void ToolTip::loadSettings()
{
    KConfigGroup cfg = KConfigGroup(KSharedConfig::openConfig(QStringLiteral("plasmarc")), "PlasmaToolTips");
    m_interval = cfg.readEntry("Delay", 700);
    m_tooltipsEnabledGlobally = (m_interval > 0);
}

QQuickItem *ToolTip::mainItem() const
{
    return m_mainItem.data();
}

ToolTipDialog *ToolTip::tooltipDialogInstance()
{
    if (!s_dialog) {
        s_dialog = new ToolTipDialog;
        s_dialogUsers = 1;
    }

    if (!m_usingDialog) {
        s_dialogUsers++;
        m_usingDialog = true;
    }

    return s_dialog;
}

void ToolTip::setMainItem(QQuickItem *mainItem)
{
    if (m_mainItem.data() != mainItem) {
        m_mainItem = mainItem;

        Q_EMIT mainItemChanged();

        if (!isValid() && s_dialog && s_dialog->owner() == this) {
            s_dialog->setVisible(false);
        }
    }
}

void ToolTip::showToolTip()
{
    if (!m_active) {
        return;
    }

    Q_EMIT aboutToShow();

    ToolTipDialog *dlg = tooltipDialogInstance();

    if (!mainItem()) {
        setMainItem(dlg->loadDefaultItem());
    }

    // Unset the dialog's old contents before reparenting the dialog.
    dlg->setMainItem(nullptr);

    Plasma::Types::Location location = m_location;
    if (m_location == Plasma::Types::Floating) {
        QQuickItem *p = parentItem();
        while (p) {
            if (p->property("location").isValid()) {
                location = (Plasma::Types::Location)p->property("location").toInt();
                break;
            }
            p = p->parentItem();
        }
    }

    if (mainItem()) {
        mainItem()->setProperty("toolTip", QVariant::fromValue(this));
        mainItem()->setVisible(true);
    }

    connect(dlg, &ToolTipDialog::visibleChanged, this, &ToolTip::toolTipVisibleChanged, Qt::UniqueConnection);

    dlg->setHideTimeout(m_timeout);
    dlg->setOwner(this);
    dlg->setLocation(location);
    dlg->setVisualParent(this);
    dlg->setMainItem(mainItem());
    dlg->setInteractive(m_interactive);
    dlg->setVisible(true);
}

QString ToolTip::mainText() const
{
    return m_mainText;
}

void ToolTip::setMainText(const QString &mainText)
{
    if (mainText == m_mainText) {
        return;
    }

    m_mainText = mainText;
    Q_EMIT mainTextChanged();

    if (!isValid() && s_dialog && s_dialog->owner() == this) {
        s_dialog->setVisible(false);
    }
}

QString ToolTip::subText() const
{
    return m_subText;
}

void ToolTip::setSubText(const QString &subText)
{
    if (subText == m_subText) {
        return;
    }

    m_subText = subText;
    Q_EMIT subTextChanged();

    if (!isValid() && s_dialog && s_dialog->owner() == this) {
        s_dialog->setVisible(false);
    }
}

int ToolTip::textFormat() const
{
    return m_textFormat;
}

void ToolTip::setTextFormat(int format)
{
    if (m_textFormat == format) {
        return;
    }

    m_textFormat = format;
    Q_EMIT textFormatChanged();
}

Plasma::Types::Location ToolTip::location() const
{
    return m_location;
}

void ToolTip::setLocation(Plasma::Types::Location location)
{
    if (m_location == location) {
        return;
    }
    m_location = location;
    Q_EMIT locationChanged();
}

void ToolTip::setActive(bool active)
{
    if (m_active == active) {
        return;
    }

    m_active = active;
    if (!active) {
        tooltipDialogInstance()->dismiss();
    }
    Q_EMIT activeChanged();
}

void ToolTip::setInteractive(bool interactive)
{
    if (m_interactive == interactive) {
        return;
    }

    m_interactive = interactive;

    Q_EMIT interactiveChanged();
}

void ToolTip::setTimeout(int timeout)
{
    m_timeout = timeout;
}

void ToolTip::hideToolTip()
{
    m_showTimer->stop();
    tooltipDialogInstance()->dismiss();
}

void ToolTip::hideImmediately()
{
    m_showTimer->stop();
    tooltipDialogInstance()->setVisible(false);
}

QVariant ToolTip::icon() const
{
    if (m_icon.isValid()) {
        return m_icon;
    } else {
        return QString();
    }
}

void ToolTip::setIcon(const QVariant &icon)
{
    if (icon == m_icon) {
        return;
    }

    m_icon = icon;
    Q_EMIT iconChanged();
}

QVariant ToolTip::image() const
{
    if (m_image.isValid()) {
        return m_image;
    } else {
        return QString();
    }
}

void ToolTip::setImage(const QVariant &image)
{
    if (image == m_image) {
        return;
    }

    m_image = image;
    Q_EMIT imageChanged();
}

bool ToolTip::containsMouse() const
{
    return m_containsMouse;
}

void ToolTip::setContainsMouse(bool contains)
{
    if (m_containsMouse != contains) {
        m_containsMouse = contains;
        Q_EMIT containsMouseChanged();
    }
    if (!contains) {
        tooltipDialogInstance()->dismiss();
    }
}

bool ToolTip::triangleMouseFiltering() const
{
    return m_triangleMouseFiltering;
}

void ToolTip::setTriangleMouseFiltering(bool filtering)
{
    if (filtering != m_triangleMouseFiltering) {
        m_triangleMouseFiltering = filtering;
        Q_EMIT triangleMouseFilteringChanged();
    }
}

QPoint ToolTip::toolTipOpenedPoint() const
{
    return m_toolTipOpenedPoint;
}

QPoint ToolTip::triangleMouseStartPoint() const
{
    return m_triangleMouseStartPoint;
}

void ToolTip::setTriangleMouseStartPoint(QPoint startPoint)
{
    if (startPoint != m_triangleMouseStartPoint) {
        m_triangleMouseStartPoint = startPoint;
        m_triangleMouseLastPoint = QPoint(0, 0);
        Q_EMIT triangleMouseStartPointChanged();
    }
}

ToolTip *ToolTip::triangleMouseStartAreaItem() const
{
    return m_triangleMouseStartAreaItem;
}

void ToolTip::setTriangleMouseStartAreaItem(ToolTip *startItem)
{
    if (startItem != m_triangleMouseStartAreaItem) {
        m_triangleMouseStartAreaItem = startItem;
        Q_EMIT triangleMouseStartAreaItemChanged();
    }
}

/* a point P is in triangle ABC if ar(ABC) == ar(PAB) + ar(PBC) + ar (PCA)
   the actual area of a triangle is (determinant)/2
   but for comparing areas, we don't need the division by 2 (it cancels out),
   so by not doing it we can avoid dealing with floats plus save on a division operation */

int twiceAreaTriangle(QPoint A, QPoint B, QPoint C)
{
    return std::abs((A.x() * (B.y() - C.y()) + B.x() * (C.y() - A.y()) + C.x() * (A.y() - B.y())));
}

bool inTriangle(QPoint P, QPoint A, QPoint B, QPoint C)
{
    int arABC = twiceAreaTriangle(A, B, C);
    int arPAB = twiceAreaTriangle(P, A, B);
    int arPBC = twiceAreaTriangle(P, B, C);
    int arPCA = twiceAreaTriangle(P, C, A);
    return (arABC == (arPAB + arPBC + arPCA));
}

void ToolTip::hoverEnterEvent(QHoverEvent *event)
{
    if (!m_tooltipsEnabledGlobally) {
        return;
    }

    if (!isValid()) {
        return;
    }

    m_toolTipOpenedPoint = event->pos();
    Q_EMIT toolTipOpenedPointChanged();

    setContainsMouse(true);

    if (tooltipDialogInstance()->isVisible()) {
        // We signal the tooltipmanager that we're "potentially interested,
        // and ask to keep it open for a bit, so other items get the chance
        // to update the content before the tooltip hides -- this avoids
        // flickering
        // It need to be considered only when other items can deal with tooltip area
        if (m_active) {
            tooltipDialogInstance()->keepalive();
            // FIXME: showToolTip needs to be renamed in sync or something like that
            if (m_triangleMouseFiltering) {
                QPoint toolTipLeft, toolTipRight, eventPos, tmStartPoint, tmLastPoint, tmBasePoint;
                const ToolTipDialog *const tldgi = tooltipDialogInstance();

                // toolTip{Left,Right} refer to the two corners of the tooltip which are adjacent to the ToolTipArea
                switch (m_location) {
                case Plasma::Types::BottomEdge:
                    toolTipLeft = QPoint(tldgi->x(), tldgi->y() + tldgi->height());
                    toolTipRight = QPoint(tldgi->x() + tldgi->width(), tldgi->y() + tldgi->height());
                    break;
                case Plasma::Types::LeftEdge:
                    toolTipLeft = QPoint(tldgi->x(), tldgi->y());
                    toolTipRight = QPoint(tldgi->x(), tldgi->y() + tldgi->height());
                    break;
                case Plasma::Types::RightEdge:
                    toolTipLeft = QPoint(tldgi->x() + tldgi->width(), tldgi->y());
                    toolTipRight = QPoint(tldgi->x() + tldgi->width(), tldgi->y() + tldgi->height());
                    break;
                case Plasma::Types::TopEdge:
                    toolTipLeft = QPoint(tldgi->x(), tldgi->y());
                    toolTipRight = QPoint(tldgi->x() + tldgi->width(), tldgi->y());
                    break;
                }

                eventPos = mapToGlobal(event->pos()).toPoint();
                tmStartPoint = mapToGlobal(m_triangleMouseStartPoint).toPoint();
                tmLastPoint = mapToGlobal(m_triangleMouseLastPoint).toPoint();

                if (m_triangleMouseStartAreaItem) {
                    /* Sometimes the tooltip opens up when the user is entering the task manager area,
                       so we also check from slightly off the midpoint of the base of the ToolTipArea.
                       This makes the filter significantly more reliable in practice, while still
                       allowing instant switching by sliding along the actual screen edge. */
                    switch (m_location) {
                    case Plasma::Types::BottomEdge:
                        tmBasePoint = QPoint(m_triangleMouseStartAreaItem->width() / 2, m_triangleMouseStartAreaItem->height() - 2);
                        break;
                    case Plasma::Types::LeftEdge:
                        tmBasePoint = QPoint(2, m_triangleMouseStartAreaItem->height() / 2);
                        break;
                    case Plasma::Types::RightEdge:
                        tmBasePoint = QPoint(m_triangleMouseStartAreaItem->width() - 2, m_triangleMouseStartAreaItem->height() / 2);
                        break;
                    case Plasma::Types::TopEdge:
                        tmBasePoint = QPoint(m_triangleMouseStartAreaItem->width() / 2, 2);
                        break;
                    }
                    tmBasePoint = m_triangleMouseStartAreaItem->mapToGlobal(tmBasePoint).toPoint();
                }

                if (inTriangle(eventPos, tmStartPoint, toolTipLeft, toolTipRight)
                    || (tmLastPoint != QPoint(0, 0) && inTriangle(eventPos, tmLastPoint, toolTipLeft, toolTipRight))
                    || (tmBasePoint != QPoint(0, 0) && inTriangle(eventPos, tmBasePoint, toolTipLeft, toolTipRight))) {
                    m_showTimer->start(500);
                } else {
                    showToolTip();
                }
            } else {
                showToolTip();
            }
        }
    } else {
        m_showTimer->start(m_interval);
    }
}

void ToolTip::hoverMoveEvent(QHoverEvent *event)
{
    if (m_triangleMouseFiltering && this == m_triangleMouseStartAreaItem) {
        m_triangleMouseLastPoint = event->pos();
    }
}

void ToolTip::hoverLeaveEvent(QHoverEvent *event)
{
    Q_UNUSED(event)
    setContainsMouse(false);
    m_showTimer->stop();
}

bool ToolTip::childMouseEventFilter(QQuickItem *item, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        hideToolTip();
    }
    return QQuickItem::childMouseEventFilter(item, event);
}

bool ToolTip::isValid() const
{
    return m_mainItem || !mainText().isEmpty() || !subText().isEmpty();
}
