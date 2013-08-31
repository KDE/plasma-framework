#ifndef DAYSMODEL_H
#define DAYSMODEL_H

#include <QAbstractListModel>
#include "daydata.h"

class DaysModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        isPreviousMonth = Qt::UserRole + 1,
        isCurrentMonth,
        isNextMonth,
        //containsHolidayItems,
        //containsEventItems,
        //containsTodoItems,
        //containsJournalItems,
        dayNumber,
        monthNumber,
        yearNumber
    };
    explicit DaysModel(QObject *parent = 0);
    void setSourceData(QList<DayData>* data);
    virtual int rowCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    void update();
    
Q_SIGNALS:
    
public Q_SLOTS:
    
private:
    QList<DayData>* m_data;
};

#endif // DAYSMODEL_H
