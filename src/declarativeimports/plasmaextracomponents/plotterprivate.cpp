/*
 *   Copyright 2014 Bhushan Shah <bhush94@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "plotterprivate.h"

#include <QDebug>
PlotterPrivate::PlotterPrivate(QObject* parent)
    : QObject(parent)
{
}

// void PlotterPrivate::setHeight(int height)
// {
// 	m_height = height;
// 	emit heightChanged();
// }
//
// void PlotterPrivate::setWidth(int width)
// {
// 	m_width = width;
// 	emit widthChanged();
// }

void PlotterPrivate::addPlot(QColor color)
{
	m_plots.append(color);
	QList<qreal> temp;
	m_samples.append(temp);
	emit plotsChanged();
}

void PlotterPrivate::addSample(QList<qreal> sample)
{
	if(sample.size() != m_plots.size()) {
		qDebug() << "Count of plots and data in sample mismatch, discarding sample";
		return;
	}
	if(m_plots.size() == 0) {
		qDebug() << "You need to add plot first";
		return;
	}
	for( int i=0; i<sample.size(); i++) {
		m_samples[i].append(sample.at(i));
	}
}

QList<qreal> PlotterPrivate::getSamples(int plotPosition)
{
	return m_samples.at(plotPosition);
}

QColor PlotterPrivate::getPlotColor(int plotPosition)
{
	return m_plots.at(plotPosition);
}

// int PlotterPrivate::getYPos(int samplePos)
// {
//
// }