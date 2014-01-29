/*
 * Copyright 2013  Bhushan Shah <bhush94@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

import QtQuick 2.0

Canvas {

	id: graphCanvas

	property variant samples

	property bool showAxis: true
	property bool showHorizontalLines: true

	property int graphPadding: 10  //Replace with units later

	QtObject {
		id: internal
		property int availableVSpace: height - (2 * graphPadding)
		property int availableHSpace: width - (2 * graphPadding)
		property int sampleCount: availableHSpace / graphPadding
	}

	function addSample(sample) {
		var temp = samples;
		temp.push(sample);
		samples = temp;
		requestPaint();
	}

	function drawGraph(context) {

		print(internal.sampleCount);
		// Draw Axis
		if (showAxis) {
			context.beginPath();
			context.strokeStyle = "rgb(150,150,150)"; //TODO: use from theme
			context.lineWidth = 2;
			context.moveTo(graphPadding, graphPadding);
			context.lineTo(graphPadding, height - graphPadding);
			context.lineTo(width - graphPadding, height - graphPadding);
			context.stroke();
			context.closePath();
		}

		// Draw horizontal lines
		if (showHorizontalLines) {
			context.beginPath();
			context.lineWidth = 1;
			context.strokeStyle = "rgb(150, 150, 150)" //TODO: use from theme
			for (var y = 0; y < height - graphPadding; y += height/10) {
				context.moveTo(graphPadding, y);
				context.lineTo(width - graphPadding, y);
			}
			context.stroke();
			context.closePath();
		}

		// Time to draw graph

	}

	onPaint: {

		var ctx = getContext("2d");

		//TODO: from theme
		var grad = ctx.createLinearGradient(0, 0, 0, height);
		grad.addColorStop(0,   '#aaa');
		grad.addColorStop(1, '#fff');
		ctx.fillStyle = grad;
		ctx.fillRect(0, 0, width, height);

		drawGraph(ctx);

	}


}