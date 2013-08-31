#include "daysmodel.h"
#include <QDebug>
#include <QByteArray>

DaysModel::DaysModel(QObject *parent) :
    QAbstractListModel(parent)
{
    QHash<int, QByteArray> roleNames;

    roleNames.insert(isPreviousMonth,        "isPreviousMonth");
    roleNames.insert(isCurrentMonth,         "isCurrentMonth");
    roleNames.insert(isNextMonth,            "isNextMonth");
    //roleNames.insert(containsHolidayItems,   "containsHolidayItems");
    //roleNames.insert(containsEventItems,     "containsEventItems");
   // roleNames.insert(containsTodoItems,      "containsTodoItems");
   // roleNames.insert(containsJournalItems,   "containsJournalItems");
    roleNames.insert(dayNumber,              "dayNumber");
    roleNames.insert(monthNumber,            "monthNumber");
    roleNames.insert(yearNumber,             "yearNumber");

    setRoleNames(roleNames);

}

void DaysModel::setSourceData(QList<DayData> *data)
{
    if(m_data != data) {
        m_data = data;
        reset();
    }
}

int DaysModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if(m_data->size() <= 0) {
        return 0;
    } else {
        return m_data->size();
    }
}

QVariant DaysModel::data(const QModelIndex &index, int role) const
{
    if(index.row() >= 0 && role > Qt::UserRole) {

        DayData currentData = m_data->at(index.row());

        switch(role) {
        case isPreviousMonth:
            return currentData.isPreviousMonth;
        case isNextMonth:
            return currentData.isNextMonth;
  //      case containsHolidayItems:
  //          return currentData.containsHolidayItems;
       /* case containsEventItems:
            return currentData.containsEventItems;
        case containsTodoItems:
            return currentData.containsTodoItems;
        case containsJournalItems:
            return currentData.containsJournalItems;*/
        case dayNumber:
            return currentData.dayNumber;
        case monthNumber:
            return currentData.monthNumber;
        case yearNumber:
            return currentData.yearNumber;
        }
    }
    return QVariant();
}

void DaysModel::update()
{
    if(m_data->size() <= 0) {
        return;
    }

    // We always have 42 items (or weeks * num of days in week) so we only have to tell the view that the data changed.
    layoutChanged();
}

