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

#ifndef PLOTTERPRIVATE_H
#define PLOTTERPRIVATE_H

#include <QColor>

class PlotterPrivate : public QObject
{
	Q_OBJECT

public:

	PlotterPrivate(QObject* parent=0);

// 	Q_PROPERTY(int height READ height WRITE setHeight NOTIFY heightChanged)
// 	int height() {return m_height;}
// 	void setHeight(const int height);
//
// 	Q_PROPERTY(int width READ width WRITE setWidth NOTIFY widthChanged)
// 	int width() {return m_width;}
// 	void setWidth(const int width);

	Q_PROPERTY(int plotCount READ plotCount NOTIFY plotsChanged)
	int plotCount() {return m_plots.size();}

	Q_INVOKABLE void addPlot(QColor color);
	Q_INVOKABLE void addSample(QList<qreal> sample);
	Q_INVOKABLE QList<qreal> getSamples(int plotPosition);
	Q_INVOKABLE QColor getPlotColor(int plotPosition);
// 	Q_INVOKABLE int getYPos(int samplePos);

Q_SIGNALS:

	void plotsChanged();

private:

	QList<QList<qreal>> m_samples;
	QList<QColor> m_plots;

// 	int m_height;
// 	int m_width;

};

#endif