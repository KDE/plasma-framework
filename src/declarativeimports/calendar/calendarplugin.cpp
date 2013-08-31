#include "calendarplugin.h"
#include "calendardata.h"
#include "calendar.h"
#include <QtQml>
#include <QAbstractItemModel>
#include <QAbstractListModel>




void CalendarPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.plasma.calendar"));
     qmlRegisterType<CalendarData>(uri, 2, 0, "CalendarData");
    qmlRegisterType<Calendar>(uri, 2, 0, "Calendar");
 //   qmlRegisterType<CalendarData>(uri, 1, 0, "CalendarData");
//    qmlRegisterType<CalendarData>(uri, 1, 0, "Calendar");
    qmlRegisterType<QAbstractItemModel>();
    qmlRegisterType<QAbstractListModel>();
}

//Q_EXPORT_PLUGIN2(calendarplugin, CalendarPlugin)
#include "moc_calendarplugin.cpp"
