#include "animationgroup.h"


ParallelAnimationGroup::ParallelAnimationGroup(QObject *parent)
    : QParallelAnimationGroup(parent)
{
}

void ParallelAnimationGroup::addAnimation(QAbstractAnimation *animation)
{
    QParallelAnimationGroup::addAnimation(animation);
}

QAbstractAnimation *ParallelAnimationGroup::animationAt(int index) const
{
    return QParallelAnimationGroup::animationAt(index);
}

int ParallelAnimationGroup::animationCount() const
{
    return QParallelAnimationGroup::animationCount();
}

void ParallelAnimationGroup::clearAnimations()
{
    QParallelAnimationGroup::clearAnimations();
}

int ParallelAnimationGroup::indexOfAnimation(QAbstractAnimation *animation) const
{
    return QParallelAnimationGroup::indexOfAnimation(animation);
}

void ParallelAnimationGroup::insertAnimationAt(int index, QAbstractAnimation *animation)
{
    QParallelAnimationGroup::insertAnimationAt(index, animation);
}

void ParallelAnimationGroup::removeAnimation(QAbstractAnimation *animation)
{
    QParallelAnimationGroup::removeAnimation(animation);
}


SequentialAnimationGroup::SequentialAnimationGroup(QObject *parent)
    : QSequentialAnimationGroup(parent)
{
}

void SequentialAnimationGroup::addAnimation(QAbstractAnimation *animation)
{
    QSequentialAnimationGroup::addAnimation(animation);
}

QAbstractAnimation *SequentialAnimationGroup::animationAt(int index) const
{
    return QSequentialAnimationGroup::animationAt(index);
}

int SequentialAnimationGroup::animationCount() const
{
    return QSequentialAnimationGroup::animationCount();
}

void SequentialAnimationGroup::clearAnimations()
{
    QSequentialAnimationGroup::clearAnimations();
}

int SequentialAnimationGroup::indexOfAnimation(QAbstractAnimation *animation) const
{
    return QSequentialAnimationGroup::indexOfAnimation(animation);
}

void SequentialAnimationGroup::insertAnimationAt(int index, QAbstractAnimation *animation)
{
    QSequentialAnimationGroup::insertAnimationAt(index, animation);
}

void SequentialAnimationGroup::removeAnimation(QAbstractAnimation *animation)
{
    QSequentialAnimationGroup::removeAnimation(animation);
}

#include "animationgroup.moc"
