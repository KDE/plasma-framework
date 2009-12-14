#ifndef ANIMATIONGROUP_H
#define ANIMATIONGROUP_H

#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>

class ParallelAnimationGroup : public QParallelAnimationGroup
{
    Q_OBJECT

public:
    ParallelAnimationGroup(QObject *parent);

public Q_SLOTS:
    void addAnimation(QAbstractAnimation * animation);
    QAbstractAnimation *animationAt(int index) const;
    int animationCount() const;
    void clearAnimations();
    int indexOfAnimation(QAbstractAnimation *animation) const;
    void insertAnimationAt(int index, QAbstractAnimation * animation);
    void removeAnimation(QAbstractAnimation * animation);
};

class SequentialAnimationGroup : public QSequentialAnimationGroup
{
    Q_OBJECT

public:
    SequentialAnimationGroup(QObject *parent);

public Q_SLOTS:
    void addAnimation(QAbstractAnimation * animation);
    QAbstractAnimation *animationAt(int index) const;
    int animationCount() const;
    void clearAnimations();
    int indexOfAnimation(QAbstractAnimation *animation) const;
    void insertAnimationAt(int index, QAbstractAnimation * animation);
    void removeAnimation(QAbstractAnimation * animation);
};

#endif

